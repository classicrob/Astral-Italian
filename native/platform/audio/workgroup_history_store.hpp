#pragma once
#include "workgroup_history.hpp"
namespace PetariNative::Platform::AudioWorkgroup::History {
using FlightHistory=Recorder<32768,24576>;
extern FlightHistory audioHistory;
}
