#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorStateParamScale;

class Dossun : public LiveActor {
public:
    Dossun(const char*);

    virtual void init(const JMapInfoIter&) override;
    virtual void movement() override;
    virtual void appear() override;
    virtual void control() override;

    void initMapToolInfo(const JMapInfoIter&);
    void initCollision(const JMapInfoIter&);
    void initShadow();
    void calcParameters();
    s32 getUpperFrame() const;
    s32 getStepUntilRising();
    void exeReady();
    void exeUpper();
    void exeFallSign();
    void exeFalling();
    void exeOnGround();
    void exeRising();

    /* 0x90 */ ActorStateParamScale* mState;
    /* 0x94 */ TVec3f _94;
    /* 0xA0 */ TVec3f _A0;
    /* 0xAC */ s32 mFallingTime;
    /* 0xB0 */ s32 mHoldTime;
    /* 0xB4 */ f32 mMovementDist;
    /* 0xB8 */ s32 mRisenWaitTime;
    /* 0xBC */ s32 mOnGroundTime;
    /* 0xC0 */ s32 mShadowType;
    /* 0xC4 */ bool mHasShadow;
};
