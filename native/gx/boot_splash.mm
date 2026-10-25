#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <SDL3/SDL.h>
#include "window.hpp"
#include "boot_splash.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <unistd.h>

namespace {
NSView* overlay;
NSTextField* label;
NSProgressIndicator* bar;
std::atomic<bool> shown{false}, handedOff{false};
void logEvent(const char* event, std::size_t published, std::size_t total) {
    const auto mono = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    const auto wall = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::fprintf(stderr, "[gx boot splash] event=%s published=%zu total=%zu monotonic_ns=%lld wall_ns=%lld\n",
                 event, published, total, (long long)mono, (long long)wall);
}
}
// A no-input, read-only OS event-loop probe addressed by kernel PID. The reply
// can only be produced when Cocoa dispatches this event on the main thread.
@interface PetariBootHeartbeat : NSObject
- (void)ping:(NSAppleEventDescriptor*)event reply:(NSAppleEventDescriptor*)reply;
@end
@implementation PetariBootHeartbeat
- (void)ping:(NSAppleEventDescriptor*)event reply:(NSAppleEventDescriptor*)reply {
    [reply setParamDescriptor:[NSAppleEventDescriptor descriptorWithInt32:0x50455441] forKeyword:keyDirectObject];
}
@end

extern "C" void petari_boot_splash_progress(std::size_t published, std::size_t total) {
    if (![NSThread isMainThread] || SDL_GetHintBoolean("PETARI_SMOKE_BACKGROUND", false) || handedOff.load()) return;
    @autoreleasepool {
        if (!overlay) {
            auto* window = aurora::window::get_sdl_window();
            if (!window) return;
            NSWindow* cocoa = (__bridge NSWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
            if (!cocoa || !SDL_ShowWindow(window)) { logEvent("failed", published, total); return; }
            static PetariBootHeartbeat* heartbeat = [PetariBootHeartbeat new];
            [[NSAppleEventManager sharedAppleEventManager] setEventHandler:heartbeat andSelector:@selector(ping:reply:)
                                                            forEventClass:'Ptra' andEventID:'ping'];
            NSView* content = cocoa.contentView;
            overlay = [[NSView alloc] initWithFrame:content.bounds];
            overlay.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
            overlay.wantsLayer = YES; overlay.layer.backgroundColor = NSColor.blackColor.CGColor;
            const CGFloat width = 320, x = (content.bounds.size.width - width) / 2, y = (content.bounds.size.height - 60) / 2;
            label = [NSTextField labelWithString:@"Preparing graphics… 0%"];
            label.frame = NSMakeRect(x, y + 25, width, 25);
            label.alignment = NSTextAlignmentCenter; label.textColor = NSColor.whiteColor;
            label.font = [NSFont systemFontOfSize:16 weight:NSFontWeightMedium];
            label.autoresizingMask = NSViewMinXMargin | NSViewMaxXMargin | NSViewMinYMargin | NSViewMaxYMargin;
            bar = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(x, y, width, 10)];
            bar.indeterminate = NO; bar.minValue = 0; bar.maxValue = 1;
            bar.autoresizingMask = label.autoresizingMask;
            [overlay addSubview:label]; [overlay addSubview:bar];
            [content addSubview:overlay positioned:NSWindowAbove relativeTo:nil];
            [cocoa displayIfNeeded];
            shown = true; logEvent("shown", published, total);
            std::fprintf(stderr, "[gx boot splash window] id=%ld pid=%d\n", (long)cocoa.windowNumber, getpid());
        }
        const double fraction = total ? std::min(1.0, double(published) / total) : 0;
        label.stringValue = [NSString stringWithFormat:@"Preparing graphics… %u%%", unsigned(fraction * 100)];
        bar.doubleValue = fraction;
        [overlay displayIfNeeded];
    }
}
extern "C" void petari_boot_splash_presented() {
    if (!shown.load() || handedOff.exchange(true)) return;
    dispatch_async(dispatch_get_main_queue(), ^{
        [overlay removeFromSuperview]; overlay = nil; label = nil; bar = nil;
        logEvent("handed_off", 0, 0);
    });
}
