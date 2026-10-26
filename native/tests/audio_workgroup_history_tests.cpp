#include "workgroup_history.hpp"
#include <cassert>
#include <cstdio>
#include <thread>
#include <vector>
namespace H = PetariNative::Platform::AudioWorkgroup::History;
int main() {
    H::Recorder<8,8> ring;
    H::Recorder<8,8>::Dump dump;
    ring.push(10,1,2,3); assert(!ring.take(20,dump)); // no trigger, no dump
    ring.trigger(20); assert(ring.take(20,dump) && dump.count==1 && !dump.drops && !dump.unstable && !dump.clipped);
    assert(dump.events[0].sequence==1 && dump.events[0].mach==10 && dump.events[0].value==3);
    assert(!ring.take(20,dump)); // consumed once
    ring.trigger(30); ring.trigger(31); assert(ring.take(20,dump) && dump.trigger==30 && dump.triggersSincePrevious==2);
    // Overwritten pre-trigger window is explicit, including no retained row in the window.
    for(unsigned n=0;n<30;++n) ring.push(100+n,2,3,n);
    ring.trigger(20); assert(ring.take(20,dump) && dump.clipped && !dump.count);
    H::Recorder<16,2> small;
    H::Recorder<16,2>::Dump limited;
    for(unsigned n=1;n<8;++n) small.push(n,1,1,n);
    small.trigger(10); assert(small.take(10,limited) && limited.count==2 && limited.clipped==5);
    H::Recorder<8192,8192> concurrent;
    std::vector<std::thread> threads;
    for(unsigned t=0;t<4;++t) threads.emplace_back([&,t] {
        for(unsigned n=1;n<=1000;++n) {
            const auto value=t*1000+n;
            concurrent.push(value,3,t,value+10000);
        }
    });
    for(auto& t:threads) t.join();
    concurrent.trigger(10000);
    H::Recorder<8192,8192>::Dump all;
    assert(concurrent.take(10000,all) && all.count==4000 && !all.unstable && !all.drops && !all.clipped);
    for(unsigned n=0;n<all.count;++n) assert(all.events[n].value==all.events[n].mach+10000 && all.events[n].kind==3);
    // Old failure: out-of-window cells in the frozen prefix were overwritten
    // between trigger and report, and counted as unstable before time filtering.
    H::Recorder<8,8> wrapped;
    for(unsigned n=1;n<=8;++n) wrapped.push(n,6,23,n,100+n,99,0x1234);
    wrapped.trigger(8);
    wrapped.push(9,6,23,9); wrapped.push(10,6,23,10);
    assert(wrapped.take(2,dump) && dump.count==3 && dump.recovered==2 && dump.outsideWindow==5);
    assert(!dump.retentionUnknown && !dump.unstable && !dump.drops && !dump.clipped);
    for(unsigned n=0;n<3;++n) {
        assert(dump.events[n].sequence==6+n && dump.events[n].steadyNs==106+n);
        assert(dump.events[n].ticket==99 && dump.events[n].object==0x1234);
    }
    // More than one post-trigger wrap really loses a required payload. Refuse
    // it, and expose the exact cell identities; do not call it outside-window.
    H::Recorder<8,8> lost;
    for(unsigned n=1;n<=8;++n) lost.push(n,6,23,n);
    lost.trigger(8);
    for(unsigned n=9;n<=24;++n) lost.push(n,6,23,n);
    assert(lost.take(2,dump) && dump.retentionUnknown==8 && dump.rejectedCount==8 && !dump.count);
    for(unsigned n=0;n<8;++n) {
        const auto& r=dump.rejected[n];
        assert(r.expected==n+1 && r.currentSequence==n+17 && r.previousSequence==n+9);
        assert(r.currentStable && r.previousStable);
        assert(!r.inWindow && !r.kind && !r.thread); // unknown, never invented
    }
    H::Recorder<8,8> post;
    post.push(10,17,1,0); post.trigger(10);
    post.push(11,19,1,0); post.push(12,20,1,0); post.push(14,25,1,0);
    assert(post.take(10,dump,20,2) && dump.endMach==12 && dump.count==3 && dump.outsideWindow==1);
    // Snapshot concurrently with a real wrap. Each cell is overwritten at
    // most once, so every original payload must survive, even while publishing.
    // Initial timestamps are deliberately outside half of the output window.
    for(unsigned iteration=0;iteration<40;++iteration) {
        H::Recorder<1024,1024> live;
        H::Recorder<1024,1024>::Dump snapshot;
        for(unsigned n=1;n<=1000;++n) live.push(n,3,7,n+10000,10*n,77,88);
        live.trigger(1000);
        std::atomic<bool> go{false};
        std::thread writer([&] {
            while(!go.load(std::memory_order_acquire)) std::this_thread::yield();
            for(unsigned n=1001;n<=2000;++n) live.push(n,4,8,n+10000);
        });
        go.store(true,std::memory_order_release);
        assert(live.take(500,snapshot));
        writer.join();
        assert(snapshot.count==501 && !snapshot.retentionUnknown && !snapshot.unstable && !snapshot.drops && !snapshot.clipped);
        for(unsigned n=0;n<501;++n) {
            const auto& e=snapshot.events[n];
            assert(e.sequence==n+500 && e.value==e.mach+10000 && e.steadyNs==10*e.mach);
            assert(e.ticket==77 && e.object==88 && e.thread==7 && e.kind==3);
        }
    }
    std::puts("bounded audio history / post-trigger wrap / refused loss / concurrent snapshot PASS");
}
