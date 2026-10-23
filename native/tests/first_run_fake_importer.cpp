// Test-only importer: accepts synthetic markers, never treats actual disc data as validated.
#include "../disc_import/disc_import.hpp"
#include <fstream>
#include <thread>
#include <chrono>

namespace PetariNative::DiscImport {
bool validateExtracted(const std::filesystem::path& root, bool quick, std::string* error) {
    if (std::filesystem::is_regular_file(root / "synthetic-disc")) return true;
    if (error) *error = "The synthetic game folder is missing its test marker. Choose the folder again.";
    return false;
}
Probe probe(const std::filesystem::path& source) {
    Probe result;
    if (std::filesystem::is_directory(source)) {
        result.format = Format::ExtractedFolder;
        if (!validateExtracted(source, true, &result.problem)) return result;
    } else if (source.extension() == ".synthetic") {
        result.format = Format::GcmIso;
    } else {
        result.problem = "This test importer accepts synthetic fixtures only. Build the real importer to read your disc.";
        return result;
    }
    result.gameId = "SB4E01";
    result.isSupportedGame = true;
    return result;
}
Result importDisc(const std::filesystem::path&, const std::filesystem::path& destination, const ProgressFn& progress, std::string*) {
    for (unsigned i = 0; i <= 100; ++i) {
        if (!progress({i, 100, "synthetic file (not game data)"})) return Result::Cancelled;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::filesystem::create_directories(destination);
    std::ofstream(destination / "synthetic-disc") << "synthetic test data only\n";
    return Result::Ok;
}
}  // namespace PetariNative::DiscImport
