#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

class ActorCameraInfo;
class GalaxyConfirmLayout;

namespace NrvWorldMapResultStateEnterHome {
    WMR_NERVE_DECL(WorldMapResultStateEnterHomeApproach);
    WMR_NERVE_DECL(WorldMapResultStateEnterHomeWait);
    WMR_NERVE_DECL(WorldMapResultStateEnterHomeEnter);
    WMR_NERVE_DECL(WorldMapResultStateEnterHomeExit);
};

WMR_OWNER_STATE_DECL(WorldMapResultStateEnterHome, virtual void control(); void exeApproach(); void exeWait(); void exeEnter(); void exeExit();
               /* 0x10 */ GalaxyConfirmLayout* mLayout; /* 0x14 */ ActorCameraInfo* mCamInfo;);
