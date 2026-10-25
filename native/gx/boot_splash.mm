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
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.0, true);
        SDL_PumpEvents();
    }
    return true;
}
extern "C" void petari_boot_splash_progress(std::size_t published, std::size_t total) {
    if (![NSThread isMainThread] || SDL_GetHintBoolean("PETARI_SMOKE_BACKGROUND", false)) return;
    if (shown) return;
    @autoreleasepool {
        auto* window = aurora::window::get_sdl_window();
        NSWindow* cocoa = ownedWindow();
        if (!window || !cocoa || !SDL_ShowWindow(window)) { logEvent("failed", published, total); return; }
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
