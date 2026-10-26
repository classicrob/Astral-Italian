#pragma once
#include "output_au_core.hpp"
namespace PetariNative::AudioOutputAU::Detail {
// Control-thread probe only; never read commpage or invoke Mach in render.
// Failed/partial reads and every mode except the reviewed mode3 refuse clocks.
struct ObserverClockProbe {
    unsigned mode=255;
    int readError=0;
    std::uint64_t readBytes=0;
};
constexpr bool observerClockPermitted(const ObserverClockProbe& probe) noexcept {
    return probe.mode==3 && probe.readError==0 && probe.readBytes==1;
}
ObserverClockProbe probeObserverClock() noexcept;
// Configure only after the preceding callback generation is quiescent.
void configureObserverClock(std::uint32_t numer,std::uint32_t denom,std::int64_t steadyOffset) noexcept;
std::uint64_t observerClock() noexcept;
void publishRenderEvent(const RenderEvent&) noexcept;
}
