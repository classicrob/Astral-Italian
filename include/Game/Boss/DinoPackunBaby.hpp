#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/ActorMovementUtil.hpp"

// SMG2-only boss "DinoPackunBaby" (ベビーディノパックン). No Petari counterpart.
// Units: DinoPackunBaby (actor), DinoPackunBabyTU01 (actor helpers + DinoPackunBabyBattle),
// DinoPackunBabyTU02 (DinoPackunBabyDemo state + DinoPackunBabyShell ctor).
// The unsplit region after TU02 (0x800BAFC0..) holds DinoPackunBabyShell's other members and DinoPackunBabyState.

class ActorCameraInfo;
class AnimScaleController;
class DinoPackunBaby;
class DinoPackunBabyBattle;
class DinoPackunBabyDemo;
class DinoPackunBabyShell;

// Shell part (size 0xD4). Ctor lives in TU02, the rest in the following (unsplit) unit.
class DinoPackunBabyShell : public PartsModel {
public:
    DinoPackunBabyShell(LiveActor*, const char*, const char*, const char*);

    virtual ~DinoPackunBabyShell();
    virtual void init(const JMapInfoIter&);
    virtual void kill();

    void startBreak(const char*);
    void updateModelMtx();
    void requestMovementOnModel();
    void killModel();

    /* 0xA0 */ LiveActor* mModel;
    /* 0xA4 */ TPos3f mModelMtx;
};

// Generic 0x28-byte walking state (ctor (host, name)), lives in the unsplit region after TU02.
class DinoPackunBabyState : public ActorStateBase< DinoPackunBaby > {
public:
    DinoPackunBabyState(DinoPackunBaby*, const char*);

    virtual ~DinoPackunBabyState();
    virtual void init();
    virtual void appear();
};

// TU02: demo state (size 0x20).
class DinoPackunBabyDemo : public ActorStateBase< DinoPackunBaby > {
public:
    DinoPackunBabyDemo(DinoPackunBaby*, const char*);

    virtual ~DinoPackunBabyDemo();
    virtual void init();
    virtual void appear();

    void setupDemo(const char*, const char*, const char*);
    void setRestoreShells();
    void clearPuppetEnd();
    void exeWait();
    void exeDemo();

    /* 0x10 */ const char* mDemoName;
    /* 0x14 */ const char* mPlayerBck;
    /* 0x18 */ LiveActor* mModel;
    /* 0x1C */ bool mIsRestoreShells;
    /* 0x1D */ bool mIsPuppetEnd;
};

// TU01: battle state machine (size 0x48), one for each half of the fight.
class DinoPackunBabyBattle : public ActorStateBase< DinoPackunBaby > {
public:
    DinoPackunBabyBattle(DinoPackunBaby*, bool);

    virtual ~DinoPackunBabyBattle();
    virtual void init();
    virtual void appear();
    virtual void control();

    void endChaseCamera();
    void attackSensor(HitSensor*, HitSensor*);
    bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeWander();
    void exeSearch();
    void exeNotice();
    void exeChase();
    void exeAttackJumpSign();
    void exeAttackJump();
    void exeAttackJumpLand();
    void exeAttackSuccess();
    void exeDamageShell();
    void exeDamageShellLand();
    void exeEscapeStart();
    void exeEscapeLand();
    void exeEscape();
    void exeEscapeTurnStart();
    void exeEscapeTurn();
    void exeDown();
    void exeDownLand();

    /* 0x10 */ bool mIsFirstHalf;
    /* 0x11 */ bool mIsAttackHit;
    /* 0x14 */ s32 mWanderStep;
    /* 0x18 */ const s32* mWanderTime;
    /* 0x1C */ const s32* mJumpTime;
    /* 0x20 */ const f32* mChaseJumpSpeed;
    /* 0x24 */ const s32* mLandChaseTime;
    /* 0x28 */ const s32* mLandAttackTime;
    /* 0x2C */ const s32* mDownDeathTime;
    /* 0x30 */ const f32* mShellDamageSpeed;
    /* 0x34 */ const f32* mEscapeJumpSpeed;
    /* 0x38 */ const f32* mDownSpeed;
    /* 0x3C */ const f32* mWanderSight;
    /* 0x40 */ const f32* mChaseSight;
    /* 0x44 */ const f32* mEscapeSight;
};

class DinoPackunBaby : public LiveActor {
public:
    DinoPackunBaby(const char*);

    virtual ~DinoPackunBaby();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeDemoBattleStart();
    void exeBattle1st();
    void exeDemoPowerUp();
    void exeBattle2nd();
    void exeDemoDeath();

    // TU01 helpers (names provisional, see report: fn_XXXX -> mangled)
    void chasePlayer(const MR::ActorMoveParam*, bool);                // fn_800B81E0
    void escapeFromPlayer(const MR::ActorMoveParam*, bool);           // fn_800B8260
    bool updateJumpState(ActorStateBaseInterface*);                   // fn_800B82E0
    bool isInSight(const f32*);                                       // fn_800B8370
    bool isPlayerInFront();                                           // fn_800B8390
    void requestMovementOnParts();                                    // fn_800B8420
    void startActionShells(const char*);                              // fn_800B84C0
    bool breakShellLeft(HitSensor*);                                  // fn_800B8520
    bool breakShellRight(HitSensor*);                                 // fn_800B85C0
    bool breakShellHip(HitSensor*);                                   // fn_800B8660
    bool breakAllShells();                                            // fn_800B8720
    void startBreakShells(const char*);                               // fn_800B87E0
    void updateShellsMtx();                                           // fn_800B8840
    void restoreShells();                                             // fn_800B8890
    bool isShellsAllAlive();                                          // fn_800B8950
    bool isShellLeftDead();                                           // fn_800B89C0
    bool isShellRightDead();                                          // fn_800B89D0
    bool isShellHipDead();                                            // fn_800B89E0
    bool isShellsAllDead();                                           // fn_800B89F0
    bool isShellSensor(HitSensor*);                                   // fn_800B8A60
    void appearPetalDownLand();                                       // fn_800B8B10
    void appearPetalDeath();                                          // fn_800B8B70
    void killPetal();                                                 // fn_800B8BD0
    void hitReaction();                                               // fn_800B8BF0
    void initDemoCamera(const char*);                                 // fn_800B8C30
    bool tryStartDemoWithCamera(const char*, const char*, s32);       // fn_800B8C40
    bool tryEndDemoWithCamera(const char*, bool);                     // fn_800B8D20
    void initMultiCamera(const char*);                                // fn_800B8DE0
    void startMultiCamera(const char*);                               // fn_800B8DF0
    void endMultiCamera(const char*);                                 // fn_800B8E00

    /* 0x90 */ TVec3f mFront;
    /* 0x9C */ ActorCameraInfo* mCameraInfo;
    /* 0xA0 */ DinoPackunBabyDemo* mDemoStart;
    /* 0xA4 */ DinoPackunBabyBattle* mBattle1st;
    /* 0xA8 */ DinoPackunBabyDemo* mDemoPowerUp;
    /* 0xAC */ DinoPackunBabyBattle* mBattle2nd;
    /* 0xB0 */ DinoPackunBabyDemo* mDemoDeath;
    /* 0xB4 */ DinoPackunBabyShell* mShellLeft;
    /* 0xB8 */ DinoPackunBabyShell* mShellRight;
    /* 0xBC */ DinoPackunBabyShell* mShellHip;
    /* 0xC0 */ DinoPackunBabyShell* mShellCenter;
    /* 0xC4 */ LiveActor* mPetal;
    /* 0xC8 */ LiveActor* mTail;
    /* 0xCC */ LiveActor* mEgg;
    /* 0xD0 */ AnimScaleController* mScaleController;
};
