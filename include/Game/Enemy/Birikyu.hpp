#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorStateParamScale;

class Birikyu : public LiveActor {
public:
    Birikyu(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void control();
    virtual void movement();
    virtual void startClipped();
    virtual void endClipped();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    f32 getHitRadius() const;
    char* getCenterJointName() const;

    void initFromJmpArgs(const JMapInfoIter&);
    void initRail(const JMapInfoIter&);
    void initCollision();
    void initShadow();
    bool tryStopPointing();
    void goMove();
    void exeMove();
    void exeMoveCircle();
    void exeWaitAtEdge();
    void exeAttack();
    void exeAttackWait();
    void exeStopPointing();
    void endStopPointing();

    /* 0x90 */ ActorStateParamScale* mParamScale;
    /* 0x94 */ const char* mObjectName;
    /* 0x98 */ TVec3f mRailClippingCenter;
    /* 0xA4 */ TVec3f mOrbitCenter;
    /* 0xB0 */ bool mIsShadowEnabled;
    /* 0xB1 */ bool mIsRailMovement;
    /* 0xB4 */ TVec3f mOrbitAxis;
    /* 0xC0 */ TVec3f mOrbitFront;
    /* 0xCC */ f32 mOrbitAngle;
    /* 0xD0 */ f32 mMoveSpeed;
};

class BirikyuWithFace : public Birikyu {
public:
    BirikyuWithFace(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAndSetBaseMtx();
    f32 getHitRadius() const;
    char* getCenterJointName() const;

    /* 0xD4 */ bool mIsFaceReversed;
    /* 0xD8 */ s32 mOrientationMode;
};
