#include "first_run.hpp"
#import <Cocoa/Cocoa.h>
#include <cstdio>
#include <cstdlib>

int main() {
    @autoreleasepool {
        // Refuse accidental imports into the player's actual support folder from this harness.
        const char* support = std::getenv("ASTRAL_SUPPORT_DIR");
        if (!support || !*support) {
            std::fputs("demo: set ASTRAL_SUPPORT_DIR to a temporary folder first.\n", stderr);
            return 2;
        }
        const auto locations = PetariNative::App::AppLocations::fromEnvironment();
        std::filesystem::path disc;
        const bool ok = PetariNative::App::FirstRun::chooseDisc(locations, &disc);
        std::printf("first-run result: %s; disc=%s; config=%s\n", ok ? "ready" : "quit", disc.c_str(), locations.config().c_str());
        return ok ? 0 : 1;
    }
}
