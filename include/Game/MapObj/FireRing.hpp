#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/MtxUtil.hpp"

class FireRingBall;
class WalkerStateBindStarPointer;

// SMG2-only unit (written from the target asm). Offsets are real SMG2 offsets.

// A ring of fire balls that rotates around the ring's front axis.
class FireRing : public LiveActor {
public:
    FireRing(const char*);

    virtual ~FireRing();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorDead();
    virtual MtxPtr getBaseMtx() const;
    virtual void startClipped();
    virtual void endClipped();

    void initBalls(const JMapInfoIter&);
    void calcBallPositions();
    bool isAnyBallBinding() const;
    f32 calcRadiusSpeed() const;

    void exeWait();
    void exeAccel();
    void exeRotate();

    /* 0x90 */ TPos3f mBaseMtx;
    /* 0xC0 */ FireRingBall** mBalls;
    /* 0xC4 */ s32 mBallCount;
    /* 0xC8 */ f32 mRotSpeed;
    /* 0xCC */ f32 mRadius;
    /* 0xD0 */ f32 mRadiusMax;
    /* 0xD4 */ f32 mRadiusMin;
    /* 0xD8 */ f32 mRadiusSpeed;
    /* 0xDC */ s32 mDuration;
    /* 0xE0 */ bool mIsShadowDropDirPtr;
    /* 0xE4 */ s32 mShadowLength;
};

class FireRingBall : public PartsModel {
public:
    FireRingBall(LiveActor*, const char*, const char*);

    virtual ~FireRingBall();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void setFrontAndUp(const TVec3f&, const TVec3f&);
    void setShadowDropDirectionPtr();
    void setShadowDropDirection(const TVec3f&);
    void setShadowDropLength(f32);
    bool isNerveBindStarPointer() const;

    /* 0xA0 */ WalkerStateBindStarPointer* mBindStarPointerState;
    /* 0xA4 */ TVec3f mFront;
    /* 0xB0 */ TVec3f mUp;
    /* 0xBC */ TVec3f mShadowDropDir;
    /* 0xC8 */ bool mIsShadowDropDirPtr;
};
