#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class LiveActorGroup;

// SMG2-only unit (no Petari source). ElectricRail2Point drives an array of rail-following element actors.
// Element stride is 0x94 (LiveActor 0x90 + 4).

class ElectricRail2PointElem : public LiveActor {
public:
    /* 0x90 */ s32 _90;
};

class ElectricRail2Point : public LiveActor {
public:
    ElectricRail2Point(const char* pName);

    virtual ~ElectricRail2Point();
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual void draw() const;

    void exeMove();
    // unnamed fn_8029B490 in the target: keeps the two side vectors perpendicular to the rail direction
    void updateSideVectors(const TVec3f& rDir);
    // unnamed fn_8029B550 / fn_8029B5B0: debug line strip of the electric rail
    void drawRailSide(f32 width) const NO_INLINE;
    void drawRailQuad(f32 w0, f32 w1, f32 h0, f32 h1) const;

    /* 0x90 */ LiveActorGroup* mRailGroup;
    /* 0x94 */ TVec3f mFootPos;
    /* 0xA0 */ s32 mCurIndex;
    /* 0xA4 */ s32 mDivCount;
    /* 0xA8 */ TVec3f mSideA;
    /* 0xB4 */ TVec3f mSideB;
    /* 0xC0 */ ElectricRail2PointElem* mElems;
    /* 0xC4 */ s32 mElemCount;
};

