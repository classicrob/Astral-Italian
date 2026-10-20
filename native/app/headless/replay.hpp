#pragma once
// Compatibility surface: GX and headless use the same strict tape reader.
#include <petari/replay.hpp>
namespace PetariNative::Headless {
using ReplayInput::ReplayError;
using ReplayInput::Sample;
using ReplayInput::Read;
using ReplayInput::Replay;
using ReplayInput::FrameClock;
}
