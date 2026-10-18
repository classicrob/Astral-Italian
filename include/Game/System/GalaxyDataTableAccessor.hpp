#pragma once

#include <revolution/types.h>
#include "Inline.hpp"

class GameSystemDataHolder;

// SMG2-only helper owned by GameSequenceDirector (+0x04): reads GameSystemDataHolder's galaxy data table
// (GalaxyName-keyed rows: GrandStarNo, CometAppearMedalNum, ...). Real class name unknown.
class GalaxyDataTableAccessor {
public:
    GalaxyDataTableAccessor(GameSystemDataHolder* pDataHolder);

    s32 getValueS32(const char* pGalaxyName, const char* pKey) const NO_INLINE;
    const char* getGalaxyNameByValue(const char* pKey, s32 value) const NO_INLINE;

    /* 0x00 */ GameSystemDataHolder* mDataHolder;
};
