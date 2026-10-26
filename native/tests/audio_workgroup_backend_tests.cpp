// Native lifecycle tests with a fake workgroup/policy API. The real native OS
// scheduler is used for the parked-guest control wake; no RT policy is set.
#include <petari/platform/audio_workgroup.hpp>
#include "workgroup_backend.hpp"
#include "os_internal.hpp"
#include <mach/mach_time.h>
#include <cassert>
#include <cerrno>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <petari/platform/os_host.hpp>
extern "C" void __OSThreadInit();
namespace WG = PetariNative::Platform::AudioWorkgroup;
namespace OS = PetariNative::Platform::OS;
extern "C" __attribute__((noinline)) void petari_audio_workgroup_off_probe(WG::Identity id) {
    if(WG::enabled.load(std::memory_order_relaxed)) WG::guestSafePoint(id);
}
extern "C" __attribute__((noinline)) void petari_audio_workgroup_history_off_probe(WG::Identity id) {
    if(WG::observing.load(std::memory_order_relaxed)) WG::noteAssigned(id,mach_absolute_time());
}
namespace {
std::atomic<unsigned> created{0}, released{0}, promoted{0}, restored{0}, joined{0}, left{0}, starts{0}, ends{0};
std::atomic<int> createError{0}, policyError{0}, joinError{0}, startError{0}, finishError{0}, restoreError{0};
std::function<void()> duringJoin;
std::mutex ownershipLock;
std::vector<std::pair<std::uintptr_t,std::uintptr_t>> memberships;
os_workgroup_interval_t create() {
    ++created; if (createError) { errno = createError; return nullptr; }
    return reinterpret_cast<os_workgroup_interval_t>(new int(1));
}
void release(os_workgroup_interval_t group) { ++released; delete reinterpret_cast<int*>(group); }
int promote(WG::Backend::Policy& saved) {
    ++promoted; saved.changed = true;
    mach_timebase_info_data_t tb{}; mach_timebase_info(&tb);
    saved.actual = {0,static_cast<std::uint32_t>(1000000ull*tb.denom/tb.numer),
        static_cast<std::uint32_t>(2000000ull*tb.denom/tb.numer),TRUE};
    return policyError.load();
}
int restore(WG::Backend::Policy& saved) {
    if (saved.changed) { ++restored; saved.changed = false; }
    return restoreError.load();
}
int join(os_workgroup_t group, os_workgroup_join_token_t token) {
    if (duringJoin) duringJoin();
    if (joinError) return joinError.load();
    ++joined;
    std::lock_guard<std::mutex> lock(ownershipLock);
    memberships.push_back({reinterpret_cast<std::uintptr_t>(token), reinterpret_cast<std::uintptr_t>(pthread_self())});
    return 0;
}
void leave(os_workgroup_t, os_workgroup_join_token_t token) {
    std::lock_guard<std::mutex> lock(ownershipLock);
    const auto id = reinterpret_cast<std::uintptr_t>(token);
    auto it = memberships.begin();
    for (; it != memberships.end() && it->first != id; ++it) {}
    assert(it != memberships.end() && it->second == reinterpret_cast<std::uintptr_t>(pthread_self()));
    memberships.erase(it); ++left;
}
int start(os_workgroup_interval_t, std::uint64_t a, std::uint64_t b) { assert(b > a); ++starts; return startError.load(); }
int finish(os_workgroup_interval_t) { ++ends; return finishError.load(); }
std::string status() {
    auto* f = std::tmpfile(); assert(f); WG::report(f); std::rewind(f);
    std::string result; char line[4096]; while (std::fgets(line,sizeof(line),f)) result += line;
    std::fclose(f); return result;
}
std::uint64_t counter(const char* name) {
    const auto text = status(); const std::string key = std::string(name) + "=";
    const auto at = text.find(key); assert(at != std::string::npos);
    return std::strtoull(text.c_str() + at + key.size(), nullptr, 10);
}
WG::Identity id{0x1000,1};
void open(WG::Identity who = id) { WG::bindCallback(who, 0x55, true); WG::confirmStart(who,0x55,true); WG::startSink(who,true); }
void close(WG::Identity who = id) { WG::stopSink(); WG::guestSafePoint(who); WG::collect(); }
void render(WG::Origin o, bool complete = true) {
    std::thread t([=] {
        WG::rendererSafePoint(); WG::beginCycle(o,7,32000,0x2000,0x3000);
        const auto before = ends.load();
        for (unsigned n=0; n<(complete ? 7u : 3u); ++n) {
            WG::subframeComplete();
            if (n==2) {
                WG::noteRegistration(o.caller,99,560,0x7000); // injected earlier-buffer registration
                assert(ends==before); // never substitutes for the final DSP completion
            }
        }
        WG::rendererExit();
    }); t.join();
}
OSMessageQueue readyQueue, commandQueue;
OSMessage readySlots[2], commandSlots[2];
std::mutex controlLock;
std::condition_variable controlCv;
std::atomic<unsigned> parkedLeft{0};
void* guest(void*) {
    OS::markAudioGuest();
    const auto identity = OS::audioIdentity(); assert(identity);
    open(identity);
    OSSendMessage(&readyQueue, reinterpret_cast<OSMessage>(1), OS_MESSAGE_BLOCK);
    OSMessage message{}; OSReceiveMessage(&commandQueue,&message,OS_MESSAGE_BLOCK);
    // Control wake must never return this receive before a guest message.
    assert(message == reinterpret_cast<OSMessage>(2));
    return nullptr; // OSExitThread must leave the successful membership
}
void notifyLeave(os_workgroup_t group, os_workgroup_join_token_t token) {
    leave(group,token);
    { std::lock_guard<std::mutex> lock(controlLock); parkedLeft.fetch_add(1); }
    controlCv.notify_all();
}
}
int main() {
    WG::Backend::setApi({create,release,promote,restore,join,leave,start,finish});
    WG::Backend::requestForTest(false);
    petari_audio_workgroup_off_probe({}); petari_audio_workgroup_history_off_probe({});
    open(); assert(!created && !promoted && !joined); // disabled arm never reaches API/TLS helpers
    WG::Backend::requestForTest(true);
    open(); assert(WG::epoch() && joined == 1);
    const auto source = WG::origin(id); assert(source.epoch && source.caller == id);
    assert(!WG::origin({id.thread,id.generation+1}).epoch); // reused address rejected
    render(source); assert(starts==1 && ends==1 && counter("completed")==1 && counter("samples")==560);
    assert(counter("errors")==0 && counter("qualifying")==1);
    close(); assert(created==released && joined==left && promoted==restored);
    // A partial DSP cycle finishes as CANCELLED on the owning renderer, never completed.
    open(); render(WG::origin(id),false); close(); assert(counter("cancelled")==1 && counter("completed")==1);
    // Join EALREADY/EINVAL are failures, never a successful adoption or malformed leave.
    for (int error : {EALREADY, EINVAL}) {
        auto previous = counter("errors"); auto before = left.load(); joinError=error;
        open(); assert(counter("errors")==previous+1 && counter("qualifying")==0 && left==before);
        joinError=0; close(); assert(promoted==restored && created==released);
    }
    // Policy/readback failure restores a partially changed policy and never joins.
    auto previous = counter("errors"); const auto before = joined.load(); policyError=EPROTO;
    open(); policyError=0; close(); assert(counter("errors")==previous+1 && joined==before && promoted==restored);
    previous=counter("errors"); open(); restoreError=EIO; close(); restoreError=0;
    assert(counter("errors")==previous+1);
    // Creation failure has no live group to join or release.
    previous=counter("errors"); createError=ENOMEM; open(); createError=0; close();
    assert(counter("errors")==previous+1 && !WG::epoch());
    // Wrong actual start caller and non-CoreAudio output both refuse the arm.
    previous=counter("errors"); WG::bindCallback(id,0x55,true); WG::confirmStart({id.thread,2},0x55,true);
    WG::startSink(id,true); close(); assert(counter("errors")>=previous+2);
    previous=counter("errors"); WG::bindCallback(id,0x55,true); WG::confirmStart(id,0x55,true);
    WG::startSink(id,false); close(); assert(counter("errors")==previous+1);
    // Interval failures remain failures; no deadline extension/retry-until-success.
    open(); startError=EINVAL; render(WG::origin(id)); startError=0; close();
    assert(counter("errors")>previous+1);
    open(); finishError=EIO; render(WG::origin(id)); finishError=0; close(); assert(counter("qualifying")==0);
    // Closing concurrently with joining retains the old object until same-thread leave.
    duringJoin=[] { WG::stopSink(); }; open(); duringJoin={}; WG::guestSafePoint(id); WG::collect();
    assert(!WG::epoch() && memberships.empty());
    // A new HostThread generation at the same address can adopt after the old token left.
    open({id.thread,2}); close({id.thread,2}); assert(memberships.empty());
    // Actual native untimed wait: a host-only close wake causes leave, but does
    // NOT make the sleeping guest runnable or consume a guest message.
    __OSThreadInit(); WG::setGuestWake(OS::wakeAudioGuestControl);
    WG::Backend::setApi({create,release,promote,restore,join,notifyLeave,start,finish});
    OSInitMessageQueue(&readyQueue,readySlots,2); OSInitMessageQueue(&commandQueue,commandSlots,2);
    OSThread thread{}; alignas(32) unsigned char stack[65536];
    assert(OSCreateThread(&thread,guest,nullptr,stack+sizeof(stack),sizeof(stack),2,0)); OSResumeThread(&thread);
    OSMessage message{}; OSReceiveMessage(&readyQueue,&message,OS_MESSAGE_BLOCK);
    const auto beforeControl = parkedLeft.load(); WG::stopSink();
    petari_os_begin_host_blocking();
    {
        std::unique_lock<std::mutex> lock(controlLock);
        controlCv.wait(lock,[&] { return parkedLeft.load() > beforeControl; });
    }
    petari_os_end_host_blocking();
    assert(thread.state == OS_THREAD_STATE_WAITING); // native control wake did not grant guest execution
    OSSendMessage(&commandQueue,reinterpret_cast<OSMessage>(2),OS_MESSAGE_BLOCK); OSJoinThread(&thread,nullptr);
    WG::collect(); assert(memberships.empty());
    std::puts("audio workgroup API errors / same-thread lifecycle / native parked control PASS");
}
