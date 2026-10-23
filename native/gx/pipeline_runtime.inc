static void petari_note_stage_config(HashType hash, PipelineRef runtimeKey);
static void petari_stage_reset();
static void petari_stage_flush_logs();
static void petari_seed_global();
static PetariPipeline::Clock::time_point petariGlobalStarted{}, petariGlobalLastLog{};
static size_t petariGlobalInitialPending = 0;
static bool petariGlobalActive = false;
static void petari_global_progress() {
  std::lock_guard lock{g_pipelineMutex};
  if (!petariGlobalActive) return;
  const auto now = PetariPipeline::Clock::now();
  const auto remaining = g_pendingPipelines.size();
  if (remaining && now - petariGlobalLastLog < std::chrono::seconds(5)) return;
  petariGlobalLastLog = now;
  const auto completed = petariGlobalInitialPending - std::min(petariGlobalInitialPending, remaining);
  std::fprintf(stderr, "[gx global prep] completed=%zu total=%zu remaining=%zu elapsed_ms=%.3f state=%s\n",
               completed, petariGlobalInitialPending, remaining,
               PetariPipeline::milliseconds(now - petariGlobalStarted), remaining ? "preparing" : "ready");
  if (!remaining) petariGlobalActive = false;
}
struct PetariPipelineSample {
  ShaderType type = ShaderType::GX;
  HashType configKey = 0;
  PetariPipeline::Clock::time_point queued{}, started{}, finished{}, requested{}, firstSkip{}, lastSkip{};
  PetariPipeline::Stages stages{};
  uint64_t skipped = 0;
  bool failed = false;
  bool drawRequested = false;
  bool drawBlocking = false; // a draw or clear waited for (or skipped) this variant
};
static absl::flat_hash_map<PipelineRef, PetariPipelineSample> petariPipelineSamples;
static std::atomic<unsigned> petariPipelineWaiters{0};
static std::atomic<uint64_t> petariSkippedDraws{0};
static uint32_t petariFailedPipelines = 0; // guarded by g_pipelineMutex
static unsigned petariActiveWorkers = PetariPipeline::workerCount(); // guarded by g_pipelineMutex
// Workers with an index at or above this are burst workers (PetariPipeline::burstWorkerCount()).
static unsigned petariNormalWorkers = PetariPipeline::workerCount(); // guarded by g_pipelineMutex
// Guarded by g_pipelineMutex: global-backlog jobs compiling now and their cap
// outside gameplay; stage jobs no draw requested yet compiling now; and
// whether the last frame was active gameplay (set from the frame seam).
static unsigned petariBackgroundInFlight = 0;
static unsigned petariStageSpeculativeInFlight = 0;
static bool petariGameplay = false;
// Bounded GX draw waits: remaining wait this frame, and draws whose pipeline was
// still compiling when the budget ran out (encoded as skips until it lands).
static PetariPipeline::Clock::duration petariDrawBudgetLeft = PetariPipeline::drawWaitBudget();
static uint64_t petariDeferredDraws = 0;
// Global prewarm (the __global__ backlog), guarded by g_pipelineMutex. It is speculative work. It runs from boot
// (PetariPipeline::prewarmBeforePlay) on every worker except the two reserved for draw-requested and current-stage
// jobs, so menu draws never wait behind it; PETARI_PIPELINE_PREWARM_WORKERS (1..8, default 8 = no cap beyond that
// reservation) caps it outside gameplay. In gameplay it runs 1 in flight and pauses for a while after any frame over
// budget. Locking it until play (26feb80a) left a true-cold first stage with ~100 hot variants pending at its gate
// and a 2 s freeze (r30 bisect, 2026-10-23: the revert passed 4/5 true-cold Yoshi runs, 5.3.2 failed 4/4).
static unsigned petariBackgroundCap = [] {
  const char* value = std::getenv("PETARI_PIPELINE_PREWARM_WORKERS");
  char* end = nullptr;
  const unsigned long count = value && *value ? std::strtoul(value, &end, 10) : 0;
  return end && *end == '\0' && count >= 1 && count <= 8 ? static_cast<unsigned>(count) : 8u;
}();
static bool petariPrewarmUnlocked = false;
static unsigned petariBacklogHoldFrames = 0;
constexpr unsigned kPetariBacklogHoldFrames = 30;
constexpr double kPetariFrameBudgetMs = 20.0;
// Workers 0..kPetariOnDemandWorkers-1 take only jobs a draw (or a draw-blocking clear) waits for, so on-demand work
// never queues behind speculative stage or prewarm compiles. Applies when there are more normal workers than this.
// Outside active gameplay they also take the current stage's manifest (PetariPipeline::stageOnReservedWorkers).
constexpr unsigned kPetariOnDemandWorkers = 2;
// Populated only by opt-in fallback preparation. Existing workers borrow their
// speculative slots for this finite set; actual draw-blocking jobs stay first.
// Shutdown joins those workers before dropping queued callbacks.
struct PetariUberJob {
  PipelineRef key = 0; // zero is an enumeration task, never a compiled variant
  unsigned priority = 3; // enumeration, front-end/drawn, current-stage, remaining variants
  std::function<void()> run;
};
static std::deque<PetariUberJob> petariUberJobs; // g_pipelineMutex
static bool petari_uber_worker_ready(unsigned workerIndex) {
  return workerIndex < petariActiveWorkers && !petariUberJobs.empty();
}
static bool petari_take_uber_job(unsigned workerIndex, std::function<void()>& job) {
  if (!petari_uber_worker_ready(workerIndex)) return false;
  // A real draw gets the next slot; fallback preparation precedes speculation.
  if (!g_pipelineQueue.empty()) {
    const auto it = petariPipelineSamples.find(g_pipelineQueue.front().hash);
    if (it != petariPipelineSamples.end() && it->second.drawBlocking) return false;
  }
  job = std::move(petariUberJobs.front().run);
  petariUberJobs.pop_front();
  return true;
}
static void petari_uber_schedule_known();
static void petari_uber_schedule_draw(const gx::PipelineConfig&, const RenderTargetLayout&);
static void petari_uber_schedule_stage(const gx::PipelineConfig&, const RenderTargetLayout&,
                                       const RenderTargetLayout&, const std::string&);
static void petari_uber_stage_report(const std::string&);
static void petari_uber_observe_title(bool covered, bool fallbackReady);
static void petari_uber_async_report();
static void petari_uber_async_stop();
static bool petari_uber_or_specialized_ready(PipelineRef);
// Recent compile cost (exponential average of build_ms over every finished job,
// cache hits included), guarded by g_pipelineMutex. The stage gate uses it to skip
// waits that cannot finish within the budget: a frozen frame is worse than pop-in.
static double petariCompileEwmaMs = 0;

extern "C" void petari_gx_pipeline_startup_finished() {
  unsigned previous, current;
  {
    std::lock_guard lock{g_pipelineMutex};
    previous = petariActiveWorkers;
    current = petariActiveWorkers = PetariPipeline::burstWorkerCount();
  }
  g_pipelineQueueCv.notify_all();
  std::fprintf(stderr, "[gx pipeline pool] startup_finished=1 previous_workers=%u background_workers=%u "
               "normal_workers=%u gameplay_stage_cap=%u gameplay_backlog_cap=%u extra_inflight=drain\n",
               previous, current, petariNormalWorkers, PetariPipeline::gameplayStageCap(), PetariPipeline::gameplayBacklogCap());
}


extern "C" void petari_gx_pipeline_preparation_status(uint32_t* total, uint32_t* pending, uint32_t* failed) {
  std::lock_guard lock{g_pipelineMutex};
  *pending = static_cast<uint32_t>(g_pendingPipelines.size());
  *total = static_cast<uint32_t>(g_pipelines.size()) + *pending;
  *failed = petariFailedPipelines;
}

extern "C" bool petari_gx_pipeline_full_preparation() {
  return PetariPipeline::globalPrecompile();
}

// Complete scenes (set from the frame seam each frame): outside active play (intros,
// menus, loading, pause) a GX draw waits for its pipeline instead of being skipped,
// so a frame never shows part of the scene; the frame (the fade) holds instead, as
// the original never draws a partial scene. Active play keeps the bounded wait.
// PETARI_PIPELINE_COMPLETE_SCENES=0 turns it off.
static bool petari_complete_scenes_enabled() {
  static const bool enabled = [] {
    const char* value = std::getenv("PETARI_PIPELINE_COMPLETE_SCENES");
    return !(value && std::strcmp(value, "0") == 0);
  }();
  return enabled;
}
// On from the first frame (boot, logo) until the frame seam first reports play.
static std::atomic<bool> petariCompleteScenes{petari_complete_scenes_enabled()};
extern "C" void petari_gx_pipeline_set_complete_scenes(bool complete) {
  petariCompleteScenes.store(petari_complete_scenes_enabled() && complete, std::memory_order_relaxed);
  if (!complete) {  // Mario is in play: the global prewarm may start (once unlocked it stays unlocked)
    bool unlocked = false;
    {
      std::lock_guard lock{g_pipelineMutex};
      unlocked = !petariPrewarmUnlocked;
      petariPrewarmUnlocked = true;
    }
    if (unlocked) {
      g_pipelineQueueCv.notify_all();
      std::fprintf(stderr, "[gx pipeline pool] prewarm unlocked (play reached) prewarm_workers=%u gameplay_backlog_cap=%u\n",
                   petariBackgroundCap, PetariPipeline::gameplayBacklogCap());
    }
  }
}

// Called from the frame seam with each finished frame's interval: a gameplay frame over budget pauses the
// global prewarm for kPetariBacklogHoldFrames frames.
extern "C" void petari_gx_pipeline_note_frame(double intervalMs) {
  bool resumed = false;
  {
    std::lock_guard lock{g_pipelineMutex};
    if (petariGameplay && intervalMs > kPetariFrameBudgetMs) {
      petariBacklogHoldFrames = kPetariBacklogHoldFrames;
    } else if (petariBacklogHoldFrames > 0) {
      resumed = --petariBacklogHoldFrames == 0;
    }
  }
  if (resumed) g_pipelineQueueCv.notify_all();
}

extern "C" void petari_gx_pipeline_set_gameplay(bool gameplay) {
  {
    std::lock_guard lock{g_pipelineMutex};
    if (petariGameplay == gameplay) return;
    petariGameplay = gameplay;
  }
  // Leaving gameplay lifts the speculative throttle for parked workers.
  if (!gameplay) g_pipelineQueueCv.notify_all();
}

static void petari_note_variant(PipelineRef runtimeKey, HashType configKey, ShaderType type,
                                PipelinePriority priority) {
  std::lock_guard lock{g_pipelineMutex};
  auto& sample = petariPipelineSamples[runtimeKey];
  sample.drawBlocking |= priority == PipelinePriority::Blocking;
  sample.type = type;
  sample.configKey = configKey;
  if (priority != PipelinePriority::Background && sample.requested == PetariPipeline::Clock::time_point{}) {
    sample.requested = PetariPipeline::Clock::now();
  }
}

struct PetariPipelineWait {
  bool active;
  PipelineRef key;
  PetariPipeline::Clock::time_point start;
  PetariPipelineWait(bool blocking, PipelineRef runtimeKey) : active(blocking), key(runtimeKey) {
    if (active) {
      start = PetariPipeline::Clock::now();
      petariPipelineWaiters.fetch_add(1, std::memory_order_relaxed);
    }
  }
  ~PetariPipelineWait() {
    if (!active) return;
    petariPipelineWaiters.fetch_sub(1, std::memory_order_relaxed);
    const auto waited = PetariPipeline::Clock::now() - start;
    // Frame telemetry: real waits only; a ready pipeline returns in microseconds.
    const auto waitedNs = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(waited).count());
    if (waitedNs >= 50000) {
      PetariNative::FrameTelemetry::add(PetariNative::FrameTelemetry::PipelineWait, waitedNs);
    }
    const auto ms = PetariPipeline::milliseconds(waited);
    if (PetariPipeline::diagnostics() && ms >= 10.0) {
      std::fprintf(stderr, "[gx pipeline] blocking resolve %.2f ms key=%016llx\n", ms,
                   static_cast<unsigned long long>(key));
    }
  }
};

// Next queue a worker may take from, or null (guarded by g_pipelineMutex).
// Draw-requested jobs sit at the front of the requested queue and are never
// throttled; stage work precedes the global backlog. During gameplay, burst
// workers (index >= petariNormalWorkers) take only draw-requested jobs.
static std::deque<PendingPipeline>* petari_runnable_queue(unsigned workerIndex) {
  const bool burst = workerIndex >= petariNormalWorkers;
  const bool onDemandOnly = petariNormalWorkers > kPetariOnDemandWorkers && workerIndex < kPetariOnDemandWorkers;
  if (!g_pipelineQueue.empty()) {
    const auto sample = petariPipelineSamples.find(g_pipelineQueue.front().hash);
    const bool drawn = sample != petariPipelineSamples.end() && sample->second.drawBlocking;
    if (drawn) return &g_pipelineQueue;
    if (onDemandOnly && (petariGameplay || !PetariPipeline::stageOnReservedWorkers())) return nullptr;
    if (!petariGameplay || (!burst && petariStageSpeculativeInFlight < PetariPipeline::gameplayStageCap()))
      return &g_pipelineQueue;
    return nullptr;
  }
  if (onDemandOnly || (burst && petariGameplay)) return nullptr;
  if (!(petariPrewarmUnlocked || PetariPipeline::prewarmBeforePlay()) || (petariGameplay && petariBacklogHoldFrames > 0))
    return nullptr;
  const unsigned backlogCap = petariGameplay ? PetariPipeline::gameplayBacklogCap() : petariBackgroundCap;
  if (!g_backgroundPipelineQueue.empty() && petariBackgroundInFlight < backlogCap) return &g_backgroundPipelineQueue;
  return nullptr;
}

template <typename Ready>
static void petari_bounded_draw_wait(std::unique_lock<std::mutex>& lock, Ready ready) {
  if (petariDrawBudgetLeft > PetariPipeline::Clock::duration::zero()) {
    const auto start = PetariPipeline::Clock::now();
    g_pipelineReadyCv.wait_until(lock, start + petariDrawBudgetLeft, ready);
    petariDrawBudgetLeft -= std::min(petariDrawBudgetLeft, PetariPipeline::Clock::now() - start);
  }
  if (!ready()) ++petariDeferredDraws;
}

// Complete scenes in parallel (PetariPipeline::parallelCompleteScenes): each draw used to wait for its pipeline in
// turn on the recording thread, so the next draw could not even request its own and a frame with many new pipelines
// compiled them one by one (Starship first arrival, user playtest 2026-10-23: 49 and 43 pipelines, 4.3 s and 5.8 s).
// Now such a draw is queued at the front (every worker may take it) and recorded; end_pipeline_frame waits once for
// all of them before the frame is encoded, so the frame still never shows part of the scene.
static std::vector<PipelineRef> petariScenePending;  // guarded by g_pipelineMutex

static void petari_defer_scene_draw(PipelineRef key) {  // caller holds g_pipelineMutex
  petariScenePending.push_back(key);
}

static void petari_wait_scene_pipelines() {
  std::unique_lock lock{g_pipelineMutex};
  if (petariScenePending.empty()) return;
  std::vector<PipelineRef> keys;
  keys.swap(petariScenePending);
  const auto ready = [&] {
    return g_pipelineThreadEnd ||
        std::all_of(keys.begin(), keys.end(), [](auto key) { return g_pipelines.contains(key); });
  };
  if (ready()) return;
  petariPipelineWaiters.fetch_add(1, std::memory_order_relaxed);
  const auto start = PetariPipeline::Clock::now();
  const bool done = g_pipelineReadyCv.wait_until(lock, start + PetariPipeline::completeSceneWaitBudget(), ready);
  const auto waited = PetariPipeline::Clock::now() - start;
  petariPipelineWaiters.fetch_sub(1, std::memory_order_relaxed);
  PetariNative::FrameTelemetry::add(PetariNative::FrameTelemetry::PipelineWait,
      static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(waited).count()));
  if (!done) {
    size_t pending = 0;
    for (const auto key : keys) pending += !g_pipelines.contains(key);
    std::fprintf(stderr, "[gx pipeline] complete-scene wait hit its %.0f ms bound: %zu of %zu draws' pipelines pending (skipped)\n",
                 PetariPipeline::milliseconds(PetariPipeline::completeSceneWaitBudget()), pending, keys.size());
  }
}

static void petari_pipeline_frame_budget() {
  std::lock_guard lock{g_pipelineMutex};
  petariDrawBudgetLeft = PetariPipeline::drawWaitBudget();
}

static void petari_pipeline_summary() {
  uint64_t missedConfigs = 0, skipped = 0, unfinished = 0, failed = 0;
  double maxPendingMs = 0;
  const auto now = PetariPipeline::Clock::now();
  for (const auto& [key, sample] : petariPipelineSamples) {
    failed += sample.failed;
    if (!sample.skipped) continue;
    ++missedConfigs;
    skipped += sample.skipped;
    const bool pending = sample.finished == PetariPipeline::Clock::time_point{};
    unfinished += pending;
    const double pendingMs = PetariPipeline::milliseconds((pending || sample.failed ? now : sample.finished) - sample.firstSkip);
    maxPendingMs = std::max(maxPendingMs, pendingMs);
    std::fprintf(stderr,
        "[gx pipeline miss] key=%016llx config=%016llx skipped=%llu first_skip_to_terminal_or_exit_ms=%.3f "
        "visible_skip_span_ms=%.3f state=%s\n",
        static_cast<unsigned long long>(key), static_cast<unsigned long long>(sample.configKey),
        static_cast<unsigned long long>(sample.skipped), pendingMs,
        PetariPipeline::milliseconds(sample.lastSkip - sample.firstSkip),
        pending ? "pending" : sample.failed ? "failed" : "ready");
  }
  std::fprintf(stderr,
      "[gx pipeline summary] policy=%s skipped_draws=%llu deferred_draws=%llu missed_configs=%llu unfinished=%llu "
      "failed_compiles=%llu pending_compiles=%zu terminal_compiles=%zu max_first_skip_to_terminal_or_exit_ms=%.3f\n",
      PetariPipeline::asynchronous() ? "async" : PetariPipeline::unboundedBlocking() ? "blocking" : "bounded",
      static_cast<unsigned long long>(skipped), static_cast<unsigned long long>(petariDeferredDraws),
      static_cast<unsigned long long>(missedConfigs), static_cast<unsigned long long>(unfinished),
      static_cast<unsigned long long>(failed), g_pendingPipelines.size(), g_pipelines.size(), maxPendingMs);
}
