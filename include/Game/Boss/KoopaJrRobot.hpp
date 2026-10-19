#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ShadowVolumeQuakeWave.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry.hpp>

// SMG2-only unit (Bowser Jr. robot boss). Class names are provisional (see report).
class ActorCameraInfo;
class BreakModel;
class StarPieceGroup;
class KoopaJrRobot;
class KoopaJrRobotSeq;
class KoopaJrRobotPart;
class KoopaJrRobotBinderActor;
class KoopaJrRobotCollision;
class KoopaJrRobotPod;
class KoopaJrRobotStateA;
class KoopaJrRobotStateB;
class KoopaJrRobotStateC;
class KoopaJrRobotStateIntro;
class KoopaJrRobotStateTU07;
class KoopaJrRobotStateTU08;

// array that owns a heap array of child pointers (dtor = fn_800DB560)
class KoopaJrRobotChildren {
public:
    inline KoopaJrRobotChildren() : mArray(nullptr), mCapacity(0), mCount(0) {}
    ~KoopaJrRobotChildren();
    /* 0x0 */ NameObj** mArray;
    /* 0x4 */ s32 mCapacity;
    /* 0x8 */ s32 mCount;
};

// base of the three phase states (TU04 ctor fn_800DE1E0); vtable +0x20.. are SMG2 additions
class KoopaJrRobotStateBase : public ActorStateBase< KoopaJrRobot > {
public:
    KoopaJrRobotStateBase(const char*, KoopaJrRobot*);  // fn_800DE1E0
    virtual void attackSensor(HitSensor*, HitSensor*);   // fn_800DE2B0
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);  // fn_800DE350
    virtual const char* getWaitActionName();             // fn_800DE680

    NO_INLINE void turnToPlayerFast();    // fn_800DE3E0
    NO_INLINE void turnToPlayerSlow();    // fn_800DE3F0
    s32 updateSearch();         // fn_800DE400
    s32 calcTurnDir();          // fn_800DE4A0
    s32 updateTurn(s32);        // fn_800DE530
    bool updateTurnEnd(s32);    // fn_800DE5E0
    NO_INLINE bool isPlayerNearFront();   // fn_800DE670

    /* 0x10 */ f32 _10;
    /* 0x14 */ TVec3f _14;
};

// TU04 attack state
class KoopaJrRobotStateAttack : public KoopaJrRobotStateBase {
public:
    KoopaJrRobotStateAttack(KoopaJrRobot*, s32);  // fn_800DE6F0
    virtual ~KoopaJrRobotStateAttack();
    virtual void appear();                         // fn_800DE7C0
    virtual void kill();                           // fn_800DE840

    void updateMode();                             // fn_800DE770
    bool tryAppearFire();                          // fn_800DE8E0
    bool tryTurnToPlayer();
    void exeAttackB();

    void exeWait() {}
    void exeAttackSearch();                        // fn_800DEAB0
    void exeAttackA();                             // fn_800DED90 (shared by 3 nerves)

    /* 0x20 */ s32 mType;
    /* 0x24 */ s32 mMode;
};

class KoopaJrRobotStateA : public KoopaJrRobotStateBase {
public:
    KoopaJrRobotStateA(KoopaJrRobot*);  // TU09 fn_800E0220
    virtual ~KoopaJrRobotStateA();
    virtual void appear();                   // fn_800E0420
    bool isAnyPartBroken();                  // fn_800E0470
    bool receiveMsgEnemyAttackImpl(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    bool checkTimeout();
    bool tryTurnToSearch();
    bool tryTurnStart();
    void exeStart();
    void exeWait();
    void exeSearch();
    void exeTurn();
    void exeTurnEnd();
    void exeDamage();
    void exeAttack();
};

class KoopaJrRobotStateB : public KoopaJrRobotStateBase {
public:
    KoopaJrRobotStateB(KoopaJrRobot*);  // TU10 fn_800E0B50
    virtual ~KoopaJrRobotStateB();
    virtual const char* getWaitActionName();  // fn_800E0FC0
    bool receiveMsgEnemyAttackImpl(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    bool checkTimeout();
    bool tryTurnToSearch();
    bool tryTurnStart();
    void exeStart();
    void exeWait();
    void exeSearch();
    void exeTurn();
    void exeTurnEnd();
    void exeDamage();
    void exeAttack();
};

class KoopaJrRobotStateC : public KoopaJrRobotStateBase {
public:
    KoopaJrRobotStateC(KoopaJrRobot*);  // TU11 fn_800E1400
    virtual ~KoopaJrRobotStateC();
    virtual void appear();                    // fn_800E14D0
    virtual const char* getWaitActionName();  // fn_800E15B0
    bool receiveMsgEnemyAttackImpl(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    bool checkTimeout();
    bool tryTurnToSearch();
    bool tryTurnStart();
    void exeStart();
    void exeWait();
    void exeSearch();
    void exeTurn();
    void exeTurnEnd();
    void exeDamage();
    void exeAttack();

    /* 0x20 */ bool mFlag;
};

// TU05 damage state (mode 0: phase 1/2, mode 1: last phase)
class KoopaJrRobotStateDamage : public ActorStateBase< KoopaJrRobot > {
public:
    KoopaJrRobotStateDamage(KoopaJrRobot*, s32);  // fn_800DF090
    virtual ~KoopaJrRobotStateDamage();
    virtual void appear();                        // fn_800DF110
    void exeDamageLong();   // fn_800DF1B0
    void exeDamageShort();  // fn_800DF230
    void exeDamageHead();   // fn_800DF2A0
    void exeDamageLast();   // fn_800DF370

    /* 0x10 */ s32 mMode;
};

class KoopaJrRobotStateIntro : public KoopaJrRobotStateBase {
public:
    KoopaJrRobotStateIntro(KoopaJrRobot*, const JMapInfoIter&);  // TU06 fn_800DF5A0
    virtual ~KoopaJrRobotStateIntro();
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual void kill();
    static const char* getCameraName();  // fn_800DF590
    void exeWait();
    void exeDemo();
};

class KoopaJrRobotStateTU07 : public ActorStateBase< KoopaJrRobot > {
public:
    KoopaJrRobotStateTU07(KoopaJrRobot*);  // TU07 fn_800DF9F0
    virtual ~KoopaJrRobotStateTU07();
    virtual void kill();
    static const char* getCameraName();  // fn_800DF9E0
    void exeWait();
    void exeDown();
    void exeEnd();

    /* 0x10 */ BreakModel* mBreakModel;
};

class KoopaJrRobotStateTU08 : public ActorStateBase< KoopaJrRobot > {
public:
    KoopaJrRobotStateTU08(KoopaJrRobot*);  // TU08 fn_800DFE40
    virtual ~KoopaJrRobotStateTU08();
    virtual void kill();
    static const char* getCameraName();  // fn_800DFE30
    void exeWait();
    void exeDemo();
};

class HomingKiller;
class FixedPosition;

// SMG2 CollisionObj (0x94 bytes); only the ctor is needed here (defined in another unit)
class CollisionObj : public LiveActor {
public:
    CollisionObj(LiveActor*, const char*, const char*, HitSensor*, MtxPtr, s32);
    virtual ~CollisionObj();
    /* 0x90 */ u32 _90;
};

// TU03: pool of homing killers shared by the launchers (no vtable, 0x10 bytes)
class KoopaJrRobotKillerPool {
public:
    struct Entry {
        HomingKiller* mKiller;
        LiveActor* mOwner;
    };

    KoopaJrRobotKillerPool(LiveActor*, s32);              // fn_800DCDF0
    void movementAll();                                   // fn_800DCED0
    void killUnused();                                    // fn_800DCF40
    HomingKiller* allocate(LiveActor*);                   // fn_800DCFD0
    bool release(LiveActor*, HomingKiller*);              // fn_800DD090
    bool isShotEnd(HitSensor*);                           // fn_800DD140
    bool isReserved(HomingKiller*) const;                 // fn_800DD1D0
    LiveActor* getOwner(HomingKiller*) const;             // fn_800DD220
    void releaseFinished();                               // fn_800DD260

    /* 0x0 */ LiveActor* mHost;
    /* 0x4 */ Entry* mEntries;
    /* 0x8 */ s32 mNum;
    /* 0xC */ s32 mCursor;
};

// TU03: robot killer launcher
class KoopaJrRobotKillerLauncher : public LiveActor {
public:
    KoopaJrRobotKillerLauncher(KoopaJrRobot*, s32, FixedPosition*);  // fn_800DD2F0
    virtual ~KoopaJrRobotKillerLauncher();                           // fn_800DDA00
    virtual void init(const JMapInfoIter&);                          // fn_800DD360
    virtual void appear();                                           // fn_800DD410
    virtual void kill();                                             // fn_800DD450
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);       // fn_800DD840

    bool releaseKiller();                    // fn_800DD500
    void startShot();                        // fn_800DD520
    void updateTransform();                  // fn_800DD5E0
    static f32 normalizeAngle(f32);          // fn_800DD7A0
    bool isShootable();                      // fn_800DD7F0
    bool launch(const TVec3f&, const TVec3f&);  // fn_800DD850
    HomingKiller* getKiller() const;         // fn_800DD990
    void exeShot();                          // fn_800DD9B0

    /* 0x90 */ KoopaJrRobot* mHost;
    /* 0x94 */ s32 mType;
    /* 0x98 */ HomingKiller* mMagnumKiller;
    /* 0x9C */ HomingKiller* mKiller;
    /* 0xA0 */ FixedPosition* mFixedPosition;
};

class KoopaJrRobotSeq : public NerveExecutor {
public:
    KoopaJrRobotSeq(KoopaJrRobot*);  // TU02 fn_800DC310
    virtual ~KoopaJrRobotSeq();      // fn_800DCC10
    void initLaunchers();
    void killLaunchers();
    void updateLauncherTransforms();
    void movementLaunchers();
    void movementKillerPool();
    void appearLaunchers();
    void stopLauncherSequence();
    void startFastFrontLauncherShots();
    void startRearLauncherSequence();
    void startSlowFrontLauncherShots();
    void startMagnumShot();
    void startRearLauncherShots();
    void attackShootableKillers();
    void exeA();  // fn_800DCA00
    void exeB();  // fn_800DCAD0
    void exeC();  // fn_800DCB70

    /* 0x08 */ KoopaJrRobot* mHost;
    /* 0x0C */ KoopaJrRobotKillerPool* mPool;
    /* 0x10 */ KoopaJrRobotKillerLauncher* mMagnumLauncher;
    /* 0x14 */ KoopaJrRobotKillerLauncher* mLaunchers[8];
    /* 0x34 */ bool mLaunchersDisabled;
    /* 0x35 */ bool mLauncherSide;
};

class KoopaJrRobotPart : public NerveExecutor {
public:
    // entry of the weak-point table (TU13, 0x14 bytes each)
    struct PartInfo {
        s32 mId;
        const char* mName;
        const char* mJoint;
        const char* mSensor;
        const char* mAction;
    };

    KoopaJrRobotPart(KoopaJrRobot*, const char*, const char*, const char*, const char*);  // fn_800E21F0
    virtual ~KoopaJrRobotPart();                                                          // fn_800E27A0
    virtual void init();                                                                  // fn_800E22C0
    virtual void appear();                                                                // fn_800E22D0
    virtual void kill();                                                                  // fn_800E2340
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);                     // fn_800E23A0
    virtual bool receiveMsgExplosion(u32, HitSensor*, HitSensor*);                        // fn_800E2420

    static const PartInfo* getInfo(s32);                  // fn_800E20A0
    static KoopaJrRobotPart* create(KoopaJrRobot*, s32);  // fn_800E20E0
    static const char* getSensorName(s32);                // fn_800E21C0
    void appearStarPiece();                               // fn_800E2510
    bool isBroken();
    void exeNormal();   // fn_800E2650
    void exeDamage1();  // fn_800E26A0
    void exeDamage2();  // fn_800E2700

    /* 0x08 */ KoopaJrRobot* mHost;
    /* 0x0C */ PartsModel* mModel;
    /* 0x10 */ BreakModel* mBreakModel;
    /* 0x14 */ const char* mJoint;
    /* 0x18 */ const char* mSensor;
    /* 0x1C */ const char* mAction;
};

class KoopaJrRobotBinderActor : public LiveActor {
public:
    KoopaJrRobotBinderActor(HitSensor*);  // TU01 fn_800DB790
    virtual ~KoopaJrRobotBinderActor();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    void exeFollow();
    /* 0x90 */ HitSensor* mSensor;
};

class KoopaJrRobotPod : public LiveActor {
public:
    KoopaJrRobotPod(LiveActor*, MtxPtr);  // TU04 fn_800DDB50
    virtual ~KoopaJrRobotPod();
    virtual void init(const JMapInfoIter&);  // fn_800DDCE0
    virtual void appear();                   // fn_800DDD80
    virtual void calcAndSetBaseMtx();        // fn_800DDDD0
    void setMtx(MtxPtr);                     // fn_800DDDC0

    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ MtxPtr mMtx;
    /* 0x98 */ PartsModel* mCrack;
};

class RingBeam;

// TU04: beam shooter actor ("JrRobotBeam")
class KoopaJrRobotBeam : public LiveActor {
public:
    KoopaJrRobotBeam(const char*, LiveActor*, MtxPtr, const TVec3f&, s32, f32, f32, f32);  // fn_800DDE80
    virtual ~KoopaJrRobotBeam();
    virtual void init(const JMapInfoIter&);  // fn_800DDF50
    virtual void movement();                  // fn_800DE030
    virtual void appear();                    // fn_800DE0C0
    virtual void kill();                      // fn_800DE120
    virtual MtxPtr getBaseMtx() const;        // fn_800DE170
    void updateMtx();                         // fn_800DE070

    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ RingBeam* mRingBeam;
    /* 0x98 */ MtxPtr mMtx;
    /* 0x9C */ TPos3f mBaseMtx;
    /* 0xCC */ TVec3f mOffset;
    /* 0xD8 */ f32 mSpeed;
    /* 0xDC */ s32 mLife;
    /* 0xE0 */ f32 mStartRadius;
    /* 0xE4 */ f32 mMaxRadius;
};

namespace KoopaJrRobotFunction {
    s32 getStarPieceCountPerPart();
    const char* getDemoName();
    bool isPlayerInRushAir();                      // fn_800DBAE0
    void attachBodyToAllRoot(KoopaJrRobot*);       // fn_800DBCC0
    void attachBodyToHead(KoopaJrRobot*);          // fn_800DBD00
    void showStarPieces(KoopaJrRobot*);            // fn_800DBD40
    void hideStarPieces(KoopaJrRobot*);            // fn_800DBDB0
    void hideAll(KoopaJrRobot*);                   // fn_800DC130
    void showAll(KoopaJrRobot*);                   // fn_800DC200
    void offDrawEffect(KoopaJrRobot*);             // fn_800DC270
    void onDrawEffect(KoopaJrRobot*);              // fn_800DC2C0
    // TU02 helpers
    LiveActor* createHand(KoopaJrRobot*, const char*);               // fn_800DBE20
    KoopaJrRobotPart* createPart(KoopaJrRobot*, s32);                 // fn_800DBEC0
    LiveActor* createCollision(KoopaJrRobot*, const JMapInfoIter&, s32);  // fn_800DBED0
    KoopaJrRobotBinderActor* createFollower(LiveActor*, s32);      // fn_800DBF90
    KoopaJrRobotPod* createBody(KoopaJrRobot*, MtxPtr);              // fn_800DC040
    PartsModel* createHeadModel(KoopaJrRobotPod*);                      // fn_800DC0B0
};

class KoopaJrRobot : public LiveActor {
public:
    KoopaJrRobot(const char*);
    virtual ~KoopaJrRobot();

    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual void calcAnim();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    void killParts();
    void attackShootableKillers();
    HomingKiller* allocKiller(LiveActor*);              // fn_800DB990
    bool releaseKiller(LiveActor*, HomingKiller*);      // fn_800DB9A0
    LiveActor* getKillerOwner(HomingKiller*);           // fn_800DBCB0
    bool isKillerShotEnd(HitSensor*);
    bool tryAttackMapSensorsAroundJoint(const char*, const TVec3f&);
    bool isPlayerNearBack(f32);
    bool isWeakPointSensor(HitSensor*);
    bool isArmSensor(HitSensor*);
    bool isAllBodyPartsBroken();
    bool isStateNerve() const;
    void initChildren(const JMapInfoIter&);

    /* 0x90 */ KoopaJrRobotPart* mParts[4];
    /* 0xA0 */ KoopaJrRobotBinderActor* mFollowers[5];
    /* 0xB4 */ LiveActor* mCollisionA;
    /* 0xB8 */ LiveActor* mCollisionB;
    /* 0xBC */ LiveActor* mCollisionC;
    /* 0xC0 */ LiveActor* mHandL;
    /* 0xC4 */ LiveActor* mHandR;
    /* 0xC8 */ StarPieceGroup* mStarPieceGroup;
    /* 0xCC */ KoopaJrRobotChildren mChildren;
    /* 0xD8 */ KoopaJrRobotStateA* mStateA;
    /* 0xDC */ KoopaJrRobotStateB* mStateB;
    /* 0xE0 */ KoopaJrRobotStateC* mStateC;
    /* 0xE4 */ KoopaJrRobotSeq* mSeq;
    /* 0xE8 */ ActorCameraInfo* mCameraInfo;
    /* 0xEC */ PartsModel* mHeadModel;
    /* 0xF0 */ KoopaJrRobotPod* mBody;
    /* 0xF4 */ TVec3f mZAxis;
    /* 0x100 */ TVec3f mZAxis2;
    /* 0x10C */ f32 _10C;
};

// TU12
class KoopaJrRobotBreakableStep : public LiveActor {
public:
    KoopaJrRobotBreakableStep(const char*);
    virtual ~KoopaJrRobotBreakableStep();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    void exeBreak();
    void exeBlow();
    void exeFall();
};

// TU16: "KoopaPunchRock" actor
class KoopaJrRobotRock : public LiveActor {
public:
    KoopaJrRobotRock(const char*);  // fn_800E3F90
    virtual ~KoopaJrRobotRock();
    virtual void init(const JMapInfoIter&);        // fn_800E3FF0
    virtual void control();                         // fn_800E4050
    virtual void calcAndSetBaseMtx();               // fn_800E4060
    virtual void attackSensor(HitSensor*, HitSensor*);  // fn_800E4070

    void appearAtArenaEdge();
    void startDisappear();
    void exeAppear();     // fn_800E4170
    void exeDisappear();  // fn_800E41D0
    void exeWait();       // fn_800E4320

    /* 0x90 */ TQuat4f mQuat;
};

class FixedPosition;
class ProjmapEffectMtxSetter;

// TU20: "KoopaSealChain" actor
class KoopaJrRobotChain : public LiveActor {
public:
    KoopaJrRobotChain(const char*);  // fn_800E58D0
    virtual ~KoopaJrRobotChain();
    virtual void init(const JMapInfoIter&);        // fn_800E5920
    virtual void calcAndSetBaseMtx();               // fn_800E5990
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);  // fn_800E5A10

    void setFixedPosition(LiveActor*, const char*);  // fn_800E5A20
    void startDisappear();                           // fn_800E5A80
    void startDisappearEnd();                        // fn_800E5A90
    bool isDisappearEnd() const;                     // fn_800E5AA0
    void exeWait();          // fn_800E5C40
    void exeDisappear();     // fn_800E5AB0
    void exeDisappearEnd();  // fn_800E5BE0

    /* 0x90 */ FixedPosition* mFixedPosition;
    /* 0x94 */ ProjmapEffectMtxSetter* mMtxSetter;
};

// TU18: "KoopaQuakeWaveVertical" actor
class KoopaJrRobotQuakeWave : public LiveActor {
public:
    KoopaJrRobotQuakeWave(const char*);  // fn_800E4A00
    virtual ~KoopaJrRobotQuakeWave();
    virtual void init(const JMapInfoIter&);         // fn_800E4A90
    virtual void control();                          // fn_800E4B20
    virtual void calcAndSetBaseMtx();                // fn_800E4B30
    virtual void attackSensor(HitSensor*, HitSensor*);  // fn_800E4B40
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);  // fn_800E4C80

    void start(f32);        // fn_800E4D00
    void setParam(s32);     // fn_800E4DB0
    void exeHide();         // fn_800E4DC0
    void exeSign();         // fn_800E4E20
    void exeEnd();          // fn_800E4EE0

    /* 0x90 */ ShadowVolumeQuakeWave* mShadow;
    /* 0x94 */ TQuat4f mQuat;
    /* 0xA4 */ TQuat4f mTargetQuat;
    /* 0xB4 */ f32 _B4;
    /* 0xB8 */ f32 _B8;
    /* 0xBC */ f32 _BC;
    /* 0xC0 */ s32 _C0;
};

// TU17: "KoopaQuakeWave" actor (ground shockwave)
class KoopaJrRobotShockWave : public LiveActor {
public:
    KoopaJrRobotShockWave(const char*);  // fn_800E4420
    virtual ~KoopaJrRobotShockWave();
    virtual void init(const JMapInfoIter&);             // fn_800E4480
    virtual void control();                              // fn_800E4510
    virtual void attackSensor(HitSensor*, HitSensor*);   // fn_800E4520
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);  // fn_800E4680

    void start(const TVec3f&);  // fn_800E4700
    void setDuration(s32);      // fn_800E47E0
    void exeHide();             // fn_800E47F0
    void exeAttack();           // fn_800E4850

    /* 0x90 */ ShadowVolumeQuakeWave* mShadow;
    /* 0x94 */ f32 _94;
    /* 0x98 */ s32 _98;
};

// TU19: "KoopaSeal" actor (chained seal made of KoopaJrRobotChain pieces)
class KoopaJrRobotSeal : public LiveActor {
public:
    KoopaJrRobotSeal(const char*);  // fn_800E50A0
    virtual ~KoopaJrRobotSeal();
    virtual void init(const JMapInfoIter&);         // fn_800E50F0
    virtual void appear();                           // fn_800E5260
    virtual void makeActorAppeared();                // fn_800E5170
    virtual void makeActorDead();                    // fn_800E51F0
    virtual void calcAndSetBaseMtx();                // fn_800E5310
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);  // fn_800E54A0

    void makeChainsAppeared();                       // fn_800E52A0
    void setup(LiveActor*, const char*, s32);        // fn_800E5380
    void startDisappearAll();                        // fn_800E55C0
    void startDisappearEndAll();                     // fn_800E5620
    bool isDisappearEndAll() const;                  // fn_800E5680
    bool isBroken() const;                           // fn_800E56F0
    void breakImmediately();                         // fn_800E5700
    void setClipping(const TVec3f*, f32);            // fn_800E5750
    void exeNormal();
    void exeBreak();   // fn_800E5520
    void exeBroken();

    /* 0x90 */ FixedPosition* mFixedPosition;
    /* 0x94 */ BreakModel* mBreakModel;
    /* 0x98 */ KoopaJrRobotChain** mChains;
    /* 0x9C */ s32 mNumChains;
};
