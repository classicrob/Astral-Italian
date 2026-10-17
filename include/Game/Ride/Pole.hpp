#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;

class Pole : public LiveActor {
public:
    Pole(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void calcAnim();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual MtxPtr getBaseMtx() const {
        if (mNoModel) {
            return (MtxPtr)&mBaseMtx;
        } else {
            return LiveActor::getBaseMtx();
        }
    }

    bool isSquare() const {
        return mIsSquare || m2Way;
    }

    bool isTree() const {
        return mIsTreeA || mIsTreeB;
    }

    void exeDemoAppear();
    void exeFree();
    void exeFreeInvalid();
    void exeBindStart();
    void exeBindWait();
    void exeBindTurn();
    void exeBindClimbUp();
    void exeBindFallDown();
    void exeBindHandstandStart();
    void exeBindHandstandWait();
    void exeBindHandstandEnd();
    void exeBindHandstandTurn();
    bool tryJump(bool, f32);
    bool tryTurn();
    bool tryClimbUp();
    bool tryFallDown();
    bool tryHandstandTurn();
    bool isEnableTurn() const;
    void updateBindTrans();
    void calcGravityMtx(TPos3f*);
    void updateTopPos(f32);
    f32 getPoleSubPadStickX() const;
    f32 getPoleSubPadStickY() const;
    bool tryStartTreeAnim(const char*);

    /* 0x90 */ TVec3f mBasePos;
    /* 0x9C */ TVec3f mTopPos;
    /* 0xA8 */ f32 mHeight;
    /* 0xAC */ f32 mTopMargin;
    /* 0xB0 */ bool mDisableHandstand;
    /* 0xB1 */ bool mIsSquare;
    /* 0xB2 */ bool m2Way;
    /* 0xB3 */ bool mNoModel;
    /* 0xB4 */ bool mInvertStick;
    /* 0xB5 */ bool mUsePole9mCollision;
    /* 0xB6 */ bool mIsTreeA;
    /* 0xB7 */ bool mIsTreeB;
    /* 0xB8 */ bool mIsTwisterTower;
    /* 0xBC */ f32 mClimbCoord;
    /* 0xC0 */ f32 mClimbSpeed;
    /* 0xC4 */ LiveActor* mRider;
    /* 0xC8 */ ActorCameraInfo* mCameraInfo;
    /* 0xCC */ TVec3f mCenterPos;
    /* 0xD8 */ TVec3f mSide;
    /* 0xE4 */ TVec3f mUp;
    /* 0xF0 */ TVec3f mFront;
    /* 0xFC */ TPos3f mPosMtx;
    /* 0x12C */ MtxPtr mTopMtx;
    /* 0x130 */ MtxPtr mBottomMtx;
    /* 0x134 */ TMtx34f mBaseMtx;
};
