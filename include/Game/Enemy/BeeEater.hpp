#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/JointController.hpp"

class AnimScaleController;
class BeeEater;
class HitSensor;
class JointController;
class WalkerStateBindStarPointer;

class BeeEaterJointControl {
public:
    BeeEaterJointControl(LiveActor*, const char*);
    void setBeeEaterJointDirection(const TVec3f*) NO_INLINE;
    bool calcBeeEaterJointMatrix(TPos3f*, const JointControllerInfo&) NO_INLINE;

    JointController* mController;
    TVec3f mPos;
    TVec3f mDir;
};

class BeeEaterBody : public PartsModel {
public:
    BeeEaterBody(BeeEater* pHost, const char* pName, const char* pJointName);

    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();

public:
    /* 0xA0 */ BeeEaterJointControl** mJointCtrls;
    /* 0xA4 */ s32 mJointCount;
};

class BeeEater : public LiveActor {
public:
    BeeEater(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    bool isPlayerInBeeEaterAimRange() const NO_INLINE;
    bool isPlayerBehindBeeEaterRotation() const NO_INLINE;
    void updateBeeEaterHover() NO_INLINE;
    bool updateBeeEaterDash() NO_INLINE;
    void exeBeeEaterWaitAfterHit() NO_INLINE;
    void exeBeeEaterFind() NO_INLINE;
    void exeBeeEaterAttackSign() NO_INLINE;
    void exeBeeEaterAttack() NO_INLINE;
    void exeBeeEaterAttackEnd() NO_INLINE;
    void exeBeeEaterAttackSuccess() NO_INLINE;

public:
    /* 0x90 */ TQuat4f mRot;
    /* 0xA0 */ TVec3f mBasePos;
    /* 0xAC */ TVec3f mSideVec;
    /* 0xB8 */ TVec3f mUpVec;
    /* 0xC4 */ f32 mRangeLimit;      // 450.0f in the ctor
    /* 0xC8 */ TVec3f mTargetVec;
    /* 0xD4 */ TVec3f mFaceVec;      // (0, 0, 1) in the ctor
    /* 0xE0 */ TVec3f mCurveStart;
    /* 0xEC */ TVec3f mCurveControl1;
    /* 0xF8 */ TVec3f mCurveControl2;
    /* 0x104 */ TVec3f mCurveEnd;
    /* 0x110 */ BeeEaterBody* mBody;
    /* 0x114 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x118 */ WalkerStateBindStarPointer* mStarPointBind;
};
