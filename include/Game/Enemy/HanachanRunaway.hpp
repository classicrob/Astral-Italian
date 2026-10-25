#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/MathUtil.hpp"

class AnimScaleController;
class HitSensor;
class LiveActorGroup;
class TubeSlider;

class HanachanRunaway;

// Body part (one per extra segment). Forwards hit callbacks to the owner.
class HanachanRunawayParts : public LiveActor {
public:
    HanachanRunawayParts(HanachanRunaway* pOwner, s32 index);

    virtual ~HanachanRunawayParts();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

public:
    /* 0x90 */ HanachanRunaway* mOwner;
    /* 0x94 */ TPos3f mBaseMtx;
    /* 0xC4 */ s32 mIndex;
};

// SMG2-only unit (runaway Hanachan that rides a tube slider). LiveActor = 0x90 bytes in SMG2.
class HanachanRunaway : public LiveActor {
public:
    HanachanRunaway(TubeSlider* pSlider);

    virtual ~HanachanRunaway();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWhistle();
    void exeBindStarPointer();
    bool isStarPointerPointing();
    void updateTubeSliderParts();

public:
    /* 0x90 */ TubeSlider* mTubeSlider;
    /* 0x94 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x98 */ bool mIsStrong;
    /* 0x9C */ s32 mPartsCount;      // parts = mPartsCount - 1
    /* 0xA0 */ LiveActorGroup* mParts;
    /* 0xA4 */ TVec3f mDir;
    /* 0xB0 */ f32 mRailCoord;
    /* 0xB4 */ f32 mRotation;
    /* 0xB8 */ s32 mSpeedA;
    /* 0xBC */ s32 mSpeedB;
    /* 0xC0 */ bool mIsPointed;
    /* 0xC4 */ f32 mPrevRailCoord;
};
