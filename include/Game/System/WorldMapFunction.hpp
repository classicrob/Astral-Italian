#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class GalaxyStatusIcon;

/// Helpers of the world-map model unit (0x804E7AC0 ..) shared by the map actors.
namespace WorldMapFunction {
    s32 getGrandStarNum();
    bool hasIncompleteGreenStar(const char* pGalaxyName);
    void setStatusIconKind(GalaxyStatusIcon*, const char* pGalaxyName);
    void initDrawFog(LiveActor*);
    void setFogRange(LiveActor*, f32 scale);
    void setFogDrawOn(LiveActor*);
    void setFogDrawOff(LiveActor*);
    bool isPointing(LiveActor*);
    void approachScale(LiveActor*, f32 speed, f32 max);
    void returnScale(LiveActor*, f32 speed, f32 min);
};
