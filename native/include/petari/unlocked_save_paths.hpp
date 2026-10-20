#pragma once
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace PetariNative::UnlockedSave {
inline bool privateServerDataPath(const std::filesystem::path& path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    const auto canonical = fs::weakly_canonical(path, ec);
    if (ec || path.empty()) return false;
    const std::set<std::string> forbidden{"maciogalaxy2-team", ".bb-machines", ".bb", "attachments", "build", "builds"};
    for (const auto& component : canonical) {
        auto name = component.string();
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
        if (forbidden.count(name)) return false;
    }
    for (auto parent = canonical; !parent.empty();) {
        if (fs::exists(parent / ".git", ec) || ec) return false;
        if (parent == parent.parent_path()) break;
        parent = parent.parent_path();
    }
    return true;
}

// Fixed command, no user interpolation or asset reads. Match the owner's
// approved LocalHostName plus its non-asset README marker, not all macOS hosts.
inline bool approvedServerHost() {
#ifdef __APPLE__
    const char* home = std::getenv("HOME");
    if (!home) return false;
    std::FILE* pipe = popen("/usr/sbin/scutil --get LocalHostName", "r");
    if (!pipe) return false;
    char buffer[256]{};
    const bool read = std::fgets(buffer, sizeof(buffer), pipe) != nullptr;
    const int status = pclose(pipe);
    std::string host = buffer;
    while (!host.empty() && (host.back() == '\n' || host.back() == '\r')) host.pop_back();
    if (!read || status != 0 || host != "MacBook-Pro-52") return false;
    std::error_code ec;
    const auto marker = std::filesystem::path(home) / "MacioGalaxy2-gamedata/README.txt";
    return !std::filesystem::is_symlink(marker, ec) && !ec &&
           std::filesystem::is_regular_file(marker, ec) && !ec;
#else
    return false;
#endif
}

// App CLI boundary, in addition to the Python driver's copy/receipt checks.
inline bool validateUserDirectory(const std::filesystem::path& user, const std::filesystem::path& home,
                                  std::string* error) {
    namespace fs = std::filesystem;
    const auto fail = [&](const char* why) { if (error) *error = why; return false; };
    if (user.empty() || home.empty()) return fail("explicit isolated --user and HOME are required");
    std::error_code ec;
    const auto root = fs::weakly_canonical(user, ec);
    if (ec) return fail("cannot resolve isolated user directory");
    const auto personal = fs::weakly_canonical(home / "Library/Application Support/Petari", ec);
    if (ec) return fail("cannot resolve personal user directory");
    auto r = root.begin();
    auto p = personal.begin();
    while (r != root.end() && p != personal.end() && *r == *p) { ++r; ++p; }
    if (p == personal.end()) return fail("personal user directory and descendants are forbidden");
    if (!privateServerDataPath(root)) return fail("save data must be outside repo/build/team/upload trees");
    if (fs::is_symlink(user, ec) || ec || !fs::is_directory(root, ec) || ec)
        return fail("user directory must be an existing non-symlink directory");
    const auto marker = root / ".petari-make-unlocked-save";
    if (fs::is_symlink(marker, ec) || ec || !fs::is_regular_file(marker, ec) || ec)
        return fail("isolated user marker must be a regular non-symlink file");
    const auto nand = root / "NAND";
    if (fs::is_symlink(nand, ec) || ec || !fs::is_directory(nand, ec) || ec)
        return fail("isolated NAND must be an existing non-symlink directory");
    fs::recursive_directory_iterator it(nand, fs::directory_options::none, ec), end;
    while (!ec && it != end) {
        const auto status = it->symlink_status(ec);
        if (ec || fs::is_symlink(status) || (!fs::is_directory(status) && !fs::is_regular_file(status)))
            return fail("isolated NAND contains a symlink or nonregular entry");
        it.increment(ec);
    }
    if (ec) return fail("cannot inspect isolated NAND");
    if (error) error->clear();
    return true;
}
} // namespace PetariNative::UnlockedSave
