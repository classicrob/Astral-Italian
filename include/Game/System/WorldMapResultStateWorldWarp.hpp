#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStateWorldWarp {
    WMR_NERVE_DECL(WorldMapResultStateWorldWarpMove);
};

WMR_STATE_DECL_HOST_KILL(WorldMapOwner, WorldMapResultStateWorldWarp, WMR_STATE_NO_KILL, void exeMove();
               /* 0x10 */ s32 mInt10; /* 0x14 */ s32 mInt14; /* 0x18 */ s32 mInt18;
               /* 0x1C */ TVec3f mPosA; /* 0x28 */ TVec3f mPosB; /* 0x34 */ TVec3f mDir;);
