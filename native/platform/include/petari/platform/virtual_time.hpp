#pragma once
#include <cstdint>
namespace PetariNative::Platform::OS {
// Select once, before OSInit, VIInit, WPADInit, or any alarms are installed.
void enableVirtualTime(std::int64_t epochTicks);
bool virtualTimeEnabled();
std::int64_t virtualSystemTime();
// Host interrupt thread only; dispatches all alarm deadlines through the target.
void advanceVirtualTimeTo(std::int64_t systemTicks);
// Virtual I/O is tracked from bootstrap. Dispatch stays legacy until the
// headless driver activates ordering at its first post-ready reload boundary.
// Activation drains in-flight legacy work with the OS CPU released, then
// submissions run in order at SDK sleep/yield/device boundaries.
void activateVirtualAsync();
bool virtualAsyncActive();
// Zero is a no-op ticket in real-time mode. These calls never advance time.
std::uint64_t queueVirtualAsync();
bool waitVirtualAsync(std::uint64_t ticket);
// Cancel a not-yet-started request during shutdown. Claimed work must finish.
bool cancelVirtualAsync(std::uint64_t ticket);
void finishVirtualAsync(std::uint64_t ticket);
// Host driver only. Wait for all queued I/O and runnable OS work. A frame
// driver allows only its one sleeping owner. Polling loaders use an OS queue
// and must finish or sleep again before this fence permits another frame.
void waitVirtualWork();
}
