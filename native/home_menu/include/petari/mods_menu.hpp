#pragma once

namespace PetariNative::HomeMenu {
class Menu;
// Shared by the HBM bridge and its regression test. Row IDs are supportedAt()
// indices, not raw Mod IDs (which retain unavailable donor slots).
void refreshSupportedMods(Menu& menu);
// Consumes the row toggles and applies them to their stable Mod IDs. Saving is
// the bridge's responsibility. Returns true when at least one row changed.
bool applySupportedModToggles(Menu& menu);
}  // namespace PetariNative::HomeMenu
