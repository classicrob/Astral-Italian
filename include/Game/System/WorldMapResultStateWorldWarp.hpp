#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStateWorldWarp {
    WMR_NERVE_DECL(WorldMapResultStateWorldWarpMove);
};

WMR_OWNER_STATE_DECL(WorldMapResultStateWorldWarp, void exeMove();
               /* 0x10 */ s32 mInt10; /* 0x14 */ s32 mInt14; /* 0x18 */ s32 mInt18;
               /* 0x1C */ TVec3f mPosA; /* 0x28 */ TVec3f mPosB; /* 0x34 */ TVec3f mDir;);
