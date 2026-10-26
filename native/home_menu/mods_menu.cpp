#include <petari/home_menu.hpp>
#include <petari/mods.hpp>
#include <petari/mods_menu.hpp>

#include <cstring>

namespace PetariNative::HomeMenu {
namespace {
static_assert(static_cast<int>(Mods::Mod::Count) <= kMaxMods, "supported mods must fit the Mods page");
int rowFor(Mods::Mod mod) {
    for (int row = 0; row < Mods::supportedCount(); ++row) {
        if (Mods::supportedAt(row) == mod) return row;
    }
    return -1;
}
}  // namespace

void refreshSupportedMods(Menu& menu) {
    ModsEntry entries[kMaxMods];
    const int count = Mods::supportedCount();
    for (int row = 0; row < count; ++row) {
        const auto mod = Mods::supportedAt(row);
        std::strncpy(entries[row].label, Mods::description(mod), sizeof(entries[row].label) - 1);
        entries[row].on = Mods::enabled(mod);
        if (mod == Mods::Mod::Cappy || mod == Mods::Mod::OdysseyJumpDive) {
            entries[row].needs = rowFor(Mods::Mod::OdysseyMovement);
        }
    }
    menu.setMods(entries, count);
}

bool applySupportedModToggles(Menu& menu) {
    const auto toggles = menu.takeModToggles();
    if (!toggles) return false;
    for (int row = 0; row < Mods::supportedCount(); ++row) {
        if (toggles & (1u << row)) Mods::setEnabled(Mods::supportedAt(row), menu.mods()[row].on);
    }
    return true;
}
}  // namespace PetariNative::HomeMenu
