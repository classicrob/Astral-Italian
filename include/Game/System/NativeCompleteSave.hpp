#pragma once

#ifdef GALAXY_NATIVE
namespace NativeCompleteSave {
// Opt-in development operation; only an empty file in the separate unlocked NAND.
void checkSelection(int file, bool isNew);
// Called by file select while the ordinary game/NAND update loop keeps running.
bool beforeStageLaunch();
} // namespace NativeCompleteSave
#endif
