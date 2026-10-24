#include "display_window.hpp"
#include "display_settings.hpp"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_error.h>
#include <aurora/gfx.h>
#include <cstdio>
#include <cstdlib>

namespace PetariNative::App::DisplayWindow {
namespace {
SDL_Window* gWindow = nullptr;
int gApplied = -1;  // the VSync state Aurora presents with now (1 Fifo, 0 Immediate); -1 unknown
class SdlWindow final : public DisplaySettings::Window {
public:
    bool fullscreen() override { return (SDL_GetWindowFlags(gWindow) & SDL_WINDOW_FULLSCREEN) != 0; }
    bool setFullscreen(bool on) override {
        const bool ok = SDL_SetWindowFullscreen(gWindow, on) && SDL_SyncWindow(gWindow) && fullscreen() == on;
        if (!ok) std::fprintf(stderr, "petari: could not change fullscreen: %s\n", SDL_GetError());
        return ok;
    }
    bool setSize(int width, int height) override {
        // SDL ignores size changes on maximized windows until they are restored.
        const bool ok = SDL_RestoreWindow(gWindow) && SDL_SyncWindow(gWindow) &&
                        SDL_SetWindowSize(gWindow, width, height) && SDL_SyncWindow(gWindow);
        if (!ok) std::fprintf(stderr, "petari: could not change window size: %s\n", SDL_GetError());
        return ok;
    }
};
}
void attach(void* window) {
    gWindow = static_cast<SDL_Window*>(window);
    HomeMenu::instance().setDisplayBackend(&DisplaySettings::controller());
}
void setStartupVsync(bool on) { gApplied = on ? 1 : 0; }
void update() {
    if (!gWindow) return;
    SdlWindow window;
    DisplaySettings::controller().tick(window);
    // VSync Auto: follow the window into and out of fullscreen (Fifo windowed, Immediate fullscreen) through Aurora's live
    // present-mode switch. Only when this launch started in Auto (On/Off apply at the next launch), and never against an explicit
    // PETARI_PRESENT_MODE (measurement runs pin the mode).
    if (gApplied < 0 || !DisplaySettings::controller().startupVsyncAuto()) return;
    if (const char* pinned = std::getenv("PETARI_PRESENT_MODE"); pinned && pinned[0]) return;
    const bool full = window.fullscreen();
    const int want = full ? 0 : 1;
    if (want == gApplied) return;
    aurora_enable_vsync(want != 0);
    gApplied = want;
    std::fprintf(stderr, "[display] vsync auto: %s, now %s\n", full ? "fullscreen" : "windowed",
                 want ? "with VSync (Fifo)" : "without VSync (Immediate)");
}
}
