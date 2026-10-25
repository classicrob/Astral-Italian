// Deterministic protocol/error/race proofs; no native timer, sleeps or RT policy.
#include "monitor_wait_protocol.hpp"
#include <cassert>
#include <deque>
#include <functional>
#include <vector>
using namespace PetariNative::Platform::MonitorWait;
namespace {
Batch timer(std::uint64_t generation) { Batch b; b.events[0] = {Kind::Timer, generation}; b.count = 1; return b; }
Batch notification() { Batch b; b.events[0] = {Kind::Notification, 0}; b.count = 1; return b; }
struct Fake {
    struct Arm { std::uint64_t deadline, generation, leeway; bool critical; };
    std::vector<Arm> arms;
    std::vector<std::uint64_t> cancellations;
    std::deque<Batch> batches;
    bool stickyNotification = false;
    int armError = 0, cancelError = 0;
    std::function<void()> duringArm, duringCancel;
    int arm(std::uint64_t d, std::uint64_t g, bool c, std::uint64_t l) {
        arms.push_back({d,g,l,c}); if (duringArm) duringArm(); return armError;
    }
    Batch receive() {
        if (stickyNotification) { stickyNotification = false; return notification(); }
        assert(!batches.empty()); const Batch b = batches.front(); batches.pop_front(); return b;
    }
    int cancel(std::uint64_t g) { cancellations.push_back(g); if (duringCancel) duringCancel(); return cancelError; }
};
}
int main() {
    Mode m = Mode::Realtime;
    assert(parseMode(nullptr,m) && m == Mode::Legacy);
    assert(parseMode("legacy",m) && m == Mode::Legacy);
    assert(parseMode("critical",m) && m == Mode::Critical);
    assert(parseMode("realtime",m) && m == Mode::Realtime);
    assert(!parseMode("critical-realtime",m) && !parseMode("invalid",m));
    {
        NotificationGate gate; unsigned signals=0, currentPending=0;
        gate.request(false,[&]{++signals;}); // Uninitialized/fork-child backend.
        currentPending=1;gate.request(true,[&]{++signals;});assert(signals==1);
        currentPending=2;gate.request(true,[&]{++signals;});assert(signals==1); // Coalesced while old timer cancels.
        gate.consumed();assert(currentPending==2); // The subsequent inspection sees the NEW request.
        currentPending=3;gate.request(true,[&]{++signals;});assert(signals==2); // After clear: a fresh edge.
        gate.consumed();gate.request(true,[&]{++signals;});assert(signals==3);
    }
    std::uint64_t n = 0, d = 0;
    assert(nsToTicks(1,{125,3},n) && n == 1); // Round UP; never request an early deadline.
    assert(ticksToNs(n,{125,3},n) && n == 42);
    assert(deadline(100,500000,{125,3},d,n) && d == 12100 && n == 500000);
    assert(deadline(100,0,{125,3},d,n) && d == 100 && n == 0);
    assert(!nsToTicks(1,{0,1},n) && !ticksToNs(1,{1,0},n));
    assert(!nsToTicks(UINT64_MAX,{1,2},n));
    assert(!deadline(INT64_MAX-1,3,{1,1},d,n));
    RealtimeContract rt;
    assert(realtimeContract({125,3},rt) && rt.period == 12000 && rt.computation == 6000 && rt.constraint == 12000);
    assert(sameContract(rt,rt,false) && !sameContract(rt,rt,true));
    assert(!sameContract(rt,{rt.period,rt.computation-1,rt.constraint},false));
    assert(!sameContract(rt,{rt.period+1,rt.computation,rt.constraint},false));
    assert(!realtimeContract({0,3},rt) && !realtimeContract({1,UINT32_MAX},rt));
    {
        Fake f; Protocol<Fake> p(f); f.batches.push_back(timer(1));
        const auto r = p.wait(123456,true);
        assert(!r.error && r.timer && !r.notification && r.returns == 1);
        assert(f.arms.size() == 1 && f.arms[0].deadline == 123456 && f.arms[0].critical && !f.arms[0].leeway);
        assert(f.cancellations.size() == 1 && f.cancellations[0] == 1);
    }
    {
        // Notification BEFORE registration remains sticky and cancels the timer.
        Fake f; f.stickyNotification = true; Protocol<Fake> p(f);
        const auto r = p.wait(200,false);
        assert(!r.error && r.notification && !r.timer && !f.stickyNotification && !f.arms[0].critical);
    }
    {
        // Notification between arm and blocking receive; no periodic backup poll.
        Fake f; f.duringArm = [&] { f.stickyNotification = true; }; Protocol<Fake> p(f);
        const auto r = p.wait(300,true); assert(r.notification && r.returns == 1);
    }
    {
        // A NEW pending transition during cancellation belongs to the next wait.
        Fake f; Protocol<Fake> p(f); f.stickyNotification = true;
        f.duringCancel = [&] { if (f.cancellations.size() == 1) f.stickyNotification = true; };
        assert(p.wait(400,true).notification); assert(p.wait(500,true).notification);
        assert(f.arms.size() == 2 && f.arms[1].generation == 2 && !f.stickyNotification);
    }
    {
        // Simultaneous timer+notification produce ONE monitor iteration.
        Fake f; Protocol<Fake> p(f); Batch both=timer(1); both.events[1]={Kind::Notification,0};both.count=2;f.batches.push_back(both);
        const auto r=p.wait(600,true);assert(r.timer && r.notification && r.returns==1 && f.cancellations.size()==1);
    }
    {
        // EINTR cannot restart the relative timeout and postpone the deadline.
        Fake f; Protocol<Fake> p(f); Batch interrupted;interrupted.interrupted=true;
        f.batches={interrupted,interrupted,timer(1)};
        const auto r=p.wait(700,true);assert(!r.error && r.interrupts==2 && r.returns==3);
        assert(f.arms.size()==1 && f.arms[0].deadline==700);
    }
    {
        // Cancelled old generation cannot masquerade as the newly armed timeout.
        Fake f; Protocol<Fake> p(f);f.batches={timer(0),timer(1)};
        const auto r=p.wait(800,true);assert(r.timer && r.stale==1 && r.returns==2 && f.arms.size()==1);
    }
    {
        Fake f;Protocol<Fake> p(f);f.batches.push_back(timer(1));
        assert(p.wait(0,true).timer && f.arms[0].deadline==0); // Original immediate retry, not a busy timer loop.
    }
    {
        Fake f;Protocol<Fake> p(f);f.armError=ENOMEM;
        const auto r=p.wait(900,true);assert(r.error==ENOMEM && r.errorSite && r.returns==0 && f.cancellations.size()==1);
    }
    {
        Fake f;Protocol<Fake> p(f);Batch bad;bad.error=EBADF;f.batches.push_back(bad);f.cancelError=EIO;
        const auto r=p.wait(900,true);assert(r.error==EBADF && r.cleanupError==EIO && !r.timer && !r.notification);
    }
    {
        Fake f;Protocol<Fake> p(f);f.batches.push_back(notification());f.cancelError=EIO;
        const auto r=p.wait(900,true);assert(r.error==EIO && r.notification); // Never a clean successful arm.
    }
    {
        Fake f;Protocol<Fake> p(f);f.batches.push_back({});
        assert(p.wait(900,true).error==EPROTO);
    }
    {
        Fake f;Protocol<Fake> p(f);Batch bad;bad.count=3;f.batches.push_back(bad);
        assert(p.wait(900,true).error==EPROTO); // Bound before indexing.
    }
    {
        Fake f;Protocol<Fake> p(f);for(unsigned i=0;i<9;++i)f.batches.push_back(timer(0));
        const auto r=p.wait(900,true);assert(r.error==EPROTO && r.stale==9 && f.arms.size()==1);
    }
    {
        Fake f;Protocol<Fake> p(f,UINT64_MAX);
        assert(p.wait(900,true).error==EOVERFLOW && f.arms.empty() && f.cancellations.empty());
    }
}
