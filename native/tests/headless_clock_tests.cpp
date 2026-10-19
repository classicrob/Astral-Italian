#include <petari/platform/virtual_time.hpp>
#include <petari/platform/vi.hpp>
#include <petari/platform/os_host.hpp>
#include <petari/floating_point.hpp>
#include "fp_mode.hpp"
#include <revolution/os.h>
#include <revolution/vi.h>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
extern "C" void __OSThreadInit(void);
namespace OS=PetariNative::Platform::OS;
namespace VI=PetariNative::Platform::VI;
namespace {
std::vector<OSTime> fired;
void check(bool value,const char* message){if(!value){std::fprintf(stderr,"%s\n",message);std::exit(1);}}
void alarm(OSAlarm*,OSContext*){fired.push_back(__OSGetSystemTime());}
void wait(void*){std::thread interrupt([]{OS::advanceVirtualTimeTo(OS::virtualSystemTime()+100);VI::signalRetrace();});interrupt.join();}
void* threadMode(void*){return reinterpret_cast<void*>(PetariNative::Headless::FloatingPointMode::flushToZero()?1:2);}
}
int main(){
    OS::enableVirtualTime(60750000);
    __OSThreadInit();
    check(OSGetTime()==60750000 && OSGetTick()==60750000 && __OSGetSystemTime()==0,"fixed clock epoch");
    OSAlarm periodic;OSCreateAlarm(&periodic);OSSetPeriodicAlarm(&periodic,OSGetTime()+10,10,alarm);
    std::thread clock([]{OS::advanceVirtualTimeTo(35);});clock.join();
    check(fired==std::vector<OSTime>({10,20,30}),"all periodic alarm deadlines execute without wall-clock waits");
    check(OSGetTime()==60750035,"OSGetTime uses virtual elapsed ticks");OSCancelAlarm(&periodic);
    VI::setClock(VI::Clock::External);VI::setWaitDriver({wait,nullptr});VIInit();
    VIWaitForRetrace();check(VIGetRetraceCount()==1 && __OSGetSystemTime()==135,"virtual retrace wait releases OS CPU");
    for(bool flush:{true,false}) {
        PetariNative::FloatingPointThreadPolicy policy(flush);
        OSThread thread{};alignas(32) unsigned char stack[32768];
        check(OSCreateThread(&thread,threadMode,nullptr,stack+sizeof(stack),sizeof(stack),15,0),"create FP policy OS thread");
        OSResumeThread(&thread);void* result=nullptr;
        check(OSJoinThread(&thread,&result),"join FP policy OS thread");
        check(result==reinterpret_cast<void*>(flush?1:2),"OS game thread applies selected FP mode");
    }
    VI::shutdown();std::puts("headless virtual time and VI checks passed");
}
