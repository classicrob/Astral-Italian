#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStateLeaveHome {
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeStart);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeMove);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeZoomIn);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeZoomOut);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeZoomOutSub);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeMessage);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeFovOut);
    WMR_NERVE_DECL(WorldMapResultStateLeaveHomeMessageEnd);
};

WMR_OWNER_STATE_DECL(WorldMapResultStateLeaveHome, void exeStart(); void exeMove(); void exeZoomIn(); void exeZoomOut(); void exeZoomOutSub();
               void exeMessage(); void exeFovOut(); void exeMessageEnd(); void moveCamera(s32); void calcCenter(); void playEnterSound();
               /* 0x10 */ f32 mFovy; /* 0x14 */ TVec3f mPos; /* 0x20 */ TVec3f mCenter;);
