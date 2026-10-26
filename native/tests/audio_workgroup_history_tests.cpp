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
    std::puts("bounded audio history / trigger / clipping / MPSC payload PASS");
}
