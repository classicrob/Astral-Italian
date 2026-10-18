#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

class ActorCameraInfo;
class GalaxyConfirmLayout;

namespace NrvWorldMapResultStateEnterGalaxy {
    WMR_NERVE_DECL(WorldMapResultStateEnterGalaxyZoomIn);
    WMR_NERVE_DECL(WorldMapResultStateEnterGalaxyWait);
    WMR_NERVE_DECL(WorldMapResultStateEnterGalaxyDemo);
    WMR_NERVE_DECL(WorldMapResultStateEnterGalaxyZoomOut);
};  // namespace NrvWorldMapResultStateEnterGalaxy

/// Galaxy-entry step ("State[ギャラクシーに入る]"): zooms the camera into the galaxy and starts the demo.
WMR_OWNER_STATE_DECL(WorldMapResultStateEnterGalaxy, virtual void control(); void exeZoomIn(); void exeWait(); void exeDemo(); void exeZoomOut();
               /* 0x10 */ TVec3f mPos; /* 0x1C */ TVec3f* mTargetPos; /* 0x20 */ const char* mCameraName; /* 0x24 */ const char* mAnimName;
               /* 0x28 */ GalaxyConfirmLayout* mConfirmLayout; /* 0x2C */ ActorCameraInfo* mCamInfo; /* 0x30 */ const s32* mBlurNear;
               /* 0x34 */ const s32* mBlurFar; /* 0x38 */ f32 mRate; /* 0x3C */ bool mFlag;);
