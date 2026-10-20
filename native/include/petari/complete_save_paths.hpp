#pragma once

#include <cstdlib>
#include <filesystem>

namespace PetariNative::CompleteSave {
inline bool isIsolatedUserAt(const std::filesystem::path& user, const std::filesystem::path& home) {
    if (home.empty() || user.empty()) return false;
    std::error_code error;
    const auto allowed = (home / "Library/Application Support/AstralItalian-unlocked").lexically_normal();
    const auto base = std::filesystem::weakly_canonical(allowed, error);
    if (error || base != allowed) return false;
    const auto actual = std::filesystem::weakly_canonical(user, error);
    if (error) return false;
    const auto relative = actual.lexically_relative(base);
    return !relative.empty() && *relative.begin() != "..";
}
inline bool isIsolatedUser(const std::filesystem::path& user) {
    const char* home = std::getenv("HOME");
    return home && isIsolatedUserAt(user, home);
}
} // namespace PetariNative::CompleteSave
