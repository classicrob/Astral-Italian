#pragma once
#include "output_au_core.hpp"

namespace PetariNative::AudioOutputAU::Detail {
// Shared by real control and fake-device race tests. An empty reader count is
// necessary, never sufficient: API shutdown and unpublication are also required.
struct Retirement {
    bool unpublished=false, stopped=false, uninitialized=false, disposed=false;
    void begin(RenderOwner& owner) { owner.published.exchange(nullptr,std::memory_order_seq_cst); unpublished=true; }
    bool mayUninitialize(const RenderOwner& owner) const {
        return unpublished && stopped && owner.readers.load(std::memory_order_seq_cst)==0;
    }
    bool mayReclaim(const RenderOwner& owner) const { return mayUninitialize(owner) && uninitialized && disposed; }
};
// Listener refCons are numeric generation tokens, not destructible pointers.
// Late old-device notifications cannot dereference freed state or reopen a new
// unit. Token 0 is the process-lifetime default/service listener.
class ChangeMailbox {
public:
    void notify(std::uint64_t token) noexcept {
        if (enabled.load(std::memory_order_relaxed) && (!token || token==live.load(std::memory_order_acquire)))
            changes.fetch_add(1,std::memory_order_release);
    }
    std::atomic<bool> enabled{false};
    std::atomic<std::uint64_t> live{0}, changes{0}, overloads{0}, services{0};
};
}
