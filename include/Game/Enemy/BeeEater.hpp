#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"

class AnimScaleController;
class BeeEater;
class HitSensor;
class JointController;
class WalkerStateBindStarPointer;

// Stem/body model of the BeeEater. Owns five joint-controller delegates (created by fn_8015BA20 in init).
class BeeEaterBody : public PartsModel {
public:
    BeeEaterBody(BeeEater* pHost, const char* pName, const char* pJointName);

    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();

public:
    /* 0xA0 */ JointController** mJointCtrls;
    /* 0xA4 */ s32 mJointCount;
};

// SMG2-only unit (BeeEater, bee-shaped walker that bites star-pointer targets). LiveActor = 0x90 bytes.
// The ctor stores quaternions/vectors into an overlapping run (0x90..0xDC); the layout below follows
// the ctor and the init/exe accesses (TQuat4 at 0x90, TVec3 at 0xA0/0xAC/0xB8/0xC8, face vec at 0xD4).
class BeeEater : public LiveActor {
public:
    BeeEater(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    void exeHit();
    void exeBindStarPointer();
    void exeFind();
    void exeAttack();
    void exeAttackEnd();
    void exeAttackSuccess();
    void exeAttackDash();

    bool isPlayerNearFront() const;
    bool isFacingPlayer() const;
    bool moveAttackDash();

public:
    /* 0x90 */ TQuat4f mRot;
    /* 0xA0 */ TVec3f mBasePos;
    /* 0xAC */ TVec3f mSideVec;
    /* 0xB8 */ TVec3f mUpVec;
    /* 0xC4 */ f32 mRangeLimit;      // 450.0f in the ctor
    /* 0xC8 */ TVec3f mTargetVec;
    /* 0xD4 */ TVec3f mFaceVec;      // (0, 0, 1) in the ctor
    /* 0x110 */ BeeEaterBody* mBody;
    /* 0x114 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x118 */ WalkerStateBindStarPointer* mStarPointBind;
};
