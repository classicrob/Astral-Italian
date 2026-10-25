#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/ParabolicPath.hpp"
#include <JSystem/JGeometry.hpp>

class ActorCameraInfo;
class ActorJointCtrl;
class AnimScaleController;
class AnimScaleParam;
class FixedPosition;
class HitSensor;
class ModelObj;
class Sandy;
class SandyBall;
class SandyBallHolder;
class SandyBlur;
class SandyHand;
class SandyMini;
class SandyStateDamage;
class SandyStatePhase1;
class SandyStatePhase2;
class SandyStatePhase3;
class WalkerStateBindStarPointer;

// SMG2-only unit (written from the target asm). Offsets are real SMG2 offsets.

void turnToPlayer(TQuat4f*, LiveActor*, f32);
void attachBallToJoint(const LiveActor*, SandyBall*, const char*, const TVec3f&);
AnimScaleParam* createScaleParam();

// Holder of all the cactus balls the Sandy boss throws (5 ball kinds).
class SandyBallHolder : public NameObj {
public:
    SandyBallHolder(Sandy*);
    virtual ~SandyBallHolder();

    NO_INLINE SandyBall* getBall(LiveActor*, s32);
    NO_INLINE SandyBall* getFreeBall(LiveActor*);
    NO_INLINE SandyBall* getFreeBallType2(LiveActor*);
    NO_INLINE SandyBall* getFreeBallType1(LiveActor*);
    NO_INLINE SandyBall* getFreeBallType3(LiveActor*);
    NO_INLINE SandyBall* getFreeBallType4(LiveActor*);
    NO_INLINE void killAll();

    /* 0x14 */ Sandy* mSandy;
    /* 0x18 */ MR::AssignableArray< SandyBall* > mBalls0;
    /* 0x20 */ MR::AssignableArray< SandyBall* > mBalls1;
    /* 0x28 */ MR::AssignableArray< SandyBall* > mBalls2;
    /* 0x30 */ MR::AssignableArray< SandyBall* > mBalls3;
    /* 0x38 */ MR::AssignableArray< SandyBall* > mBalls4;
};

// Cactus ball / coin ball / bomb thrown by Sandy.
class SandyBall : public LiveActor {
public:
    SandyBall(s32, const char*);
    virtual ~SandyBall();

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    NO_INLINE void calcTargetPos(TVec3f*);
    NO_INLINE s32 getWaitSecond() const;
    NO_INLINE bool tryShoot();
    NO_INLINE void shoot(const TVec3f&, const TVec3f&);
    NO_INLINE void setOwner(LiveActor*);
    NO_INLINE bool isFree() const;
    NO_INLINE bool tryBreak();
    NO_INLINE bool tryKill();
    NO_INLINE bool tryStartBindStarPointer();
    NO_INLINE bool tryLand();
    NO_INLINE void setBtpFrame();
    NO_INLINE void initPath(const TVec3f&, const TVec3f&);

    void exeWait();
    void exeFly();
    void exeBreak();
    void exeLand();
    void exeRoll();
    void exeExplosion();
    void exeBindStarPointer();

    /* 0x90 */ LiveActor* mOwner;
    /* 0x94 */ ParabolicPath* mPath;
    /* 0x98 */ s32 mFlyFrame;
    /* 0x9C */ s32 mStep;
    /* 0xA0 */ TQuat4f mQuat;
    /* 0xB0 */ TVec3f mVel;
    /* 0xBC */ TVec3f mStartPos;
    /* 0xC8 */ f32 mScaleRate;
    /* 0xCC */ s32 mType;
    /* 0xD0 */ bool mCoinPopped;
    /* 0xD4 */ f32 mSensorRadius;
    /* 0xD8 */ AnimScaleController* mScaleController;
    /* 0xDC */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xE0 */ const Nerve* mPrevNerve;
};

// Sandy's hand (left/right).
class SandyHand : public LiveActor {
public:
    SandyHand(Sandy*, bool, SandyBallHolder*, FixedPosition*);
    virtual ~SandyHand();

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    NO_INLINE void startActionWithBrk(const char*);
    NO_INLINE void forceWait();
    NO_INLINE void startWaitAndSetNerve(bool);
    NO_INLINE void setDamageAction(const char*);
    NO_INLINE void setWait();
    NO_INLINE void setBallType0();
    NO_INLINE void setBallType3();
    NO_INLINE void setWaitStep(s32);
    NO_INLINE void setHandThrow(const Nerve*);
    NO_INLINE SandyBall* takeBall();
    NO_INLINE void calcBallPos();
    NO_INLINE void resetBall();
    NO_INLINE void setDamage();

    void exeWait();
    void exeThrow();
    void exeThrowBall();
    void exeThrowEnd();

    /* 0x90 */ Sandy* mSandy;
    /* 0x94 */ SandyBallHolder* mHolder;
    /* 0x98 */ FixedPosition* mFixedPos;
    /* 0x9C */ SandyBall* mBall;
    /* 0xA0 */ bool mIsRight;
    /* 0xA1 */ bool mIsThrower;
    /* 0xA2 */ char mCullMode;
    /* 0xA4 */ s32 mWaitStep;
    /* 0xA8 */ s32 mThrowCount;
    /* 0xAC */ s32 mBallType;
    /* 0xB0 */ const Nerve* mPrevNerve;
    /* 0xB4 */ s32 _B4;
    /* 0xB8 */ AnimScaleController* mScaleController;
    /* 0xBC */ AnimScaleParam* mScaleParam;
};

// Sandy's small sub actor ("SandyMini").
class SandyMini : public LiveActor {
public:
    SandyMini(Sandy*, const char*, FixedPosition*);
    virtual ~SandyMini();

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    NO_INLINE void startWait();
    NO_INLINE SandyBall* takeBall();
    NO_INLINE void calcPosFromFixedPos();
    NO_INLINE void setRecover();
    NO_INLINE void setNextNerve();
    NO_INLINE void resume();
    NO_INLINE bool tryStartNext();
    NO_INLINE bool isNerveStop() const;
    NO_INLINE bool isDownOrDead() const;
    NO_INLINE bool isShot() const;
    NO_INLINE bool canReceive() const;

    void exeStart();
    void exeWait();
    void exeDamage();
    void exeDown();
    void exeRecover();
    void exeShot();
    void exeShotEnd();

    /* 0x90 */ Sandy* mSandy;
    /* 0x94 */ FixedPosition* mFixedPos;
    /* 0x98 */ SandyBall* mBall;
    /* 0x9C */ TQuat4f mQuat;
    /* 0xAC */ s32 mBallCount;
    /* 0xB0 */ s32 mWaitStep;
    /* 0xB4 */ const Nerve* mNextNerve;
    /* 0xB8 */ AnimScaleController* mScaleController;
    /* 0xBC */ AnimScaleParam* mScaleParam;
};

// Screen blur controller used at the boss start.
class SandyBlur : public NerveExecutor {
public:
    SandyBlur(const char*);
    virtual ~SandyBlur();

    NO_INLINE bool tryStart();
    void exeBlur();
};

// Reconstructed descriptive name; the original name is unknown. The ctor/dtor and vtable 0x8067A760 prove this base layer.
class SandyStateBase : public ActorStateBase<Sandy> {
public:
    SandyStateBase(const char*, Sandy*);
    virtual ~SandyStateBase() {}
};

// Helpers shared by the three battle phases (taking the state as first argument).
bool sandyStateOpenStart(ActorStateBase< Sandy >*, s32);
bool sandyStateOpenWait(ActorStateBase< Sandy >*, s32);
bool sandyStateClose(ActorStateBase< Sandy >*, bool);
bool sandyStateThrow(ActorStateBase< Sandy >*, s32, s32, bool);

// Damage state shared by the three battle phases.
class SandyStateDamage : public ActorStateBase< Sandy > {
public:
    SandyStateDamage(Sandy*, s32, s32, s32);
    virtual ~SandyStateDamage();

    virtual void appear();
    virtual void kill();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    NO_INLINE void incDamage();

    void exeDamageFirst();
    void exeOpenWait();
    void exeOpenWaitTurn();
    void exeDamageMini();
    void exeDamage();

    /* 0x10 */ s32 mDamageCount;
    /* 0x14 */ TVec3f mFrontVec;
    /* 0x20 */ f32 mTurnRate;
    /* 0x24 */ bool mIsEnd;
    /* 0x28 */ s32 mDamageStep;
    /* 0x2C */ s32 mOpenStep;
    /* 0x30 */ TVec3f mPlayerPos;
    /* 0x3C */ s32 mDamageMax;
};

class SandyStateDemoStart : public ActorStateBase< Sandy > {
public:
    SandyStateDemoStart(Sandy*);
    virtual ~SandyStateDemoStart();

    virtual void appear();
    virtual void kill();

    void exeStart();
    void exeDemo();
};

class SandyStateDemoDown : public ActorStateBase< Sandy > {
public:
    SandyStateDemoDown(Sandy*);
    virtual ~SandyStateDemoDown();

    virtual void kill();

    void exeStart();
    void exeDemo();

    /* 0x10 */ ModelObj* mPowerStar;
};

class SandyStateDemoPowerUp : public ActorStateBase< Sandy > {
public:
    SandyStateDemoPowerUp(Sandy*);
    virtual ~SandyStateDemoPowerUp();

    virtual void kill();

    void exeStart();
    void exeDemo();
};

class SandyStatePhase1 : public SandyStateBase {
public:
    SandyStatePhase1(Sandy*);
    virtual ~SandyStatePhase1();

    virtual void appear();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeStart();
    void exeDamage();

    /* 0x10 */ SandyStateDamage* mDamage;
};

class SandyStatePhase2 : public SandyStateBase {
public:
    SandyStatePhase2(Sandy*);
    virtual ~SandyStatePhase2();

    virtual void appear();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeStart();
    void exeDamage();

    /* 0x10 */ SandyStateDamage* mDamage;
};

class SandyStatePhase3 : public SandyStateBase {
public:
    SandyStatePhase3(Sandy*);
    virtual ~SandyStatePhase3();

    virtual void appear();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    NO_INLINE void updateMinis();
    void exeStart();
    void exeOpen();
    void exeOpenWait();
    void exeDamage();
    void exeThrow();
    void exeThrowBomb();
    NO_INLINE bool isMinisReady() const;

    /* 0x10 */ SandyStateDamage* mDamage;
    /* 0x14 */ s32 mMiniWait0;
    /* 0x18 */ s32 mMiniWait1;
};

class Sandy : public LiveActor {
public:
    Sandy(const char*);
    virtual ~Sandy();

    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    NO_INLINE void killAll();
    NO_INLINE void appearAll();
    NO_INLINE SandyHand* getLeftHand() const;
    NO_INLINE SandyHand* getRightHand() const;
    NO_INLINE SandyMini* getLeftMini() const;
    NO_INLINE SandyMini* getRightMini() const;
    NO_INLINE void updateTarget();
    NO_INLINE void appearStarPieceOnDamage();
    NO_INLINE SandyHand* createHand(s32, const TVec3f&, const TVec3f&);
    NO_INLINE SandyMini* createMini(s32, const TVec3f&, const TVec3f&);
    NO_INLINE bool isTotalCount(s32) const;
    NO_INLINE void resetCount();
    NO_INLINE void validateHands();
    NO_INLINE void invalidateHands();
    NO_INLINE void forceWaitHands();
    NO_INLINE void setWaitFreeHands();
    NO_INLINE void setHandsStep(s32);
    NO_INLINE void setHandsBallType0();
    NO_INLINE void setWaitHands();
    NO_INLINE void startScaleHitReaction();
    NO_INLINE void setHandsBallType3();
    NO_INLINE void setMinisNext();
    NO_INLINE bool tryStartMiniNext(s32);
    NO_INLINE bool isMinisDownOrDead() const;
    NO_INLINE void showAll();
    NO_INLINE void startActionAll(const char*, const char*);
    NO_INLINE bool tryStartBlur();
    NO_INLINE void copyQuat();
    NO_INLINE bool isSensorBack(HitSensor*) const;

    /* 0x90 */ SandyStatePhase1* mPhase1;
    /* 0x94 */ SandyStatePhase2* mPhase2;
    /* 0x98 */ SandyStatePhase3* mPhase3;
    /* 0x9C */ SandyBallHolder* mHolder;
    /* 0xA0 */ SandyBlur* mBlur;
    /* 0xA4 */ ActorJointCtrl* mJointCtrl;
    /* 0xA8 */ AnimScaleController* mScaleController;
    /* 0xAC */ AnimScaleParam* mScaleParam;
    /* 0xB0 */ ActorCameraInfo* mCameraInfo;
    /* 0xB4 */ SandyHand* mHands[2];
    /* 0xBC */ SandyMini* mMinis[2];
    /* 0xC4 */ MR::AssignableArray< LiveActor* > mChildren;
    /* 0xCC */ TQuat4f mQuat;
    /* 0xDC */ TQuat4f mTargetQuat;
    /* 0xEC */ TPos3f mMtx;
    /* 0x11C */ TVec3f mFollowPos;
};
