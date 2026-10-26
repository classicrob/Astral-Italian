#pragma once
#include "output_au_core.hpp"
#include <AudioToolbox/AudioUnit.h>

namespace PetariNative::AudioOutputAU::Detail {
// Kept out of line and retained in tests for Release disassembly.
OSStatus render(void*, AudioUnitRenderActionFlags*, const AudioTimeStamp*, UInt32,
                UInt32, AudioBufferList*) noexcept;
}
