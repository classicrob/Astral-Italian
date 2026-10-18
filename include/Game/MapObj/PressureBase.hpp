#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroupArray.hpp"
#include "Game/Util/JointController.hpp"

class ActorStateParamScale;

class PressureMessenger : public LiveActor {
public:
    PressureMessenger(MsgSharedGroup*, ActorStateParamScale*, const char*);

    virtual ~PressureMessenger(){};
    virtual void init(const JMapInfoIter&);

    void exeSync();

    /* 0x90 */ MsgSharedGroup* mSharedGroup;
    /* 0x94 */ ActorStateParamScale* mParamScale;
    /* 0x98 */ s32 mWaitSyncTime;
};

class PressureBase : public LiveActor {
public:
    enum ShotType {
        ShotType_OnGravity = 0,
        ShotType_1 = 1,
        ShotType_Follow = 2,
        ShotType_AimTurn = 3,
    };

    PressureBase(const char*);

    virtual ~PressureBase(){};

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void initBullet(const JMapInfoIter&);
    virtual bool shotBullet(f32);

    void exeBound();
    void exeRelax();
    void exeSyncWait();
    void exeFirstWait();
    void exeWait();
    void exePrepareToShot();
    void exeShot();
    void startWait();
    void startRelax();
    bool calcJointCannonV(TPos3f*, const JointControllerInfo&);
    bool isShotTypeOnGravity() const;
    bool isShotTypeFollow() const;

    /* 0x90 */ ActorStateParamScale* mParamScale;
    /* 0x94 */ JointControlDelegator< PressureBase >* mJointController;
    /* 0x98 */ TVec3f mFront;
    /* 0xA4 */ f32 mRelaxAngle;
    /* 0xA8 */ f32 mBaseAngle;
    /* 0xAC */ s32 mWaitTime;
    /* 0xB0 */ f32 mBallSpeed;
    /* 0xB4 */ s32 mShotType;
    /* 0xB8 */ bool mIsShortShot;
    /* 0xBC */ PressureMessenger* mMessenger;
    /* 0xC0 */ LiveActorGroup* mGroup;
    /* 0xC4 */ bool _BC;
};
