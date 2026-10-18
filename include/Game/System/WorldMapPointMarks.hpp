#pragma once

#include "Game/System/MiniPartsActors.hpp"

class WorldMapResultWorld;

/// Markers shown on the nodes of one world (WorldMapResultWorld::mUnit18).
class WorldMapPointMarks {
public:
    WorldMapPointMarks(WorldMapResultWorld* pWorld);

    void init();
    s32 findReachablePoint();
    bool tryUseMark(s32 pointIndex);
    void emitGoEffect(s32 pointIndex);
    MiniRoutePoint* find(s32 pointIndex);
    void appearOpened();
    void killAll();
    void updateReachable();
    void updateAll();
    void setupMarks();

    WorldMapResultWorld* mWorld;  // 0x00
    s32 mNum;                     // 0x04
    MiniRoutePoint** mMarks;       // 0x08
};
