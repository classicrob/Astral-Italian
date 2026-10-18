#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/Util/JointController.hpp"
#include <JSystem/JGeometry.hpp>

// SMG2-only boss "TwoLegs" (hub = TwoLegs.cpp, TU00; the state objects live in TwoLegsBulletTU04..TU12).
// Class names are provisional; see the unit report.
class ActorCameraInfo;
class ActorJointCtrl;
class AnimScaleController;
class JointController;
class ModelObj;
class BreakModel;
class WalkerStateBindStarPointer;
class TwoLegs;
class TwoLegsBullet;
class TwoLegsBodyDir;
class TwoLegsCamera;
class TwoLegsJointCtrl;
class TwoLegsReflectCtrl;
class TwoLegsCoin;
class TwoLegsStateShot;
class TwoLegsStateChangePos;
class TwoLegsStateDamage;
class TwoLegsStateJump;
class TwoLegsStateWalkNoDamage;
class TwoLegsStateWalkDamage;

namespace MR {
    // SMG2 helpers used by the TwoLegs states (additive declarations, mangled names taken from the target asm)
    LiveActor* getPlayerCarrySensorActor();
    void hideModelIfShown(LiveActor*);
    void showModelIfHidden(LiveActor*);
    void calcDirectionVec(TVec3f*, const TVec3f&, const TVec3f&);
    // SMG2 overload: (host, item type, joint name, offset, rotate); target symbol is createDummyDisplayModel(LiveActor*, JMapInfoIter&, MtxPtr, long, ...) in symbols.txt
    LiveActor* createDummyDisplayModel(LiveActor*, s32, const char*, const TVec3f&, const TVec3f&);
    void createCenterScreenBlur();
    void startCenterScreenBlur(s32, f32, u8, s32, s32);
};  // namespace MR

// TU03: 0x18 byte position/front holder (fn_80106880)
class TwoLegsBodyDir {
public:
    TwoLegsBodyDir();                // fn_80106880
    f32 getRadius() const;           // fn_801068B0

    /* 0x00 */ TVec3f mPos;
    /* 0x0C */ TVec3f mFront;
};

// TU03: programmable camera controller (0x24 bytes)
class TwoLegsCamera {
public:
    TwoLegsCamera(LiveActor*);       // fn_80105C70
    void start();                    // fn_80105CD0
    void keepAlive();                // fn_80105D20
    void end(bool);                  // fn_80105D70
    void update();                   // fn_80105D90
    void addTarget(LiveActor*);      // fn_80105F30
    void reset();                    // fn_80105F60
    void calcBox(TVec3f*);           // fn_80105F80
    void calcFov(f32*, TVec3f*);     // fn_80106180

    /* 0x00 */ LiveActor* mHost;
    /* 0x04 */ bool mActive;
    /* 0x08 */ f32 mFovScale;
    /* 0x0C */ TVec3f mTarget;
    /* 0x18 */ u32 mNumTargets;
    /* 0x1C */ LiveActor* mTargets[2];
};

// TU03: coin popper (4 bytes)
class TwoLegsCoin {
public:
    TwoLegsCoin(LiveActor*);               // fn_80106340
    void appear(const TVec3f&);            // fn_80106380
    /* 0x0 */ LiveActor* mHost;
};

// TU03: joint controller wrapper (0x80 bytes)
class TwoLegsJointCtrl {
public:
    TwoLegsJointCtrl(LiveActor*);          // fn_80106390
    void update();                         // fn_80106500
    void registerCallBack();               // fn_80106550
    bool calcJoint(TPos3f*, const JointControllerInfo&);  // fn_80106560

    /* 0x00 */ bool mSpin;
    /* 0x04 */ void* mJointR;
    /* 0x08 */ void* mJointL;
    /* 0x0C */ TPos3f mMtxR;
    /* 0x3C */ TPos3f mMtxL;
    /* 0x6C */ bool mTrack;
    /* 0x6D */ bool mReturn;
    /* 0x70 */ JointController* mCtrl;
    /* 0x74 */ TVec3f mDir;
};

// TU03: "reflect" joint IK controller
class TwoLegsReflectCtrl : public NerveExecutor {
public:
    TwoLegsReflectCtrl(LiveActor*);        // fn_801068C0
    virtual ~TwoLegsReflectCtrl();         // fn_80106B40
    void update();                         // fn_80106960
    void setCallBack();                    // fn_801069A0
    void start(bool);                      // fn_801069B0
    void setIK(f32);                       // fn_80106A70

    /* 0x08 */ LiveActor* mHost;
    /* 0x0C */ ActorJointCtrl* mJointCtrl;
    /* 0x10 */ const char* mJointName;
    /* 0x14 */ TVec3f mDir;
};

// ---------------------------------------------------------------------------------------------
// states (all derive ActorStateBase<TwoLegs>, vtable: dtor, init, appear)
// ---------------------------------------------------------------------------------------------
class TwoLegsStateShot : public ActorStateBase< TwoLegs > {  // TU04 "State[弾を発射]"
public:
    TwoLegsStateShot(TwoLegs*, const JMapInfoIter&);  // fn_80106D00
    virtual ~TwoLegsStateShot();                       // fn_80107480
    virtual void init();                               // fn_80106E40
    virtual void appear();                             // fn_80106E50

    bool isAllBulletDead();            // fn_80106EC0
    void killBullets();                // fn_80106F30
    s32 countAliveBullets();           // fn_80106FB0
    void shootBullet();                // fn_80107020
    bool tryAttackStrong(HitSensor*, HitSensor*);  // fn_80107120
    bool tryAttack(HitSensor*, HitSensor*);        // fn_80107190
    void exeStandUp();                 // fn_80107200
    void exeAttack();                  // fn_80107270
    void exeReverseSign();             // fn_801073A0
    void exeSitDown();                 // fn_80107420

    /* 0x10 */ TwoLegsBullet** mBullets;
    /* 0x14 */ s32 mBulletNum;
    /* 0x18 */ s32 mShotCount;
};

class TwoLegsStateChangePos : public ActorStateBase< TwoLegs > {  // TU05 "State[立ち位置変更]"
public:
    TwoLegsStateChangePos(TwoLegs*);   // fn_801075A0
    virtual ~TwoLegsStateChangePos();  // fn_80107990
    virtual void init();               // fn_80107600
    virtual void appear();             // fn_80107610

    bool isRunningAway();              // fn_80107680
    void exeStandUp();                 // fn_80107700
    void exeJump();                    // fn_80107760
    void exeHide();                    // fn_801077E0
    void exeLand();                    // fn_80107850
    void exeSitDown();                 // fn_80107930
};

class TwoLegsStateDamage : public ActorStateBase< TwoLegs > {  // TU06 "State[ダメージ]"
public:
    TwoLegsStateDamage(TwoLegs*);      // fn_80107AE0
    virtual ~TwoLegsStateDamage();     // fn_80107EB0
    virtual void init();               // fn_80107B40
    virtual void appear();             // fn_80107BA0

    bool isHide();                     // fn_80107BF0
    void appearStarPiece();            // fn_80107C00
    void exeDamage();                  // fn_80107C70
    void exeRunAwayJump();             // fn_80107D00
    void exeHide();                    // fn_80107D80
    void exeRunAwayLand();             // fn_80107DF0
};

class TwoLegsStateExplosion : public ActorStateBase< TwoLegs > {  // TU07 "State[爆発デモ]"
public:
    TwoLegsStateExplosion(TwoLegs*);   // fn_80107FD0
    virtual ~TwoLegsStateExplosion();  // fn_80108400
    virtual void init();               // fn_80108030
    virtual void appear();             // fn_80108080

    void explode();                    // fn_801080D0
    void exeBlurWait();                // fn_80108160
    void exeDemo();                    // fn_80108210
    void exeExplosion();               // fn_80108300
    void exeEnd();                     // fn_80108370

    /* 0x10 */ BreakModel* mBreakModel;
};

class TwoLegsStateOpening : public ActorStateBase< TwoLegs > {  // TU08 "State[オープニングデモ]"
public:
    TwoLegsStateOpening(TwoLegs*);     // fn_80108520
    virtual ~TwoLegsStateOpening();    // fn_80108780
    virtual void init();               // fn_80108580
    virtual void appear();             // fn_801085C0
    virtual void kill();               // fn_801085D0

    void exeWaitDemo();                // fn_80108610
    void exeDemo();                    // fn_80108690
};

class TwoLegsStateJump : public ActorStateBase< TwoLegs > {  // TU09 "State[ジャンプ]"
public:
    TwoLegsStateJump(TwoLegs*);        // fn_80108850
    virtual ~TwoLegsStateJump();       // fn_80108CC0
    virtual void init();               // fn_801088C0
    virtual void appear();             // fn_801088D0

    bool tryAttackStrong(HitSensor*, HitSensor*);  // fn_80108920
    bool tryAttack(HitSensor*, HitSensor*);        // fn_80108990
    bool isStamped(HitSensor*);        // fn_80108A00
    u8 getAttackFlag() const;          // fn_801089F0
    bool isJumpEnd();                  // fn_80108AB0
    bool tryNextJump();                // fn_80108B50
    void exeJump();                    // fn_80108C20

    /* 0x10 */ s32 mJumpCount;
    /* 0x14 */ bool mAttacked;
    /* 0x15 */ bool mFlag;
};

class TwoLegsStateWait : public ActorStateBase< TwoLegs > {  // TU10 "State[ウェイト]"
public:
    TwoLegsStateWait(TwoLegs*);        // fn_80108D50
    virtual ~TwoLegsStateWait();       // fn_80108EE0
    virtual void init();               // fn_80108DB0
    virtual void appear();             // fn_80108DC0

    void exeWait();                    // fn_80108E20
};

class TwoLegsStateWalkNoDamage : public ActorStateBase< TwoLegs > {  // TU11 "State[歩き・ダメージ無し]"
public:
    TwoLegsStateWalkNoDamage(TwoLegs*);   // fn_80108F70
    virtual ~TwoLegsStateWalkNoDamage();  // fn_80109440
    virtual void init();                  // fn_80108FE0
    virtual void appear();                // fn_80108FF0

    bool tryAttackStrong(HitSensor*, HitSensor*);  // fn_80109040
    static bool tryStrongStamp(NerveExecutor*, TwoLegs*, HitSensor*, HitSensor*);  // fn_801090C0
    static bool tryStamp(NerveExecutor*, TwoLegs*, HitSensor*, HitSensor*);        // fn_80109120
    static bool isStampTiming(NerveExecutor*, TwoLegs*, HitSensor*);               // fn_80109180
    bool isWalkEnd();                  // fn_801092D0
    void exeWalk();                    // fn_80109390

    /* 0x10 */ s32 mStepCount;
    /* 0x14 */ bool mAttacked;
};

class TwoLegsStateWalkDamage : public ActorStateBase< TwoLegs > {  // TU12 "State[歩き・ダメージ有り]"
public:
    TwoLegsStateWalkDamage(TwoLegs*);   // fn_801094D0
    virtual ~TwoLegsStateWalkDamage();  // fn_80109CB0
    virtual void init();                // fn_80109540
    virtual void appear();              // fn_80109550

    bool isDamageable();                // fn_80109570
    s32 getDamageSide();                // fn_801095D0
    bool tryAttackStrong(HitSensor*, HitSensor*);  // fn_801095F0
    bool tryAttack(HitSensor*, HitSensor*);        // fn_801096C0
    static bool tryStrongStamp(NerveExecutor*, TwoLegs*, HitSensor*, HitSensor*);  // fn_80109770
    static bool tryStamp(NerveExecutor*, TwoLegs*, HitSensor*, HitSensor*);        // fn_801097D0
    static bool isStampTiming(NerveExecutor*, TwoLegs*, HitSensor*);               // fn_80109830
    bool isWalkEnd();                   // fn_80109920
    void exeReverseSign();              // fn_80109990
    void exeDown();                     // fn_80109A10
    void exeWalk();                     // fn_80109A90
    void exeWaitDown();                 // fn_80109B70
    void exeUp();                       // fn_80109C40

    /* 0x10 */ s32 mStepCount;
    /* 0x14 */ s32 mWalkCount;
    /* 0x18 */ bool mAttacked;
};

// TU01/TU02: boss bullet ("二脚ボスの弾")
class TwoLegsBullet : public LiveActor {
public:
    TwoLegsBullet(TwoLegs*, const char*);                              // __ct__13TwoLegsBulletFP7TwoLegsPCc
    virtual ~TwoLegsBullet();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void reverseMove();                                // fn_801045C0
    void calcSideVec();                                // fn_80104AC0
    void calcPosAhead(TVec3f*, f32);                   // fn_80104BA0
    void turnToward(const TVec3f&, const TVec3f&, f32);  // fn_80104C00
    bool isTramplable();                               // fn_80104C90
    void killHill();                                   // fn_80104D20
    bool isReflectSource(HitSensor*, HitSensor*);      // fn_80104D90

    /* 0x90 */ TwoLegs* mHost;
    /* 0x94 */ TVec3f mFront;
    /* 0xA0 */ TVec3f mSide;
    /* 0xAC */ TVec3f mShadowDir;
    /* 0xB8 */ TPos3f mMtx;
    /* 0xE8 */ LiveActor* mHill;
    /* 0xEC */ TVec3f mHillDir;
    /* 0xF8 */ AnimScaleController* mScaleCtrl;
    /* 0xFC */ WalkerStateBindStarPointer* mStateBind;
};

// ---------------------------------------------------------------------------------------------
class TwoLegs : public LiveActor {
public:
    TwoLegs(const char*);                                               // __ct__7TwoLegsFPCc
    virtual ~TwoLegs();                                                  // __dt__7TwoLegsFv
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void startFirstNerve();                          // fn_80103150
    void placeAtAllRoot();                           // fn_801031D0
    void placeOnBodyPlane(const TVec3f&);            // fn_80103210
    void resetToInitPos();                           // fn_80103250
    void hideAndKillDummy();                         // fn_80103290
    bool calcJointCallback(TPos3f*, const JointControllerInfo&);  // fn_801032E0
    static s32 getPhaseNumInner();                   // fn_801032F0
    bool tryAttackSensorState(HitSensor*, HitSensor*);   // fn_801035C0
    bool tryAttackSensorStateEnemy(HitSensor*, HitSensor*);  // fn_80103710
    bool tryPush(HitSensor*, HitSensor*);            // fn_80103840
    bool tryReflectEnemy(HitSensor*, HitSensor*);    // fn_801038A0
    bool tryDrillDamage(HitSensor*, HitSensor*);     // fn_80103960
    bool tryReflectDrill(HitSensor*, HitSensor*, HitSensor*);  // fn_80103A30
    void startBva();                                 // fn_80103B40
    void damage();                                   // fn_80103BA0
    bool isInvincible();                             // fn_80103C60
    void selectNextState();                          // fn_80103D20
    bool updateStateAndNext();                       // fn_80103E40
    void updateCamera();                             // fn_80103EB0

    // TU12 helpers
    void startActionLR(const char*);                 // fn_80109E00
    void startActionUp();                            // fn_80109E70
    void startActionDown();                          // fn_80109EB0
    bool isPhase0() const;                           // fn_80109EF0
    bool isLastPhase() const;                        // fn_80109F00
    static s32 getPhaseNum();                        // fn_80109F20
    void getAllRootPos(TVec3f*);                     // fn_80109F30
    void calcAllRootPosOnPlane(TVec3f*);             // fn_80109F40
    void calcPosOnPlane(TVec3f*, const TVec3f*);     // fn_80109FA0
    void calcPosOnPlaneImpl(TVec3f*, const TVec3f&); // fn_80109FF0
    TwoLegsBodyDir* getBodyDir() const;              // fn_8010A0D0
    const TVec3f* getBodyFront() const;              // fn_8010A0E0
    void addCameraTarget(LiveActor*);                // fn_8010A0F0
    void resetCamera();                              // fn_8010A100
    void disableCamera();                            // fn_8010A110
    void toggleSide();                               // fn_8010A120
    void jointTrackOn();                             // fn_8010A150
    void jointTrackOnReturn();                       // fn_8010A170
    void jointTrackOff();                            // fn_8010A190
    void jointSpinOn();                              // fn_8010A1B0
    void jointReset();                               // fn_8010A1C0
    void popCoins(const TVec3f&);                    // fn_8010A200
    bool isPlayerBetweenFeet();                      // fn_8010A210
    bool tryKillStateIfPlayerBetweenFeet(ActorStateBaseInterface*);  // fn_8010A310
    bool trySetNerveIfPlayerBetweenFeet(NerveExecutor*, const Nerve*);  // fn_8010A370

    /* 0x90 */ s32 mPhase;
    /* 0x94 */ TVec3f mInitPos;
    /* 0xA0 */ LiveActor* mDummyModel;
    /* 0xA4 */ TwoLegsBodyDir* mBodyDir;
    /* 0xA8 */ ActorCameraInfo* mCameraInfo;
    /* 0xAC */ TwoLegsCamera* mCamera;
    /* 0xB0 */ TwoLegsJointCtrl* mJointCtrl;
    /* 0xB4 */ TwoLegsReflectCtrl* mReflectCtrl;
    /* 0xB8 */ TwoLegsCoin* mCoin;
    /* 0xBC */ bool mIsLeft;
    /* 0xBD */ bool mIsUp;
    /* 0xBE */ u8 mHitFlag;
    /* 0xBF */ bool mAlternate;
    /* 0xC0 */ u8 mFlagC0;
    /* 0xC4 */ TwoLegsStateShot* mStateShot;
    /* 0xC8 */ TwoLegsStateChangePos* mStateChangePos;
    /* 0xCC */ TwoLegsStateDamage* mStateDamage;
    /* 0xD0 */ TwoLegsStateJump* mStateJump;
    /* 0xD4 */ TwoLegsStateWalkNoDamage* mStateWalkNoDamage;
    /* 0xD8 */ TwoLegsStateWalkDamage* mStateWalkDamage;
};
