#include "display_window.hpp"
#include "display_settings.hpp"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_error.h>
#include <cstdio>

namespace PetariNative::App::DisplayWindow {
namespace {
SDL_Window* gWindow = nullptr;
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
void update() {
    if (!gWindow) return;
    SdlWindow window;
    DisplaySettings::controller().tick(window);
}
}
