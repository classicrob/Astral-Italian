#pragma once
#include "replay.hpp"
#include <revolution/kpad.h>
namespace PetariNative::Headless {
KPADStatus decodeSample(const Sample& sample);
void advanceDevices(std::uint64_t ticks);
void configurePlatform(const char* disc, const char* user);
}
