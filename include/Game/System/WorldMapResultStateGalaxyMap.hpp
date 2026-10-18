#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStateGalaxyMap {
    WMR_NERVE_DECL(WorldMapResultStateGalaxyMapStart);
    WMR_NERVE_DECL(WorldMapResultStateGalaxyMapWait);
    WMR_NERVE_DECL(WorldMapResultStateGalaxyMapEnd);
};

WMR_OWNER_STATE_DECL(WorldMapResultStateGalaxyMap, virtual void control(); void exeStart(); void exeWait(); void exeEnd();
               /* 0x10 */ s32 mInt10; /* 0x14 */ s32 mInt14;);
