#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;

class QuestionCoin : public LiveActor {
public:
    QuestionCoin(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void kill();
    virtual void startClipped();
    virtual void endClipped();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initFromJMapInfoArg(const JMapInfoIter&);
    void initShadow();
    void updateActorMtx();
    void exeWait() {
    }

    void exeThrowActor();
    void exeCaught();
    bool isThrowActor() const {
        return 0.0f < mLaunchSpeed;
    }

    /* 0x90 */ TPos3f mBaseMtx;
    /* 0xC0 */ LiveActor* mBoundActor;
    /* 0xC4 */ TPos3f mActorMtx;
    /* 0xF4 */ TQuat4f mLaunchQuat;
    /* 0x104 */ ActorCameraInfo* mCameraInfo;
    /* 0x108 */ f32 mLaunchSpeed;
    /* 0x10C */ bool mIsWeakGravity;
    /* 0x10D */ bool mCalcShadow;
    /* 0x110 */ s32 mPoseType;
    /* 0x114 */ s32 mShadowDropLengthType;
    /* 0x118 */ TVec3f mShadowPos;
    /* 0x124 */ f32 mShadowDropLength;
};
