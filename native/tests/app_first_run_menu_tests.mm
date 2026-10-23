// Real SDL/AppKit integration; no renderer, game data or user state needed.
#include "first_run.hpp"
#include <SDL3/SDL.h>
#import <Cocoa/Cocoa.h>
#include <cstdio>
#include <cstring>

int main() {
    @autoreleasepool {
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
            std::fprintf(stderr, "SDL video: %s\n", SDL_GetError());
            return 1;
        }
        // This is the ordering used before startup dialogs in app_main.cpp.
        [NSApplication sharedApplication];
        NSMenu* original = NSApp.mainMenu;
        const bool sdlApplication = [NSStringFromClass([NSApp class]) isEqualToString:@"SDL3Application"];
        SDL_Window* first = SDL_CreateWindow("First-run menu test", 320, 200, SDL_WINDOW_HIDDEN);
        PetariNative::App::FirstRun::installMenu({"/unused-first-run-menu-test"});
        NSMenu* appMenu = original.numberOfItems ? [original itemAtIndex:0].submenu : nil;
        NSMenuItem* item = [appMenu itemWithTitle:@"Choose Game Disc…"];
        SDL_Window* second = SDL_CreateWindow("Menu survival test", 320, 200, SDL_WINDOW_HIDDEN);
        SDL_PumpEvents();
        PetariNative::App::FirstRun::installMenu({"/unused-first-run-menu-test"});
        unsigned count = 0;
        for (NSMenuItem* entry in appMenu.itemArray) if ([entry.title isEqualToString:@"Choose Game Disc…"]) ++count;
        const bool ok = sdlApplication && first && second && original == NSApp.mainMenu && item && item.target &&
            item.action == @selector(choose:) && [appMenu itemWithTitle:@"Choose Game Disc…"] == item && count == 1;
        std::printf("SDL %d.%d.%d: application=%s; standard menu=%s; item survives windows without duplicates=%s\n",
            SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION, sdlApplication ? "SDL3Application" : "unexpected",
            original ? "present" : "absent", ok ? "yes" : "no");
        if (second) SDL_DestroyWindow(second);
        if (first) SDL_DestroyWindow(first);
        SDL_Quit();
        return ok ? 0 : 1;
    }
}
