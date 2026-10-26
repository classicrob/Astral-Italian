#include <petari/platform/audio_workgroup.hpp>
#include "workgroup_protocol.hpp"
#include "workgroup_backend.hpp"
#include "workgroup_history.hpp"
#include <AudioToolbox/AudioWorkInterval.h>
#include <mach/mach_time.h>
#include <mach/mach.h>
#include <pthread.h>
#include <petari/host_allocation.hpp>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

namespace PetariNative::Platform::AudioWorkgroup {
std::atomic<bool> enabled{false};
std::atomic<bool> observing{false};
namespace {
using Backend::Policy;
os_workgroup_interval_t createGroup() {
    return AudioWorkIntervalCreate("Petari guest DSP production", OS_CLOCK_MACH_ABSOLUTE_TIME, nullptr);
}
void releaseGroup(os_workgroup_interval_t group) { os_release(group); }
int promote(Policy& saved) {
    const auto self = pthread_mach_thread_np(pthread_self());
    mach_msg_type_number_t count = THREAD_EXTENDED_POLICY_COUNT;
    boolean_t defaults = false;
    auto result = thread_policy_get(self, THREAD_EXTENDED_POLICY,
        reinterpret_cast<thread_policy_t>(&saved.extended), &count, &defaults);
    if (result != KERN_SUCCESS) return result;
    count = THREAD_TIME_CONSTRAINT_POLICY_COUNT; defaults = false;
    result = thread_policy_get(self, THREAD_TIME_CONSTRAINT_POLICY,
        reinterpret_cast<thread_policy_t>(&saved.realtime), &count, &defaults);
    if (result != KERN_SUCCESS) return result;
    saved.wasRealtime = !defaults; saved.saved = true;
    mach_timebase_info_data_t tb{}; mach_timebase_info(&tb);
    const auto ticks = [&](std::uint64_t ns) { return static_cast<std::uint32_t>(ns * tb.denom / tb.numer); };
    // Same event-driven budget as existing Audio::setRealtimeAudioThread.
    thread_time_constraint_policy_data_t wanted{0, ticks(500000), ticks(2000000), TRUE};
    result = thread_policy_set(self, THREAD_TIME_CONSTRAINT_POLICY,
        reinterpret_cast<thread_policy_t>(&wanted), THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    if (result != KERN_SUCCESS) return result;
    saved.changed = true;
    thread_time_constraint_policy_data_t actual{};
    count = THREAD_TIME_CONSTRAINT_POLICY_COUNT; defaults = false;
    result = thread_policy_get(self, THREAD_TIME_CONSTRAINT_POLICY,
        reinterpret_cast<thread_policy_t>(&actual), &count, &defaults);
    if (result != KERN_SUCCESS) return result;
    return !defaults && actual.period == wanted.period && actual.computation == wanted.computation &&
        actual.constraint == wanted.constraint && actual.preemptible == wanted.preemptible ? 0 : EPROTO;
}
int restore(Policy& saved) {
    if (!saved.changed) return 0;
    const auto self = pthread_mach_thread_np(pthread_self());
    const int result = saved.wasRealtime ?
        thread_policy_set(self, THREAD_TIME_CONSTRAINT_POLICY,
            reinterpret_cast<thread_policy_t>(&saved.realtime), THREAD_TIME_CONSTRAINT_POLICY_COUNT) :
        thread_policy_set(self, THREAD_EXTENDED_POLICY,
            reinterpret_cast<thread_policy_t>(&saved.extended), THREAD_EXTENDED_POLICY_COUNT);
    saved.changed = false;
    return result;
}
int startGroup(os_workgroup_interval_t group, std::uint64_t start, std::uint64_t end) {
    return os_workgroup_interval_start(group, start, end, nullptr);
}
int finishGroup(os_workgroup_interval_t group) { return os_workgroup_interval_finish(group, nullptr); }
Backend::Api api{createGroup, releaseGroup, promote, restore, os_workgroup_join, os_workgroup_leave, startGroup, finishGroup};

struct Session {
    std::uint64_t epoch = 0;
    Identity guest{};
    std::uintptr_t callback = 0;
    unsigned rate = 0;
    os_workgroup_interval_t group = nullptr;
    std::atomic<unsigned> members{0};
    std::atomic<bool> primaryActive{false};
    std::atomic<std::uint64_t> completions{0};
    ~Session() { if (group) api.release(group); }  // retained owner is control thread
};
struct Registry {
    std::mutex control;
    std::shared_ptr<Session> active;
    std::vector<std::shared_ptr<Session>> retired;
};
Registry& registry() {
    // No eager allocation when disabled, and no static destructor racing a
    // surviving guest pthread at process exit. collect() releases quiescent
    // sessions explicitly on the reporter/control thread.
    static auto* value = [] { PetariNative::HostAllocationScope host; return new Registry; }();
    return *value;
}
Identity callbackCaller{}, startCaller{};
std::uintptr_t callbackIdentity = 0, startCallback = 0;
std::uint64_t nextEpoch = 1;
std::atomic<std::uint64_t> activeEpoch{0}, guestPointer{0}, guestGeneration{0}, guestJoined{0};
std::atomic<std::uint64_t> controlEpoch{0};
std::atomic<bool> paused{false};
std::atomic<void (*)()> wakeGuest{nullptr}, wakeRenderer{nullptr};
std::atomic<std::uint64_t> errors{0}, cycles{0}, finished{0}, cancelled{0}, late{0}, samples{0},
    guestJoins{0}, rendererJoins{0}, leaves{0}, sessions{0}, completedSessions{0}, worstLate{0}, lastGeneration{0}, registrations{0},
    guestHost{0}, rendererHost{0},
    batonAssigned{0}, batonWorst{0}, batonOver20{0}, batonOver175{0};
std::atomic<int> lastError{0};
History::Recorder<16384,4096> history;
std::atomic<std::uint64_t> callbackLast{0}, callbackWorst{0}, callbackOver20{0}, historyInvalid{0};
std::uint64_t observerStartedMach = 0;
std::once_flag historyPrepared;
void event(History::Kind kind, std::uint64_t value, bool trigger=false) {
    if (!observing.load(std::memory_order_relaxed)) return;
    const auto now=mach_absolute_time();
    history.push(now,static_cast<unsigned>(kind),reinterpret_cast<std::uintptr_t>(pthread_self()),value);
    if(trigger) history.trigger(now);
}
std::atomic<const char*> lastSite{"none"};
bool modeValid = true;
std::uint64_t ticksPerSecond() {
    static const auto value = [] { mach_timebase_info_data_t t{}; mach_timebase_info(&t);
        return std::uint64_t(1000000000) * t.denom / t.numer; }();
    return value;
}
std::uint64_t toUs(std::uint64_t ticks) { return static_cast<std::uint64_t>(__uint128_t(ticks) * 1000000 / ticksPerSecond()); }
int apiError(int result) { return result == -1 ? errno : result; }
void maxValue(std::atomic<std::uint64_t>& dst, std::uint64_t value) {
    auto previous = dst.load(std::memory_order_relaxed);
    while (value > previous && !dst.compare_exchange_weak(previous, value, std::memory_order_relaxed)) {}
}
void fail(const char* site, int error = EPROTO) {
    lastSite.store(site, std::memory_order_relaxed); lastError.store(error, std::memory_order_relaxed);
    errors.fetch_add(1, std::memory_order_relaxed);
}
void wake() {
    if (auto f = wakeGuest.load(std::memory_order_acquire)) f();
    if (auto f = wakeRenderer.load(std::memory_order_acquire)) f();
}

struct Member {
    std::shared_ptr<Session> session;
    Protocol::Membership ownership;
    os_workgroup_join_token_s token{};
    Policy policy{};
    Protocol::Cycle cycle{};
    bool intervalOpen = false;
    std::uint64_t observed = UINT64_MAX;
    bool guest = false;
    ~Member() { leave(); }
    void cancelCycle() {
        if (intervalOpen) {
            const int e = api.finish(session->group);
            if (e) fail("cancel-finish", apiError(e));
            else { intervalOpen = false; session->primaryActive.store(false, std::memory_order_release); }
        }
        if (cycle.active) {
            const auto now = mach_absolute_time();
            if (now > cycle.deadline) { ++late; maxValue(worstLate, now-cycle.deadline); }
            cycle.cancel();
            cancelled.fetch_add(1, std::memory_order_relaxed);
        }
    }
    void leave() {
        cancelCycle();
        if (ownership.joined) {
            auto joinedEpoch = ownership.epoch;
            if (!ownership.leave(reinterpret_cast<std::uintptr_t>(pthread_self()))) {
                fail("wrong-thread-leave"); return;
            }
            api.leave(session->group, &token);
            if (guest) guestJoined.compare_exchange_strong(joinedEpoch, 0, std::memory_order_acq_rel);
            session->members.fetch_sub(1, std::memory_order_acq_rel);
            leaves.fetch_add(1, std::memory_order_relaxed);
        }
        if (const int e = api.restore(policy)) fail("restore-policy", e);
        session.reset(); policy = {}; token = {}; intervalOpen = false;
    }
    void sync(bool isGuest, Identity identity = {}) {
        const auto current = activeEpoch.load(std::memory_order_acquire);
        if (observed == current) {
            if (session && guest != isGuest) fail("member-role-mismatch");
            return;
        }
        leave(); observed = current; guest = isGuest;
        if (!current) return;
        {
            std::lock_guard<std::mutex> lock(registry().control);
            if (!registry().active || registry().active->epoch != current) return; // next safe point retries newer epoch
            session = registry().active;
        }
        if (isGuest && !(session->guest == identity)) { session.reset(); return; }
        int e = api.promote(policy);
        if (e) { fail(isGuest ? "guest-policy" : "renderer-policy", e); leave(); return; }
        e = api.join(session->group, &token);
        if (e) { fail(isGuest ? "guest-join" : "renderer-join", e); leave(); return; }
        if (!ownership.adopt(reinterpret_cast<std::uintptr_t>(pthread_self()), current, 0, 0)) {
            api.leave(session->group, &token); fail("ownership-adopt"); leave(); return;
        }
        session->members.fetch_add(1, std::memory_order_acq_rel);
        const auto physical = reinterpret_cast<std::uintptr_t>(pthread_self());
        if (isGuest) { guestHost.store(physical); guestJoined.store(current, std::memory_order_release); ++guestJoins; }
        else { rendererHost.store(physical); ++rendererJoins; }
    }
};
// Helpers using this TLS are out of line. No reference is touched by off-path wrappers.
thread_local Member member;
}

void configure() {
    static const bool requested = [] { const char* s = std::getenv("PETARI_AUDIO_WORKGROUP");
        if (s && *s && std::strcmp(s, "off") && std::strcmp(s, "async")) {
            modeValid = false; fail("unknown-mode", EINVAL); return true; // visible refused arm
        }
        return s && !std::strcmp(s, "async"); }();
    enabled.store(requested, std::memory_order_release);
    const char* diag=std::getenv("PETARI_AUDIO_WORKGROUP_DIAG");
    if (requested && diag && !std::strcmp(diag,"1"))
        std::call_once(historyPrepared,[] { history.prepare(); observerStartedMach=mach_absolute_time(); });
    observing.store(requested && diag && !std::strcmp(diag,"1"),std::memory_order_release);
}
void setGuestWake(void (*f)()) { wakeGuest.store(f, std::memory_order_release); }
void setRendererWake(void (*f)()) { wakeRenderer.store(f, std::memory_order_release); }
std::uint64_t epoch() { return activeEpoch.load(std::memory_order_acquire); }
std::uint64_t controlVersion() { return controlEpoch.load(std::memory_order_acquire); }
void pauseProduction(bool value) {
    paused.store(value,std::memory_order_release);
    controlEpoch.fetch_add(1,std::memory_order_acq_rel);
    if (auto f=wakeRenderer.load(std::memory_order_acquire)) f();
}
void bindCallback(Identity identity, std::uintptr_t callback, bool safe) {
    if (!enabled.load(std::memory_order_relaxed)) return;
    if (!safe || !identity || !callback) { fail("callback-binding"); return; }
    std::lock_guard<std::mutex> lock(registry().control);
    if (registry().active) { fail("callback-rebound-live"); return; }
    callbackCaller = identity; callbackIdentity = callback; startCaller = {}; startCallback = 0;
}
void confirmStart(Identity identity, std::uintptr_t callback, bool safe) {
    if (!enabled.load(std::memory_order_relaxed)) return;
    if (!safe) { fail("start-under-interrupt-lock"); return; }
    std::lock_guard<std::mutex> lock(registry().control);
    if (!identity || !(identity == callbackCaller) || callback != callbackIdentity) { fail("start-binding"); return; }
    startCaller = identity; startCallback = callback;
}
void startSink(Identity identity, bool coreAudio, unsigned sourceRate) {
    if (!enabled.load(std::memory_order_relaxed)) return;
    {
        std::lock_guard<std::mutex> lock(registry().control);
        if (!modeValid || registry().active || !coreAudio || (sourceRate!=32000 && sourceRate!=48000) || !identity || !(identity == startCaller) ||
            !(identity == callbackCaller) || !startCallback || startCallback != callbackIdentity) {
            fail("real-sink-binding"); return;
        }
        auto session = std::make_shared<Session>();
        session->epoch = nextEpoch++; session->guest = identity; session->callback = callbackIdentity;
        session->rate = sourceRate;
        session->group = api.create();
        if (!session->group) { fail("create", errno); return; }
        registry().active = session;
        ++sessions;
        guestPointer.store(identity.thread, std::memory_order_relaxed);
        guestGeneration.store(identity.generation, std::memory_order_relaxed);
        lastGeneration.store(identity.generation, std::memory_order_relaxed);
        callbackLast.store(0,std::memory_order_relaxed);
        batonAssigned.store(0,std::memory_order_relaxed);
        activeEpoch.store(session->epoch, std::memory_order_release);
        controlEpoch.fetch_add(1,std::memory_order_acq_rel);
    }
    // startSink runs on the bound guest inside the sink's host-only startup.
    guestSafePoint(identity); wake();
}
void stopSink() {
    if (!enabled.load(std::memory_order_relaxed)) return;
    {
        std::lock_guard<std::mutex> lock(registry().control);
        activeEpoch.store(0, std::memory_order_release);
        controlEpoch.fetch_add(1,std::memory_order_acq_rel);
        if (registry().active) { registry().retired.push_back(registry().active); registry().active.reset(); }
    }
    wake();
}
__attribute__((noinline)) void guestSafePoint(Identity id) { member.sync(true, id); }
__attribute__((noinline)) void guestExit(Identity id) {
    if (member.session && member.guest && member.session->guest == id) {
        if (epoch()==member.session->epoch && !paused.load(std::memory_order_acquire)) fail("guest-exit-live");
        member.leave();
    }
}
__attribute__((noinline)) void rendererSafePoint() {
    member.sync(false);
    if (paused.load(std::memory_order_acquire)) member.cancelCycle();
}
__attribute__((noinline)) void rendererExit() { member.leave(); }
Origin origin(Identity id) {
    const auto current = activeEpoch.load(std::memory_order_acquire);
    if (!current || current != guestJoined.load(std::memory_order_acquire) ||
        id.thread != guestPointer.load(std::memory_order_relaxed) ||
        id.generation != guestGeneration.load(std::memory_order_relaxed)) return {};
    return {id, current, mach_absolute_time()};
}
void checkArgument(Origin command, Origin argument) {
    if (!command.epoch && !epoch()) return; // pre-sink boot, no real-device interval
    if (command.epoch != argument.epoch || !(command.caller == argument.caller) ||
        !argument.queuedMach || argument.queuedMach < command.queuedMach) fail("command-argument-origin");
}
__attribute__((noinline)) void beginCycle(Origin source, unsigned subframes, unsigned rate,
                                         std::uintptr_t left, std::uintptr_t right) {
    if (!epoch() || paused.load(std::memory_order_acquire)) return;
    member.sync(false);
    auto& m = member;
    if (!m.session || !m.ownership.joined || m.guest) { fail("cycle-without-renderer"); return; }
    if (m.session->rate != rate) { fail("source-rate-changed-without-reopen"); return; }
    if (source.epoch != m.session->epoch || !(source.caller == m.session->guest) ||
        guestJoined.load(std::memory_order_acquire) != source.epoch) { fail("cycle-origin"); return; }
    const auto serial = cycles.fetch_add(1, std::memory_order_relaxed) + 1;
    if (!m.cycle.begin(source, serial, subframes, rate, ticksPerSecond(), left, right)) {
        fail("cycle-ledger"); return;
    }
    if (const int e = api.start(m.session->group, source.queuedMach, m.cycle.deadline)) {
        fail("interval-start", apiError(e)); m.cycle.cancel(); return;
    }
    m.session->primaryActive.store(true, std::memory_order_release);
    m.intervalOpen = true;
    event(History::Kind::Cycle, serial);
    event(History::Kind::Shape, (std::uint64_t(rate)<<32)|subframes);
    event(History::Kind::Left, left); event(History::Kind::Right, right);
    event(History::Kind::Queued, source.queuedMach); event(History::Kind::Deadline, m.cycle.deadline);
}
__attribute__((noinline)) void subframeComplete() {
    auto& m = member;
    if (!m.cycle.active) return;
    if (!m.cycle.completeSubframe()) { fail("subframe-ledger"); return; }
    event(History::Kind::Subframe, m.cycle.completed);
    if (!m.cycle.ready()) return;
    const auto now = mach_absolute_time();
    if (now > m.cycle.deadline) { ++late; maxValue(worstLate, now - m.cycle.deadline); }
    const int finishError = api.finish(m.session->group);
    if (finishError) fail("interval-finish", apiError(finishError));
    else { m.intervalOpen = false; m.session->primaryActive.store(false, std::memory_order_release); }
    samples.fetch_add(std::uint64_t(m.cycle.subframes) * 80, std::memory_order_relaxed);
    m.cycle.finish();
    if (finishError) return; // cleanup may attempt one finish; never count this as completion
    ++finished;
    event(History::Kind::Finish, m.cycle.serial);
    if (m.session->completions.fetch_add(1, std::memory_order_relaxed) == 0) ++completedSessions;
}
void collect() {
    std::lock_guard<std::mutex> lock(registry().control);
    registry().retired.erase(std::remove_if(registry().retired.begin(), registry().retired.end(), [](const auto& s) {
        return s.use_count() == 1 && s->members.load(std::memory_order_acquire) == 0 &&
            !s->primaryActive.load(std::memory_order_acquire); }), registry().retired.end());
}
bool isCurrentGuest(Identity id) {
    return epoch() && id.thread == guestPointer.load(std::memory_order_relaxed) &&
        id.generation == guestGeneration.load(std::memory_order_relaxed);
}
void noteAssigned(Identity id, std::uint64_t stamp) {
    if (isCurrentGuest(id)) { batonAssigned.store(stamp, std::memory_order_relaxed); event(History::Kind::Assigned, stamp); }
}
void noteResumed(Identity id, std::uint64_t stamp) {
    if (!isCurrentGuest(id)) return;
    const auto assigned = batonAssigned.exchange(0, std::memory_order_relaxed);
    if (!assigned || stamp < assigned) return;
    const auto gap = toUs(stamp - assigned);
    maxValue(batonWorst, gap);
    if (gap > 20000) ++batonOver20;
    if (gap >= 175000) ++batonOver175;
    event(History::Kind::Resumed,gap,gap>20000);
}
void noteCallback(std::uint64_t level, std::uint64_t requested) {
    const auto now=mach_absolute_time();
    const auto previous=callbackLast.exchange(now,std::memory_order_relaxed);
    const auto gap=previous && now>=previous ? toUs(now-previous) : 0;
    maxValue(callbackWorst,gap);
    if(gap>20000) ++callbackOver20;
    // A single packed value: high 32 bits level, low 32 bits request.
    event(History::Kind::Callback,(level<<32)|(requested&UINT32_MAX),gap>20000);
}
void noteShortfall(std::uint64_t frames) {
    event(History::Kind::Shortfall,frames,true);
}
void noteRegistration(Identity id, std::uint64_t generation, unsigned frames, std::uintptr_t address) {
    if (!epoch()) return; // initial pre-callback registration is setup, not an output cycle
    if (!isCurrentGuest(id)) { fail("registration-origin"); return; }
    ++registrations;
    event(History::Kind::Registration,generation);
    event(History::Kind::RegistrationFrames,frames);
    event(History::Kind::RegistrationAddress,address);
    // Deliberately no interval finish: this can register an earlier DMA buffer
    // while current DSP work is pending in the triple-buffered guest pipeline.
}
void report(std::FILE* out) {
    if (!enabled.load(std::memory_order_relaxed)) return;
    // Cumulative status; reporter only. No per-mutex or per-cycle streaming.
    std::lock_guard<std::mutex> lock(registry().control);
    History::Recorder<16384,4096>::Dump dump;
    const bool hasDump = observing.load(std::memory_order_relaxed) && history.take(ticksPerSecond()*2,dump);
    if (hasDump && (dump.drops || dump.unstable || dump.clipped)) ++historyInvalid;
    const auto e = errors.load();
    const auto count = sessions.load();
    const bool adopted = count && guestJoins.load() == count && rendererJoins.load() == count &&
        completedSessions.load() == count && !e;
    static bool wroteConfig = false;
    if (!wroteConfig) {
        wroteConfig = true;
        std::fprintf(out,"[audio-workgroup-config] schema=1 clock=mach_absolute_time tick_hz=%llu "
            "history_capacity=16384 dump_limit=4096 observer_started_mach=%llu pre_ticks=%llu history_kind=1:cycle,2:subframe,3:finish,"
            "4:assigned,5:resumed,6:callback,7:shortfall,8:trigger,9:shape,10:left,11:right,12:queued,"
            "13:deadline,14:registration,15:registration_frames,16:registration_address\n",
            (unsigned long long)ticksPerSecond(),(unsigned long long)observerStartedMach,
            (unsigned long long)(ticksPerSecond()*2));
    }
    std::fprintf(out, "[audio-workgroup] mode=async epoch=%llu guest=%llx host_generation=%llu "
        "guest_host=%llx renderer_host=%llx required_policy=RT-0-500us-2000us policy_readback=on-adoption-only "
        "sessions=%llu completed_sessions=%llu guest_joins=%llu renderer_joins=%llu leaves=%llu cycles=%llu completed=%llu cancelled=%llu "
        "samples=%llu registrations=%llu late=%llu worst_late_us=%llu errors=%llu last_site=%s last_error=%d "
        "assigned_resume_max_us=%llu over20ms=%llu over175ms=%llu observing=%d callback_gap_max_us=%llu "
        "callback_over20ms=%llu history_invalid=%llu paused=%d adopted=%d qualifying=%d\n",
        (unsigned long long)activeEpoch.load(), (unsigned long long)guestPointer.load(),
        (unsigned long long)lastGeneration.load(), (unsigned long long)guestHost.load(),
        (unsigned long long)rendererHost.load(), (unsigned long long)count, (unsigned long long)completedSessions.load(),
        (unsigned long long)guestJoins.load(),
        (unsigned long long)rendererJoins.load(), (unsigned long long)leaves.load(),
        (unsigned long long)cycles.load(), (unsigned long long)finished.load(),
        (unsigned long long)cancelled.load(), (unsigned long long)samples.load(),
        (unsigned long long)registrations.load(),
        (unsigned long long)late.load(), (unsigned long long)toUs(worstLate.load()),
        (unsigned long long)e, lastSite.load(), lastError.load(),
        (unsigned long long)batonWorst.load(), (unsigned long long)batonOver20.load(),
        (unsigned long long)batonOver175.load(), observing.load(), (unsigned long long)callbackWorst.load(),
        (unsigned long long)callbackOver20.load(), (unsigned long long)historyInvalid.load(),
        paused.load(), adopted, adopted && !batonOver175.load() && !historyInvalid.load());
    if (hasDump) {
        // Reporter-thread stack, maximum 4096 records per report (at most once
        // per second in the live reporter). Nothing is streamed per mutex.
            std::fprintf(out,"[audio-workgroup-history] begin trigger_mach=%llu clock=mach_absolute_time rows=%u "
                "triggers_since_previous=%llu drops=%llu unstable=%llu clipped=%llu\n",
                (unsigned long long)dump.trigger,dump.count,(unsigned long long)dump.triggersSincePrevious,
                (unsigned long long)dump.drops,(unsigned long long)dump.unstable,(unsigned long long)dump.clipped);
            for(unsigned n=0;n<dump.count;++n) { const auto& r=dump.events[n];
                std::fprintf(out,"[audio-workgroup-history] event seq=%llu mach=%llu kind=%u thread=%llx value=%llu\n",
                    (unsigned long long)r.sequence,(unsigned long long)r.mach,r.kind,
                    (unsigned long long)r.thread,(unsigned long long)r.value);
            }
            std::fprintf(out,"[audio-workgroup-history] end rows=%u\n",dump.count);
    }
}
#ifdef PETARI_AUDIO_WORKGROUP_TESTING
namespace Backend {
void setApi(const Api& value) { std::lock_guard<std::mutex> lock(registry().control); api = value; }
void requestForTest(bool value) { enabled.store(value); }
}
#endif
}
