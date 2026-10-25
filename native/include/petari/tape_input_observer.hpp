#pragma once
// Installed on the game thread before tape input begins. The ordinary path is
// exactly Core(), behind one cached flag branch: no clock, allocation or hook.
#include <cstdint>
#include <utility>

namespace PetariNative::Input::TapeObserver {
struct Hooks {
    void (*before)(std::int32_t channel, std::uint32_t capacity, void* user) = nullptr;
    void (*after)(std::int32_t channel, std::uint32_t capacity, std::int32_t returned,
                  std::int32_t error, std::int32_t probe, std::uint32_t device,
                  const void* actualBuffer, void* user) = nullptr;
    void* user = nullptr;
};
inline Hooks hooks;
inline bool enabled = false;
inline void install(Hooks value) { hooks = value; enabled = value.before || value.after; }

template<class Core, class Connection>
int dispatch(std::int32_t channel, std::uint32_t capacity, const void* buffer,
             const std::int32_t* error, const std::int32_t* returned,
             Core&& core, Connection&& connection) {
    if (!enabled) return core();
    if (hooks.before) hooks.before(channel, capacity, hooks.user);
    const int handled = core();
    if (handled && hooks.after && returned) {
        const auto state = connection();
        hooks.after(channel, capacity, *returned, error ? *error : 0,
                    state.first, state.second, buffer, hooks.user);
    }
    return handled;
}
}  // namespace PetariNative::Input::TapeObserver
