#include "first_run.hpp"
#include "../disc_import/disc_import.hpp"

#import <Cocoa/Cocoa.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>

namespace {
namespace App = PetariNative::App;
namespace DI = PetariNative::DiscImport;
namespace fs = std::filesystem;
bool prompts() {
    const char* value = std::getenv("ASTRAL_FIRSTRUN_PROMPTS");
    return !value || std::strcmp(value, "off") != 0;
}
NSString* string(const std::string& text) { return [NSString stringWithUTF8String:text.c_str()]; }
void activate() {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];
    [NSApp activateIgnoringOtherApps:YES];
}
NSInteger alert(const char* title, const std::string& message, NSArray<NSString*>* buttons) {
    std::fprintf(stderr, "[setup] %s: %s\n", title, message.c_str());
    if (!prompts()) return NSAlertSecondButtonReturn;
    activate();
    NSAlert* panel = [[NSAlert alloc] init];
    panel.messageText = string(title);
    panel.informativeText = string(message);
    for (NSString* button in buttons) [panel addButtonWithTitle:button];
    return [panel runModal];
}
}

@interface AstralSetupAction : NSObject {
@public
    std::atomic<bool>* cancelled;
    App::AppLocations locations;
    bool busy;
}
- (void)cancel:(id)sender;
- (void)choose:(id)sender;
@end

@implementation AstralSetupAction
- (void)cancel:(id)sender { if (cancelled) cancelled->store(true); }
- (void)choose:(id)sender {
    if (busy) return;
    busy = true;
    fs::path disc;
    if (App::FirstRun::chooseDisc(locations, &disc)) {
        alert("Game disc remembered", "Restart Astral Italian to use this game folder. Your current game will keep using its original folder.", @[@"OK"]);
    }
    busy = false;
}
@end

namespace {
DI::Result perform(const fs::path& source, const fs::path& destination, bool folder, std::string* error) {
    activate();
    NSWindow* window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 560, 185)
        styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
    window.releasedWhenClosed = NO;
    window.title = folder ? @"Checking game folder" : @"Importing your game disc";
    NSTextField* file = [NSTextField labelWithString:folder ? @"Checking all game files…" : @"Preparing to import…"];
    file.frame = NSMakeRect(20, 128, 520, 34);
    file.lineBreakMode = NSLineBreakByTruncatingMiddle;
    NSProgressIndicator* bar = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(20, 101, 520, 18)];
    bar.indeterminate = NO;
    bar.minValue = 0;
    bar.maxValue = 1;
    NSTextField* detail = [NSTextField labelWithString:folder ? @"Full validation may take a moment." : @"0 bytes — estimating time left…"];
    detail.frame = NSMakeRect(20, 66, 520, 24);
    NSButton* cancel = [NSButton buttonWithTitle:@"Cancel" target:nil action:@selector(cancel:)];
    cancel.frame = NSMakeRect(438, 18, 104, 32);
    std::atomic<bool> cancelled{false}, done{false};
    AstralSetupAction* action = [[AstralSetupAction alloc] init];
    action->cancelled = &cancelled;
    cancel.target = action;
    for (NSView* view in @[file, bar, detail, cancel]) [window.contentView addSubview:view];
    [window center];
    [window makeKeyAndOrderFront:nil];
    const auto start = std::chrono::steady_clock::now();
    DI::Progress latest;
    std::mutex mutex;
    DI::Result result = DI::Result::Failed;
    std::thread worker([&] {
        @autoreleasepool {
            try {
                if (folder) {
                    result = DI::validateExtracted(source, false, error) ? DI::Result::Ok : DI::Result::Failed;
                } else {
                    // A quit, crash or power loss mid-import leaves the importer's private working folder behind;
                    // it is ours and holds nothing worth keeping, and the player could not be asked to remove it.
                    std::error_code ignored;
                    fs::remove_all(fs::path(destination).concat(".partial"), ignored);
                    result = DI::importDisc(source, destination, [&](const DI::Progress& progress) {
                        std::lock_guard<std::mutex> lock(mutex);
                        latest = progress;
                        return !cancelled.load();
                    }, error);
                }
            } catch (const std::exception& exception) {
                *error = "Could not finish checking or importing your disc. " + std::string(exception.what());
            }
            done.store(true);
        }
    });
    NSModalSession session = [NSApp beginModalSessionForWindow:window];
    while (!done.load()) {
        @autoreleasepool {
            [NSApp runModalSession:session];
            DI::Progress progress;
            { std::lock_guard<std::mutex> lock(mutex); progress = latest; }
            if (!folder) {
                file.stringValue = progress.currentFile.empty() ? @"Preparing to import…" : string(progress.currentFile);
                bar.maxValue = static_cast<double>(std::max<std::uint64_t>(1, progress.totalBytes));
                bar.doubleValue = static_cast<double>(progress.doneBytes);
                const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
                NSString* remaining = @"estimating time left…";
                if (progress.doneBytes && progress.totalBytes >= progress.doneBytes) {
                    const double seconds = elapsed * static_cast<double>(progress.totalBytes - progress.doneBytes) / progress.doneBytes;
                    remaining = [NSString stringWithFormat:@"about %.0f seconds left", seconds];
                }
                detail.stringValue = [NSString stringWithFormat:@"%llu / %llu bytes — %@",
                    static_cast<unsigned long long>(progress.doneBytes), static_cast<unsigned long long>(progress.totalBytes), remaining];
            }
            if (cancelled.load()) { cancel.enabled = NO; detail.stringValue = @"Cancelling…"; }
            [window displayIfNeeded];
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.03]];
        }
    }
    worker.join();
    [NSApp endModalSession:session];
    [window close];
    action->cancelled = nullptr;
    // A late click can arrive during the importer's final validation, after its last callback.
    // The destination was absent/empty on entry; remove only this successful import on cancellation.
    if (cancelled.load() && !folder && result == DI::Result::Ok) fs::remove_all(destination);
    // Folder validation has no cancellable callback; discard its result when cancelled.
    return cancelled.load() ? DI::Result::Cancelled : result;
}
}

namespace PetariNative::App::FirstRun {
void showProblem(const std::string& problem) {
    @autoreleasepool { alert("Astral Italian needs your attention", problem, @[@"OK"]); }
}
bool chooseDisc(const AppLocations& locations, fs::path* disc) {
    @autoreleasepool {
        try {
            while (true) {
                if (prompts()) {
                    const auto answer = alert("Welcome to Astral Italian",
                        "Astral Italian is a fan port. You need your own copy of Super Mario Galaxy 2 for the USA (SB4E01, revision 0). No game data is included.",
                        @[@"Choose Game Disc…", @"Quit", @"How do I get this?"]);
                    if (answer == NSAlertThirdButtonReturn) {
                        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:string(helpUrl)]];
                        continue;
                    }
                    if (answer != NSAlertFirstButtonReturn) return false;
                }
                fs::path source;
                if (const char* pick = std::getenv("ASTRAL_FIRSTRUN_PICK")) {
                    source = [string(pick) stringByExpandingTildeInPath].fileSystemRepresentation;
                } else if (prompts()) {
                    NSOpenPanel* panel = [NSOpenPanel openPanel];
                    panel.title = @"Choose your USA Super Mario Galaxy 2 disc or extracted folder";
                    panel.canChooseFiles = YES;
                    panel.canChooseDirectories = YES;
                    panel.allowsMultipleSelection = NO;
                    // Directories remain selectable with this filter. Probe checks actual bytes, not extensions.
                    NSMutableArray<UTType*>* types = [NSMutableArray array];
                    for (NSString* extension in @[@"iso", @"wbfs", @"rvz", @"wia", @"gcm", @"ciso"]) {
                        UTType* type = [UTType typeWithFilenameExtension:extension];
                        if (type) [types addObject:type];
                    }
                    panel.allowedContentTypes = types;
                    if ([panel runModal] != NSModalResponseOK) continue;
                    source = panel.URL.fileSystemRepresentation;
                } else {
                    std::fputs("[setup] Choose a disc with ASTRAL_FIRSTRUN_PICK when prompts are off.\n", stderr);
                    return false;
                }
                source = fs::absolute(source);
                const auto probe = DiscImport::probe(source);
                std::string error = probe.problem;
                bool folder = probe.format == DiscImport::Format::ExtractedFolder;
                if (error.empty() && !folder) {
                    fs::create_directories(locations.gameData().parent_path());
                    // Wii images can be compressed; reserve a single-layer disc plus working headroom.
                    const auto required = std::max<std::uint64_t>(probe.sourceBytes, 5ULL * 1024 * 1024 * 1024) + 512ULL * 1024 * 1024;
                    if (fs::space(locations.gameData().parent_path()).available < required)
                        error = "There is not enough free space to import this disc. Free at least 6 GB on this drive, then try again.";
                    if (fs::exists(locations.gameData()) && !fs::is_empty(locations.gameData()))
                        error = "An imported game folder already exists at " + locations.gameData().string() + ". Choose that folder, or move it aside before importing a replacement disc.";
                }
                DI::Result result = DI::Result::Failed;
                if (error.empty()) result = perform(source, locations.gameData(), folder, &error);
                if (result == DI::Result::Cancelled) {
                    std::fputs("[setup] Cancelled.\n", stderr);
                    if (!prompts()) return false;
                    continue;
                }
                if (result == DI::Result::Ok) {
                    auto config = readAppConfig(locations);
                    const auto selected = folder ? source : locations.gameData();
                    config["disc"] = selected.string();
                    config["disc_source"] = folder ? "folder" : "imported";
                    writeAppConfig(locations, config);
                    *disc = selected;
                    std::fprintf(stderr, "[setup] Game folder ready: %s\n", disc->c_str());
                    return true;
                }
                if (error.empty()) error = "The game disc could not be imported. Choose your USA Super Mario Galaxy 2 disc and try again.";
                if (alert("Could not use this game disc", error, @[@"Try again", @"Quit"]) != NSAlertFirstButtonReturn) return false;
            }
        } catch (const std::exception& exception) {
            showProblem(exception.what());
            return false;
        }
    }
}
bool offerMigration(const AppLocations& locations) {
    try {
        auto config = readAppConfig(locations);
        const char* home = std::getenv("HOME");
        const auto source = migrationSource(locations, home ? fs::path(home) : fs::path{}, config);
        if (source.empty()) return true;
        const auto answer = alert("Found saves from a development install",
            "Copy them to Astral Italian? The original saves will stay in " + source.string() + ". Existing files in Astral Italian will be kept.",
            @[@"Copy", @"Don't copy"]);
        migrateUserState(locations, source, answer == NSAlertFirstButtonReturn, &config);
        return true;
    } catch (const std::exception& exception) {
        showProblem(exception.what());
        return false;
    }
}
void installMenu(const AppLocations& locations) {
    @autoreleasepool {
        static AstralSetupAction* action = [[AstralSetupAction alloc] init];
        action->locations = locations;
        NSMenu* menu = NSApp.mainMenu.numberOfItems ? [NSApp.mainMenu itemAtIndex:0].submenu : nil;
        if (!menu) { std::fputs("[setup] The application menu is not available. Use --setup to choose a game disc.\n", stderr); return; }
        if ([menu itemWithTag:0x41535452]) return;
        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:@"Choose Game Disc…" action:@selector(choose:) keyEquivalent:@""];
        item.target = action;
        item.tag = 0x41535452;
        [menu insertItem:item atIndex:std::min<NSInteger>(2, menu.numberOfItems)];
        std::fprintf(stderr, "[setup] Choose Game Disc menu installed: %s\n", [menu itemWithTag:0x41535452] ? "yes" : "no");
    }
}
}  // namespace PetariNative::App::FirstRun
