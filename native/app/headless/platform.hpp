#pragma once
#include "replay.hpp"
#include <petari/replay_kpad.hpp>
namespace PetariNative::Headless {
using ReplayInput::decodeSample;
void advanceDevices(std::uint64_t ticks);
void configurePlatform(const char* disc, const char* user);
}
