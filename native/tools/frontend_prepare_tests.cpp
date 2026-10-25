// The actual normal-frame frontend loop with a deterministic event/GPU clock.
// No timing tolerance, GPU, or game thread. Compiled/run on the server.
#include <petari/pipeline_startup.hpp>
#include "../app/host.hpp"
#include "../app/smoke_background.hpp"
#include "../gx/boot_splash.h"
#include <aurora/aurora.h>
#include <aurora/event.h>
#include <SDL3/SDL.h>
#include <cstdio>
#include <stdexcept>
#include <vector>
namespace {
Uint64 now, releasedAt, readyAfter, quitAt;
unsigned releases, finishes, frames, pumps, drawn;
bool readyWindow, compileFailed, exitEvent;
std::vector<Uint64> seams;
void check(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
void reset(Uint64 duration=100000000) {
    now=releasedAt=0; readyAfter=duration; quitAt=0;
    releases=finishes=frames=pumps=drawn=0; readyWindow=true;compileFailed=exitEvent=false;seams.clear();
    PetariBootSplash::active=false;PetariBootSplash::presented=0;PetariBootSplash::lastPresentNs=0;
}
}
extern "C" Uint64 SDL_GetTicksNS() { return now; }
extern "C" void SDL_DelayNS(Uint64 ns) { now+=ns; }
extern "C" bool SDL_GetWindowSize(SDL_Window*,int* w,int* h) { *w=1280;*h=720;return true; }
extern "C" const AuroraEvent* aurora_update() {
    static AuroraEvent events[2];events[0]={};events[1]={};
    if(quitAt && now>=quitAt) events[0].type=AURORA_EXIT;
    return events;
}
extern "C" bool aurora_begin_frame() { return true; }
extern "C" void aurora_end_frame() {
    ++frames;now+=1000000;
    PetariBootSplash::lastPresentNs=now;
    PetariBootSplash::presented.fetch_add(1);
}
extern "C" void petari_boot_splash_progress(std::size_t,std::size_t) { PetariBootSplash::active=true; }
extern "C" bool petari_boot_splash_pump() { ++pumps;return true; }
extern "C" bool petari_boot_splash_window_ready() { return readyWindow; }
extern "C" void petari_boot_splash_finish() { PetariBootSplash::active=false; }
extern "C" bool petari_gx_frontend_armed() { return true; }
extern "C" void petari_gx_frontend_begin() {
    check(frames>=1 && readyWindow && pumps>0,"Generic jobs started before visible normal presentation");
    ++releases;releasedAt=now;
}
extern "C" void petari_gx_frontend_status(unsigned* t,unsigned* p,unsigned* f,double* elapsed) {
    *t=54;*p=releases && now-releasedAt>=readyAfter?0:54;*f=compileFailed?1:0;
    *elapsed=releases?(now-releasedAt)/1e6:0;
}
extern "C" bool petari_gx_frontend_finish(unsigned count,unsigned n,double,bool main) {
    ++finishes;check(count==frames && n==pumps && main,"lost presentations or pump evidence");return true;
}
namespace PetariNative::HomeMenu {
void drawPreparingGraphics(float p,float,float,float w,float h) {
    check(p>=0 && p<=1 && w==1280 && h==720,"invalid determinate progress");++drawn;
}
}
namespace PetariNative::App::Seam {
void recordStartupPresent(std::uint64_t ns) { seams.push_back(ns); }
std::uint64_t startupFrameCount() { return seams.empty()?0:seams.size()-1; }
}
int main() try {
    auto* window=reinterpret_cast<SDL_Window*>(1);
    reset();check(PetariNative::App::prepareFrontend(window),"normal prep failed");
    check(releases==1 && frames>=3 && frames==drawn && seams.size()==frames && finishes==1,"lost frame coverage");
    for(size_t i=1;i<seams.size();++i)check(seams[i]-seams[i-1]>=16666666 && seams[i]-seams[i-1]<=18666666,"splash not normally paced");
    reset(0);check(PetariNative::App::prepareFrontend(window) && frames==3,"warm must still present visible evidence");
    reset(10000000000ull);check(PetariNative::App::prepareFrontend(window),"cap should restore scheduler, not kill app");
    check(now-releasedAt>=5000000000ull && now-releasedAt<5030000000ull && finishes==1,"lost hard cap");
    reset();readyWindow=false;check(PetariNative::App::prepareFrontend(window) && releases==0 && finishes==1,"unready window released jobs");
    reset();quitAt=20000000;check(!PetariNative::App::prepareFrontend(window) && finishes==1,"quit ignored");
    reset();PetariNative::App::SmokeBackground::enabled=true;
    check(PetariNative::App::prepareFrontend(window) && !frames && !releases && finishes==1,"background smoke became visible");
    std::puts("Normal frontend frames: first-present release, 60Hz, exact CSV coverage, warm evidence, cap, quit, hidden PASS");
} catch(const std::exception& e) {std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
