#pragma once
namespace PetariNative::App::DisplayWindow {
// Called only on the SDL host thread, before and between frames.
void attach(void* window);
void update();
}
