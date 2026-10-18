#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class CollisionParts;
class YoshiLockOnTarget;
class YoshiCapture : public LiveActor {
public:
    YoshiCapture(const char*);
    virtual ~YoshiCapture();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exeWait();
    void exeCatch();
    void exeRelease();
    void facePlayer();
    void setPose(const TPos3f&);
    /* 0x90 */ CollisionParts* mGround;
    /* 0x94 */ YoshiLockOnTarget* mLockOnTarget;
    /* 0x98 */ TPos3f mGroundRelativePose;
    /* 0xC8 */ TVec3f mClippingPosition;
    /* 0xD4 */ bool mFixedDirection;
};
