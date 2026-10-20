#pragma once
#include <petari/replay.hpp>
#include <revolution/kpad.h>
namespace PetariNative::ReplayInput {
KPADStatus decodeSample(const Sample& sample);
}
