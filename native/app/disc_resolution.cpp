#include "app_locations.hpp"
#include "../disc_import/disc_import.hpp"

namespace PetariNative::App {
namespace fs = std::filesystem;
DiscResolution resolveDisc(const fs::path& argument, const char* environment, const AppConfig& config, const fs::path& cwd) {
    if (!argument.empty()) return {argument, {}};
    if (environment && *environment) return {environment, {}};
    const auto remembered = config.find("disc");
    if (remembered != config.end() && !remembered->second.empty()) {
        std::string error;
        if (DiscImport::validateExtracted(remembered->second, true, &error)) return {remembered->second, {}};
        return {{}, "Your remembered game folder is no longer ready to use. " + error};
    }
    const auto dev = cwd / "build/game-data/SB4E01";
    return {fs::is_directory(dev) ? dev : fs::path{}, {}};
}
}  // namespace PetariNative::App
