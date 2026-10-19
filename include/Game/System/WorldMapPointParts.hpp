#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class WorldMapResultWorld;

/// One row of the "PointParts" list (0x1C bytes).
struct PointPart {
    PointPart();

    const char* mType;  // 0x00
    s32 mPointIndex;    // 0x04
    s32 mPartsIndex;    // 0x08
    s32 mParam0;        // 0x0C
    s32 mParam1;        // 0x10
    s32 mParam2;        // 0x14
    LiveActor* mActor;  // 0x18
};

/// Warp info filled for the callers (the warp parameters sit at 0x10).
class WorldMapResultStateWorldWarp;

struct PartsWarpInfo {
    u8 _0[0x10];
    s32 mParam0;
    s32 mParam1;
    s32 mPointIndex;
};

/// The parts (warps, check points, pipes ...) sitting on the map nodes of one world (WorldMapResultWorld::mUnit14).
class PointPartsHolder {
public:
    PointPartsHolder(WorldMapResultWorld* pWorld);

    void init();
    bool getWorldWarp(s32 pointIndex, WorldMapResultStateWorldWarp* pOut);
    bool getStarRoadWarp(s32 pointIndex, PartsWarpInfo* pOut) NO_INLINE;
    s32 getWarpTarget(s32 pointIndex);
    s32 getPipeTarget(s32 pointIndex);
    s32 getPipeOtherEnd(s32 pointIndex);
    LiveActor* getTicoCreator(s32 pointIndex);
    LiveActor* getWorldWarpActor(s32 pointIndex);
    bool isTicoCreatorNotFull(s32 pointIndex);
    LiveActor* getActor(s32 pointIndex);
    bool isPipeTarget(s32 pointIndex, s32 target);
    bool isNotStarPieceMine(s32 pointIndex);
    bool isActorAlive(s32 pointIndex);
    void callStarCheckPoint(s32 pointIndex);
    s32 getActorField94(s32 pointIndex);
    bool isStarCheckPointReady(s32 pointIndex);
    bool isStarCheckPointPending(s32 pointIndex);
    bool isAnyStarCheckPointPending();
    bool isWarpOpen(s32 pointIndex);
    bool isEarthenPipe(s32 pointIndex);
    bool isWorldWarp(s32 pointIndex);
    bool tryTalkTico(s32 pointIndex);
    bool tryTalkStarCheckPoint(s32 pointIndex);
    void appearAll();
    void killAll();
    void makeActorAppearedAll();
    void makeActorDeadAll();
    void resetAll();
    void resetStarCheckPoints();
    PointPart* find(s32 pointIndex);
    s32 findActivePoint();
    bool tryTico72C0(s32 pointIndex);
    bool tryStarCheck6000(s32 pointIndex);
    bool tryPipe3420(s32 pointIndex);
    bool updateAll();
    void setEffectDrawOrder(s32 order);

    WorldMapResultWorld* mWorld;  // 0x00
    PointPart* mParts;            // 0x04
    s32 mNum;                     // 0x08
    s32 mUnkC;                    // 0x0C
};
