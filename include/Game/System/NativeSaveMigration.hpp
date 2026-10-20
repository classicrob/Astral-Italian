#pragma once
#ifdef GALAXY_NATIVE
#include <revolution/types.h>
class SaveDataHandler;
namespace NativeSaveMigration {
void prepareLoaded(u8* bytes, u32 length);
void checkRoundTrip(SaveDataHandler& handler);
void pollRoundTrip(SaveDataHandler& handler);
}
#endif
