#include "output_au_observer.hpp"
#include <petari/platform/audio_workgroup.hpp>
#include <mach/mach_time.h>
namespace PetariNative::AudioOutputAU::Detail {
namespace {
std::uint32_t numerator=0,denominator=1;
std::int64_t offset=0;
}
void configureObserverClock(std::uint32_t n,std::uint32_t d,std::int64_t delta) noexcept {
    numerator=n; denominator=d; offset=delta;
}
// Retained out of line for the exact library/callee no-trap codegen proof.
__attribute__((noinline)) std::uint64_t observerClock() noexcept { return mach_absolute_time(); }
__attribute__((noinline)) void publishRenderEvent(const RenderEvent& e) noexcept {
    // No 128-bit division helper, clock/identity query, TLS or host service.
    const auto ns=(e.mach/denominator)*numerator+(e.mach%denominator)*numerator/denominator+offset;
    Platform::AudioWorkgroup::noteOutputRender(e.kind,e.mach,ns,e.ticket,e.generation,e.value);
}
}
