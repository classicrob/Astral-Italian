#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class ActorCameraInfo;
class BossJugem;
class BossJugemCloud;
class DemoObj;
class HitSensor;
class Nerve;
class Togezo;
class TogezoMgr;

// SMG2-only boss "BossJugem" (Lakitu boss) family, no Petari counterpart. Layouts and names derived from SB4E01
// (LiveActor is 0x90 bytes in SMG2). Declared across BossJugem.cpp and BossJugemTU01..TU05.cpp.

class BossJugemStateMove : public ActorStateBase< BossJugem > {
public:
    BossJugemStateMove(BossJugem*);

    virtual ~BossJugemStateMove() {}
    virtual void init();
    virtual void appear();

    void exeMove();   // fn_800A19D0
    void prepare();   // fn_800A1C20

    /* 0x10 */ f32 mRailStart;
    /* 0x14 */ s32 mDirIdx;
    /* 0x18 */ s32 mLengthIdx;
};

class BossJugemStateThrowOne : public ActorStateBase< BossJugem > {
public:
    BossJugemStateThrowOne(BossJugem*);

    virtual ~BossJugemStateThrowOne() {}
    virtual void init();
    virtual void appear();

    void exeThrowOneSign();  // fn_800A2120
    void exeThrowOne();      // fn_800A21D0
    void prepareMove();      // fn_800A2280
    void updateMove();       // fn_800A22E0

    /* 0x10 */ f32 mSpeed;
    /* 0x14 */ f32 mAngle;
    /* 0x18 */ bool mIsDouble;
    /* 0x1C */ s32 mMoveStep;
    /* 0x20 */ f32 mRailStart;
};

class BossJugemStateThrowTwo : public ActorStateBase< BossJugem > {
public:
    BossJugemStateThrowTwo(BossJugem*);

    virtual ~BossJugemStateThrowTwo() {}
    virtual void init();
    virtual void appear();

    void exeThrowTwoSign();  // fn_800A24B0
    void exeThrowTwo();      // fn_800A2580
    void prepareMove();      // fn_800A2650
    void updateMove();       // fn_800A26B0

    /* 0x10 */ f32 mSpeed;
    /* 0x14 */ f32 mAngle;
    /* 0x18 */ bool mIsDouble;
    /* 0x1C */ s32 mMoveStep;
    /* 0x20 */ f32 mRailStart;
};

class BossJugemStateThrowAll : public ActorStateBase< BossJugem > {
public:
    BossJugemStateThrowAll(BossJugem*);

    virtual ~BossJugemStateThrowAll() {}
    virtual void init();
    virtual void appear();

    void exeMove();       // fn_800A2B00
    void exeThrowOne();   // fn_800A2CC0
    void exeThrowTwo();   // fn_800A2D30
    void exeChance();     // fn_800A2DC0
    void exeMoveBack();   // fn_800A2E50
    void selectThrow();   // fn_800A2F80

    /* 0x10 */ s32 _10;
    /* 0x14 */ BossJugemStateThrowOne* mThrowOne;
    /* 0x18 */ BossJugemStateThrowTwo* mThrowTwo;
    /* 0x1C */ TVec3f mTargetPos;
    /* 0x28 */ TVec3f mOriginPos;
    /* 0x34 */ s32 mMoveFrames;
    /* 0x38 */ f32 _38;
    /* 0x3C */ f32 mSpeedBonus;
    /* 0x40 */ s32 mRemain;
};

class BossJugemThunder;

class BossJugemStateDash : public ActorStateBase< BossJugem > {
public:
    BossJugemStateDash(BossJugem*);

    virtual ~BossJugemStateDash() {}
    virtual void init();
    virtual void appear();
    virtual void kill();

    bool isInvincible();  // fn_800A0A20: true unless the "DashEnd" action plays
    void exeDashSign();         // fn_800A0A60
    void exeDashCharge();       // fn_800A0B70
    void exeDash();             // fn_800A0D90
    void exeDashEnd();          // fn_800A0FE0
    f32 calcRushTarget(TVec3f*, TVec3f*, const TVec3f&);  // fn_800A1130

    /* 0x10 */ BossJugemThunder* mThunder;
    /* 0x14 */ TVec3f mDashDir;
    /* 0x20 */ TVec3f mStartPos;
};

class BossJugemStateSleep : public ActorStateBase< BossJugem > {
public:
    BossJugemStateSleep(BossJugem*);

    virtual ~BossJugemStateSleep() {}
    virtual void init();
    virtual void appear();

    void exeSleep();      // fn_800A1E00
    void exeSleepWait();  // fn_800A1E60
    void exeAwake();      // fn_800A1EC0
};

class BossJugem : public LiveActor {
public:
    BossJugem(const char*);

    virtual ~BossJugem() {}
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void startActionAll(const char*);                  // fn_8009DA30
    void killHoldTogezo();                             // fn_8009DA80
    void appearHoldTogezoR();                          // fn_8009DB00
    void appearHoldTogezoL();                          // fn_8009DBA0
    void releaseTogezoR(f32, f32);                     // fn_8009DBF0
    void releaseTogezoL(f32, f32);                     // fn_8009DCB0
    s32 getMaxTogezoNum() const;                       // fn_8009DD70
    s32 getRemainTogezoNum() const;                    // fn_8009DDA0
    void updateMoveAnim(s32, s32);                     // fn_8009DDF0
    void updateMoveBackAnim(s32, s32);                 // fn_8009DF80
    void turnToPlayer(f32);                            // fn_8009E020
    void exeStandBy();                                 // fn_8009E090
    void exeDemoOpening();                             // fn_8009E160
    void exeWait();                                    // fn_8009E240
    void exeMove();                                    // fn_8009E2F0
    void exeWaitTogezo();                              // fn_8009E380
    void exeThrowOne();                                // fn_8009E460
    void exeThrowTwo();                                // fn_8009E4C0
    void exeThrowAll();                                // fn_8009E520
    void exeDash();                                    // fn_8009E580
    void exeDamage();                                  // fn_8009E5E0
    void exeAngry();                                   // fn_8009E6D0
    void exeMoveLeft();                                // fn_8009E760
    void exeMoveRight();                               // fn_8009E850
    void exeStarPieceReaction();                       // fn_8009E940
    void exeDemoDown();                                // fn_8009E9B0
    void exeDemoEnding();                              // fn_8009EA50
    bool isDamageable() const;                         // fn_8009EE00
    bool tryDamage(HitSensor*, HitSensor*);            // fn_8009EED0
    bool tryStarPieceReaction();                       // fn_8009EFA0
    bool isAngrySmoke() const;                         // fn_8009F010
    void resetBattle();                                // fn_8009F090
    void nextActionIndex();                            // fn_8009F120
    void selectNextAction();                           // fn_8009F1A0
    void selectThrowNerve();                           // fn_8009F3A0
    bool isPlayerTooFar();                             // fn_8009F4C0
    void killAllTogezo();                              // fn_8009F530
    void breakAllTogezo();                             // fn_8009F5D0
    void releaseHoldTogezoAll();                       // fn_8009F670
    void throwTogezo(Togezo*, const TVec3f&, f32, f32);  // fn_8009F6F0
    void updateCameraTarget();                         // fn_8009F830
    bool tryStartDemoCamera(const char*) NO_INLINE;    // fn_8009FA30
    bool tryEndDemoCamera(const char*);                // fn_8009FA40
    bool tryEndDemoCameraIfActionEnd(const char*);     // fn_8009FAB0
    void appearStarPieceToPlayer();                    // fn_8009FB20
    void updateCloudSway();                            // fn_8009FC20
    bool calcRootJoint(TPos3f*, const JointControllerInfo&);    // fn_8009FE60
    bool calcWristRJoint(TPos3f*, const JointControllerInfo&);  // fn_8009FE90
    bool calcWristLJoint(TPos3f*, const JointControllerInfo&);  // fn_8009FF00

    /* 0x090 */ JointController* mJointRoot;
    /* 0x094 */ JointController* mJointWristL;
    /* 0x098 */ JointController* mJointWristR;
    /* 0x09C */ BossJugemCloud* mCloud;
    /* 0x0A0 */ BossJugemStateMove* mStateMove;
    /* 0x0A4 */ BossJugemStateThrowOne* mStateThrowOne;
    /* 0x0A8 */ BossJugemStateThrowTwo* mStateThrowTwo;
    /* 0x0AC */ BossJugemStateThrowAll* mStateThrowAll;
    /* 0x0B0 */ BossJugemStateDash* mStateDash;
    /* 0x0B4 */ BossJugemStateSleep* mStateSleep;
    /* 0x0B8 */ TogezoMgr* mTogezoMgr;
    /* 0x0BC */ Togezo* mTogezoL;
    /* 0x0C0 */ Togezo* mTogezoR;
    /* 0x0C4 */ DemoObj* mPipo;
    /* 0x0C8 */ TMtx34f mHandMtxR;
    /* 0x0F8 */ TMtx34f mHandMtxL;
    /* 0x128 */ TVec3f mFrontVec;
    /* 0x134 */ s32 mPhase;
    /* 0x138 */ s32 mActionIdx;
    /* 0x13C */ bool mIsAngry;
    /* 0x13D */ bool mIsDashNext;
    /* 0x140 */ TVec3f mHomePos;
    /* 0x14C */ TVec3f mCameraTarget;
    /* 0x158 */ s32 mCameraRegisterId;
    /* 0x15C */ s32 mFarCount;
    /* 0x160 */ ActorCameraInfo* mCameraInfo;
    /* 0x164 */ TVec3f mCloudOffset;
    /* 0x170 */ s32 mSwayStepA;
    /* 0x174 */ s32 mSwayStepB;
    /* 0x178 */ f32 mSwayRate;
    /* 0x17C */ f32 mRailCoordStart;
};

class BossJugemCloud : public LiveActor {
public:
    BossJugemCloud(BossJugem*, const char*, MtxPtr);

    virtual ~BossJugemCloud() {}
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();

    void updateFloatSound();                                    // fn_800A0110
    bool calcRootJoint(TPos3f*, const JointControllerInfo&);  // fn_800A0200

    /* 0x90 */ BossJugem* mHost;
    /* 0x94 */ MtxPtr mBaseMtx;
    /* 0x98 */ JointController* mJointRoot;
    /* 0x9C */ TVec3f mPrevPos;
};

class VolumeDrawer;

// Lightning column drawn while the BossJugem dashes (two VolumeDrawers: the bolt and the dash range).
class BossJugemThunder : public LiveActor {
public:
    BossJugemThunder(BossJugem*, const char*);

    virtual ~BossJugemThunder() {}
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();

    void exeDisappear();  // fn_800A1580

    /* 0x090 */ BossJugem* mHost;
    /* 0x094 */ VolumeDrawer* mRangeDrawer;
    /* 0x098 */ TMtx34f mRangeMtx;
    /* 0x0C8 */ VolumeDrawer* mBoltDrawer;
    /* 0x0CC */ TMtx34f mBoltMtx;
    /* 0x0FC */ TVec3f _FC;
    /* 0x108 */ bool _108;
};
