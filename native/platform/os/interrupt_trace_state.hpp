#pragma once
#include <cstdint>
namespace PetariNative::Platform::OS::InterruptTrace {
// Pure per-host-thread protocol. Explicit request/return boundaries are
// distinct from implicit CV unlock/reacquire bounds; unknown is never FREE.
struct State {
    std::uint64_t span = 0, since = 0, requestNs = 0;
    bool pending = false, held = false, cvUnknown = false;
    bool request(std::uint64_t id, std::uint64_t ns) {
        if (pending || held || cvUnknown) return false;
        span = id; requestNs = ns; pending = true; return true;
    }
    bool acquired(std::uint64_t ns) {
        if (!pending || held || cvUnknown) return false;
        pending = false; held = true; since = ns; return true;
    }
    bool failed() {
        if (!pending || held || cvUnknown) return false;
        pending = false; return true;
    }
    bool releaseBegin() const { return held && !pending && !cvUnknown; }
    bool released() {
        if (!releaseBegin()) return false;
        held = false; return true;
    }
    bool cvRelease() {
        if (!releaseBegin()) return false;
        held = false; cvUnknown = true; return true;
    }
    bool cvAcquire(std::uint64_t id, std::uint64_t ns) {
        if (!cvUnknown || held || pending) return false;
        cvUnknown = false; held = true; span = id; since = ns; requestNs = 0; return true;
    }
    std::uint64_t heldNs(std::uint64_t ns) const { return held && ns >= since ? ns - since : 0; }
};
}
