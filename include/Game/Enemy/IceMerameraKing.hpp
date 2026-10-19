#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util/Array.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class CameraTargetDemoActor;
class FixedPosition;
template < typename T >
class JointControlDelegator;
class JointControllerInfo;
class Meramera;
class SpinPullParticleCallBack;
class ThrowingIce;

class IceMerameraKing : public LiveActor {
public:
    IceMerameraKing(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeSearch();
    void exeThrow();
    void endThrow();
    void tearDownThrow();
    void exeExtinguish();
    void exeEscape();
    void endEscape();
    void exeEscapeJump();
    void exeDamage();
    void exePreRecover();
    void exeRecover();
    void exePreAttack();
    void exeAttack();
    void exeAttackAfter();
    void exeAngryDemo();
    void exeDeathDemoWait();
    void exeDeathDemo();
    void addVelocityToInitPos();
    bool isDeadAllIce();
    bool isEnableThrow();
    ThrowingIce* getDeadWeaponAndAppear();
    bool calcJoint(TPos3f*, const JointControllerInfo&);

    /* 0x08C */ FixedPosition* mFixedPos;
    /* 0x090 */ ThrowingIce* mThrowingIce;
    /* 0x094 */ Meramera* mMeramera;
    /* 0x098 */ SpinPullParticleCallBack* mSpinParticle;
    /* 0x09C */ MR::Vector< MR::AssignableArray< ThrowingIce* > > mIce;
    /* 0xAC */ LiveActor* mShockWave;
    /* 0xB0 */ LiveActor* mBreakModel;
    /* 0xB4 */ TVec3f mFront;
    /* 0xC0 */ TVec3f mUp;
    /* 0xCC */ TVec3f mHomePos;
    /* 0xD8 */ TVec3f mHomeRotate;
    /* 0xE4 */ s32 mThrowCount;
    /* 0xE8 */ s32 mNextIceIndex;
    /* 0xEC */ u32 mEscapeFrames;
    /* 0xF0 */ s32 mHealth;
    /* 0xF4 */ s32 mIceCount;
    /* 0x0F4 */ Meramera** mModelArray;
    /* 0x0F8 */ CameraTargetDemoActor* mCameraTarget;
    /* 0x0FC */ JointControlDelegator< IceMerameraKing >* mJointController;
    /* 0x104 */ TQuat4f mRollQuat;
    /* 0x114 */ TVec3f mBinderOffset;
    /* 0x120 */ f32 mFlightHeight;
    /* 0x124 */ bool mHasPlayedPinchBgm;
    /* 0x125 */ bool mIsIntroComplete;
};

class IceMerameraKingShockWave : public ModelObj {
public:
    IceMerameraKingShockWave();

    virtual void appear();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
};
