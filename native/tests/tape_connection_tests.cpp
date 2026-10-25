// Actual native WPAD state/callbacks at the raw KPAD hook; no game or renderer.
#include <petari/input.hpp>
#include <petari/kpad_replay.hpp>
#include <petari/tape_input_observer.hpp>
#include <revolution/kpad/KPADInternal.h>
#include <revolution/os.h>
#include <revolution/wpad.h>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

extern "C" void __OSThreadInit(void);
extern KPADInsideStatus inside_kpads[4];
namespace In = PetariNative::Input;
namespace {
void check(bool ok, const char* why) {
    if (!ok) { std::fprintf(stderr,"FAIL: %s\n",why); std::exit(1); }
}
std::vector<std::string> events;
void connected(s32 channel, s32 error) {
    check(OSDisableInterrupts()==FALSE,"connect callback holds interrupt lock");
    events.push_back("connect:"+std::to_string(channel)+":"+std::to_string(error));
}
void extension(s32 channel, s32 device) {
    check(OSDisableInterrupts()==FALSE,"extension callback holds interrupt lock");
    events.push_back("extension:"+std::to_string(channel)+":"+std::to_string(device));
}
void setup() {
    In::resetForTesting(); In::TapeObserver::install({}); events.clear();
}
void callbacks() {
    WPADSetConnectCallback(0,connected); WPADSetExtensionCallback(0,extension);
}
struct Source {
    s32 error=WPAD_ERR_NO_CONTROLLER;
    u32 device=WPAD_DEV_NOT_FOUND;
    int reads=0, observed=0;
};
bool connection(s32 channel,s32* error,u32* device,void* user) {
    check(channel==0,"expected channel");
    auto& source=*static_cast<Source*>(user);
    *error=source.error; *device=source.device; return true;
}
s32 read(s32 channel,KPADStatus* out,u32 capacity,s32* error,void* user) {
    auto& source=*static_cast<Source*>(user); u32 actual=0;
    check(WPADProbe(channel,&actual)==source.error && actual==source.device,
          "LIVE WPAD connection exact BEFORE sample return, independent of feeder");
    check(capacity==120,"capacity unchanged"); ++source.reads;
    std::memset(out,0,sizeof(*out));out->hold=0xabc;out->dev_type=source.device;
    *error=source.error==0 ? 0 : -2; return source.error==0 ? 1 : 0;
}
void observe(s32,u32,s32,s32,s32 error,u32 device,const void*,void* user) {
    auto& source=*static_cast<Source*>(user);
    check(error==source.error && device==source.device,"actual post-read observation is exact");
    ++source.observed;
}
void take(Source& source,s32 error,u32 device) {
    source.error=error;source.device=device;
    KPADStatus samples[120]{};s32 result=-99,reported=-99;
    check(petari_kpad_replay_read(0,samples,120,&reported,&result)==TRUE,"raw hook handled");
    check(result==(error==0?1:0) && reported==(error==0?0:-2),"raw return/count exact");
    check(samples[0].hold==0xabc && samples[0].dev_type==device,"actual sample unchanged");
}
}
int main() {
    __OSThreadInit();
    // The live GameRead -> WPADProbe -> tickChannel path establishes the
    // callback order/arguments, including extension detach and remote detach.
    setup(); In::setClock(In::Clock::GameRead); WPADInit();callbacks();
    for(int i=0;i<60;++i) {
        inside_kpads[0].readLocked=TRUE;WPADProbe(0,nullptr);inside_kpads[0].readLocked=FALSE;
    }
    check(events==std::vector<std::string>({"connect:0:0","extension:0:1"}),"live attach order and arguments");
    In::setNunchukAttached(0,false);
    for(int i=0;i<5;++i) {
        inside_kpads[0].readLocked=TRUE;WPADProbe(0,nullptr);inside_kpads[0].readLocked=FALSE;
    }
    In::setConnected(0,false);
    inside_kpads[0].readLocked=TRUE;WPADProbe(0,nullptr);inside_kpads[0].readLocked=FALSE;
    const auto live=events;
    check(live==std::vector<std::string>({"connect:0:0","extension:0:1","extension:0:0","connect:0:-1"}),"complete live transition trace");

    setup();Source source;
    In::setKpadReplayHooks({read,&source,connection});In::setClock(In::Clock::Manual);WPADInit();callbacks();
    In::TapeObserver::install({nullptr,observe,&source});
    // Deliberately prevent the former feeder from running until AFTER the
    // extension's exact read. No production wait or sleep is used by the hook.
    std::mutex mutex;std::condition_variable cv;bool release=false;
    std::thread delayed([&] {
        {std::unique_lock<std::mutex> lock(mutex);cv.wait(lock,[&]{return release;});}
        In::replayConnection(0,0,WPAD_DEV_CORE); // stale frame10 update must be ignored
        In::serviceReplayCommands();
    });
    take(source,-1,WPAD_DEV_NOT_FOUND);
    take(source,0,WPAD_DEV_CORE); // frame10
    const BOOL enabled=OSDisableInterrupts();
    take(source,0,WPAD_DEV_FREESTYLE); // frame11, nested interrupt lock is safe
    OSRestoreInterrupts(enabled);
    {std::lock_guard<std::mutex> lock(mutex);release=true;}cv.notify_one();delayed.join();
    u32 actual=0;
    check(WPADProbe(0,&actual)==0 && actual==WPAD_DEV_FREESTYLE,"delayed feeder cannot regress published state");
    take(source,0,WPAD_DEV_FREESTYLE); // frame12, no duplicate callback
    take(source,0,WPAD_DEV_CORE);
    take(source,-1,WPAD_DEV_NOT_FOUND);
    check(events==live,"synchronous replay callback ORDER AND ARGUMENTS equal live GameRead");
    check(source.reads==6 && source.observed==6,"all actual reads observed");
    // Registration after connection is delivered on the next read, once.
    WPADSetConnectCallback(0,nullptr);WPADSetExtensionCallback(0,nullptr);
    take(source,0,WPAD_DEV_FREESTYLE);events.clear();callbacks();
    take(source,0,WPAD_DEV_FREESTYLE);take(source,0,WPAD_DEV_FREESTYLE);
    check(events==std::vector<std::string>({"connect:0:0","extension:0:1"}),"late callbacks delivered once at read");
    // Neutral/post-roll changes are real state, not metadata substitution.
    take(source,-1,WPAD_DEV_NOT_FOUND);take(source,0,WPAD_DEV_FREESTYLE);
    check(WPADProbe(0,&actual)==0 && actual==1,"neutral/post-roll connection applied");
    // Command completion cannot asynchronously move the recorded connection.
    WPADDisconnect(0);
    std::thread commands([]{In::serviceReplayCommands();});commands.join();
    check(WPADProbe(0,&actual)==0 && actual==1,"command service cannot replace tape connection");

    // Null provider remains the legacy host-thread path; no automatic state
    // update is performed by the sample hook.
    setup();source={};
    In::setKpadReplayHooks({read,&source});In::setClock(In::Clock::Manual);WPADInit();
    take(source,-1,WPAD_DEV_NOT_FOUND);
    std::thread old([]{In::replayConnection(0,0,WPAD_DEV_FREESTYLE);});old.join();
    take(source,0,WPAD_DEV_FREESTYLE);
    setup(); // live-off hook falls through untouched
    s32 error=77,result=88;KPADStatus sample{};
    check(petari_kpad_replay_read(0,&sample,1,&error,&result)==FALSE && error==77 && result==88,"live off path unchanged");
    std::puts("tape synchronous connection, delayed feeder, live callback parity, nested lock and legacy/off PASS");
}
