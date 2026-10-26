#include "../audio/output_au_core.hpp"
#include "../audio/output_au_lifecycle.hpp"
#include "../audio/paced_ring.hpp"
#include <array>
#include <cassert>
#include <thread>
#include <cstdio>
using namespace PetariNative::AudioOutputAU::Detail;
int main() {
    Ring ring;
    assert(!ring.configure(0,0,2) && !ring.configure(8,8,2) && !ring.configure(8,0,3));
    assert(ring.configure(8,0,2));
    std::array<float,16> left{},right{},outL{},outR{};
    for(unsigned n=0;n<16;++n) { left[n]=float(n+1); right[n]=-left[n]; }
    assert(ring.write(left.data(),right.data(),6));
    assert(ring.read(outL.data(),outR.data(),4,false)==4);
    assert(ring.write(left.data()+6,right.data()+6,6));
    assert(!ring.write(left.data(),right.data(),1));
    assert(ring.read(outL.data(),outR.data(),10,false)==8);
    for(unsigned n=0;n<8;++n) { assert(outL[n]==float(n+5)); assert(outR[n]==-outL[n]); }
    assert(outL[8]==0 && outL[9]==0 && ring.minimumLevel()==0 && ring.largestRequest()==10);
    assert(ring.configure(16,3,1));
    assert(ring.read(outL.data(),nullptr,3,false)==3 && outL[0]==0);
    assert(ring.write(left.data(),nullptr,8));
    assert(ring.read(outL.data(),nullptr,8,true)==8);
    for(unsigned n=0;n<8;++n) assert(outL[n]==0);

    ShortfallUnits units;
    assert(units.take(1,32000,44100)==1); // one physical frame cannot disappear
    std::uint64_t sum=1;
    for(unsigned n=2;n<=44100;++n) sum+=units.take(n,32000,44100);
    assert(sum==32000 && units.remainder()==0);
    assert(units.take(44100,32000,44100)==0);

    RenderOwner owner; RenderData generation;
    owner.published.store(&generation); owner.readers.fetch_add(1);
    auto* borrowed=owner.published.load(); assert(borrowed==&generation);
    Retirement retire; retire.begin(owner); retire.stopped=true;
    assert(!owner.published.load() && !retire.mayUninitialize(owner));
    owner.readers.fetch_sub(1); assert(retire.mayUninitialize(owner));
    assert(!retire.mayReclaim(owner)); retire.uninitialized=true;
    assert(!retire.mayReclaim(owner)); retire.disposed=true; assert(retire.mayReclaim(owner));
    owner.readers.fetch_add(1); assert(owner.published.load()==nullptr && !retire.mayReclaim(owner));
    owner.readers.fetch_sub(1); assert(retire.mayReclaim(owner));
    ChangeMailbox changes; changes.enabled.store(true); changes.live.store(7);
    changes.notify(6); assert(changes.changes.load()==0);
    changes.notify(7); changes.notify(0); assert(changes.changes.load()==2);
    changes.live.store(8); changes.notify(7); assert(changes.changes.load()==2);
    changes.enabled.store(false); changes.notify(0); assert(changes.changes.load()==2);

    // Two real threads exercise SPSC publication and wrapping, no device/API.
    assert(ring.configure(1024,0,2));
    std::thread writer([&] {
        for(unsigned n=1;n<=50000;++n) {
            const float value=float(n),negative=-value;
            while(!ring.space()) std::this_thread::yield(); // fixture only
            assert(ring.write(&value,&negative,1));
        }
    });
    for(unsigned n=1;n<=50000;++n) {
        while(!ring.level()) std::this_thread::yield();
        assert(ring.read(outL.data(),outR.data(),1,false)==1);
        assert(outL[0]==float(n) && outR[0]==-float(n));
    }
    writer.join(); assert(ring.level()==0);

    // Device-clock feedback at fractional rates. Converter-held source is
    // included in the ledger; a separate hidden prebuffer cannot add credit.
    for(auto guest:{32000u,48000u}) for(auto device:{44100.,48000.,96000.}) {
        assert(ring.configure(16384,std::size_t(std::ceil(.048*device)),2));
        ProductionLedger ledger{guest,device};
        PetariNative::AudioSDL::Detail::Pacer pacer(guest);
        std::array<float,512> zero{};
        double nextCallback=0;
        std::uint64_t sourceTotal=0,deviceTotal=0;
        for(unsigned tick=0;tick<120000;++tick) {
            if(tick>=nextCallback) {
                std::array<float,256> l{},r{};
                assert(ring.read(l.data(),r.data(),256,false)==256);
                nextCallback+=256000/device;
            }
            auto allowance=pacer.allowance(.001,ring.level()<ring.lowWater(device,guest));
            const auto wanted=ledger.wanted(ring,allowance,guest/1000);
            sourceTotal+=wanted; ledger.pulled+=wanted;
            const auto output=static_cast<std::uint64_t>(sourceTotal*device/guest)-deviceTotal;
            assert(output<=zero.size()); assert(ring.write(zero.data(),zero.data(),output));
            deviceTotal+=output;
            assert(ledger.ahead(ring)<=guest*.080);
        }
        assert(std::fabs(double(sourceTotal)/guest-120)<.1);
        const auto wantedBefore=ledger.wanted(ring,100,100);
        ledger.pulled+=guest; // a second of converter-held/staged source
        assert(ledger.wanted(ring,100,100)==0 && ledger.ahead(ring)>guest);
        (void)wantedBefore;
    }
    std::puts("output AU ring / conservation / fractional pacing / retirement / stale notifications PASS");
}
