#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SupportTicoStop;
class JMapInfoIter;

namespace MR {
    bool isMsgRollingWaterPlayerEnter(u32);
    bool isMsgRollingWaterPlayerExit(u32);
}

/// @brief Rolling water ball (SMG2-only). Spawned and recycled by RollingWaterCreator through a LiveActorGroup.
class RollingWater : public LiveActor {
public:
    RollingWater(const char*);

    virtual ~RollingWater();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool isNerveRide() const;
    bool moveRoute();
    void updateVelocityAndRotation();
    bool updateTimerAndCheckArea();
    bool moveOnRail();
    void updateRollMoment();
    void resetToStart();
    void appearFromCreator(LiveActor*, s32);
    s32 calcRailStepCount() const;
    bool tryAttachTicoTouch();
    bool checkPlayerAttack();

    void exeWait() NO_INLINE;
    void exeRide() NO_INLINE;
    void exeRush() NO_INLINE;
    void exeFall() NO_INLINE;

    /* 0x90 */ LiveActor* mCreator;
    /* 0x94 */ SupportTicoStop* mTicoStop;
    /* 0x98 */ TQuat4f mQuatBase;
    /* 0xA8 */ TQuat4f mQuatTarget;
    /* 0xB8 */ TQuat4f mQuatRail;
    /* 0xC8 */ TVec3f mBlendVec;
    /* 0xD4 */ TVec3f mCenterVec;
    /* 0xE0 */ TVec3f mStartPos;
    /* 0xEC */ TVec3f mAddVel;
    /* 0xF8 */ TVec3f mUpVec;
    /* 0x104 */ s32 mTimer;
    /* 0x108 */ f32 mRadius;
    /* 0x10C */ f32 mRailSpeed;
    /* 0x110 */ f32 mSpeedRate;
    /* 0x114 */ f32 mAngVel;
    /* 0x118 */ s32 mStepCount;
    /* 0x11C */ bool mIsTouched;
};
