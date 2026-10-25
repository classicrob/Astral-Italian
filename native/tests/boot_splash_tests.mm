#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <SDL3/SDL.h>
#include "boot_splash.h"
#include <cstdio>
#include <atomic>
#include <algorithm>
#include <thread>
#include <string>
#include <spawn.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
namespace { SDL_Window* testWindow; }
namespace aurora::window { SDL_Window* get_sdl_window() { return testWindow; } }
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr,"failed %d: %s\n",__LINE__,#x);return 1; } } while(0)
int sendHeartbeat(pid_t pid) {
        AEAddressDesc target={};AppleEvent event={},reply={};
        auto err=AECreateDesc(typeKernelProcessID,&pid,sizeof(pid),&target);
        if(!err)err=AECreateAppleEvent('Ptra','ping',&target,kAutoGenerateReturnID,kAnyTransactionID,&event);
        if(!err)err=AESendMessage(&event,&reply,kAEWaitReply|kAENeverInteract|kAEDoNotPromptForUserConsent,30);
        SInt32 cookie=0,replyError=0;DescType actual=0;Size size=0,errorSize=0;
        const auto sendStatus=err;
        const auto errorStatus=!err?AEGetParamPtr(&reply,keyErrorNumber,typeSInt32,&actual,&replyError,sizeof(replyError),&errorSize):err;
        if(!err)err=AEGetParamPtr(&reply,keyDirectObject,typeSInt32,&actual,&cookie,sizeof(cookie),&size);
        std::fprintf(stderr,"cross-PID no-prompt heartbeat status=%d send_status=%d reply_type=%08x reply_error_status=%d reply_error=%d cookie=%x size=%ld\n",err,sendStatus,(unsigned)reply.descriptorType,errorStatus,replyError,(unsigned)cookie,(long)size);
        AEDisposeDesc(&reply);AEDisposeDesc(&event);AEDisposeDesc(&target);
        return err==noErr && cookie==0x50455441 && size==4?0:1;
}
int main(int argc,char** argv) {
    if(argc==3 && std::string(argv[1])=="--sender") return sendHeartbeat(std::stoi(argv[2]));
    @autoreleasepool {
        CHECK(SDL_Init(SDL_INIT_VIDEO));
        testWindow=SDL_CreateWindow("Petari normal-frame splash unit",640,480,SDL_WINDOW_HIDDEN);CHECK(testWindow);
        NSWindow* cocoa=(__bridge NSWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(testWindow),SDL_PROP_WINDOW_COCOA_WINDOW_POINTER,nullptr);
        CHECK(cocoa);const auto before=cocoa.contentView.subviews.count;
        if(argc>1) {
            SDL_SetHint("PETARI_SMOKE_BACKGROUND","1");
            petari_boot_splash_progress(1,2);
            CHECK((SDL_GetWindowFlags(testWindow)&SDL_WINDOW_HIDDEN)!=0);
            CHECK(!PetariBootSplash::active && cocoa.contentView.subviews.count==before);
        } else {
            petari_boot_splash_progress(1,2);
            CHECK((SDL_GetWindowFlags(testWindow)&SDL_WINDOW_HIDDEN)==0 && PetariBootSplash::active);
            // AppKit draws no surrogate overlay: actual progress pixels come
            // from the ImGui draw in the submitted Aurora surface texture.
            CHECK(cocoa.contentView.subviews.count==before);
            const auto until=SDL_GetTicksNS()+2000000000;
            while(!petari_boot_splash_window_ready() && SDL_GetTicksNS()<until) { CHECK(petari_boot_splash_pump());SDL_Delay(1); }
            CHECK(petari_boot_splash_window_ready());
            std::string pid=std::to_string(getpid());
            char* childArgs[]={argv[0],const_cast<char*>("--sender"),pid.data(),nullptr};
            pid_t child=0;CHECK(posix_spawn(&child,argv[0],nullptr,nullptr,childArgs,environ)==0);
            int status=0;pid_t ended=0;const auto limit=SDL_GetTicksNS()+2000000000;
            while(!(ended=waitpid(child,&status,WNOHANG)) && SDL_GetTicksNS()<limit) { CHECK(petari_boot_splash_pump());SDL_Delay(1); }
            if(!ended) { kill(child,SIGTERM);waitpid(child,&status,0); }
            // Observation only: -1744 is expected when Automation consent is
            // absent. Never prompt, request permission, or reinterpret as pass.
            std::fprintf(stderr,"cross-PID AppleEvent observation child=%d status=%d; not a gate (TCC)\n",(int)child,status);
            std::atomic<bool> done{false};int selfStatus=-1;
            std::thread self([&] { @autoreleasepool { selfStatus=sendHeartbeat(getpid());done=true; } });
            auto lastTurn=SDL_GetTicksNS();Uint64 maxGap=0;
            const auto selfLimit=lastTurn+2000000000;
            while(!done && SDL_GetTicksNS()<selfLimit) {
                CHECK(petari_boot_splash_pump());
                const auto turn=SDL_GetTicksNS();maxGap=std::max(maxGap,turn-lastTurn);lastTurn=turn;SDL_Delay(1);
            }
            self.join();CHECK(done && selfStatus==0 && maxGap<=100000000);
            CHECK(petari_boot_splash_window_ready());
            petari_boot_splash_presented(0);CHECK(PetariBootSplash::presented==1);
            petari_boot_splash_finish();CHECK(!PetariBootSplash::active);
        }
        SDL_DestroyWindow(testWindow);SDL_Quit();
    }
    std::puts("Native normal splash: window/launch state, self event reply, <=100ms main-loop turns, hidden and handoff PASS (cross-PID observation only)");return 0;
}
