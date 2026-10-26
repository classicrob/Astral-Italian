#pragma once
#include <cstdint>

namespace PetariNative::AudioOutputAU {
// Explicit opt-in only. The existing AudioSDL facade dispatches here; no SDL
// playback device is opened. Device setters are deliberately not exposed.
void start(std::uint32_t guestRate, bool backgroundMuted=false);
void stop();
bool active();
bool outputMuted();
std::uint64_t submittedFrames(); // guest-time equivalent, including priming
}
