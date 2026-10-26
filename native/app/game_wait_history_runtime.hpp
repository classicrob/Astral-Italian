#pragma once
#include <cstdint>
namespace PetariNative::GameWaitHistory {
// Called once on the main thread. Enabled only by the exact opt-in plus a NEW
// explicit output file; no default user cache, no inherited audio transport.
bool initializeFromEnvironment(std::uint64_t windowId=0) noexcept;
}
