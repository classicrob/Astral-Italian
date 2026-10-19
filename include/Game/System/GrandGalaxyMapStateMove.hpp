#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class GrandGalaxyMap;

/// State "State[移動]" of the grand galaxy map sequence: flies the rocket along the path to the selected world (0x58 bytes).
class GrandGalaxyMapStateMove : public ActorStateBase< GrandGalaxyMap > {
public:
    GrandGalaxyMapStateMove(GrandGalaxyMap* pHost);

    virtual ~GrandGalaxyMapStateMove();
    virtual void init();
    virtual void appear();
    virtual void kill();

    void exeMove();

    /* 0x10 */ f32 mAccel;
    /* 0x14 */ f32 mMaxSpeed;
    /* 0x18 */ f32 mFriction;
    /* 0x1C */ TVec3f mVelocity;
    /* 0x28 */ TVec3f mPos;
    /* 0x34 */ TVec3f mStartPos;
    /* 0x40 */ TVec3f mCameraStartPos;
    /* 0x4C */ s32 mFrom;
    /* 0x50 */ s32 mTo;
    /* 0x54 */ s32 mStep;
};
