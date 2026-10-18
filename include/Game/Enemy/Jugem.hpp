#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/JointController.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;
class JointController;
class Togezo;
class JugemCloud;

/// @brief Jugem (lakitu-like cloud boss that spawns and throws Togezo children).
/// Layout derived from SB4E01 (LiveActor is 0x90 bytes in SMG2).
class Jugem : public LiveActor {
public:
    Jugem(const char*);
    virtual ~Jugem();

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initBinder();
    bool calcRootJoint(TPos3f*, const JointControllerInfo&);
    bool calcWristJoint(TPos3f*, const JointControllerInfo&);

    // Nerve execs (names provisional, see Jugem2.cpp for the nerve classes).
    void exeWait();
    void exeReady();
    void exeAim();
    void exeApproach();
    void exeMove();
    void exeMoveUp();
    void exeThrowSign();
    void exeThrow();
    void exeChase();
    void exeHitReaction();
    void exeFall();
    void exeDrop();
    void exeLand();
    void exeHide();
    void exeReset();
    void exeBlowDown();
    void exeFallDown();
    void exeEmerge();
    void exeDie();
    void exeFallLand();
    void exeFallRecovery();
    void exeSupportFreeze();

    /* 0x090 */ JointController* mJointCtrlA;
    /* 0x094 */ JointController* mJointCtrlB;
    /* 0x098 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x09C */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x0A0 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x0A4 */ YoshiLockOnTarget* mYoshiLockOn2;
    /* 0x0A8 */ TMtx34f mBaseMtx;
    /* 0x0D8 */ TMtx34f mSubMtx;
    /* 0x108 */ JugemCloud* mCloud;
    /* 0x10C */ Togezo* mCurrentTogezo;
    /* 0x110 */ NameObj** mChildObjs;
    /* 0x114 */ LiveActor** mChildren;
    /* 0x118 */ TVec3f* mChildPoses;
    /* 0x11C */ s32* mChildFlags;
    /* 0x120 */ TVec3f mFrontVec;
    /* 0x12C */ TVec3f mBasePos;
    /* 0x138 */ f32 _138;
    /* 0x13C */ s32 mChildNum;
    /* 0x140 */ s32 mChildNumPlusOne;
    /* 0x144 */ s32 mChildIndex;
    /* 0x148 */ s32 mStepCounter;
    /* 0x14C */ s32 mChildCount;
    /* 0x150 */ s32 mWaitTime;
    /* 0x154 */ s32 _154;
    /* 0x158 */ u8 _158[0xC];
    /* 0x164 */ TVec3f _164;
    /* 0x170 */ s32 _170;
    /* 0x174 */ s32 _174;
    /* 0x178 */ f32 _178;
    /* 0x17C */ s32 mTimer;
    /* 0x180 */ u8 mIsActive;
    /* 0x181 */ s8 mIsHitDone;
};

/// @brief Cloud model that the Jugem stands on.
class JugemCloud : public PartsModel {
public:
    JugemCloud(Jugem* pHost, const char* pName);
    virtual ~JugemCloud();

    virtual void makeActorAppeared();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void disappear();
    void tryDisappear();

    /* 0xA0 */ Jugem* mJugem;
    /* 0xA4 */ bool mIsAppear;
};
