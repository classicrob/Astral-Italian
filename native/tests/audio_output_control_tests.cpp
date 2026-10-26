// Fake CoreAudio device. Executes the production controller/recovery/producer,
// real CPU-only AudioConverter and production render callback. NO real output
// device open/default setter/guest application. IOKit port is local only.
#include "../audio/output_au_backend.hpp"
#include "../audio/output_au_render.hpp"
#include <petari/audio_output_au.hpp>
#include <petari/host_allocation.hpp>
#include <petari/platform/audio.hpp>
#include <petari/platform/audio_workgroup.hpp>
#include <petari/platform/power.hpp>
#include <IOKit/IOMessage.h>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>
#include <mach/mach_time.h>
namespace AU=PetariNative::AudioOutputAU;
using namespace AU::Detail;
using namespace std::chrono_literals;
namespace {
std::atomic<unsigned> device{11},created{0},disposed{0},powerRequests{0},closed{0};
const char* rejected="none"; // fixture changes only while controller is stopped
struct Unit {
    unsigned device=0;
    AudioStreamBasicDescription format{};
    AURenderCallbackStruct callback{};
    std::atomic<bool> quit{false};
    bool initialized=false,stopped=true;
    std::thread worker;
};
std::mutex listenerMutex;
AudioObjectPropertyListenerProc systemListener=nullptr;
void* powerRef=nullptr;
IOServiceInterestCallback powerCallback=nullptr;
bool reject(const char* step) { return !std::strcmp(rejected,step); }
AudioComponent component(AudioComponent,const AudioComponentDescription* d) {
    assert(d->componentSubType==kAudioUnitSubType_DefaultOutput);
    return reinterpret_cast<AudioComponent>(1);
}
OSStatus create(AudioComponent,AudioComponentInstance* result) {
    if(reject("create")) return kAudioUnitErr_InvalidProperty;
    auto* unit=new Unit; unit->device=device.load(); *result=reinterpret_cast<AudioUnit>(unit); ++created; return noErr;
}
OSStatus dispose(AudioComponentInstance ref) {
    auto* unit=reinterpret_cast<Unit*>(ref);
    assert(unit->stopped && !unit->initialized && !unit->worker.joinable());
    if(reject("dispose")) return kAudioUnitErr_CannotDoInCurrentContext;
    delete unit; ++disposed; return noErr;
}
template<class T> OSStatus result(void* out,UInt32* bytes,T value) {
    if(*bytes<sizeof(T)) return kAudioUnitErr_InvalidPropertyValue;
    std::memcpy(out,&value,sizeof(T)); *bytes=sizeof(T); return noErr;
}
OSStatus getUnit(AudioUnit ref,AudioUnitPropertyID property,AudioUnitScope scope,AudioUnitElement,void* out,UInt32* bytes) {
    auto* unit=reinterpret_cast<Unit*>(ref);
    if(property==kAudioOutputUnitProperty_CurrentDevice) return result(out,bytes,AudioDeviceID(unit->device));
    if(property==kAudioOutputUnitProperty_EnableIO) return result(out,bytes,UInt32(1));
    if(property==kAudioUnitProperty_MaximumFramesPerSlice) {
        if(reject("maximum")) return kAudioUnitErr_InvalidProperty;
        return result(out,bytes,UInt32(256));
    }
    if(property==kAudioUnitProperty_StreamFormat) {
        if(scope==kAudioUnitScope_Input) return result(out,bytes,unit->format);
        AudioStreamBasicDescription hardware{}; hardware.mSampleRate=unit->device==11 ? 44100 : 48000; hardware.mChannelsPerFrame=2;
        return result(out,bytes,hardware);
    }
    if(property==kAudioUnitProperty_LastRenderError) return result(out,bytes,OSStatus(0));
    if(property==kAudioUnitProperty_Latency) return result(out,bytes,Float64(0));
    return kAudioUnitErr_InvalidProperty;
}
OSStatus setUnit(AudioUnit ref,AudioUnitPropertyID property,AudioUnitScope,AudioUnitElement,const void* data,UInt32) {
    auto* unit=reinterpret_cast<Unit*>(ref);
    if(property==kAudioUnitProperty_StreamFormat) { unit->format=*static_cast<const AudioStreamBasicDescription*>(data); return noErr; }
    if(property==kAudioUnitProperty_SetRenderCallback) {
        if(reject("callback")) return kAudioUnitErr_InvalidProperty;
        unit->callback=*static_cast<const AURenderCallbackStruct*>(data); return noErr;
    }
    return noErr;
}
OSStatus initialize(AudioUnit ref) {
    if(reject("initialize")) return kAudioUnitErr_FailedInitialization;
    reinterpret_cast<Unit*>(ref)->initialized=true; return noErr;
}
OSStatus uninitialize(AudioUnit ref) {
    auto* unit=reinterpret_cast<Unit*>(ref); assert(unit->stopped); unit->initialized=false; return noErr;
}
OSStatus start(AudioUnit ref) {
    auto* unit=reinterpret_cast<Unit*>(ref); unit->stopped=false;
    unit->worker=std::thread([unit] {
        struct Buffers { UInt32 n; AudioBuffer b[2]; } buffers{2,{}};
        std::array<float,256> l{},r{};
        buffers.b[0]={1,1024,l.data()}; buffers.b[1]={1,1024,r.data()};
        AudioTimeStamp time{}; time.mFlags=kAudioTimeStampSampleTimeValid|kAudioTimeStampHostTimeValid;
        mach_timebase_info_data_t base{}; assert(mach_timebase_info(&base)==KERN_SUCCESS);
        const double ticksPerSecond=1e9*base.denom/base.numer;
        while(!unit->quit.load()) {
            time.mHostTime=1000000+static_cast<std::uint64_t>(time.mSampleTime*ticksPerSecond/unit->format.mSampleRate);
            AudioUnitRenderActionFlags flags=0;
            assert(unit->callback.inputProc(unit->callback.inputProcRefCon,&flags,&time,0,256,
                reinterpret_cast<AudioBufferList*>(&buffers))==noErr);
            time.mSampleTime+=256;
            std::this_thread::sleep_for(5800us);
        }
        // Deliberately one late callback after control has unpublished the
        // generation. It must use no retired ring/scratch/converter memory.
        AudioUnitRenderActionFlags flags=0;
        time.mHostTime=1000000+static_cast<std::uint64_t>(time.mSampleTime*ticksPerSecond/unit->format.mSampleRate);
        assert(unit->callback.inputProc(unit->callback.inputProcRefCon,&flags,&time,0,256,
            reinterpret_cast<AudioBufferList*>(&buffers))==noErr);
    });
    return reject("start") ? kAudioUnitErr_FailedInitialization : noErr; // partial start error
}
OSStatus stop(AudioUnit ref) {
    auto* unit=reinterpret_cast<Unit*>(ref); unit->quit.store(true);
    if(unit->worker.joinable()) unit->worker.join(); unit->stopped=true; ++closed; return noErr;
}
OSStatus listenUnit(AudioUnit,AudioUnitPropertyID,AudioUnitPropertyListenerProc,void*) { return noErr; }
OSStatus unlistenUnit(AudioUnit,AudioUnitPropertyID,AudioUnitPropertyListenerProc,void*) { return noErr; }
OSStatus getObject(AudioObjectID id,const AudioObjectPropertyAddress* address,UInt32,const void*,UInt32* bytes,void* out) {
    if(address->mSelector==kAudioHardwarePropertyDefaultOutputDevice) return result(out,bytes,AudioDeviceID(device.load()));
    if(address->mSelector==kAudioDevicePropertyDeviceIsAlive) return result(out,bytes,UInt32(device.load()!=0));
    if(address->mSelector==kAudioDevicePropertyBufferFrameSize) return result(out,bytes,UInt32(256));
    if(address->mSelector==kAudioDevicePropertyNominalSampleRate) return result(out,bytes,Float64(id==11 ? 44100 : 48000));
    return result(out,bytes,UInt32(0));
}
OSStatus objectSize(AudioObjectID,const AudioObjectPropertyAddress*,UInt32,const void*,UInt32* bytes) { *bytes=0; return noErr; }
Boolean hasObject(AudioObjectID,const AudioObjectPropertyAddress*) { return true; }
OSStatus listenObject(AudioObjectID id,const AudioObjectPropertyAddress* a,AudioObjectPropertyListenerProc listener,void*) {
    if(reject("listener") && id!=kAudioObjectSystemObject) return kAudioHardwareUnspecifiedError;
    if(id==kAudioObjectSystemObject && a->mSelector==kAudioHardwarePropertyDefaultOutputDevice) {
        std::lock_guard<std::mutex> lock(listenerMutex); systemListener=listener;
    }
    return noErr;
}
OSStatus unlistenObject(AudioObjectID id,const AudioObjectPropertyAddress* a,AudioObjectPropertyListenerProc,void*) {
    if(id==kAudioObjectSystemObject && a->mSelector==kAudioHardwarePropertyDefaultOutputDevice) {
        std::lock_guard<std::mutex> lock(listenerMutex); systemListener=nullptr;
    }
    return noErr;
}
io_connect_t powerRegister(void* ref,IONotificationPortRef* port,IOServiceInterestCallback cb,io_object_t* notifier) {
    *port=IONotificationPortCreate(kIOMainPortDefault); assert(*port);
    powerRef=ref; powerCallback=cb; *notifier=0; return 42; // local notification port only
}
IOReturn powerUnregister(io_object_t*) { powerRef=nullptr; powerCallback=nullptr; return kIOReturnSuccess; }
kern_return_t powerClose(io_connect_t) { return KERN_SUCCESS; }
IOReturn powerAllow(io_connect_t,intptr_t) { return kIOReturnSuccess; }
template<class Predicate> void waitFor(Predicate predicate) {
    const auto end=std::chrono::steady_clock::now()+2s;
    while(!predicate() && std::chrono::steady_clock::now()<end) std::this_thread::sleep_for(1ms);
    assert(predicate());
}
void change(unsigned next) {
    device.store(next);
    std::lock_guard<std::mutex> lock(listenerMutex);
    assert(systemListener);
    AudioObjectPropertyAddress a{kAudioHardwarePropertyDefaultOutputDevice,kAudioObjectPropertyScopeGlobal,kAudioObjectPropertyElementMain};
    assert(systemListener(kAudioObjectSystemObject,1,&a,nullptr)==noErr);
}
}
// Host/guest seams are inert CPU fixtures, not game execution or audio IO.
namespace PetariNative {
HostAllocationScope::HostAllocationScope() {} HostAllocationScope::~HostAllocationScope() {}
namespace Platform::Audio {
std::uint32_t outputRate() { return 32000; }
bool setRealtimeAudioThread() { return true; }
void setTimingDiagnostics(bool) {} void setRegistrationWaitLimit(std::uint32_t) {}
std::uint64_t replayedBlocks() { return 0; }
DmaStats takeDmaStats() { return {}; } TimingStats takeTimingStats() { return {}; }
std::size_t pull(std::int16_t* out,std::size_t count) { for(unsigned n=0;n<count*2;++n) out[n]=1234; return count; }
}
namespace Platform::AudioWorkgroup {
std::atomic<bool> enabled{false},observing{false};
void startCurrentSink(bool,unsigned) {} void stopSink() {} void report(FILE*) {} void collect() {}
void setOutputAUObservation(bool) {} void noteOutputRender(unsigned,std::uint64_t,std::uint64_t,std::uint64_t,std::uint64_t,std::uint64_t) noexcept {}
}
namespace Platform::Power { void pressPowerButton() { ++powerRequests; } }
}
extern "C" int petari_os_try_begin_host_blocking() { return 0; }
extern "C" void petari_os_end_host_blocking() {}
int main() {
    Api api;
    api.component=component; api.create=create; api.dispose=dispose; api.unitGet=getUnit; api.unitSet=setUnit;
    api.initialize=initialize; api.uninitialize=uninitialize; api.start=start; api.stop=stop;
    api.unitListen=listenUnit; api.unitUnlisten=unlistenUnit; api.objectGet=getObject; api.objectSize=objectSize;
    api.objectHas=hasObject; api.objectListen=listenObject; api.objectUnlisten=unlistenObject;
    api.powerRegister=powerRegister; api.powerUnregister=powerUnregister; api.powerClose=powerClose; api.powerAllow=powerAllow;
    assert(setApiForTest(api));
    AU::start(32000); assert(AU::active()); assert(!setApiForTest(api));
    std::this_thread::sleep_for(20ms);
    change(11); std::this_thread::sleep_for(40ms); assert(created.load()==1); // unchanged notification does not reopen
    change(12); waitFor([] { return created.load()>=2 && disposed.load()>=1 && AU::active(); });
    auto old=created.load();
    assert(powerCallback); powerCallback(powerRef,0,kIOMessageSystemWillSleep,nullptr);
    waitFor([] { return !AU::active(); });
    powerCallback(powerRef,0,kIOMessageSystemHasPoweredOn,nullptr);
    waitFor([&] { return created.load()>old && AU::active(); });
    AU::stop(); AU::stop(); assert(!AU::active() && created.load()==disposed.load());
    AU::start(32000,true); assert(AU::outputMuted() && AU::active()); AU::stop(); assert(!AU::outputMuted());
    // No real default is ever changed. Every failed partial-open stage goes
    // through the same production close protocol, including a partial start.
    for(const auto* stage:{"create","maximum","callback","initialize","listener","start"}) {
        rejected=stage; AU::start(32000); AU::stop(); assert(created.load()==disposed.load());
    }
    rejected="dispose"; AU::start(32000); AU::stop();
    assert(created.load()==disposed.load()+1); // explicitly retained terminal owner
    std::puts("fake-device actual AU control / default+sleep recovery / partial-open / partial-start / safe failed-retire PASS");
}
