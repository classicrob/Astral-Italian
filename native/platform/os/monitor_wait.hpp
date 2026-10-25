#pragma once
#include "monitor_wait_protocol.hpp"
#include <cstdint>
namespace PetariNative::Platform::MonitorWait {
// Startup-only selection. Experimental setup/runtime errors fail closed.
void configure();
Mode mode();
bool notificationAvailable();
void notify();
void traceThreadPolicy(); // Caller must guard with PETARI_GPU_TRACE; includes API error/readback.
void applyThreadPolicy(); // Called by the monitor itself, after its UI QoS setup.
Result waitCritical(std::uint64_t ns, bool forcedPending);
void afterForkChild(); // Only closes this child's inherited kqueue descriptor.
}
