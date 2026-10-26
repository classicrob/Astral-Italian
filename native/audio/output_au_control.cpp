// All Apple lifecycle, converter, scheduling, clocks and I/O belong here,
// outside output_au_render.cpp. No SDL tree change is needed.
#include <petari/audio_output_au.hpp>
#include <petari/host_allocation.hpp>
#include <petari/platform/audio.hpp>
#include <petari/platform/audio_workgroup.hpp>
#include <petari/platform/os_host.hpp>
#include <petari/platform/power.hpp>
#include "output_au_render.hpp"
#include "output_au_converter.hpp"
#include "output_au_lifecycle.hpp"
#include "output_au_backend.hpp"
#include "output_au_observer.hpp"
#include "paced_ring.hpp"
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <IOKit/IOMessage.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <mach/mach_time.h>
#include <pthread/qos.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

namespace PetariNative::AudioOutputAU {
namespace {
using namespace Detail;
Api api;
namespace WG=Platform::AudioWorkgroup;
using Clock=std::chrono::steady_clock;
ChangeMailbox mailbox;
std::atomic<bool> publicRunning{false}, publicHealthy{true}, publicMuted{false};
std::atomic<std::uint64_t> publicSubmitted{0};
std::uint64_t nextGeneration=0;
std::uint32_t timeNumer=0,timeDenom=0;
std::int64_t steadyOffset=0;
double processSeconds() {
    kinfo_proc info{}; std::size_t bytes=sizeof(info); int mib[]={CTL_KERN,KERN_PROC,KERN_PROC_PID,getpid()};
    timeval now{};
    if(sysctl(mib,4,&info,&bytes,nullptr,0) || !bytes || gettimeofday(&now,nullptr)) return -1;
    return now.tv_sec-info.kp_proc.p_starttime.tv_sec+(now.tv_usec-info.kp_proc.p_starttime.tv_usec)/1e6;
}
std::uint64_t steadyNs() { return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count(); }
void calibrateClock(std::uint64_t generation) {
    const auto before=observerClock(),ns=steadyNs(),after=observerClock();
    const auto middle=before+(after-before)/2;
    const auto scaled=(middle/timeDenom)*timeNumer+(middle%timeDenom)*timeNumer/timeDenom;
    steadyOffset=static_cast<std::int64_t>(ns)-static_cast<std::int64_t>(scaled);
    configureObserverClock(timeNumer,timeDenom,steadyOffset);
    std::fprintf(stderr,"[audio-au-clock-calibration] generation=%llu mach_before=%llu mach_after=%llu steady_ns=%llu offset_ns=%lld "
        "bound_ticks=%llu mapping=per-device-generation-after-quiescence\n",(unsigned long long)generation,
        (unsigned long long)before,(unsigned long long)after,(unsigned long long)ns,(long long)steadyOffset,(unsigned long long)(after-before));
}
std::uint64_t ticksForUs(std::uint64_t us) { return us*1000*timeDenom/timeNumer; }
template<class T> OSStatus getDevice(AudioObjectID id, AudioObjectPropertySelector selector,
    AudioObjectPropertyScope scope,T& out) {
    AudioObjectPropertyAddress a{selector,scope,kAudioObjectPropertyElementMain}; UInt32 n=sizeof(T);
    const auto error=api.objectGet(id,&a,0,nullptr,&n,&out);
    return error ? error : n==sizeof(T) ? noErr : kAudioHardwareBadPropertySizeError;
}
AudioDeviceID defaultOutput() {
    AudioDeviceID id=0;
    return getDevice(kAudioObjectSystemObject,kAudioHardwarePropertyDefaultOutputDevice,kAudioObjectPropertyScopeGlobal,id) ? 0 : id;
}
OSStatus changed(AudioObjectID,UInt32 count,const AudioObjectPropertyAddress* addresses,void* token) {
    const auto tag=reinterpret_cast<std::uintptr_t>(token);
    if(mailbox.enabled.load(std::memory_order_relaxed) && (!tag || tag==mailbox.live.load(std::memory_order_acquire))) {
        for(UInt32 n=0;n<count;++n) {
            if(addresses[n].mSelector==kAudioDeviceProcessorOverload) mailbox.overloads.fetch_add(1,std::memory_order_relaxed);
            if(addresses[n].mSelector==kAudioHardwarePropertyServiceRestarted) mailbox.services.fetch_add(1,std::memory_order_relaxed);
        }
        mailbox.notify(tag);
    }
    return noErr;
}
void unitChanged(void* token,AudioUnit,AudioUnitPropertyID,AudioUnitScope,AudioUnitElement) { mailbox.notify(reinterpret_cast<std::uintptr_t>(token)); }

struct Generation {
    AudioUnit unit=nullptr;
    RenderOwner owner;
    RenderData render;
    Converter converter;
    ProductionLedger ledger;
    AudioDeviceID device=0;
    double rate=0;
    double reportedHardwareMs=-1,reportedAUMs=-1;
    UInt32 ioFrames=0;
    AudioStreamBasicDescription hardwareFormat{},clientFormat{};
    unsigned guestRate=0;
    bool initialized=false,started=false;
    const char* openSite="default-device";
    OSStatus openError=0;
    std::vector<AudioObjectPropertyAddress> listeners;
    std::vector<AudioUnitPropertyID> unitListeners;
    std::atomic<bool> producerStop{false};
    std::thread producer;
    std::atomic<std::uint64_t> sourcePulled{0},sourceAccepted{0},deviceProduced{0};
    std::atomic<std::uint64_t> producerGapUs{0},producerPullUs{0};
    std::uint64_t submittedBase=0,oldRaw=0,oldReplay=0;
    std::uint64_t lastCallbacks=0;
    ShortfallUnits units;
    Clock::time_point lastProgress=Clock::now(),lastReport=Clock::now();
};
struct Runtime {
    unsigned guestRate=0;
    double cushionMs=48;
    bool observe=false,mute=false;
    std::atomic<bool> stop{false},sleeping{false};
    std::atomic<std::uint64_t> powerEpoch{0};
    std::thread control;
    std::mutex startMutex;
    std::condition_variable startWake;
    bool attempted=false;
    std::unique_ptr<Generation> generation;
    std::uint64_t transitions=0,healthErrors=0,totalRaw=0,totalGuest=0;
    unsigned faultRecoveries=0;
    std::uint64_t globalMinimum=UINT64_MAX;
    double globalMinimumMs=std::numeric_limits<double>::infinity();
    Clock::time_point lastSample=Clock::now();
    io_connect_t powerRoot=0;
};
std::unique_ptr<Runtime> runtime; // install/start/stop serialized by the sink owner

void failure(Runtime& r,const char* site,OSStatus error) {
    ++r.healthErrors; publicHealthy.store(false,std::memory_order_release);
    std::fprintf(stderr,"[audio-au-fault] site=%s error=%d generation=%llu health_errors=%llu action=no-silent-fallback\n",
        site,int(error),(unsigned long long)(r.generation ? r.generation->render.generation : 0),(unsigned long long)r.healthErrors);
}
void produce(Generation& g) {
    HostAllocationScope allocation;
    if(!Platform::Audio::setRealtimeAudioThread()) pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE,0);
    AudioSDL::Detail::Pacer pacer(g.guestRate);
    std::array<std::int16_t,Converter::kInputFrames*2> source{};
    auto previous=Clock::now(),next=previous;
    while(!g.producerStop.load(std::memory_order_acquire)) {
        const auto now=Clock::now();
        const auto gap=std::chrono::duration_cast<std::chrono::microseconds>(now-previous).count();
        if(gap>static_cast<std::int64_t>(g.producerGapUs.load())) g.producerGapUs.store(gap);
        const auto allowance=pacer.allowance(std::chrono::duration<double>(now-previous).count(),
            g.render.ring.level()<g.render.ring.lowWater(g.rate,g.guestRate));
        previous=now;
        if(!g.converter.convert(g.render.ring)) { g.render.faults.fetch_or(ConverterError); break; }
        // Reserve output before advancing AI. The converter can hold data but
        // cannot authorize extra source pulls or overwrite pending output.
        const auto reserve=static_cast<std::uint64_t>(std::ceil(Converter::kInputFrames*g.rate/g.guestRate))+32;
        const auto wanted=g.ledger.wanted(g.render.ring,allowance,g.guestRate/1000);
        if(wanted && !g.converter.hasPendingInput() && g.render.ring.space()>=reserve) {
            const auto got=Platform::Audio::pull(source.data(),wanted);
            if(got>wanted || (got && !g.converter.push(source.data(),static_cast<unsigned>(got)))) {
                g.render.faults.fetch_or(ConverterError); break;
            }
            g.ledger.pulled+=got;
            if(g.render.observing && got) {
                const auto stamp=mach_absolute_time(),ns=steadyNs();
                WG::noteOutputRender(54,stamp,ns,0,g.render.generation,g.ledger.pulled);
            }
            if(!g.converter.convert(g.render.ring)) { g.render.faults.fetch_or(ConverterError); break; }
        }
        g.ledger.accepted=g.converter.consumed(); g.ledger.produced=g.converter.output();
        g.sourcePulled.store(g.ledger.pulled); g.sourceAccepted.store(g.ledger.accepted); g.deviceProduced.store(g.ledger.produced);
        if(g.render.observing && wanted) {
            const auto stamp=mach_absolute_time(),ns=steadyNs();
            WG::noteOutputRender(50,stamp,ns,0,g.render.generation,g.ledger.produced);
            WG::noteOutputRender(52,stamp,ns,0,g.render.generation,g.ledger.accepted);
        }
        const auto pull=std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-now).count();
        if(pull>static_cast<std::int64_t>(g.producerPullUs.load())) g.producerPullUs.store(pull);
        next=std::max(next+std::chrono::milliseconds(1),Clock::now());
        std::this_thread::sleep_until(next);
    }
}

void latency(Generation& g) {
    std::fprintf(stderr,"[audio-au-latency] generation=%llu rate=%.9f client_channels=%u max_frames=%zu io_buffer_frames=%u "
        "ring_base_frames=%llu ring_base_ms=%.6f aq_buffers=0 converter=normal-high prime_leading=%u prime_trailing=%u "
        "end_to_end=estimate-not-acoustic latency_ledger=source_enqueue-54_converter-50_52_consume-53_sample-48_host-49\n",(unsigned long long)g.render.generation,g.rate,g.render.ring.channels(),
        g.render.maxFrames,g.ioFrames,(unsigned long long)g.render.ring.initialFrames(),
        g.render.ring.initialFrames()*1000/g.rate,g.converter.primeInfo().leadingFrames,g.converter.primeInfo().trailingFrames);
    CFStringRef uid=nullptr;
    const auto uidError=getDevice(g.device,kAudioDevicePropertyDeviceUID,kAudioObjectPropertyScopeGlobal,uid);
    char uidText[1024]{};
    const bool uidReadable=!uidError && uid && CFStringGetCString(uid,uidText,sizeof(uidText),kCFStringEncodingUTF8);
    std::fprintf(stderr,"[audio-au-device] generation=%llu id=%u uid=%s status=%d uid_readable=%d\n",
        (unsigned long long)g.render.generation,g.device,uidReadable ? uidText : "unknown",int(uidError),uidReadable);
    if(!uidError && uid) CFRelease(uid);
    bool hardwareKnown=true; std::uint64_t hardwareFrames=0;
    for(const auto selector:std::array<AudioObjectPropertySelector,3>{kAudioDevicePropertyLatency,kAudioDevicePropertySafetyOffset,kAudioDevicePropertyBufferFrameSize}) {
        UInt32 frames=0;
        const auto status=getDevice(g.device,selector,selector==kAudioDevicePropertyBufferFrameSize ?
            kAudioObjectPropertyScopeGlobal : kAudioObjectPropertyScopeOutput,frames);
        std::fprintf(stderr,"[audio-au-latency-component] device=%u selector=%u status=%d value=%u unit=device_frames rate=%.9f\n",
            g.device,unsigned(selector),int(status),frames,g.rate);
        hardwareKnown=hardwareKnown && !status; if(!status) hardwareFrames+=frames;
    }
    AudioObjectPropertyAddress a{kAudioDevicePropertyStreams,kAudioObjectPropertyScopeOutput,kAudioObjectPropertyElementMain};
    UInt32 bytes=0; auto status=api.objectSize(g.device,&a,0,nullptr,&bytes);
    bool streamKnown=false; UInt32 maxStream=0;
    if(!status && bytes && bytes%sizeof(AudioStreamID)==0) {
        std::vector<AudioStreamID> streams(bytes/sizeof(AudioStreamID));
        status=api.objectGet(g.device,&a,0,nullptr,&bytes,streams.data());
        streamKnown=!status;
        bool anyUsed=false;
        if(!status) for(auto stream:streams) {
            UInt32 frames=0; const auto e=getDevice(stream,kAudioStreamPropertyLatency,kAudioObjectPropertyScopeGlobal,frames);
            UInt32 first=0; AudioStreamBasicDescription format{};
            const auto startError=getDevice(stream,kAudioStreamPropertyStartingChannel,kAudioObjectPropertyScopeGlobal,first);
            const auto formatError=getDevice(stream,kAudioStreamPropertyVirtualFormat,kAudioObjectPropertyScopeGlobal,format);
            // Default stereo/mono map uses destination channels 1..N. Streams
            // are parallel paths, never summed. An unknown map stays unknown.
            const bool known=!startError && !formatError && first && format.mChannelsPerFrame &&
                g.hardwareFormat.mChannelsPerFrame==g.render.ring.channels();
            const bool used=known && first<=g.render.ring.channels();
            streamKnown=streamKnown && known && (!used || !e);
            if(used) { maxStream=std::max(maxStream,frames); anyUsed=true; }
            std::fprintf(stderr,"[audio-au-stream-latency] device=%u stream=%u status=%d frames=%u rate=%.9f "
                "first_channel=%u channels=%u map_known=%d used=%d path=default-channel-map-parallel-max\n",
                g.device,stream,int(e),frames,g.rate,first,format.mChannelsPerFrame,known,used);
        }
        streamKnown=streamKnown && anyUsed;
    }
    Float64 au=0; UInt32 n=sizeof(au);
    const auto e=api.unitGet(g.unit,kAudioUnitProperty_Latency,kAudioUnitScope_Global,0,&au,&n);
    std::fprintf(stderr,"[audio-au-latency-component] au_latency_seconds=%.9f status=%d overlap=do-not-sum-unverified\n",au,int(e));
    if(hardwareKnown && streamKnown) g.reportedHardwareMs=(hardwareFrames+maxStream)*1000/g.rate;
    if(!e && std::isfinite(au) && au>=0) g.reportedAUMs=au*1000;
    const auto mach=mach_absolute_time(),ns=steadyNs();
    std::fprintf(stderr,"[audio-au-clock] generation=%llu mach=%llu steady_ns=%llu numer=%u denom=%u offset_ns=%lld "
        "callback_clock=mach_absolute_time-fast-path-codegen-required callback_metadata=no-kernel-queries\n",
        (unsigned long long)g.render.generation,(unsigned long long)mach,(unsigned long long)ns,timeNumer,timeDenom,(long long)steadyOffset);
}

bool addListener(Generation& g,AudioObjectPropertySelector selector,AudioObjectPropertyScope scope) {
    AudioObjectPropertyAddress a{selector,scope,kAudioObjectPropertyElementMain};
    if(!api.objectHas(g.device,&a)) return false;
    if(api.objectListen(g.device,&a,changed,reinterpret_cast<void*>(g.render.generation))) return false;
    g.listeners.push_back(a); return true;
}
bool open(Runtime& r,Generation& g) {
    const auto check=[&](const char* site,OSStatus status) {
        g.openSite=site; g.openError=status; return status!=noErr;
    };
    g.guestRate=r.guestRate; g.render.generation=++nextGeneration;
    g.device=defaultOutput(); if(!g.device) return false;
    UInt32 alive=0;
    if(check("device-alive",getDevice(g.device,kAudioDevicePropertyDeviceIsAlive,kAudioObjectPropertyScopeGlobal,alive)) || !alive) return false;
    AudioComponentDescription description{kAudioUnitType_Output,kAudioUnitSubType_DefaultOutput,kAudioUnitManufacturer_Apple,0,0};
    const auto component=api.component(nullptr,&description);
    if(!component || check("instance-new",api.create(component,&g.unit))) return false;
    AudioDeviceID current=0; UInt32 n=sizeof(current);
    if(check("current-device",api.unitGet(g.unit,kAudioOutputUnitProperty_CurrentDevice,kAudioUnitScope_Global,0,&current,&n)) || current!=g.device) return false;
    // DefaultOutput is output-only by default. Do not enable/open input even
    // on hardware with a microphone; no TCC/input/device-global mutations.
    UInt32 enabled=1;
    if(check("enable-output",api.unitSet(g.unit,kAudioOutputUnitProperty_EnableIO,kAudioUnitScope_Output,0,&enabled,sizeof(enabled)))) return false;
    n=sizeof(enabled);
    if(check("read-output-enabled",api.unitGet(g.unit,kAudioOutputUnitProperty_EnableIO,kAudioUnitScope_Output,0,&enabled,&n)) || !enabled) return false;
    AudioStreamBasicDescription hardware{}; n=sizeof(hardware);
    if(check("hardware-format",api.unitGet(g.unit,kAudioUnitProperty_StreamFormat,kAudioUnitScope_Output,0,&hardware,&n))) return false;
    g.rate=hardware.mSampleRate;
    g.hardwareFormat=hardware;
    if(!std::isfinite(g.rate) || g.rate<8000 || g.rate>192000 || !hardware.mChannelsPerFrame) return false;
    unsigned channels=hardware.mChannelsPerFrame==1 ? 1 : 2;
    AudioStreamBasicDescription client{};
    client.mSampleRate=g.rate; client.mFormatID=kAudioFormatLinearPCM;
    client.mFormatFlags=kAudioFormatFlagIsFloat|kAudioFormatFlagIsPacked|kAudioFormatFlagIsNonInterleaved|kAudioFormatFlagsNativeEndian;
    client.mBytesPerPacket=4; client.mFramesPerPacket=1; client.mBytesPerFrame=4;
    client.mChannelsPerFrame=channels; client.mBitsPerChannel=32;
    g.clientFormat=client;
    if(check("client-format",api.unitSet(g.unit,kAudioUnitProperty_StreamFormat,kAudioUnitScope_Input,0,&client,sizeof(client)))) return false;
    AudioStreamBasicDescription readback{}; n=sizeof(readback);
    if(check("client-readback",api.unitGet(g.unit,kAudioUnitProperty_StreamFormat,kAudioUnitScope_Input,0,&readback,&n)) ||
        readback.mSampleRate!=client.mSampleRate || readback.mFormatID!=client.mFormatID ||
        readback.mFormatFlags!=client.mFormatFlags || readback.mChannelsPerFrame!=channels ||
        readback.mBytesPerFrame!=4 || readback.mFramesPerPacket!=1) return false;
    UInt32 maximum=0; n=sizeof(maximum);
    if(check("maximum-frames",api.unitGet(g.unit,kAudioUnitProperty_MaximumFramesPerSlice,kAudioUnitScope_Global,0,&maximum,&n)) ||
        !maximum || maximum>16384 || getDevice(g.device,kAudioDevicePropertyBufferFrameSize,kAudioObjectPropertyScopeGlobal,g.ioFrames) ||
        !g.ioFrames || g.ioFrames>maximum) return false;
    const auto base=static_cast<std::size_t>(std::ceil(r.cushionMs*g.rate/1000));
    std::size_t capacity=1024;
    while(capacity<base+maximum+4096) capacity*=2;
    if(!g.render.ring.configure(capacity,base,channels)) return false;
    g.render.maxFrames=maximum; g.render.scratch.reset(new float[std::size_t(maximum)*channels]{});
    g.render.mute=r.mute; g.render.observing=r.observe;
    g.render.hostTicksPerFrame=1e9*double(timeDenom)/timeNumer/g.rate;
    g.render.gapTriggerTicks=ticksForUs(20000); g.render.clock=observerClock; g.render.publish=publishRenderEvent;
    g.openSite="producer-converter";
    if(!g.converter.open(r.guestRate,g.rate,channels,4096) || !g.converter.prewarm()) { g.openError=g.converter.error(); return false; }
    g.ledger.guestRate=r.guestRate; g.ledger.deviceRate=g.rate;
    AURenderCallbackStruct callback{render,&g.owner};
    if(check("set-callback",api.unitSet(g.unit,kAudioUnitProperty_SetRenderCallback,kAudioUnitScope_Input,0,&callback,sizeof(callback)))) return false;
    if(check("initialize",api.initialize(g.unit))) return false;
    g.initialized=true;
    n=sizeof(maximum); UInt32 finalMaximum=0;
    if(check("maximum-readback",api.unitGet(g.unit,kAudioUnitProperty_MaximumFramesPerSlice,kAudioUnitScope_Global,0,&finalMaximum,&n)) ||
        finalMaximum>g.render.maxFrames || !finalMaximum) return false;
    g.render.maxFrames=finalMaximum;
    for(const auto selector:std::array<AudioObjectPropertySelector,5>{kAudioDevicePropertyDeviceIsAlive,kAudioDevicePropertyNominalSampleRate,
            kAudioDevicePropertyBufferFrameSize,kAudioDevicePropertyStreamConfiguration,kAudioDevicePropertyStreams})
        if(!addListener(g,selector,selector==kAudioDevicePropertyStreams || selector==kAudioDevicePropertyStreamConfiguration ?
            kAudioObjectPropertyScopeOutput : kAudioObjectPropertyScopeGlobal)) return false;
    const AudioObjectPropertyAddress overload{kAudioDeviceProcessorOverload,kAudioObjectPropertyScopeGlobal,kAudioObjectPropertyElementMain};
    if(api.objectHas(g.device,&overload)) {
        if(!addListener(g,kAudioDeviceProcessorOverload,kAudioObjectPropertyScopeGlobal)) return false;
    }
    for(const auto property:std::array<AudioUnitPropertyID,3>{kAudioOutputUnitProperty_CurrentDevice,kAudioUnitProperty_MaximumFramesPerSlice,kAudioUnitProperty_StreamFormat}) {
        if(api.unitListen(g.unit,property,unitChanged,reinterpret_cast<void*>(g.render.generation))) return false;
        g.unitListeners.push_back(property);
    }
    if(defaultOutput()!=g.device) return false;
    const double wait=std::clamp(r.cushionMs-g.ioFrames*1000/g.rate-3,0.0,40.0);
    Platform::Audio::setRegistrationWaitLimit(static_cast<std::uint32_t>(wait*1000));
    std::fprintf(stderr,"[audio-au-open] generation=%llu device=%u source_rate=%u device_rate=%.9f channels=%u "
        "io_frames=%u max_frames=%zu base_ms=%.3f wait_us=%u input_enabled=0 setter_calls=0\n",
        (unsigned long long)g.render.generation,g.device,r.guestRate,g.rate,channels,g.ioFrames,g.render.maxFrames,
        r.cushionMs,static_cast<unsigned>(wait*1000));
    calibrateClock(g.render.generation); latency(g);
    g.submittedBase=publicSubmitted.load();
    g.oldReplay=Platform::Audio::replayedBlocks();
    mailbox.live.store(g.render.generation,std::memory_order_release);
    g.owner.published.store(&g.render,std::memory_order_seq_cst);
    g.producer=std::thread(produce,std::ref(g));
    if(check("output-start",api.start(g.unit))) return false;
    g.started=true;
    if(defaultOutput()!=g.device) return false;
    g.lastProgress=Clock::now();
    publicRunning.store(true,std::memory_order_release);
    std::fprintf(stderr,"[audio-backend] driver=coreaudio sink=output-au subtype=DefaultOutput source_rate=%u device_rate=%.9f\n",r.guestRate,g.rate);
    return true;
}

bool sameFormat(const AudioStreamBasicDescription& a,const AudioStreamBasicDescription& b) {
    return a.mSampleRate==b.mSampleRate && a.mFormatID==b.mFormatID && a.mFormatFlags==b.mFormatFlags &&
        a.mBytesPerPacket==b.mBytesPerPacket && a.mFramesPerPacket==b.mFramesPerPacket &&
        a.mBytesPerFrame==b.mBytesPerFrame && a.mChannelsPerFrame==b.mChannelsPerFrame && a.mBitsPerChannel==b.mBitsPerChannel;
}
bool configurationMatches(Generation& g) {
    // Notifications may describe our own start or repeat an unchanged value.
    // Re-read, don't endlessly reopen on a harmless property notification.
    if(defaultOutput()!=g.device) return false;
    UInt32 alive=0,quantum=0; Float64 rate=0;
    if(getDevice(g.device,kAudioDevicePropertyDeviceIsAlive,kAudioObjectPropertyScopeGlobal,alive) || !alive ||
       getDevice(g.device,kAudioDevicePropertyBufferFrameSize,kAudioObjectPropertyScopeGlobal,quantum) || quantum!=g.ioFrames ||
       getDevice(g.device,kAudioDevicePropertyNominalSampleRate,kAudioObjectPropertyScopeGlobal,rate) || rate!=g.rate) return false;
    AudioStreamBasicDescription hardware{},client{}; UInt32 n=sizeof(hardware);
    if(api.unitGet(g.unit,kAudioUnitProperty_StreamFormat,kAudioUnitScope_Output,0,&hardware,&n) || !sameFormat(hardware,g.hardwareFormat)) return false;
    n=sizeof(client);
    if(api.unitGet(g.unit,kAudioUnitProperty_StreamFormat,kAudioUnitScope_Input,0,&client,&n) || !sameFormat(client,g.clientFormat)) return false;
    UInt32 maximum=0; n=sizeof(maximum);
    return !api.unitGet(g.unit,kAudioUnitProperty_MaximumFramesPerSlice,kAudioUnitScope_Global,0,&maximum,&n) && maximum==g.render.maxFrames;
}

void report(Runtime& r,Generation& g,bool final) {
    const auto now=Clock::now();
    const auto interval=std::chrono::duration<double>(now-g.lastReport).count(); g.lastReport=now;
    const auto raw=g.render.shortfalls.load(),delta=raw-g.oldRaw;
    g.oldRaw=raw; const auto guest=g.units.take(raw,g.guestRate,g.rate);
    r.totalRaw+=delta; r.totalGuest+=guest;
    r.globalMinimum=std::min(r.globalMinimum,g.render.ring.minimumLevel());
    r.globalMinimumMs=std::min(r.globalMinimumMs,g.render.ring.minimumLevel()*1000/g.rate);
    const auto replay=Platform::Audio::replayedBlocks();
    const auto consumed=g.render.ring.consumed();
    const auto pulledSnapshot=g.sourcePulled.load();
    const auto consumedAfter=g.render.ring.consumed();
    const double sourceTailDevice=pulledSnapshot*g.rate/g.guestRate+g.render.ring.initialFrames();
    const double aheadLow=std::max(0.0,sourceTailDevice-consumedAfter)*1000/g.rate;
    const double aheadHigh=std::max(0.0,sourceTailDevice-consumed)*1000/g.rate;
    publicSubmitted.store(g.submittedBase+static_cast<std::uint64_t>(consumed*g.guestRate/g.rate));
    if(g.started && g.render.callbacks.load()) {
        std::fprintf(stderr,"[audio] level %llu/%llu frames, largest request %llu, underrun %llu frames, AI replayed %llu blocks "
            "(since previous report, %.2f s; at %.3f s process time)\n",
            (unsigned long long)(g.render.ring.level()*g.guestRate/g.rate),
            (unsigned long long)(g.render.ring.target()*g.guestRate/g.rate),
            (unsigned long long)std::ceil(g.render.ring.largestRequest()*g.guestRate/g.rate),
            (unsigned long long)guest,(unsigned long long)(replay-g.oldReplay),interval,processSeconds());
    }
    g.oldReplay=replay;
    std::fprintf(stderr,"[audio-au-latency-estimate] generation=%llu software_ahead_low_ms=%.6f software_ahead_high_ms=%.6f "
        "reported_hardware_ms=%.6f reported_au_ms=%.6f components_known=%d converter_hold=included-in-source-ahead "
        "estimate_low_ms=%.6f estimate_high_ms=%.6f au_overlap=interval-not-double-counted "
        "continuation=steady-production acoustic_measurement=0\n",(unsigned long long)g.render.generation,aheadLow,aheadHigh,
        g.reportedHardwareMs,g.reportedAUMs,int(g.reportedHardwareMs>=0 && g.reportedAUMs>=0),
        g.reportedHardwareMs>=0 && g.reportedAUMs>=0 ? aheadLow+std::max(g.reportedHardwareMs,g.reportedAUMs) : -1,
        g.reportedHardwareMs>=0 && g.reportedAUMs>=0 ? aheadHigh+g.reportedHardwareMs+g.reportedAUMs : -1);
    const auto dma=Platform::Audio::takeDmaStats(); const auto timing=Platform::Audio::takeTimingStats();
    const auto faults=g.render.faults.load()|g.owner.faults.load();
    if(faults || raw) publicHealthy.store(false,std::memory_order_release);
    std::fprintf(stderr,"[audio-au] generation=%llu final=%d device_rate=%.9f guest_rate=%u underrun_unit=guest-equivalent-cumulative-ceil "
        "raw_shortfall_device_frames=%llu raw_shortfall_delta=%llu guest_fraction=%.12Lf callbacks=%llu requested_device_frames=%llu "
        "copied_device_frames=%llu consumed_device_frames=%llu source_pulled=%llu source_accepted=%llu device_produced=%llu "
        "ring_device_frames=%llu ring_min_device_frames=%llu ring_min_ms=%.6f ring_min_basis=consumer-snapshot target_device_frames=%llu faults=%llu "
        "timeline_faults=%llu host_timeline_faults=%llu host_delta_ticks=%llu host_expected_ticks=%llu invalid_timestamps=%llu overloads=%llu healthy=%d "
        "sample_time_bits=%llu presentation_host_time=%llu callback_gap_ticks=%llu producer_gap_us=%llu producer_pull_us=%llu "
        "dma_timeout=%llu dsp_holds=%llu ai_deliver_us=%lld registration_us=%lld oldest_registration_us=%lld "
        "dsp_subframe_us=%lld dsp_frame_us=%lld\n",
        (unsigned long long)g.render.generation,final,g.rate,g.guestRate,(unsigned long long)raw,(unsigned long long)delta,g.units.remainder(),
        (unsigned long long)g.render.callbacks.load(),(unsigned long long)g.render.requested.load(),(unsigned long long)g.render.copied.load(),
        (unsigned long long)consumed,(unsigned long long)g.sourcePulled.load(),(unsigned long long)g.sourceAccepted.load(),
        (unsigned long long)g.deviceProduced.load(),(unsigned long long)g.render.ring.level(),
        (unsigned long long)g.render.ring.minimumLevel(),g.render.ring.minimumLevel()*1000/g.rate,
        (unsigned long long)g.render.ring.target(),(unsigned long long)faults,
        (unsigned long long)g.render.timelineFaults.load(),(unsigned long long)g.render.hostTimelineFaults.load(),
        (unsigned long long)g.render.badHostDelta.load(),(unsigned long long)g.render.expectedHostDelta.load(),(unsigned long long)g.render.invalidTimestamps.load(),
        (unsigned long long)mailbox.overloads.load(),int(publicHealthy.load()),
        (unsigned long long)g.render.sampleBits.load(),(unsigned long long)g.render.hostTime.load(),
        (unsigned long long)g.render.maxGapTicks.load(),(unsigned long long)g.producerGapUs.load(),
        (unsigned long long)g.producerPullUs.load(),(unsigned long long)dma.waitTimeouts,(unsigned long long)dma.dspHolds,
        (long long)timing.dmaDeliverUs,(long long)timing.latestRaiseToRegisterUs,(long long)timing.oldestRaiseToRegisterUs,
        (long long)timing.subframeRenderUs,(long long)timing.frameUs);
    WG::report(stderr); WG::collect();
}

bool close(Runtime& r,std::unique_ptr<Generation>& value) {
    if(!value) return true;
    auto& g=*value; publicRunning.store(false,std::memory_order_release); mailbox.live.store(0,std::memory_order_release);
    Retirement retiring; retiring.begin(g.owner);
    g.producerStop.store(true,std::memory_order_release);
    if(g.producer.joinable()) g.producer.join();
    retiring.stopped=!g.unit || !g.initialized || api.stop(g.unit)==noErr;
    const auto deadline=Clock::now()+std::chrono::seconds(2);
    while(g.owner.readers.load(std::memory_order_seq_cst) && Clock::now()<deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    bool listenersOk=true;
    for(auto& address:g.listeners)
        listenersOk=(api.objectUnlisten(g.device,&address,changed,reinterpret_cast<void*>(g.render.generation))==noErr)&&listenersOk;
    for(auto property:g.unitListeners)
        listenersOk=(api.unitUnlisten(g.unit,property,unitChanged,reinterpret_cast<void*>(g.render.generation))==noErr)&&listenersOk;
    if(retiring.mayUninitialize(g.owner)) {
        retiring.uninitialized=!g.initialized || api.uninitialize(g.unit)==noErr;
        retiring.disposed=!g.unit || (retiring.uninitialized && api.dispose(g.unit)==noErr);
    }
    if(!retiring.mayReclaim(g.owner)) {
        failure(r,"unproven-callback-quiescence",kAudioUnitErr_CannotDoInCurrentContext);
        // Intentional retained owner on a terminal fault, never a UAF or a
        // continuing leak per retry. The controller stops after this failure.
        value.release(); return false;
    }
    if(g.rate>0 && g.render.ring.capacity()) report(r,g,true);
    if(!listenersOk) std::fputs("[audio-au-transition] stale listener removal failed; numeric stale tokens ignored\n",stderr);
    value.reset(); return true;
}

void power(void* ref,io_service_t,natural_t message,void* argument) {
    auto& r=*static_cast<Runtime*>(ref); // executed only by this control CFRunLoop
    if(message==kIOMessageSystemWillSleep) { r.sleeping.store(true); ++r.powerEpoch; mailbox.notify(0); }
    if(message==kIOMessageSystemHasPoweredOn) { r.sleeping.store(false); ++r.powerEpoch; mailbox.notify(0); }
    if(message==kIOMessageCanSystemSleep || message==kIOMessageSystemWillSleep)
        api.powerAllow(r.powerRoot,reinterpret_cast<intptr_t>(argument));
}
void controller(Runtime& r) {
    HostAllocationScope allocation;
    IONotificationPortRef port=nullptr; io_object_t notifier=0;
    r.powerRoot=api.powerRegister(&r,&port,power,&notifier);
    if(!r.powerRoot || !port) failure(r,"power-listener",kAudioHardwareUnspecifiedError);
    auto source=port ? IONotificationPortGetRunLoopSource(port) : nullptr;
    if(source) CFRunLoopAddSource(CFRunLoopGetCurrent(),source,kCFRunLoopDefaultMode);
    const AudioObjectPropertyAddress globals[]={
        {kAudioHardwarePropertyDefaultOutputDevice,kAudioObjectPropertyScopeGlobal,kAudioObjectPropertyElementMain},
        {kAudioHardwarePropertyServiceRestarted,kAudioObjectPropertyScopeGlobal,kAudioObjectPropertyElementMain}};
    unsigned registered=0;
    for(const auto& a:globals) {
        if(api.objectListen(kAudioObjectSystemObject,&a,changed,nullptr)) { failure(r,"system-listener",kAudioHardwareUnspecifiedError); break; }
        ++registered;
    }
    auto seen=mailbox.changes.load(),seenPower=r.powerEpoch.load(),seenService=mailbox.services.load(); unsigned attempts=0;
    const unsigned backoff[]={100,250,500,1000,2000};
    auto retryAt=Clock::now(); bool terminal=r.healthErrors!=0;
    while(!r.stop.load(std::memory_order_acquire)) {
        const auto serial=mailbox.changes.load(std::memory_order_acquire);
        if(serial!=seen) {
            seen=serial;
            const auto currentPower=r.powerEpoch.load(),currentService=mailbox.services.load();
            const auto faults=r.generation ? r.generation->render.faults.load() : 0;
            const bool forced=currentPower!=seenPower || currentService!=seenService || (faults&(NoCallbacks|DeviceError|ConverterError));
            seenPower=currentPower; seenService=currentService;
            if(!r.generation || forced || !configurationMatches(*r.generation)) {
                ++r.transitions;
                if(faults && ++r.faultRecoveries>3) { failure(r,"repeated-runtime-recovery",kAudioUnitErr_FailedInitialization); terminal=true; }
                if(!close(r,r.generation)) terminal=true;
                std::fprintf(stderr,"[audio-au-transition] request=%llu transitions=%llu sleeping=%d stale_pcm=discarded-not-hidden reset_ai=0\n",
                    (unsigned long long)serial,(unsigned long long)r.transitions,int(r.sleeping.load()));
                attempts=0; retryAt=Clock::now();
            }
        }
        if(!terminal && !r.sleeping.load() && !r.generation && Clock::now()>=retryAt) {
            r.generation=std::make_unique<Generation>();
            if(!open(r,*r.generation)) {
                const auto device=defaultOutput();
                std::fprintf(stderr,"[audio-au-recovery] attempt=%u device=%u status=open-failed site=%s error=%d\n",attempts+1,device,
                    r.generation->openSite,int(r.generation->openError));
                if(!close(r,r.generation)) terminal=true;
                if(attempts>=5) { failure(r,"device-reopen-exhausted",kAudioHardwareUnspecifiedError); terminal=true; }
                else retryAt=Clock::now()+std::chrono::milliseconds(backoff[attempts++]);
            } else {
                attempts=0;
                std::fprintf(stderr,"[audio-au-transition] state=running generation=%llu\n",(unsigned long long)r.generation->render.generation);
            }
            { std::lock_guard<std::mutex> lock(r.startMutex); r.attempted=true; } r.startWake.notify_one();
        }
        if(terminal) {
            { std::lock_guard<std::mutex> lock(r.startMutex); r.attempted=true; } r.startWake.notify_one();
            std::fputs("[audio-au-fault] action=graceful-power-request no-stuck-silence=1\n",stderr);
            Platform::Power::pressPowerButton(); break;
        }
        if(auto* g=r.generation.get()) {
            if(Platform::Audio::outputRate()!=r.guestRate) {
                failure(r,"guest-rate-changed-without-source-restart",kAudioUnitErr_FormatNotSupported);
                terminal=true;
            }
            const auto count=g->render.callbacks.load(std::memory_order_acquire);
            const auto now=Clock::now();
            if(count!=g->lastCallbacks) { g->lastCallbacks=count; g->lastProgress=now; }
            // Coarse independent arrival watchdog in BOTH observer modes.
            // Allow initial startup, then two actual slices + one poll, at
            // least 50ms. Large native allocations are NOT the I/O cadence.
            const auto grace=count ? std::max(.050,2*g->render.lastFrames.load()/g->rate+.025) : 2.0;
            if(std::chrono::duration<double>(now-g->lastProgress).count()>grace) {
                g->render.faults.fetch_or(NoCallbacks); failure(r,"render-callback-stopped",kAudioUnitErr_NoConnection);
                mailbox.notify(g->render.generation);
            }
            if(g->render.faults.load() || g->owner.faults.load() || mailbox.overloads.load()) publicHealthy.store(false);
            if(g->render.faults.load()&ConverterError) mailbox.notify(g->render.generation);
            OSStatus renderError=0; UInt32 size=sizeof(renderError);
            const auto query=api.unitGet(g->unit,kAudioUnitProperty_LastRenderError,kAudioUnitScope_Global,0,&renderError,&size);
            if(query || renderError) {
                g->render.faults.fetch_or(DeviceError); failure(r,"unit-render-error",query ? query : renderError);
                mailbox.notify(g->render.generation);
            }
            if(now-g->lastReport>=std::chrono::seconds(1)) report(r,*g,false);
        }
        CFRunLoopRunInMode(kCFRunLoopDefaultMode,0.025,true);
    }
    mailbox.live.store(0); mailbox.enabled.store(false);
    close(r,r.generation);
    for(unsigned n=0;n<registered;++n) api.objectUnlisten(kAudioObjectSystemObject,&globals[n],changed,nullptr);
    if(r.powerRoot) { api.powerUnregister(&notifier); api.powerClose(r.powerRoot); }
    if(source) CFRunLoopRemoveSource(CFRunLoopGetCurrent(),source,kCFRunLoopDefaultMode);
    if(port) IONotificationPortDestroy(port);
    std::fprintf(stderr,"[audio-au-final] complete=1 raw_shortfall_device_frames=%llu guest_equivalent_frames=%llu "
        "health_errors=%llu healthy=%d transitions=%llu ring_min_device_frames=%llu ring_min_ms=%.6f default_device_setter_calls=0\n",
        (unsigned long long)r.totalRaw,(unsigned long long)r.totalGuest,(unsigned long long)r.healthErrors,int(publicHealthy.load()),
        (unsigned long long)r.transitions,(unsigned long long)(r.globalMinimum==UINT64_MAX ? 0 : r.globalMinimum),
        std::isfinite(r.globalMinimumMs) ? r.globalMinimumMs : -1.0);
}
}

void start(std::uint32_t rate,bool backgroundMuted) {
    HostAllocationScope allocation;
    struct Release { int value=petari_os_try_begin_host_blocking(); ~Release() { if(value) petari_os_end_host_blocking(); } } release;
    stop();
    auto r=std::make_unique<Runtime>(); r->guestRate=rate;
    r->observe=WG::observing.load(std::memory_order_acquire);
    r->mute=backgroundMuted; // facade resolves the SAME SDL hint on control path
    // Old test-only callback file capture cannot be honored by a copies-only
    // RT sink. Refuse explicitly rather than perform RT I/O or silently ignore.
    if((rate!=32000 && rate!=48000) || std::getenv("PETARI_AUDIO_CAPTURE") || std::getenv("PETARI_AUDIO_AQ_BUFFERS_DIAG")) {
        std::fputs("[audio-au-fault] unsupported source rate/callback-file-capture/AQ override\n",stderr); std::abort();
    }
    if(const char* cushion=std::getenv("PETARI_AUDIO_BUFFER_MS")) {
        char* end=nullptr; const auto value=std::strtod(cushion,&end);
        if(!end || *end || !std::isfinite(value) || value<8 || value>128) std::abort(); r->cushionMs=value;
    }
    mach_timebase_info_data_t time{};
    if(mach_timebase_info(&time) || !time.numer || !time.denom) std::abort();
    timeNumer=time.numer; timeDenom=time.denom;
    (void)observerClock();
    publicRunning.store(false); publicHealthy.store(true); publicMuted.store(r->mute);
    mailbox.overloads.store(0); mailbox.services.store(0); mailbox.live.store(0); mailbox.enabled.store(true);
    Platform::Audio::setTimingDiagnostics(true);
    WG::setOutputAUObservation(true);
    if(WG::enabled.load(std::memory_order_relaxed)) WG::startCurrentSink(true,rate); // actual AI-start caller
    runtime=std::move(r);
    runtime->control=std::thread(controller,std::ref(*runtime));
    std::unique_lock<std::mutex> lock(runtime->startMutex);
    runtime->startWake.wait(lock,[]{ return runtime->attempted; });
}
void stop() {
    if(!runtime) return;
    HostAllocationScope allocation;
    struct Release { int value=petari_os_try_begin_host_blocking(); ~Release() { if(value) petari_os_end_host_blocking(); } } release;
    runtime->stop.store(true,std::memory_order_release);
    if(runtime->control.joinable()) runtime->control.join();
    if(WG::enabled.load(std::memory_order_relaxed)) { WG::stopSink(); WG::report(stderr); WG::collect(); }
    WG::setOutputAUObservation(false);
    Platform::Audio::setTimingDiagnostics(false);
    runtime.reset(); publicRunning.store(false); publicMuted.store(false);
}
bool active() { return publicRunning.load(std::memory_order_acquire) && publicHealthy.load(std::memory_order_acquire); }
bool outputMuted() { return publicMuted.load(std::memory_order_relaxed); }
std::uint64_t submittedFrames() { return publicSubmitted.load(std::memory_order_relaxed); }
#ifdef PETARI_OUTPUT_AU_TESTING
namespace Detail {
bool setApiForTest(const Api& value) { if(runtime) return false; api=value; return true; }
}
#endif
}
