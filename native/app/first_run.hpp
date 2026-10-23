#pragma once
#include "app_locations.hpp"

namespace PetariNative::App::FirstRun {
inline constexpr const char* helpUrl = "https://github.com/classicrob/Astral-Italian/blob/main/docs/TESTING.md";

// Env-only automation hooks: ASTRAL_FIRSTRUN_PICK bypasses just the file picker;
// ASTRAL_FIRSTRUN_PROMPTS=off logs alerts and exits on failure (never retries forever).
// ASTRAL_SUPPORT_DIR (AppLocations) keeps demo/test imports away from real saves.
void showProblem(const std::string& problem);
bool chooseDisc(const AppLocations& locations, std::filesystem::path* disc);
bool offerMigration(const AppLocations& locations);
// Call AFTER aurora_initialize: SDL installs its standard application menu there.
void installMenu(const AppLocations& locations);
}  // namespace PetariNative::App::FirstRun
