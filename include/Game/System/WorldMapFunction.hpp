#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class GalaxyStatusIcon;
class JMapInfo;
class PartsModel;

/// Helpers of the world-map model unit (0x804E7AC0 ..) shared by the map actors.
namespace WorldMapFunction {
    void movementMini();
    void calcAnimMini();
    void entryMini();
    void drawMini();
    void initMiniCamera();
    void initRocketOffsets();
    JMapInfo* createWorldParser(s32, const char*);
    PartsModel* createPartsModel(LiveActor*, const char*, const char*, const char*);
    bool mountWorldMapResources();
    s32 getGrandStarNum();
    bool hasIncompleteGreenStar(const char* pGalaxyName);
    void setStatusIconKind(GalaxyStatusIcon*, const char* pGalaxyName);
    void initDrawFog(LiveActor*);
    void setFogRange(LiveActor*, f32 scale);
    void setFogDrawOn(LiveActor*);
    void setFogDrawOff(LiveActor*);
    bool isPointing(LiveActor*);
    const TVec3f* getRocketOffsetGalaxy();     // fn_804E8530
    const TVec3f* getRocketOffsetPoint();      // fn_804E8540
    s32 getGalaxyWorldNo(const char* pGalaxyName);  // fn_804E8960
    bool tryOnGreenStarEventFlag();            // fn_804E8DC0
    bool isMapEventFlagOff();                  // fn_804E8E00
    bool isGreenStarDemoReady();               // fn_804E8E30
    bool isWorldNoMid(s32 worldNo);            // fn_804E8E90: 2..6
    void approachScale(LiveActor*, f32 speed, f32 max);
    void returnScale(LiveActor*, f32 speed, f32 min);
};

namespace MR { s32 getProgressWorldNo(); }
