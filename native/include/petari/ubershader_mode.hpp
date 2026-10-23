#pragma once

#include <cstring>
#include <optional>

namespace PetariNative::Ubershaders {
enum class Mode { Off, Hybrid, All };
struct Selection { Mode mode; const char* source; };

// Environment overrides the saved preference, including an explicit zero.
// Preserve the diagnostic "all" mode and the previous handling of other values.
inline Selection resolve(const char* environment, std::optional<bool> saved = std::nullopt) {
    if (environment) {
        if (std::strcmp(environment, "all") == 0) return {Mode::All, "env"};
        return {std::strcmp(environment, "1") == 0 ? Mode::Hybrid : Mode::Off, "env"};
    }
    if (saved) return {*saved ? Mode::Hybrid : Mode::Off, "setting"};
    return {Mode::Hybrid, "default"};
}
inline const char* name(Mode mode) {
    return mode == Mode::All ? "all" : mode == Mode::Hybrid ? "hybrid" : "off";
}
inline const char* environmentValue(Mode mode) {
    return mode == Mode::All ? "all" : mode == Mode::Hybrid ? "1" : "0";
}
}  // namespace PetariNative::Ubershaders
