#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace PetariNative::App {

struct AppLocations {
    std::filesystem::path root;
    static std::filesystem::path defaultRoot(const std::filesystem::path& home);
    // ASTRAL_SUPPORT_DIR is an env-only override for isolated setup tests/demo runs.
    static AppLocations fromEnvironment();
    std::filesystem::path gameData() const { return root / "Game Data/SB4E01"; }
    std::filesystem::path config() const { return root / "app.cfg"; }
    std::filesystem::path logs() const { return root / "logs"; }
    std::filesystem::path captures() const { return root / "Captures"; }
    std::filesystem::path tapes() const { return root / "Tapes"; }
};

bool isOtherGameUserPath(const std::filesystem::path& path, const std::filesystem::path& home);

using AppConfig = std::map<std::string, std::string>;
AppConfig readAppConfig(const AppLocations& locations);
void writeAppConfig(const AppLocations& locations, const AppConfig& config);

struct DiscResolution {
    std::filesystem::path disc;
    std::string problem;
};
// An invalid remembered choice goes directly back to setup, even on a developer checkout.
DiscResolution resolveDisc(const std::filesystem::path& argument, const char* environment,
                           const AppConfig& config, const std::filesystem::path& workingDirectory);

std::filesystem::path migrationSource(const AppLocations& locations, const std::filesystem::path& home,
                                      const AppConfig& config);
// Copies only absent top-level entries. Existing files/directories/symlinks are never replaced or merged.
// Records either answer before copying, so failed copies cannot prompt repeatedly on later launches.
void migrateUserState(const AppLocations& locations, const std::filesystem::path& source, bool copy, AppConfig* config);

// Only names owned by the direct app are eligible; unrelated files and symlinks are left alone.
void pruneRunArtifacts(const std::filesystem::path& directory, bool tapes, unsigned keep);
// Sets defaults only for absent variables. No changes to capture/tape implementations or their formats.
void prepareRunArtifacts(const AppLocations& locations);
// Redirects Finder's /dev/null (or closed) descriptors, preserving terminal/pipe/file redirection.
// Returns the chosen filename, or an empty path if the caller already owns the descriptors.
std::filesystem::path redirectAppLog(const AppLocations& locations);

}  // namespace PetariNative::App
