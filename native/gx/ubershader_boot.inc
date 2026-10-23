// Appended to the cache TU, after stage attribution is defined. All queue/state
// accesses use its existing mutex; callbacks execute outside that mutex.
#include <set>
namespace aurora::gfx {
struct PetariUberBootState {
  bool started = false, seedsDone = false, cacheDone = false, frontEndScanned = false;
  bool titleObserved = false, readyReported = false, frontReadyReported = false;
  bool genericReadyReported = false, fastReadyReported = false;
  PetariPipeline::Clock::time_point startedAt{};
  std::set<PipelineRef> scheduled, completed, failed, titleKeys;
  std::map<std::string, std::set<PipelineRef>> stageKeys;
  std::set<PipelineRef> genericKeys, fastKeys;
  std::string activeStage;
  size_t manifestErrors = 0, rejectedConfigs = 0, cancelled = 0;
};
static PetariUberBootState petariUberBoot; // g_pipelineMutex

static double petari_uber_elapsed() { // caller holds g_pipelineMutex
  return PetariPipeline::milliseconds(PetariPipeline::Clock::now() - petariUberBoot.startedAt);
}
static size_t petari_uber_title_pending() {
  return std::count_if(petariUberBoot.titleKeys.begin(), petariUberBoot.titleKeys.end(),
      [](auto key) { return !petariUberBoot.completed.contains(key); });
}
static void petari_uber_maybe_ready() { // caller holds g_pipelineMutex
  if (!petariUberBoot.frontReadyReported && petariUberBoot.frontEndScanned && !petari_uber_title_pending()) {
    petariUberBoot.frontReadyReported = true;
    const auto failed = std::count_if(petariUberBoot.titleKeys.begin(), petariUberBoot.titleKeys.end(), [](auto k) { return petariUberBoot.failed.contains(k); });
    std::fprintf(stderr, "[gx uber async] event=front_end_ready elapsed_ms=%.3f variants=%zu failed=%zu success=%u\n",
        petari_uber_elapsed(), petariUberBoot.titleKeys.size(), size_t(failed), !failed && !petariUberBoot.manifestErrors);
  }
  if (petariUberBoot.seedsDone && petariUberBoot.cacheDone) {
    for (const bool generic : {true, false}) {
      const auto& keys = generic ? petariUberBoot.genericKeys : petariUberBoot.fastKeys;
      auto& reported = generic ? petariUberBoot.genericReadyReported : petariUberBoot.fastReadyReported;
      const bool done = std::all_of(keys.begin(), keys.end(), [](auto k) { return petariUberBoot.completed.contains(k); });
      if (!reported && done) {
        reported = true;
        const auto failed = std::count_if(keys.begin(), keys.end(), [](auto k) { return petariUberBoot.failed.contains(k); });
        std::fprintf(stderr, "[gx uber async] event=tier_ready elapsed_ms=%.3f tier=%s variants=%zu failed=%zu success=%u\n",
            petari_uber_elapsed(), generic ? "generic" : "fast", keys.size(), size_t(failed), !failed && !petariUberBoot.manifestErrors);
      }
    }
  }
  if (!petariUberBoot.readyReported && petariUberBoot.seedsDone && petariUberBoot.cacheDone &&
      petariUberBoot.completed.size() == petariUberBoot.scheduled.size()) {
    petariUberBoot.readyReported = true;
    std::fprintf(stderr, "[gx uber async] event=ready elapsed_ms=%.3f variants=%zu failed=%zu manifest_errors=%zu success=%u\n",
        petari_uber_elapsed(), petariUberBoot.scheduled.size(), petariUberBoot.failed.size(),
        petariUberBoot.manifestErrors, petariUberBoot.failed.empty() && !petariUberBoot.manifestErrors);
  }
}
static unsigned petari_uber_job_priority(const PetariUberJob& job) { // g_pipelineMutex
  if (!job.key) return 0;
  const auto it = petariUberBoot.stageKeys.find(petariUberBoot.activeStage);
  const bool active = !petariUberBoot.activeStage.empty() && it != petariUberBoot.stageKeys.end() && it->second.contains(job.key);
  const bool generic = job.tier == gx::PetariUberTier::Generic;
  if (active) return generic ? 1 : 2;
  if (generic && (job.frontEnd || job.demand)) return 3;
  return generic ? 4 : 5;
}
static void petari_uber_push_job(PetariUberJob job) { // caller holds g_pipelineMutex
  job.priority = petari_uber_job_priority(job);
  const auto at = std::find_if(petariUberJobs.begin(), petariUberJobs.end(),
      [&](const auto& other) { return other.priority > job.priority; });
  petariUberJobs.insert(at, std::move(job));
}
static void petari_uber_queue_variant(const gx::PipelineConfig& config, const RenderTargetLayout& layout,
                                    unsigned priority = 3, bool title = false,
                                    gx::PetariUberTier tier = gx::PetariUberTier::Generic) {
  if (!gx::petari_uber_enabled() || std::strcmp(gx::petari_uber_reason(config.shaderConfig), "covered")) return;
  const auto key = gx::petari_uber_key(config, layout, tier); // SOL 1 owns both exact keys.
  {
    std::lock_guard lock{g_pipelineMutex};
    if (!g_hasPipelineThread || g_pipelineThreadEnd) return;
    if (title && tier == gx::PetariUberTier::Generic && petariUberBoot.titleKeys.insert(key).second)
      petariUberBoot.frontReadyReported = false;
    if (!petariUberBoot.scheduled.insert(key).second) {
      auto at = std::find_if(petariUberJobs.begin(), petariUberJobs.end(), [&](const auto& job) { return job.key == key; });
      if (at != petariUberJobs.end()) {
        auto job = std::move(*at); petariUberJobs.erase(at);
        job.frontEnd |= title; job.demand |= priority == 1 && !title;
        petari_uber_push_job(std::move(job));
      }
    } else {
      (tier == gx::PetariUberTier::Generic ? petariUberBoot.genericKeys : petariUberBoot.fastKeys).insert(key);
      (tier == gx::PetariUberTier::Generic ? petariUberBoot.genericReadyReported : petariUberBoot.fastReadyReported) = false;
      petariUberBoot.readyReported = false;
      gx::petari_uber_scheduled(key);
      petari_uber_push_job({key, 0, [config, layout, key, tier] {
        gx::petari_uber_prepare(config, layout, tier);
        CompiledPipeline result;
        const bool ready = gx::petari_uber_ready(key, result);
        std::lock_guard lock{g_pipelineMutex};
        petariUberBoot.completed.insert(key);
        if (!ready) petariUberBoot.failed.insert(key);
        std::fprintf(stderr, "[gx uber async] event=compile_ready elapsed_ms=%.3f key=%016llx tier=%s completed=%zu success=%u\n",
            petari_uber_elapsed(), (unsigned long long)key,
            tier == gx::PetariUberTier::Generic ? "generic" : "fast", petariUberBoot.completed.size(), ready);
        petari_uber_maybe_ready();
        g_pipelineReadyCv.notify_all();
      }, tier, title, priority == 1 && !title});
    }
  }
  g_pipelineQueueCv.notify_all();
}
static void petari_uber_queue_pair(const gx::PipelineConfig& config, const RenderTargetLayout& layout,
                                 unsigned priority, bool title) {
  const auto keys = gx::petari_uber_keys(config, layout);
  petari_uber_queue_variant(config, layout, priority, title, gx::PetariUberTier::Generic);
  if (keys.fast != keys.generic) petari_uber_queue_variant(config, layout, priority, title, gx::PetariUberTier::Fast);
}
static void petari_uber_queue_layouts(const gx::PipelineConfig& config, const RenderTargetLayout& scene,
                                    const RenderTargetLayout& off, unsigned priority, bool title = false) {
  petari_uber_queue_pair(config, scene, priority, title);
  if (off.key != scene.key) petari_uber_queue_pair(config, off, priority, title);
}
static void petari_uber_scan_seeds(const std::filesystem::path& dir, RenderTargetLayout scene,
                                  RenderTargetLayout off) {
  const auto scan = [&](const std::filesystem::path& path, unsigned priority) {
    auto* db = open_pipeline_cache_seed_db(path.string());
    if (!db) {
      std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; return;
    }
    sqlite3_stmt* stmt = nullptr;
    const auto sql = "SELECT config FROM pipeline_cache WHERE type=1 AND config_version=13 ORDER BY first_frame_used";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
      int status;
      while ((status = sqlite3_step(stmt)) == SQLITE_ROW && !g_pipelineThreadEnd) {
        if (sqlite3_column_bytes(stmt, 0) != sizeof(gx::PipelineConfig)) {
          std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.rejectedConfigs; continue;
        }
        gx::PipelineConfig config;
        std::memcpy(&config, sqlite3_column_blob(stmt, 0), sizeof(config));
        if (gx::petari_cached_shader_valid(config.shaderConfig, 0))
          petari_uber_queue_layouts(config, scene, off, priority, priority == 1);
        else { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.rejectedConfigs; }
      }
      if (status != SQLITE_DONE && status != SQLITE_ROW) {
        std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors;
      }
    } else { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
  };
  std::error_code ec;
  // Read the front-end manifests before traversing the complete seed union.
  for (const auto* name : {"Unknown.db", "FileSelect.db"}) {
    const auto path = dir / name;
    if (std::filesystem::exists(path, ec)) scan(path, 1);
    if (g_pipelineThreadEnd) return;
  }
  {
    std::lock_guard lock{g_pipelineMutex}; petariUberBoot.frontEndScanned = true;
  }
  std::vector<std::filesystem::path> paths;
  for (std::filesystem::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec)) {
    const auto path = it->path();
    if (path.extension() == ".db" && path.filename() != "Unknown.db" && path.filename() != "FileSelect.db")
      paths.push_back(path);
  }
  if (ec) { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; }
  std::sort(paths.begin(), paths.end());
  for (const auto& path : paths) {
    if (g_pipelineThreadEnd) return;
    scan(path, 3);
  }
  std::lock_guard lock{g_pipelineMutex};
  petariUberBoot.seedsDone = true;
  std::fprintf(stderr, "[gx uber async] event=enumerated elapsed_ms=%.3f variants=%zu title_variants=%zu rejected=%zu manifest_errors=%zu generic_variants=%zu fast_variants=%zu\n",
      petari_uber_elapsed(), petariUberBoot.scheduled.size(), petariUberBoot.titleKeys.size(),
      petariUberBoot.rejectedConfigs, petariUberBoot.manifestErrors, petariUberBoot.genericKeys.size(), petariUberBoot.fastKeys.size());
  petari_uber_maybe_ready();
}
static void petari_uber_schedule_known() {
  if (!gx::petari_uber_enabled()) return;
  const auto scene = scene_render_target_layout();
  auto off = scene; off.colorAttachmentCount = 1; off.sampleCount = 1; detail::finalize_render_target_layout(off);
  std::vector<gx::PipelineConfig> configs;
  {
    std::lock_guard lock{g_pipelineMutex};
    if (!g_hasPipelineThread || g_pipelineThreadEnd) return;
    for (const auto& [key, item] : g_knownPipelines)
      if (item.type == ShaderType::GX) configs.push_back(std::get<gx::PipelineConfig>(item.config));
    petari_uber_push_job({0, 0, [configs = std::move(configs), scene, off] {
      for (const auto& config : configs) {
        if (g_pipelineThreadEnd) return;
        petari_uber_queue_layouts(config, scene, off, 3);
      }
      std::lock_guard lock{g_pipelineMutex};
      petariUberBoot.cacheDone = true;
      petari_uber_maybe_ready();
    }});
  }
  g_pipelineQueueCv.notify_all();
}
static void petari_uber_schedule_draw(const gx::PipelineConfig& config, const RenderTargetLayout& layout) {
  bool title;
  { std::lock_guard lock{g_pipelineMutex}; title = !petariUberTitleReleased && petariUberBoot.activeStage.empty(); }
  petari_uber_queue_pair(config, layout, 1, title);
}
static void petari_uber_schedule_stage(const gx::PipelineConfig& config, const RenderTargetLayout& scene,
                                     const RenderTargetLayout& off, const std::string& stage) {
  if (!gx::petari_uber_enabled() || std::strcmp(gx::petari_uber_reason(config.shaderConfig), "covered")) return;
  const bool title = stage == "Unknown" || stage == "FileSelect";
  {
    std::lock_guard lock{g_pipelineMutex};
    const auto active = title ? std::string{} : stage;
    if (petariUberBoot.activeStage != active) {
      petariUberBoot.activeStage = active;
      for (auto& job : petariUberJobs) job.priority = petari_uber_job_priority(job);
      std::stable_sort(petariUberJobs.begin(), petariUberJobs.end(), [](const auto& a, const auto& b) { return a.priority < b.priority; });
    }
    auto& keys = petariUberBoot.stageKeys[stage];
    for (const auto& layout : {scene, off}) {
      const auto pair = gx::petari_uber_keys(config, layout);
      keys.insert(pair.generic); keys.insert(pair.fast);
    }
  }
  petari_uber_queue_layouts(config, scene, off, title ? 1 : 2, title);
}
static void petari_uber_stage_report(const std::string& stage) {
  if (!gx::petari_uber_enabled()) return;
  std::lock_guard lock{g_pipelineMutex};
  const auto it = petariUberBoot.stageKeys.find(stage);
  if (it == petariUberBoot.stageKeys.end()) return;
  const auto pending = std::count_if(it->second.begin(), it->second.end(),
      [](auto key) { return !petariUberBoot.completed.contains(key); });
  std::fprintf(stderr, "[gx uber async] event=stage_priority stage=%s elapsed_ms=%.3f variants=%zu pending=%zu workers=%u completed=%zu generic_pending=%zu fast_pending=%zu\n",
      stage.c_str(), petari_uber_elapsed(), it->second.size(), size_t(pending), petariActiveWorkers,
      petariUberBoot.completed.size(),
      size_t(std::count_if(it->second.begin(), it->second.end(), [](auto k) { return petariUberBoot.genericKeys.contains(k) && !petariUberBoot.completed.contains(k); })),
      size_t(std::count_if(it->second.begin(), it->second.end(), [](auto k) { return petariUberBoot.fastKeys.contains(k) && !petariUberBoot.completed.contains(k); })));
}
static bool petari_uber_or_specialized_ready(PipelineRef key) { // caller holds g_pipelineMutex
  if (g_pipelines.contains(key) || g_pipelineThreadEnd) return true;
  if (!gx::petari_uber_enabled()) return false;
  CompiledPipeline fallback;
  return gx::petari_uber_ready(gx::petari_uber_lookup(key), fallback);
}
static void petari_uber_observe_title(bool covered, bool fallbackReady) {
  if (!gx::petari_uber_enabled()) return;
  std::lock_guard lock{g_pipelineMutex};
  if (petariUberBoot.titleObserved || petariActiveStage != "FileSelect") return;
  petariUberBoot.titleObserved = true;
  const auto pending = petariUberBoot.scheduled.size() - petariUberBoot.completed.size();
  std::fprintf(stderr, "[gx uber async] event=title_first_draw phase=recording elapsed_ms=%.3f covered=%u fallback_ready=%u title_scanned=%u title_pending=%zu all_pending=%zu failed=%zu all_ready=%u\n",
      petari_uber_elapsed(), covered, fallbackReady, petariUberBoot.frontEndScanned,
      petari_uber_title_pending(), pending, petariUberBoot.failed.size(),
      petariUberBoot.seedsDone && petariUberBoot.cacheDone && !pending && petariUberBoot.failed.empty());
  petariUberTitleReleased = true;
  std::fprintf(stderr, "[gx uber async] event=prep_release stage=FileSelect phase=recording elapsed_ms=%.3f workers=%u\n",
      petari_uber_elapsed(), petariActiveWorkers);
  g_pipelineQueueCv.notify_all();
}
static void petari_uber_async_report() {
  if (!gx::petari_uber_enabled()) return;
  std::lock_guard lock{g_pipelineMutex};
  std::fprintf(stderr, "[gx uber async summary] variants=%zu completed=%zu pending=%zu failed=%zu cancelled=%zu enumeration_done=%u title_observed=%u manifest_errors=%zu\n",
      petariUberBoot.scheduled.size(), petariUberBoot.completed.size(),
      petariUberBoot.scheduled.size() - petariUberBoot.completed.size(), petariUberBoot.failed.size(),
      petariUberBoot.cancelled, petariUberBoot.seedsDone && petariUberBoot.cacheDone,
      petariUberBoot.titleObserved, petariUberBoot.manifestErrors);
}
static void petari_uber_async_stop() {
  if (!gx::petari_uber_enabled()) return;
  std::lock_guard lock{g_pipelineMutex};
  petariUberBoot.cancelled += petariUberJobs.size();
  petariUberJobs.clear(); // workers already joined; no callbacks can outlive the device
  petariUberBoot.started = false; // a later device/cache initialization may start a fresh generation
  petariUberTitleReleased = false;
}
} // namespace aurora::gfx

namespace aurora::gx {
void petari_uber_boot() {
  if (!petari_uber_enabled()) return;
  const auto start = PetariPipeline::Clock::now();
  const auto scene = gfx::scene_render_target_layout();
  auto off = scene; off.colorAttachmentCount = 1; off.sampleCount = 1; gfx::detail::finalize_render_target_layout(off);
  const char* env = std::getenv("PETARI_PIPELINE_SEED_DIR");
  const auto dir = env && *env ? std::filesystem::path(env) :
      std::filesystem::path(g_config.resourcesPath ? g_config.resourcesPath : "") / "pipeline-seeds";
  bool queued = false;
  unsigned workers = 0;
  {
    std::lock_guard lock{gfx::g_pipelineMutex};
    workers = gfx::petariActiveWorkers;
    if (!gfx::petariUberBoot.started) {
      gfx::petariUberBoot = {};
      gfx::petariUberTitleReleased = false;
      gfx::petariUberBoot.started = true;
      gfx::petariUberBoot.startedAt = start;
      if (gfx::g_hasPipelineThread && !gfx::g_pipelineThreadEnd) {
        gfx::petari_uber_push_job({0, 0, [dir, scene, off] { gfx::petari_uber_scan_seeds(dir, scene, off); }});
        queued = true;
      }
    }
  }
  gfx::g_pipelineQueueCv.notify_all();
  std::fprintf(stderr, "[gx uber async] event=boot_return enqueue_ms=%.3f queued=%u synchronous_compiles=0 prep_policy=frontend_generic_until_title workers=%u prep_workers=%u\n",
      PetariPipeline::milliseconds(PetariPipeline::Clock::now() - start), queued, workers, std::min(workers, gfx::kPetariOnDemandWorkers));
}
} // namespace aurora::gx
