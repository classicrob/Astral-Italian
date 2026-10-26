// Appended to the cache TU, after stage attribution is defined. All queue/state
// accesses use its existing mutex; callbacks execute outside that mutex.
#include <set>
#include "pipeline_deadline_manifest.hpp"
#include "frontend_membership.hpp"
#include "frontend_policy.hpp"
namespace aurora::gfx {
struct PetariUberBootState {
  bool started = false, seedsDone = false, cacheDone = false, frontEndScanned = false;
  bool titleObserved = false, readyReported = false, frontReadyReported = false;
  bool genericReadyReported = false, fastReadyReported = false;
  PetariPipeline::Clock::time_point startedAt{};
  std::set<PipelineRef> scheduled, completed, failed, titleKeys;
  std::map<std::string, std::set<PipelineRef>> stageKeys;
  std::map<std::string, std::map<PipelineRef, size_t>> stageOrder;
  std::map<std::string, std::map<PipelineRef, uint32_t>> stageFrames;
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
static std::string petari_uber_order_stage(const PetariUberJob& job) { // caller holds g_pipelineMutex
  if (job.priority == 1) return petariUberBoot.activeStage;
  if (job.priority == 3) {
    // The boot scene precedes title even if one sidecar is unavailable. Shared
    // keys belong to their earliest scene; each job has an independent tuple.
    for (const auto* name : {"Unknown", "FileSelect"}) {
      const auto at = petariUberBoot.stageKeys.find(name);
      if (at != petariUberBoot.stageKeys.end() && at->second.contains(job.key)) return name;
    }
  }
  return {};
}
static uint32_t petari_uber_job_frame(const PetariUberJob& job) { // caller holds g_pipelineMutex
  const auto stage = petariUberBoot.stageFrames.find(petari_uber_order_stage(job));
  if (stage == petariUberBoot.stageFrames.end()) return UINT32_MAX;
  const auto at = stage->second.find(job.key);
  return at == stage->second.end() ? UINT32_MAX : at->second;
}
static size_t petari_uber_job_rank(const PetariUberJob& job) { // caller holds g_pipelineMutex
  const auto stage = petariUberBoot.stageOrder.find(petari_uber_order_stage(job));
  if (stage == petariUberBoot.stageOrder.end()) return SIZE_MAX;
  const auto at = stage->second.find(job.key);
  return at == stage->second.end() ? SIZE_MAX : at->second;
}
static bool petari_uber_job_before(const PetariUberJob& a, const PetariUberJob& b) {
  if (a.priority != b.priority) return a.priority < b.priority;
  if (a.priority != 1 && a.priority != 3) return false;
  if (a.tier != b.tier) return a.tier == gx::PetariUberTier::Generic;
  if (a.tier != gx::PetariUberTier::Generic) return false;
  if (a.priority == 3) {
    const auto ordinal = [](const auto& job) {
      const auto stage = petari_uber_order_stage(job);
      return stage == "Unknown" ? 0 : stage == "FileSelect" ? 1 : 2;
    };
    if (ordinal(a) != ordinal(b)) return ordinal(a) < ordinal(b);
  }
  const auto af = petari_uber_job_frame(a), bf = petari_uber_job_frame(b);
  if (af != bf) return af < bf;
  // Longest first only inside one explicitly recorded frame. Unknown frame
  // values never form a guessed deadline group: retain first-use rank there.
  if (af != UINT32_MAX) {
    if (a.hasIndirect != b.hasIndirect) return a.hasIndirect;
    if (a.tevStages != b.tevStages) return a.tevStages > b.tevStages;
    if (a.texgenExtent != b.texgenExtent) return a.texgenExtent > b.texgenExtent;
  }
  return petari_uber_job_rank(a) < petari_uber_job_rank(b);
}
static void petari_uber_push_job(PetariUberJob job) { // caller holds g_pipelineMutex
  job.priority = petari_uber_job_priority(job);
  const auto at = std::find_if(petariUberJobs.begin(), petariUberJobs.end(),
      [&](const auto& other) { return petari_uber_job_before(job, other); });
  petariUberJobs.insert(at, std::move(job));
}
static void petari_uber_reorder_jobs() { // caller holds g_pipelineMutex
  for (auto& job : petariUberJobs) job.priority = petari_uber_job_priority(job);
  std::stable_sort(petariUberJobs.begin(), petariUberJobs.end(), petari_uber_job_before);
}
static void petari_uber_activate_stage(const std::string& stage) { // caller holds g_pipelineMutex
  const auto active = stage == "Unknown" || stage == "FileSelect" ? std::string{} : stage;
  if (petariUberBoot.activeStage != active) {
    petariUberBoot.activeStage = active;
    petari_uber_reorder_jobs();
  }
}
struct PetariUberAdmission {
  PetariUberAdmission() { std::lock_guard lock{g_pipelineMutex}; ++petariUberAdmissions; }
  PetariUberAdmission(const PetariUberAdmission&) = delete;
  PetariUberAdmission& operator=(const PetariUberAdmission&) = delete;
  ~PetariUberAdmission() {
    {
      std::lock_guard lock{g_pipelineMutex};
      petari_uber_reorder_jobs();
      --petariUberAdmissions;
    }
    g_pipelineQueueCv.notify_all();
  }
};
static void petari_uber_register_stage_keys(const gx::PipelineConfig& config, const RenderTargetLayout& scene,
                                           const RenderTargetLayout& off, const std::string& stage,
                                           uint32_t frame = UINT32_MAX) { // caller holds g_pipelineMutex
  if (std::strcmp(gx::petari_uber_reason(config.shaderConfig), "covered")) return;
  auto& keys = petariUberBoot.stageKeys[stage];
  auto& order = petariUberBoot.stageOrder[stage];
  for (const auto& layout : {scene, off}) {
    const auto pair = gx::petari_uber_keys(config, layout);
    for (const auto key : {pair.generic, pair.fast}) {
      keys.insert(key);
      if (!order.contains(key)) order.emplace(key, order.size());
      if (frame != UINT32_MAX) {
        auto& frames = petariUberBoot.stageFrames[stage];
        const auto at = frames.find(key);
        if (at == frames.end()) frames.emplace(key, frame);
        else at->second = std::min(at->second, frame); // one compiled alias can serve several configs
      }
    }
  }
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
      }, tier, title, priority == 1 && !title,
          config.shaderConfig.numIndStages != 0, config.shaderConfig.tevStageCount,
          (gx::petari_uber_variant(config.shaderConfig, gx::PetariUberTier::Generic) >> 5) & 15u});
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
struct PetariFrontendPump {
  PetariPipeline::Clock::time_point last = PetariPipeline::Clock::now();
  double maxGapMs = 0;
  unsigned calls = 0;
  bool mainThread = true;
  void poll() {
    const auto now = PetariPipeline::Clock::now();
    maxGapMs = std::max(maxGapMs, PetariPipeline::milliseconds(now - last));
    last = now; ++calls;
    mainThread &= ::petari_uber_pump_boot_events();
  }
};
static std::vector<gx::PipelineConfig> petari_uber_read_seed(
    const std::filesystem::path& path, PetariFrontendPump* pump = nullptr,
    PetariPipeline::Clock::time_point deadline = PetariPipeline::Clock::time_point::max()) {
  auto* db = open_pipeline_cache_seed_db(path.string());
  if (!db) {
    std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; return {};
  }
  std::vector<gx::PipelineConfig> configs;
  sqlite3_stmt* stmt = nullptr;
  const auto sql = "SELECT type,hash,config_version,config_size,config,first_frame_used FROM pipeline_cache WHERE type=1 AND config_version=13 ORDER BY first_frame_used";
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    int status;
    PetariPipelineCache::RecipeRows validation(db);
    while ((status = sqlite3_step(stmt)) == SQLITE_ROW && !g_pipelineThreadEnd) {
      if (!PetariPipelineCache::admitRecipe(validation, stmt)) {
        std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.rejectedConfigs; continue;
      }
      if (pump && (configs.size() % 16 == 0)) pump->poll();
      if (pump && (PetariPipeline::Clock::now() >= deadline || configs.size() >= 512)) {
        std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; break;
      }
      gx::PipelineConfig config;
      std::memcpy(&config, sqlite3_column_blob(stmt, 4), sizeof(config));
      if (gx::petari_cached_shader_valid(config.shaderConfig, 0))
        configs.push_back(config);
      else { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.rejectedConfigs; }
    }
    if (status != SQLITE_DONE && status != SQLITE_ROW) {
      std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors;
    }
  } else { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; }
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return configs;
}
static void petari_uber_scan_frontend(const std::filesystem::path& dir, RenderTargetLayout scene,
                                     RenderTargetLayout off, PetariFrontendPump* pump = nullptr,
                                     PetariPipeline::Clock::time_point deadline = PetariPipeline::Clock::time_point::max(),
                                     const std::vector<uint64_t>& observed = {PetariFrontendMembership::kGlobalHashes.begin(), PetariFrontendMembership::kGlobalHashes.end()}) {
  PetariUberAdmission admission;
  std::map<PipelineRef, gx::PipelineConfig> frontend;
  for (const auto* name : {"Unknown.db", "FileSelect.db"}) {
    const auto path = dir / name;
    auto configs = petari_uber_read_seed(path, pump, deadline);
    for (const auto& config : configs)
      frontend.emplace(xxh3_hash(config, static_cast<uint64_t>(ShaderType::GX)), config);
    petari_uber_admit_stage(configs, scene, off, path.stem().string(), path.string(), false);
  }
  size_t bootstrapAdded = 0;
  auto* bootstrap = open_pipeline_cache_seed_db((dir / "__global__.db").string());
  sqlite3_stmt* query = nullptr;
  if (bootstrap && sqlite3_prepare_v2(bootstrap, "SELECT type,hash,config_version,config_size,config,first_frame_used FROM pipeline_cache WHERE type=1 AND config_version=13 AND hash=?", -1, &query, nullptr) == SQLITE_OK) {
    PetariPipelineCache::RecipeRows validation(bootstrap);
    for (const auto hash : observed) {
      if (pump) pump->poll();
      if (pump && PetariPipeline::Clock::now() >= deadline) { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; break; }
      sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(hash));
      gx::PipelineConfig config{};
      const auto status = sqlite3_step(query);
      bool valid = status == SQLITE_ROW && PetariPipelineCache::admitRecipe(validation, query);
      if (valid) {
        std::memcpy(&config, sqlite3_column_blob(query, 4), sizeof(config));
        valid = xxh3_hash(config, static_cast<uint64_t>(ShaderType::GX)) == hash && gx::petari_cached_shader_valid(config.shaderConfig, hash);
      }
      if (!valid) { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; }
      else if (frontend.emplace(hash, config).second) {
        ++bootstrapAdded;
        { std::lock_guard lock{g_pipelineMutex}; petari_uber_register_stage_keys(config, scene, off, "Unknown"); }
        for (const auto& layout : {scene, off}) petari_uber_queue_variant(config, layout, 3, true, gx::PetariUberTier::Generic);
      }
      sqlite3_reset(query); sqlite3_clear_bindings(query);
    }
  } else { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; }
  sqlite3_finalize(query); if (bootstrap) sqlite3_close(bootstrap);
  std::fprintf(stderr, "[gx frontend membership] version=%u observed_global=%zu added=%zu uncovered=%zu evidence=%s\n",
      PetariFrontendMembership::kVersion, observed.size(), bootstrapAdded,
      PetariFrontendMembership::kUncovered.size(), PetariFrontendMembership::kEvidenceSha256);
  // __global__ has no stage tag. Frontend membership comes from frontend data;
  // match its exact hashed rows, never a prefix of global first-use ranks.
  size_t matched = 0, missing = 0;
  auto* db = open_pipeline_cache_seed_db((dir / "__global__.db").string());
  sqlite3_stmt* stmt = nullptr;
  if (db && sqlite3_prepare_v2(db, "SELECT type,hash,config_version,config_size,config,first_frame_used FROM pipeline_cache WHERE type=1 AND config_version=13 AND hash=?", -1, &stmt, nullptr) == SQLITE_OK) {
    PetariPipelineCache::RecipeRows validation(db);
    for (const auto& [hash, config] : frontend) {
      if (pump) pump->poll();
      if (pump && PetariPipeline::Clock::now() >= deadline) {
        std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; break;
      }
      sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(hash));
      const auto status = sqlite3_step(stmt);
      if (status == SQLITE_ROW && PetariPipelineCache::admitRecipe(validation, stmt) &&
          std::memcmp(sqlite3_column_blob(stmt, 4), &config, sizeof(config)) == 0) ++matched;
      else if (status == SQLITE_DONE) ++missing;
      else { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; }
      sqlite3_reset(stmt); sqlite3_clear_bindings(stmt);
    }
  } else { std::lock_guard lock{g_pipelineMutex}; ++petariUberBoot.manifestErrors; }
  sqlite3_finalize(stmt);
  if (db) sqlite3_close(db);
  std::lock_guard lock{g_pipelineMutex};
  petariUberBoot.frontEndScanned = true;
  std::fprintf(stderr, "[gx frontend source] policy=exact-stage-hash-membership configs=%zu global_matches=%zu global_missing=%zu manifest_errors=%zu\n",
      frontend.size(), matched, missing, petariUberBoot.manifestErrors);
}
static void petari_uber_scan_seeds(const std::filesystem::path& dir, RenderTargetLayout scene,
                                  RenderTargetLayout off) {
  const auto scan = [&](const std::filesystem::path& path, unsigned priority) {
    const auto configs = petari_uber_read_seed(path);
    if (priority == 1) petari_uber_admit_stage(configs, scene, off, path.stem().string(), path.string(), false);
    else for (const auto& config : configs) petari_uber_queue_layouts(config, scene, off, priority);
  };
  petari_uber_scan_frontend(dir, scene, off);
  std::error_code ec;
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
                                     const RenderTargetLayout& off, const std::string& stage, bool activate) {
  if (!gx::petari_uber_enabled() || std::strcmp(gx::petari_uber_reason(config.shaderConfig), "covered")) return;
  const bool title = stage == "Unknown" || stage == "FileSelect";
  {
    std::lock_guard lock{g_pipelineMutex};
    petari_uber_register_stage_keys(config, scene, off, stage);
    if (activate) petari_uber_activate_stage(stage);
  }
  petari_uber_queue_layouts(config, scene, off, title ? 1 : 2, title);
}
static void petari_uber_admit_stage_frames(const std::vector<gx::PipelineConfig>& configs,
                                          const RenderTargetLayout& scene, const RenderTargetLayout& off,
                                          const std::string& stage, const std::map<uint64_t, uint32_t>& frames,
                                          bool activate = true) {
  if (!gx::petari_uber_enabled()) return;
  PetariUberAdmission admission;
  {
    std::lock_guard lock{g_pipelineMutex};
    petariUberBoot.stageFrames[stage].clear(); // invalid/missing metadata falls back for the whole stage
    for (const auto& config : configs) {
      const auto hash = xxh3_hash(config, static_cast<uint64_t>(ShaderType::GX));
      const auto at = frames.find(hash);
      petari_uber_register_stage_keys(config, scene, off, stage, at == frames.end() ? UINT32_MAX : at->second);
    }
    // A slow frontend scan cannot take ownership back from a real scene that
    // has since begun. Likewise a superseded scene load cannot reactivate itself.
    if (activate && petariActiveStage == stage) petari_uber_activate_stage(stage);
  }
  // All aliases and deadlines exist before jobs are admitted. In particular,
  // a cheap config enumerated first cannot start ahead of a costly config from
  // the same recorded frame while this batch is still being populated.
  for (const auto& config : configs) petari_uber_schedule_stage(config, scene, off, stage, false);
}
static void petari_uber_admit_stage(const std::vector<gx::PipelineConfig>& configs,
                                   const RenderTargetLayout& scene, const RenderTargetLayout& off,
                                   const std::string& stage, const std::string& manifestPath, bool activate) {
  if (!gx::petari_uber_enabled()) return;
  PetariDeadlines::Result loaded{false, "manifest-unavailable", {}};
  if (!manifestPath.empty()) {
    auto* db = open_pipeline_cache_seed_db(manifestPath);
    if (db) {
      auto sidecar = std::filesystem::path(manifestPath); sidecar.replace_extension(".pfd");
      loaded = PetariDeadlines::loadManifest(db, sidecar, stage);
      sqlite3_close(db);
    }
  }
  std::fprintf(stderr, "[gx first-use deadlines] stage=%s policy=%s reason=%s configs=%zu\n", stage.c_str(),
               loaded.valid ? "frame-then-cost" : "first-use-order", loaded.reason.c_str(), loaded.frames.size());
  petari_uber_admit_stage_frames(configs, scene, off, stage, loaded.frames, activate);
}
static void petari_uber_stage_report(const std::string& stage) {
  if (!gx::petari_uber_enabled()) return;
  std::lock_guard lock{g_pipelineMutex};
  const auto it = petariUberBoot.stageKeys.find(stage);
  if (it == petariUberBoot.stageKeys.end()) return;
  const auto pending = std::count_if(it->second.begin(), it->second.end(),
      [](auto key) { return !petariUberBoot.completed.contains(key); });
  std::fprintf(stderr, "[gx uber async] event=stage_priority stage=%s elapsed_ms=%.3f variants=%zu pending=%zu workers=%u completed=%zu generic_pending=%zu fast_pending=%zu monotonic_ms=%.3f\n",
      stage.c_str(), petari_uber_elapsed(), it->second.size(), size_t(pending), petariActiveWorkers,
      petariUberBoot.completed.size(),
      size_t(std::count_if(it->second.begin(), it->second.end(), [](auto k) { return petariUberBoot.genericKeys.contains(k) && !petariUberBoot.completed.contains(k); })),
      size_t(std::count_if(it->second.begin(), it->second.end(), [](auto k) { return petariUberBoot.fastKeys.contains(k) && !petariUberBoot.completed.contains(k); })),
      PetariPipeline::milliseconds(PetariPipeline::Clock::now().time_since_epoch()));
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
  std::fprintf(stderr, "[gx uber async] event=title_first_draw phase=recording elapsed_ms=%.3f covered=%u fallback_ready=%u title_scanned=%u title_pending=%zu all_pending=%zu failed=%zu all_ready=%u monotonic_ns=%lld wall_ns=%lld\n",
      petari_uber_elapsed(), covered, fallbackReady, petariUberBoot.frontEndScanned,
      petari_uber_title_pending(), pending, petariUberBoot.failed.size(),
      petariUberBoot.seedsDone && petariUberBoot.cacheDone && !pending && petariUberBoot.failed.empty(),
      static_cast<long long>(std::chrono::duration_cast<std::chrono::nanoseconds>(PetariPipeline::Clock::now().time_since_epoch()).count()),
      static_cast<long long>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count()));
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
  petariUberPrePresent = false;
  petariUberFrontendArmed = false;
}
} // namespace aurora::gfx

namespace aurora::gfx {
static PetariPipeline::Clock::time_point petariFrontendStart{};
static std::function<void()> petariFrontendScan, petariFrontendRemainder;
}
namespace aurora::gx {
void petari_uber_boot() {
  if (!petari_uber_enabled()) return;
  const auto start = PetariPipeline::Clock::now();
  const auto scene = gfx::scene_render_target_layout();
  auto off = scene; off.colorAttachmentCount = 1; off.sampleCount = 1; gfx::detail::finalize_render_target_layout(off);
  const char* env = std::getenv("PETARI_PIPELINE_SEED_DIR");
  const auto dir = env && *env ? std::filesystem::path(env) :
      std::filesystem::path(g_config.resourcesPath ? g_config.resourcesPath : "") / "pipeline-seeds";
  std::lock_guard lock{gfx::g_pipelineMutex};
  if (gfx::petariUberBoot.started) return;
  gfx::petariUberBoot = {};
  gfx::petariUberTitleReleased = false;
  gfx::petariUberBoot.started = true;
  gfx::petariUberBoot.startedAt = start;
  gfx::petariUberFrontendArmed = gfx::g_hasPipelineThread && !gfx::g_pipelineThreadEnd;
  gfx::petariFrontendScan = [dir, scene, off] {
    gfx::PetariFrontendPump pump;
    gfx::petari_uber_scan_frontend(dir, scene, off, &pump,
        gfx::petariFrontendStart + std::chrono::milliseconds(PetariBootSplash::prepCapMs));
  };
  gfx::petariFrontendRemainder = [dir, scene, off] { gfx::petari_uber_scan_seeds(dir, scene, off); };
  std::fprintf(stderr, "[gx uber async] event=boot_return enqueue_ms=%.3f queued=%u main_thread_compiles=0 prep_policy=armed_normal_frames workers=%u\n",
      PetariPipeline::milliseconds(PetariPipeline::Clock::now() - start), gfx::petariUberFrontendArmed, gfx::petariActiveWorkers);
}
}
extern "C" bool petari_gx_frontend_armed() {
  std::lock_guard lock{aurora::gfx::g_pipelineMutex};
  return aurora::gfx::petariUberFrontendArmed;
}
extern "C" void petari_gx_frontend_begin() {
  using namespace aurora::gfx;
  {
    std::lock_guard lock{g_pipelineMutex};
    if (!petariUberFrontendArmed) return;
    petariFrontendStart = PetariPipeline::Clock::now();
    petariUberPrePresent = true;
    petariUberFrontendArmed = false;
  }
  // No Generic key exists before this point. Admission is atomic across the
  // exact finite set; utilities may draw the splash throughout the phase.
  if (petariFrontendScan) petariFrontendScan();
  g_pipelineQueueCv.notify_all();
}
extern "C" void petari_gx_frontend_status(unsigned* total, unsigned* pending, unsigned* failed, double* elapsedMs) {
  using namespace aurora::gfx;
  std::lock_guard lock{g_pipelineMutex};
  *total = petariUberBoot.titleKeys.size();
  *pending = petari_uber_title_pending();
  *failed = std::count_if(petariUberBoot.titleKeys.begin(), petariUberBoot.titleKeys.end(),
      [](auto key) { return petariUberBoot.failed.contains(key); });
  *elapsedMs = petariFrontendStart == PetariPipeline::Clock::time_point{} ? 0 :
      PetariPipeline::milliseconds(PetariPipeline::Clock::now() - petariFrontendStart);
}
extern "C" bool petari_gx_frontend_finish(unsigned frames, unsigned pumps, double maxGapMs, bool mainThread) {
  using namespace aurora::gfx;
  std::lock_guard lock{g_pipelineMutex};
  const auto elapsed = petariFrontendStart == PetariPipeline::Clock::time_point{} ? 0 :
      PetariPipeline::milliseconds(PetariPipeline::Clock::now() - petariFrontendStart);
  const auto pending = petari_uber_title_pending();
  const auto failed = std::count_if(petariUberBoot.titleKeys.begin(), petariUberBoot.titleKeys.end(),
      [](auto key) { return petariUberBoot.failed.contains(key); });
  const bool ready = petariUberPrePresent && mainThread && frames >= 3 && !g_pipelineThreadEnd && !pending && !failed &&
      petariUberBoot.frontEndScanned && !petariUberBoot.titleKeys.empty() && !petariUberBoot.manifestErrors &&
      !petariUberBoot.rejectedConfigs && elapsed <= PetariBootSplash::prepCapMs;
  std::string keys;
  for (auto key : petariUberBoot.titleKeys) {
    char value[18]; std::snprintf(value, sizeof(value), "%016llx,", static_cast<unsigned long long>(key)); keys += value;
  }
  petariUberPrePresent = petariUberFrontendArmed = false;
  std::fprintf(stderr, "[gx frontend pre-present] version=3 result=%s keys=%zu completed=%zu pending=%zu failed=%zu manifest_errors=%zu elapsed_ms=%.3f cap_ms=%u workers=%u pumps=%u pump_max_gap_ms=%.3f main_thread=%u presented_frames=%u key_list=%s\n",
      ready ? "ready" : pending ? "cap" : "failed", petariUberBoot.titleKeys.size(), petariUberBoot.titleKeys.size() - pending,
      pending, size_t(failed), petariUberBoot.manifestErrors, elapsed, PetariBootSplash::prepCapMs, petariActiveWorkers, pumps, maxGapMs,
      mainThread, frames, keys.c_str());
  if (petariFrontendRemainder) petari_uber_push_job({0, 0, std::move(petariFrontendRemainder)});
  petariFrontendScan = {};
  g_pipelineQueueCv.notify_all();
  return ready;
}
