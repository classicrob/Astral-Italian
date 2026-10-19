#pragma once
#include <cstdint>
namespace PetariNative::Platform::OS {
// Select once, before OSInit, VIInit, WPADInit, or any alarms are installed.
void enableVirtualTime(std::int64_t epochTicks);
bool virtualTimeEnabled();
std::int64_t virtualSystemTime();
// Host interrupt thread only; dispatches all alarm deadlines through the target.
void advanceVirtualTimeTo(std::int64_t systemTicks);
}
