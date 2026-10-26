#pragma once
#include "output_au_core.hpp"
namespace PetariNative::AudioOutputAU::Detail {
// Configure only after the preceding callback generation is quiescent.
void configureObserverClock(std::uint32_t numer,std::uint32_t denom,std::int64_t steadyOffset) noexcept;
std::uint64_t observerClock() noexcept;
void publishRenderEvent(const RenderEvent&) noexcept;
}
