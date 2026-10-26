#include "../audio/output_au_render.hpp"
#include "../audio/output_au_observer.hpp"
#include "../platform/audio/workgroup_history.hpp"
#include "../platform/audio/workgroup_history_store.hpp"
#include <petari/platform/audio_workgroup.hpp>
#include <array>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <pthread.h>
#include <thread>
#include <mach/mach_time.h>
using namespace PetariNative::AudioOutputAU::Detail;
namespace WG=PetariNative::Platform::AudioWorkgroup;
extern "C" void petari_au_guard_enter();
extern "C" unsigned petari_au_guard_leave();
static std::array<RenderEvent,2048> events;
static unsigned eventCount=0;
static std::uint64_t ticks=100;
static std::uint64_t clockRead() noexcept { return ticks+=100; }
static void observe(const RenderEvent& e) noexcept { assert(eventCount<events.size()); events[eventCount++]=e; }
static void* volatile escape=nullptr;
struct Buffers { UInt32 count=2; AudioBuffer buffers[2]{}; };
static void configure(RenderOwner& owner,RenderData& d,bool on) {
    assert(d.ring.configure(1024,0,2));
    d.maxFrames=256; d.generation=7; d.scratch.reset(new float[512]{});
    d.observing=on; d.clock=clockRead; d.publish=observe; d.gapTriggerTicks=100000;
    d.hostTicksPerFrame=1;
    owner.published.store(&d);
}
static OSStatus guarded(RenderOwner& owner,Buffers& buffers,AudioTimeStamp& timestamp,UInt32 frames,AudioUnitRenderActionFlags& flags) {
    if(timestamp.mFlags&kAudioTimeStampHostTimeValid) timestamp.mHostTime=1000+static_cast<std::uint64_t>(timestamp.mSampleTime);
    petari_au_guard_enter();
    const auto result=render(&owner,&flags,&timestamp,0,frames,reinterpret_cast<AudioBufferList*>(&buffers));
    const auto violations=petari_au_guard_leave();
    assert(violations==0); return result;
}
int main() {
    // Prove the interposer is live, not an unused declaration. Those calls
    // intentionally violate the test guard outside production rendering.
    pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
    petari_au_guard_enter();
    escape=std::malloc(23); std::free(escape); escape=nullptr;
    pthread_mutex_lock(&mutex); pthread_mutex_unlock(&mutex);
    assert(petari_au_guard_leave()>=4);
    for(bool on:{false,true}) {
        RenderOwner owner; RenderData d; configure(owner,d,on);
        std::array<float,512> left{},right{},outL{},outR{};
        for(unsigned n=0;n<512;++n) { left[n]=float(n)/1024; right[n]=-left[n]; }
        assert(d.ring.write(left.data(),right.data(),400));
        Buffers b; b.buffers[0]={1,256*4,outL.data()}; b.buffers[1]={1,256*4,outR.data()};
        AudioTimeStamp timestamp{}; timestamp.mFlags=kAudioTimeStampSampleTimeValid|kAudioTimeStampHostTimeValid;
        timestamp.mSampleTime=0; timestamp.mHostTime=1000;
        AudioUnitRenderActionFlags flags=0;
        // This is genuinely the callback's FIRST invocation; no dummy warmup.
        assert(guarded(owner,b,timestamp,256,flags)==noErr);
        assert(outL[200]==left[200] && outR[200]==right[200] && !flags);
        assert(!d.faults.load() && d.ring.minimumLevel()==144);
        timestamp.mSampleTime=256;
        assert(guarded(owner,b,timestamp,256,flags)==noErr);
        assert(d.shortfalls.load()==112 && d.copied.load()==400 && outL[200]==0);
        assert(!d.timelineFaults.load());
        const auto raw=d.shortfalls.load();
        assert(d.ring.write(left.data(),right.data(),512));
        timestamp.mSampleTime=768; // a full ring must NOT hide a missing slice
        assert(guarded(owner,b,timestamp,256,flags)==noErr);
        assert(d.timelineFaults.load()==1 && (d.faults.load()&BadTimeline) && d.shortfalls.load()==raw);
        timestamp.mSampleTime=1024;
        d.mute=true; flags=0;
        assert(guarded(owner,b,timestamp,256,flags)==noErr && (flags&kAudioUnitRenderAction_OutputIsSilence));
        for(unsigned n=0;n<256;++n) assert(outL[n]==0 && outR[n]==0);
        d.mute=false; timestamp.mSampleTime=1280;
        b.buffers[0].mData=nullptr; b.buffers[1].mData=nullptr;
        assert(guarded(owner,b,timestamp,17,flags)==noErr);
        assert(b.buffers[0].mData==d.scratch.get() && b.buffers[1].mData==d.scratch.get()+256);
        assert(guarded(owner,b,timestamp,257,flags)!=noErr && (d.faults.load()&OverLimit));
        b.count=3; assert(guarded(owner,b,timestamp,32,flags)!=noErr);
        b.count=2; b.buffers[0].mData=outL.data(); b.buffers[0].mDataByteSize=1;
        assert(guarded(owner,b,timestamp,32,flags)!=noErr);
        b.buffers[0].mDataByteSize=1024;
        owner.consumer.test_and_set();
        assert(guarded(owner,b,timestamp,32,flags)==kAudioUnitErr_CannotDoInCurrentContext);
        owner.consumer.clear(); assert(owner.faults.load()&Reentrant);
        owner.published.store(nullptr);
        assert(guarded(owner,b,timestamp,32,flags)!=noErr); // undersized closed-owner buffer cannot claim filled
        b.buffers[1].mDataByteSize=1024;
        assert(guarded(owner,b,timestamp,32,flags)==noErr && owner.readers.load()==0);
        if(!on) assert(eventCount==0 && ticks==100);
        else assert(eventCount>0 && ticks>100);
    }
    // The actual workgroup publisher, not only a fixture publisher, is inside
    // the guard on the observer/shortfall/trigger paths. No report occurs here.
    WG::History::audioHistory.prepare();
    mach_timebase_info_data_t timebase{}; assert(mach_timebase_info(&timebase)==KERN_SUCCESS);
    configureObserverClock(timebase.numer,timebase.denom,0);
    RenderOwner owner; RenderData d; configure(owner,d,true);
    d.publish=publishRenderEvent; d.clock=observerClock; d.gapTriggerTicks=0;
    std::array<float,256> l{},r{};
    Buffers b; b.buffers[0]={1,1024,l.data()}; b.buffers[1]={1,1024,r.data()};
    AudioTimeStamp time{}; time.mFlags=kAudioTimeStampSampleTimeValid|kAudioTimeStampHostTimeValid; AudioUnitRenderActionFlags flags=0;
    assert(guarded(owner,b,time,256,flags)==noErr);
    assert(!d.faults.load());
    time.mSampleTime=256; ticks+=200000;
    assert(guarded(owner,b,time,256,flags)==noErr);
    // A host-time hole with CONTIGUOUS sample numbers is its own failure,
    // independently of ring misses and without an observer clock syscall.
    time.mSampleTime=512; time.mHostTime=999999;
    petari_au_guard_enter();
    assert(render(&owner,&flags,&time,0,256,reinterpret_cast<AudioBufferList*>(&b))==noErr);
    assert(petari_au_guard_leave()==0 && d.hostTimelineFaults.load()==1 && (d.faults.load()&BadHostTimeline));
    // Repeated concurrent retire/publication uses the same callback. Its
    // retired owner remains alive until the reader has demonstrably returned.
    owner.published.store(nullptr);
    std::thread callback([&] { for(unsigned n=0;n<1000;++n) assert(guarded(owner,b,time,0,flags)==noErr); });
    callback.join(); assert(owner.readers.load()==0);
    std::puts("production AU callback first/error/observer prohibited-call guard / missing-slice / lifetime PASS");
}
