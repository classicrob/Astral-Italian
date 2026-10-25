#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <SDL3/SDL.h>
#include "window.hpp"
#include "boot_splash.h"
#include <chrono>
#include <cstdio>
#include <unistd.h>
namespace {
bool shown = false;
NSWindow* ownedWindow() {
    auto* window = aurora::window::get_sdl_window();
    return window ? (__bridge NSWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(window),
        SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr) : nil;
}
void logEvent(const char* event, std::size_t published, std::size_t total) {
    const auto mono = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    const auto wall = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::fprintf(stderr, "[gx boot splash] event=%s published=%zu total=%zu monotonic_ns=%lld wall_ns=%lld mode=normal-frames\n",
        event, published, total, (long long)mono, (long long)wall);
}
}
@interface PetariBootHeartbeat : NSObject
- (void)ping:(NSAppleEventDescriptor*)event reply:(NSAppleEventDescriptor*)reply;
@end
@implementation PetariBootHeartbeat
- (void)ping:(NSAppleEventDescriptor*)event reply:(NSAppleEventDescriptor*)reply {
    [reply setParamDescriptor:[NSAppleEventDescriptor descriptorWithInt32:0x50455441] forKeyword:keyDirectObject];
}
@end
extern "C" bool petari_boot_splash_pump() {
    if (![NSThread isMainThread]) return false;
    @autoreleasepool {
        // A real run-loop turn services AppleEvents and main-queue work, in
        // addition to SDL's NSEvent drain. Never asks for Automation consent.
        static bool first = true;
        const bool trace = first; first = false;
        if (trace) PetariBootSplash::startupEvent("first_run_loop_enter");
        { PetariBootSplash::SlowCall span{"run_loop"}; CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.0, true); }
        if (trace) PetariBootSplash::startupEvent("first_run_loop_exit");
        if (trace) PetariBootSplash::startupEvent("first_sdl_pump_enter");
        { PetariBootSplash::SlowCall span{"sdl_pump"}; SDL_PumpEvents(); }
        if (trace) PetariBootSplash::startupEvent("first_sdl_pump_exit");
    }
    return true;
}
extern "C" bool petari_boot_splash_hidden() {
    auto* window = aurora::window::get_sdl_window();
    return window && (SDL_GetWindowFlags(window) & SDL_WINDOW_HIDDEN);
}
extern "C" bool petari_boot_splash_show_window() {
    PetariBootSplash::SlowCall span{"window_show"};
    if (![NSThread isMainThread] || SDL_GetHintBoolean("PETARI_SMOKE_BACKGROUND", false)) return false;
    auto* window = aurora::window::get_sdl_window();
    if (!window || !ownedWindow()) return false;
    PetariBootSplash::startupEvent("window_show_enter");
    const bool success = SDL_ShowWindow(window);
    PetariBootSplash::startupEvent(success ? "window_shown" : "window_show_failed");
    PetariBootSplash::deferWindowShow = false;
    return success;
}
extern "C" void petari_boot_splash_progress(std::size_t published, std::size_t total) {
    if (![NSThread isMainThread] || SDL_GetHintBoolean("PETARI_SMOKE_BACKGROUND", false)) return;
    if (shown) return;
    @autoreleasepool {
        auto* window = aurora::window::get_sdl_window();
        NSWindow* cocoa = ownedWindow();
        if (!window || !cocoa || !petari_boot_splash_show_window()) { logEvent("failed", published, total); return; }
        static PetariBootHeartbeat* heartbeat = [PetariBootHeartbeat new];
        [[NSAppleEventManager sharedAppleEventManager] setEventHandler:heartbeat andSelector:@selector(ping:reply:)
            forEventClass:'Ptra' andEventID:'ping'];
        shown = true;
        PetariBootSplash::active = true;
        logEvent("shown", published, total);
        std::fprintf(stderr, "[gx boot splash window] id=%ld pid=%d\n", (long)cocoa.windowNumber, getpid());
    }
}
extern "C" bool petari_boot_splash_window_ready() {
    PetariBootSplash::SlowCall span{"window_ready"};
    if (![NSThread isMainThread] || !shown) return false;
    @autoreleasepool {
        NSWindow* window = ownedWindow();
        if (!window || !NSRunningApplication.currentApplication.isFinishedLaunching) return false;
        NSArray* entries = CFBridgingRelease(CGWindowListCopyWindowInfo(kCGWindowListOptionIncludingWindow,
            (CGWindowID)window.windowNumber));
        for (NSDictionary* entry in entries) {
            if ([entry[(id)kCGWindowNumber] longValue] != window.windowNumber ||
                [entry[(id)kCGWindowOwnerPID] intValue] != getpid()) continue;
            CGRect bounds = CGRectZero;
            const bool boundsOK = CGRectMakeWithDictionaryRepresentation((__bridge CFDictionaryRef)entry[(id)kCGWindowBounds], &bounds);
            return [entry[(id)kCGWindowIsOnscreen] boolValue] && [entry[(id)kCGWindowAlpha] doubleValue] == 1.0 &&
                boundsOK && bounds.size.width > 0 && bounds.size.height > 0;
        }
    }
    return false;
}
extern "C" void petari_boot_splash_presented(std::uint64_t frame) {
    if (!PetariBootSplash::active.load()) return;
    if (frame == 0) PetariBootSplash::startupEvent("first_successful_present");
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    PetariBootSplash::lastPresentNs.store(ns, std::memory_order_relaxed);
    std::fprintf(stderr, "[gx boot splash present] frame=%llu uptime_ns=%llu success=1\n",
        (unsigned long long)frame, (unsigned long long)ns);
    PetariBootSplash::presented.fetch_add(1, std::memory_order_release);
}
extern "C" void petari_boot_splash_finish() {
    PetariBootSplash::active = false;
    if (shown) logEvent("handed_off", 0, 0);
}
