// Exercise the production worker body using controlled CPU-only compile jobs.
#include <petari/gpu_trace.hpp>
#include "../gx/pipeline_profile.hpp"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace absl {
template<class K, class V> using flat_hash_map = std::unordered_map<K, V>;
}
namespace aurora::gx {
enum class PetariUberTier { Generic, Fast };
struct PipelineConfig;
}
namespace aurora::gfx {
struct RenderTargetLayout;
using PipelineRef = uint64_t;
using HashType = uint64_t;
enum class ShaderType { Clear, GX, Rml };
enum class PipelinePriority { Background, Normal, Blocking };
struct CompiledPipeline {
    bool main = false;
    uint32_t pipeline_count() const { return main ? 1 : 0; }
};
struct PendingPipeline {
    PipelineRef hash;
    std::function<CompiledPipeline()> create;
};
static std::mutex g_pipelineMutex;
static bool g_hasPipelineThread = true;
static unsigned g_pipelinesPerFrame = 0;
static constexpr unsigned BuildPipelinesPerFrame = 5;
static std::atomic<bool> g_pipelineThreadEnd{false};
static std::condition_variable g_pipelineQueueCv, g_pipelineReadyCv;
static std::deque<PendingPipeline> g_pipelineQueue, g_backgroundPipelineQueue;
static std::unordered_map<PipelineRef, CompiledPipeline> g_pipelines;
static std::unordered_set<PipelineRef> g_pendingPipelines;
static std::atomic<unsigned> queuedPipelines{0};
static void notify_pipeline_ready(bool queued, uint32_t) {
    if (queued) --queuedPipelines;
    g_pipelineReadyCv.notify_all();
}
#include "../gx/pipeline_runtime.inc"
// This fixture exercises worker/queue policy without the boot metadata owner.
// Production metadata and the comparator are covered by order/async fixtures.
static uint32_t petari_uber_job_frame(const PetariUberJob&) { return UINT32_MAX; }
static size_t petari_uber_job_rank(const PetariUberJob&) { return SIZE_MAX; }
#include "../gx/pipeline_worker.inc"

void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
void enqueue(uint64_t id, bool background, std::function<CompiledPipeline()> fn) {
    std::lock_guard lock(g_pipelineMutex);
    petariPipelineSamples[id].queued = PetariPipeline::Clock::now();
    (background ? g_backgroundPipelineQueue : g_pipelineQueue).push_back({id, std::move(fn)});
    g_pendingPipelines.insert(id);
    ++queuedPipelines;
    g_pipelineQueueCv.notify_one();
}
void waitReady(size_t count) {
    std::unique_lock lock(g_pipelineMutex);
    require(g_pipelineReadyCv.wait_for(lock, std::chrono::seconds(5), [=] { return g_pipelines.size() == count; }),
            "workers did not finish");
}
void stop(std::vector<std::thread>& workers) {
    g_pipelineThreadEnd = true;
    g_pipelineQueueCv.notify_all();
    for (auto& worker : workers) worker.join();
    require(queuedPipelines == 0 && g_pendingPipelines.empty(), "completion accounting is not balanced");
    g_pipelineThreadEnd = false;
    g_pipelines.clear();
    petariPipelineSamples.clear();
    petariFailedPipelines = 0;
}
void frontendOnlyTest() {
    std::lock_guard lock{g_pipelineMutex};
    g_pipelineQueue.push_back({901, [] { return CompiledPipeline{true}; }});
    petariPipelineSamples[901].drawBlocking = true;
    for (const bool armed : {true, false}) {
        petariUberFrontendArmed = armed;
        petariUberPrePresent = !armed;
        for (unsigned worker = 0; worker < 4; ++worker)
            require(petari_runnable_queue(worker) == &g_pipelineQueue,
                    "normal splash blocked a draw-requested utility pipeline");
        petariPipelineSamples[901].drawBlocking = false;
        for (unsigned worker = 0; worker < 4; ++worker)
            require(petari_runnable_queue(worker) == nullptr,
                    "normal splash allowed a speculative specialised job");
        petariPipelineSamples[901].drawBlocking = true;
    }
    petariUberPrePresent = petariUberFrontendArmed = false;
    require(petari_runnable_queue(0) == &g_pipelineQueue, "normal splash failed to restore draw scheduling");
    g_pipelineQueue.clear(); petariPipelineSamples.clear();
}
void priorityTest() {
    std::vector<int> order;
    std::promise<void> running, release;
    auto gate = release.get_future().share();
    enqueue(1, true, [&] { running.set_value(); gate.wait(); order.push_back(1); return CompiledPipeline{true}; });
    std::vector<std::thread> workers;
    workers.emplace_back(pipeline_worker, static_cast<unsigned>(workers.size()));
    require(running.get_future().wait_for(std::chrono::seconds(5)) == std::future_status::ready, "worker did not start");
    enqueue(2, true, [&] { order.push_back(2); return CompiledPipeline{true}; });
    enqueue(3, false, [&] { order.push_back(3); return CompiledPipeline{true}; });
    release.set_value();
    waitReady(3);
    stop(workers);
    require(order == std::vector<int>({1, 3, 2}), "requested pipeline must precede queued background work");
}
void parallelTest() {
    petariActiveWorkers = 2;
    std::promise<void> startedA, startedB, release;
    auto gate = release.get_future().share();
    enqueue(11, false, [&] { PetariPipeline::stages.sourceMs = 11; startedA.set_value(); gate.wait(); return CompiledPipeline{true}; });
    enqueue(22, false, [&] { PetariPipeline::stages.sourceMs = 22; startedB.set_value(); gate.wait(); return CompiledPipeline{true}; });
    std::vector<std::thread> workers;
    workers.emplace_back(pipeline_worker, static_cast<unsigned>(workers.size()));
    workers.emplace_back(pipeline_worker, static_cast<unsigned>(workers.size()));
    const auto readyA = startedA.get_future().wait_for(std::chrono::seconds(5));
    const auto readyB = startedB.get_future().wait_for(std::chrono::seconds(5));
    release.set_value();
    require(readyA == std::future_status::ready && readyB == std::future_status::ready,
            "two compile jobs must be able to enter before either completes");
    waitReady(2);
    {
        std::lock_guard lock(g_pipelineMutex);
        require(petariPipelineSamples.at(11).stages.sourceMs == 11 && petariPipelineSamples.at(22).stages.sourceMs == 22,
                "compile stage accumulators leaked across worker threads");
        require(petariPipelineSamples.at(11).stages.runtimeKey == 11 && petariPipelineSamples.at(22).stages.runtimeKey == 22,
                "compile context belongs to wrong pipeline");
    }
    stop(workers);
}
void retirementTest() {
    // After full preparation the pool shrinks to the burst pool; workers beyond it retire.
    const auto normal = PetariPipeline::burstWorkerCount();
    petariActiveWorkers = normal + 1;
    std::promise<void> started, release;
    auto gate = release.get_future().share();
    enqueue(41, true, [&] { started.set_value(); gate.wait(); return CompiledPipeline{true}; });
    std::thread extra(pipeline_worker, normal);
    const auto entered = started.get_future().wait_for(std::chrono::seconds(5));
    enqueue(42, true, [] { return CompiledPipeline{true}; });
    petari_gx_pipeline_startup_finished();
    release.set_value();
    extra.join();
    require(entered == std::future_status::ready, "extra startup worker did not enter");
    require(g_pipelines.size() == 1 && g_pendingPipelines.count(42),
            "extra worker took another compile after startup ended");
    std::vector<std::thread> workers;
    workers.emplace_back(pipeline_worker, 0);
    waitReady(2);
    stop(workers);
}
void backgroundCapTest() {
    petariActiveWorkers = 2;
    petariBackgroundCap = 1;
    std::promise<void> startedA, release;
    std::atomic<bool> startedB{false}, startedC{false};
    auto gate = release.get_future().share();
    enqueue(51, true, [&] { startedA.set_value(); gate.wait(); return CompiledPipeline{true}; });
    enqueue(52, true, [&] { startedB = true; return CompiledPipeline{true}; });
    std::vector<std::thread> workers;
    workers.emplace_back(pipeline_worker, 0u);
    workers.emplace_back(pipeline_worker, 1u);
    require(startedA.get_future().wait_for(std::chrono::seconds(5)) == std::future_status::ready, "backlog job did not start");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    require(!startedB, "second backlog job exceeded the gameplay concurrency cap");
    enqueue(53, false, [&] { startedC = true; return CompiledPipeline{true}; });
    {
        std::unique_lock lock(g_pipelineMutex);
        require(g_pipelineReadyCv.wait_for(lock, std::chrono::seconds(5), [] { return g_pipelines.count(53) != 0; }),
                "requested job waited behind the backlog cap");
    }
    require(!startedB, "cap released early");
    release.set_value();
    waitReady(3);
    require(startedB && petariBackgroundInFlight == 0, "backlog did not resume after its slot opened");
    stop(workers);
    petariBackgroundCap = ~0u;
}
void gameplayThrottleTest() {
    // During gameplay: two stage-speculative compiles at a time, the global
    // backlog waits while stage work is queued, a draw-requested job starts at
    // once, and leaving gameplay releases everything parked.
    petariActiveWorkers = 4;
    petariNormalWorkers = 4;  // all four are normal workers here (burst workers: burstWorkerTest)
    petari_gx_pipeline_set_gameplay(true);
    std::promise<void> startedA, startedB, release;
    std::atomic<bool> startedC{false}, startedG{false};
    auto gate = release.get_future().share();
    enqueue(71, false, [&] { startedA.set_value(); gate.wait(); return CompiledPipeline{true}; });
    enqueue(72, false, [&] { startedB.set_value(); gate.wait(); return CompiledPipeline{true}; });
    enqueue(75, false, [&] { startedC = true; return CompiledPipeline{true}; });
    enqueue(73, true, [&] { startedG = true; return CompiledPipeline{true}; });
    std::vector<std::thread> workers;
    for (unsigned i = 0; i < 4; ++i) workers.emplace_back(pipeline_worker, i);
    require(startedA.get_future().wait_for(std::chrono::seconds(5)) == std::future_status::ready &&
            startedB.get_future().wait_for(std::chrono::seconds(5)) == std::future_status::ready,
            "two stage jobs did not start during gameplay");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    require(!startedC, "gameplay exceeded the stage speculation cap");
    require(!startedG, "global backlog ran ahead of queued stage work");
    {
        std::lock_guard lock(g_pipelineMutex);
        petariPipelineSamples[74].drawBlocking = true;
        petariPipelineSamples[74].queued = PetariPipeline::Clock::now();
        g_pipelineQueue.push_front({74, [] { return CompiledPipeline{true}; }});
        g_pendingPipelines.insert(74);
        ++queuedPipelines;
    }
    g_pipelineQueueCv.notify_one();
    {
        std::unique_lock lock(g_pipelineMutex);
        require(g_pipelineReadyCv.wait_for(lock, std::chrono::seconds(5), [] { return g_pipelines.count(74) != 0; }),
                "draw-requested compile was throttled");
    }
    require(!startedC && !startedG, "throttle released early");
    petari_gx_pipeline_set_gameplay(false);
    {
        std::unique_lock lock(g_pipelineMutex);
        require(g_pipelineReadyCv.wait_for(lock, std::chrono::seconds(5), [] { return g_pipelines.count(75); }),
                "leaving gameplay did not release the queued stage job");
    }
    // Workers 2/3 still hold A/B. Reserved workers 0/1 may take stage work
    // outside play, but must not take the global backlog even then.
    require(!startedG, "global backlog consumed a reserved draw/stage worker");
    release.set_value();
    waitReady(5);
    require(petariStageSpeculativeInFlight == 0 && petariBackgroundInFlight == 0, "speculative accounting unbalanced");
    stop(workers);
}
void burstWorkerTest() {
    // Worker 1 is a burst worker: during gameplay it takes only draw-requested jobs,
    // never stage speculation or the backlog; outside gameplay it takes everything.
    petariActiveWorkers = 2;
    petariNormalWorkers = 1;
    petari_gx_pipeline_set_gameplay(true);
    std::promise<void> startedA, release;
    std::atomic<bool> startedB{false}, startedG{false};
    auto gate = release.get_future().share();
    enqueue(81, false, [&] { startedA.set_value(); gate.wait(); return CompiledPipeline{true}; });
    std::vector<std::thread> workers;
    workers.emplace_back(pipeline_worker, 0u);
    require(startedA.get_future().wait_for(std::chrono::seconds(5)) == std::future_status::ready, "normal worker idle");
    workers.emplace_back(pipeline_worker, 1u);
    enqueue(82, false, [&] { startedB = true; return CompiledPipeline{true}; });
    enqueue(83, true, [&] { startedG = true; return CompiledPipeline{true}; });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    require(!startedB && !startedG, "burst worker took speculative work during gameplay");
    {
        std::lock_guard lock(g_pipelineMutex);
        petariPipelineSamples[84].drawBlocking = true;
        petariPipelineSamples[84].queued = PetariPipeline::Clock::now();
        g_pipelineQueue.push_front({84, [] { return CompiledPipeline{true}; }});
        g_pendingPipelines.insert(84);
        ++queuedPipelines;
    }
    g_pipelineQueueCv.notify_all();
    {
        std::unique_lock lock(g_pipelineMutex);
        require(g_pipelineReadyCv.wait_for(lock, std::chrono::seconds(5), [] { return g_pipelines.count(84) != 0; }),
                "burst worker did not take a draw-requested job during gameplay");
    }
    require(!startedB && !startedG, "burst worker continued into speculative work");
    petari_gx_pipeline_set_gameplay(false);
    {
        std::unique_lock lock(g_pipelineMutex);
        require(g_pipelineReadyCv.wait_for(lock, std::chrono::seconds(5), [] { return g_pipelines.count(82) && g_pipelines.count(83); }),
                "burst worker idle outside gameplay");
    }
    release.set_value();
    waitReady(4);
    stop(workers);
    petariNormalWorkers = PetariPipeline::workerCount();
}
void boundedDrawTest() {
    // Nothing compiles: the first wait spends the whole frame budget (a lower bound
    // wait_until guarantees), after which the budget is empty and the next draw skips.
    // No assertion depends on how quickly another thread is scheduled.
    const auto ready = [] { return g_pipelines.count(61) != 0; };
    const auto before = petariDeferredDraws;
    petari_pipeline_frame_budget();
    std::unique_lock lock(g_pipelineMutex);
    const auto start = PetariPipeline::Clock::now();
    petari_bounded_draw_wait(lock, ready);
    const auto first = PetariPipeline::Clock::now() - start;
    require(petariDrawBudgetLeft == PetariPipeline::Clock::duration::zero(), "first wait did not spend the frame budget");
    petari_bounded_draw_wait(lock, ready);
    lock.unlock();
    require(first >= PetariPipeline::drawWaitBudget() - std::chrono::microseconds(200) &&
            first < PetariPipeline::drawWaitBudget() + std::chrono::seconds(1), "first draw wait ignored the budget");
    require(petariDeferredDraws == before + 2, "deferred draws not counted");
    // A compile that lands while a draw still has budget is not deferred. The budget
    // is made effectively unbounded so the finisher's scheduling delay cannot matter;
    // the finisher can only publish once the waiter has released the mutex in its wait.
    petari_pipeline_frame_budget();
    lock.lock();
    petariDrawBudgetLeft = std::chrono::hours(1);
    std::thread finisher([] {
        std::lock_guard guard(g_pipelineMutex);
        g_pipelines[61] = CompiledPipeline{true};
        g_pipelineReadyCv.notify_all();
    });
    petari_bounded_draw_wait(lock, ready);
    lock.unlock();
    finisher.join();
    require(petariDeferredDraws == before + 2, "ready pipeline counted as deferred");
    g_pipelines.clear();
    petari_pipeline_frame_budget();
}
void demandedFallbackTest(unsigned worker, ShaderType type, gx::PetariUberTier tier, bool gameplay = true) {
    // Hold both specialised jobs until the selected job is observed. A demanded
    // Generic must be able to run ahead of this queue on worker 1; worker 0,
    // clears/UI and Fast upgrades retain specialised-draw priority.
    petariActiveWorkers = 2;
    petariNormalWorkers = 2;
    petariUberTitleReleased = true;
    petariGameplay = gameplay;
    std::promise<void> firstSelected, release, fallbackFinished;
    auto releaseGate = release.get_future().share();
    std::atomic<bool> first{true};
    bool firstWasFallback = false;
    const auto selected = [&](bool fallback) {
        if (first.exchange(false)) { firstWasFallback = fallback; firstSelected.set_value(); }
    };
    for (const auto key : {101u, 102u}) {
        enqueue(key, false, [&] { selected(false); releaseGate.wait(); return CompiledPipeline{true}; });
    }
    {
        std::lock_guard lock(g_pipelineMutex);
        for (const auto key : {101u, 102u}) {
            petariPipelineSamples[key].drawBlocking = true;
            petariPipelineSamples[key].type = type;
        }
        petariUberJobs.push_back({201, 1, [&] { selected(true); fallbackFinished.set_value(); }, tier, false, true});
    }
    std::vector<std::thread> workers;
    workers.emplace_back(pipeline_worker, worker);
    const bool entered = firstSelected.get_future().wait_for(std::chrono::seconds(5)) == std::future_status::ready;
    release.set_value();
    waitReady(2);
    auto fallbackFuture = fallbackFinished.get_future();
    bool finished;
    if (worker == 0) {
        // The real pool has another fallback worker; this single-thread fixture
        // intentionally runs only the reserved index. It must never borrow it.
        std::lock_guard lock(g_pipelineMutex);
        finished = fallbackFuture.wait_for(std::chrono::seconds(0)) == std::future_status::timeout && petariUberJobs.size() == 1;
        petariUberJobs.clear(); // cancel this fixture's unassigned job before joining
    } else finished = fallbackFuture.wait_for(std::chrono::seconds(5)) == std::future_status::ready;
    stop(workers);
    require(entered && finished, "demanded fallback or specialised work failed to make progress");
    require(petariUberInFlight == 0 && petariUberJobs.empty(), "fallback completion accounting is unbalanced");
    require(firstWasFallback == (PetariPipeline::demandFallbackPriority() && gameplay && worker != 0 &&
                type == ShaderType::GX && tier == gx::PetariUberTier::Generic),
            "demanded Generic priority or specialised/clear reservation was lost");
    petariUberTitleReleased = false;
    petariGameplay = false;
    petariNormalWorkers = 1;
}
void progressTest() {
    enqueue(31, true, [] { return CompiledPipeline{false}; });
    uint32_t total, pending, failed;
    petari_gx_pipeline_preparation_status(&total, &pending, &failed);
    require(total == 1 && pending == 1 && failed == 0, "queued progress is inconsistent");
    std::vector<std::thread> workers;
    workers.emplace_back(pipeline_worker, static_cast<unsigned>(workers.size()));
    waitReady(1);
    petari_gx_pipeline_preparation_status(&total, &pending, &failed);
    require(total == 1 && pending == 0 && failed == 1, "failed completion was reported as a successful warmup");
    stop(workers);
}
}
int main(int argc, char** argv) {
    // The first fixtures deliberately use one unrestricted worker. Do not let
    // the host's topology silently turn it into a reserved demand-only worker.
    aurora::gfx::petariNormalWorkers = 1;
    const bool expectAsync = argc == 2 && std::strcmp(argv[1], "async") == 0;
    aurora::gfx::require(PetariPipeline::asynchronous() == expectAsync, "pipeline policy is not opt-in");
    if (!std::getenv("PETARI_PIPELINE_GLOBAL_PRECOMPILE"))
        aurora::gfx::require(!PetariPipeline::globalPrecompile() && PetariPipeline::backgroundGlobalPrecompile(),
                             "background preparation (no blocking screen) must be the default");
    setenv("PETARI_PIPELINE_GLOBAL_PRECOMPILE", "0", 1);
    aurora::gfx::require(!PetariPipeline::globalPrecompile() && !PetariPipeline::backgroundGlobalPrecompile(), "off override ignored");
    setenv("PETARI_PIPELINE_GLOBAL_PRECOMPILE", "background", 1);
    aurora::gfx::require(!PetariPipeline::globalPrecompile() && PetariPipeline::backgroundGlobalPrecompile(), "background override ignored");
    setenv("PETARI_PIPELINE_GLOBAL_PRECOMPILE", "1", 1);
    aurora::gfx::require(PetariPipeline::globalPrecompile() && !PetariPipeline::backgroundGlobalPrecompile(),
                         "explicit full preparation ignored");
    for (const auto cores : {0u, 1u, 2u, 4u, 6u, 8u, 9u})
        aurora::gfx::require(PetariPipeline::defaultStartupWorkerCount(cores) == 4,
                             "small-core startup default must remain four");
    for (const auto cores : {10u, 11u})
        aurora::gfx::require(PetariPipeline::defaultStartupWorkerCount(cores) == 5,
                             "startup default must use half the performance cores");
    for (const auto cores : {12u, 14u, 24u, 128u})
        aurora::gfx::require(PetariPipeline::defaultStartupWorkerCount(cores) == 6,
                             "startup default must cap at six");
    unsetenv("PETARI_PIPELINE_STARTUP_THREADS");
    const auto automatic = PetariPipeline::defaultStartupWorkerCount(PetariPipeline::performanceCores());
    aurora::gfx::require(PetariPipeline::startupWorkerCount() == automatic,
                         "startup default did not use performance core count");
    for (const auto value : {"", "0", "33", "invalid", "6junk"}) {
        setenv("PETARI_PIPELINE_STARTUP_THREADS", value, 1);
        aurora::gfx::require(PetariPipeline::startupWorkerCount() == automatic,
                             "invalid startup override did not fall back to automatic");
    }
    setenv("PETARI_PIPELINE_STARTUP_THREADS", "10", 1);
    aurora::gfx::require(PetariPipeline::startupWorkerCount() == 10, "startup tuning override ignored");
    setenv("PETARI_PIPELINE_GLOBAL_PRECOMPILE", "background", 1);
    aurora::gfx::require(PetariPipeline::startupWorkerCount() == PetariPipeline::workerCount(), "background used startup pool");
    unsetenv("PETARI_PIPELINE_GLOBAL_PRECOMPILE");
    aurora::gfx::require(PetariPipeline::startupWorkerCount() == PetariPipeline::workerCount(), "default used startup pool");
    unsetenv("PETARI_PIPELINE_STARTUP_THREADS");
    unsetenv("PETARI_PIPELINE_GLOBAL_PRECOMPILE");
    unsetenv("PETARI_PIPELINE_BURST_THREADS");
    const auto burst = PetariPipeline::burstWorkerCount();
    aurora::gfx::require(burst >= PetariPipeline::workerCount() && burst <= 8, "burst pool outside [normal, 8]");
    for (const auto value : {"", "0", "17", "invalid"}) {
        setenv("PETARI_PIPELINE_BURST_THREADS", value, 1);
        aurora::gfx::require(PetariPipeline::burstWorkerCount() == burst, "invalid burst override did not fall back");
    }
    setenv("PETARI_PIPELINE_BURST_THREADS", "12", 1);
    aurora::gfx::require(PetariPipeline::burstWorkerCount() == 12, "burst override ignored");
    unsetenv("PETARI_PIPELINE_BURST_THREADS");
    aurora::gfx::frontendOnlyTest();
    aurora::gfx::priorityTest();
    aurora::gfx::parallelTest();
    aurora::gfx::progressTest();
    aurora::gfx::retirementTest();
    aurora::gfx::backgroundCapTest();
    aurora::gfx::gameplayThrottleTest();
    aurora::gfx::petariNormalWorkers = PetariPipeline::workerCount();
    aurora::gfx::burstWorkerTest();
    aurora::gfx::boundedDrawTest();
    aurora::gfx::demandedFallbackTest(1, aurora::gfx::ShaderType::GX, aurora::gx::PetariUberTier::Generic);
    aurora::gfx::demandedFallbackTest(1, aurora::gfx::ShaderType::GX, aurora::gx::PetariUberTier::Generic, false);
    aurora::gfx::demandedFallbackTest(0, aurora::gfx::ShaderType::GX, aurora::gx::PetariUberTier::Generic);
    aurora::gfx::demandedFallbackTest(1, aurora::gfx::ShaderType::Clear, aurora::gx::PetariUberTier::Generic);
    aurora::gfx::demandedFallbackTest(1, aurora::gfx::ShaderType::GX, aurora::gx::PetariUberTier::Fast);
    std::puts("Pipeline worker: requested priority, concurrent progress, isolated timing state, backlog cap, gameplay throttle, burst workers, bounded draw waits and shutdown pass");
}
