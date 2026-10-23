// Cocoa flow against the synthetic importer. Drives the real Cancel button; no production test branch.
#include "first_run.hpp"
#import <Cocoa/Cocoa.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>

namespace fs = std::filesystem;
namespace App = PetariNative::App;
int main(int argc, char** argv) {
    @autoreleasepool {
        char pattern[] = "/tmp/astral-first-run-flow-XXXXXX";
        char* created = mkdtemp(pattern);
        if (!created) return 2;
        const fs::path temporary(created);
        const auto source = temporary / "test.synthetic";
        std::ofstream(source) << "synthetic, not game data\n";
        setenv("ASTRAL_FIRSTRUN_PROMPTS", "off", 1);
        setenv("ASTRAL_FIRSTRUN_PICK", source.c_str(), 1);
        const App::AppLocations cancelled{temporary / "cancelled"};
        __block bool clicked = false;
        __block bool snapshot = argc < 2;
        NSString* screenshot = argc >= 2 ? [NSString stringWithUTF8String:argv[1]] : nil;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 500 * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
            for (NSWindow* window in NSApp.windows) {
                if (![window.title isEqualToString:@"Importing your game disc"]) continue;
                if (screenshot) {
                    // Capture our own live AppKit view; Screen Recording permission is unnecessary.
                    NSView* view = window.contentView.superview;
                    NSBitmapImageRep* bitmap = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
                    [view cacheDisplayInRect:view.bounds toBitmapImageRep:bitmap];
                    NSData* png = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
                    snapshot = [png writeToFile:screenshot atomically:YES];
                }
                for (NSView* view in window.contentView.subviews) {
                    if ([view isKindOfClass:[NSButton class]] && [((NSButton*)view).title isEqualToString:@"Cancel"]) {
                        clicked = true;
                        [(NSButton*)view performClick:nil];
                    }
                }
            }
        });
        fs::path disc;
        const bool cancelResult = App::FirstRun::chooseDisc(cancelled, &disc);
        const bool clean = !fs::exists(cancelled.gameData()) && !fs::exists(cancelled.gameData().string() + ".partial") && !fs::exists(cancelled.config());
        const App::AppLocations success{temporary / "success"};
        const bool imported = App::FirstRun::chooseDisc(success, &disc);
        const auto config = App::readAppConfig(success);
        const bool remembered = imported && disc == success.gameData() && config.at("disc") == disc.string() && config.at("disc_source") == "imported";
        setenv("ASTRAL_FIRSTRUN_PICK", disc.c_str(), 1);
        const App::AppLocations folder{temporary / "folder"};
        const bool validated = App::FirstRun::chooseDisc(folder, &disc) && !fs::exists(folder.gameData()) && App::readAppConfig(folder).at("disc_source") == "folder";
        const auto invalid = temporary / "invalid.txt";
        std::ofstream(invalid) << "not a disc";
        setenv("ASTRAL_FIRSTRUN_PICK", invalid.c_str(), 1);
        const App::AppLocations refused{temporary / "refused"};
        const bool rejected = !App::FirstRun::chooseDisc(refused, &disc) && !fs::exists(refused.config());
        const bool ok = clicked && !cancelResult && clean && remembered && validated && rejected && snapshot;
        std::printf("Cocoa flow: cancel button=%s; cancellation clean=%s; import remembered=%s; folder in place=%s; invalid rejected=%s; snapshot=%s\n",
            clicked ? "clicked" : "missed", clean ? "yes" : "no", remembered ? "yes" : "no", validated ? "yes" : "no", rejected ? "yes" : "no", snapshot ? "yes" : "no");
        fs::remove_all(temporary);
        return ok ? 0 : 1;
    }
}
