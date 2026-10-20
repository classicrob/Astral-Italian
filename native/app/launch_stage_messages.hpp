#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace PetariNative::Resource { struct Bytes; }

namespace PetariNative::App::LaunchStage {
struct EnglishName {
    std::string text;
    std::string provenance;
};
using EnglishNames = std::map<std::string, EnglishName>;

// Bounded big-endian MSBT label/text decoding; no LMS host-layout casts.
// Only plain UTF-16 GalaxyName_<internal stage> labels are returned. Formatted
// control sequences are refused, never stripped into a guessed display name.
// Malformed input leaves *names unchanged.
bool readEnglishGalaxyNames(Resource::Bytes msbt, const std::string& provenance, EnglishNames* names, std::string* error);
// Uses the native RARC/Yaz0/Yay0 reader on the game's own US English resources.
// extractedDisc may be the disc root (files/) or its files directory.
bool loadEnglishGalaxyNames(const std::filesystem::path& extractedDisc, EnglishNames* names, std::string* error);
}  // namespace PetariNative::App::LaunchStage
