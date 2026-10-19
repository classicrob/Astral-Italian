#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class GrandGalaxyMap;

/// State "State[ワールドワープ]" (0x1C bytes, 0x804E2F00..). Receives the star road warp of the
/// current point at 0x10/0x14 (PointPartsHolder::getStarRoadWarp writes PartsWarpInfo::mParam0/mParam1 there).
class GrandGalaxyMapStateWorldWarp : public ActorStateBase< GrandGalaxyMap > {
public:
    GrandGalaxyMapStateWorldWarp(GrandGalaxyMap* pHost);

    virtual ~GrandGalaxyMapStateWorldWarp();
    virtual void init();
    virtual void appear();
    void exeWarp();

    /* 0x10 */ s32 mWarpParam0;
    /* 0x14 */ s32 mWarpParam1;
    /* 0x18 */ f32 mFovy;
};
