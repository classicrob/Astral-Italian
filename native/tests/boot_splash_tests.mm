#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <SDL3/SDL.h>
#include "boot_splash.h"
#include <atomic>
#include <cstdio>
#include <thread>
#include <chrono>
#include <unistd.h>
namespace { SDL_Window* testWindow; }
namespace aurora::window { SDL_Window* get_sdl_window() { return testWindow; } }
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "failed %d: %s\n", __LINE__, #x); return 1; } } while (0)
int main(int argc, char**) {
    @autoreleasepool {
        CHECK(SDL_Init(SDL_INIT_VIDEO));
        testWindow = SDL_CreateWindow("Petari boot splash unit",640,480,SDL_WINDOW_HIDDEN);
        CHECK(testWindow);
        NSWindow* cocoa = (__bridge NSWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(testWindow),SDL_PROP_WINDOW_COCOA_WINDOW_POINTER,nullptr);
        CHECK(cocoa);
        const auto before=cocoa.contentView.subviews.count;
        if (argc > 1) {
            SDL_SetHint("PETARI_SMOKE_BACKGROUND","1");
            petari_boot_splash_progress(1,2);
            CHECK((SDL_GetWindowFlags(testWindow)&SDL_WINDOW_HIDDEN)!=0);
            CHECK(cocoa.contentView.subviews.count==before);
        } else {
            petari_boot_splash_progress(1,2);
            CHECK((SDL_GetWindowFlags(testWindow)&SDL_WINDOW_HIDDEN)==0);
            CHECK(cocoa.contentView.subviews.count==before+1);
            NSView* overlay=cocoa.contentView.subviews.lastObject;
            bool text=false,bar=false;
            for(NSView* view in overlay.subviews) {
                if([view isKindOfClass:NSTextField.class]) text=[((NSTextField*)view).stringValue isEqualToString:@"Preparing graphics… 50%"];
                if([view isKindOfClass:NSProgressIndicator.class]) bar=((NSProgressIndicator*)view).doubleValue==.5;
            }
            CHECK(text && bar);
            std::atomic<bool> done=false, replied=false;
            std::thread sender([&] {
                @autoreleasepool {
                    pid_t pid=getpid();
                    auto* target=[NSAppleEventDescriptor descriptorWithDescriptorType:typeKernelProcessID bytes:&pid length:sizeof(pid)];
                    auto* event=[NSAppleEventDescriptor appleEventWithEventClass:'Ptra' eventID:'ping' targetDescriptor:target returnID:kAutoGenerateReturnID transactionID:kAnyTransactionID];
                    NSError* error=nil;
                    auto* reply=[event sendEventWithOptions:NSAppleEventSendWaitForReply | NSAppleEventSendNeverInteract timeout:.5 error:&error];
                    replied = !error && [reply paramDescriptorForKeyword:keyDirectObject].int32Value==0x50455441;
                    done=true;
                }
            });
            const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
            while(!done && std::chrono::steady_clock::now()<deadline) { SDL_PumpEvents(); SDL_Delay(1); }
            sender.join(); CHECK(replied);
            petari_boot_splash_presented();
            for(int i=0;i<100 && cocoa.contentView.subviews.count!=before;++i) { SDL_PumpEvents(); SDL_Delay(1); }
            CHECK(cocoa.contentView.subviews.count==before);
        }
        SDL_DestroyWindow(testWindow); SDL_Quit();
    }
    std::puts("native boot splash visibility / determinate progress / OS event reply / handoff PASS");
    return 0;
}
