#pragma once
namespace PetariNative::App::DisplayWindow {
// Called only on the SDL host thread, before and between frames.
void attach(void* window);
void update();
// The VSync state the window was created with (app_main's config.vsync), so VSync Auto knows what Aurora presents with now.
void setStartupVsync(bool on);
}
