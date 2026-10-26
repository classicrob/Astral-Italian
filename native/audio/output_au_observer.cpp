#include "output_au_observer.hpp"
#include <petari/platform/audio_workgroup.hpp>
#include <mach/mach_time.h>
#include <mach/mach_init.h>
#include <mach/mach_vm.h>
namespace PetariNative::AudioOutputAU::Detail {
namespace {
std::uint32_t numerator=0,denominator=1;
std::int64_t offset=0;
}
ObserverClockProbe probeObserverClock() noexcept {
    ObserverClockProbe probe;
#if defined(__aarch64__)
    // Apple Silicon commpage clock-mode byte, decoded/verified against the
    // loaded system mach_absolute_time path in the mandatory codegen gate.
    // A task-local, fallible read avoids dereferencing an unmapped address on
    // another OS layout. This syscall is ONLY on the control thread at open.
    unsigned char mode=255;
    mach_vm_size_t bytes=0;
    probe.readError=mach_vm_read_overwrite(mach_task_self(),0x0000000fffffc090ULL,
        sizeof(mode),reinterpret_cast<mach_vm_address_t>(&mode),&bytes);
    probe.mode=mode; probe.readBytes=bytes;
#else
    probe.readError=KERN_NOT_SUPPORTED;
#endif
    return probe;
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
