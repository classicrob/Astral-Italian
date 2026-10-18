#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util/FixedPosition.hpp"

class BossBussun;
class BossBussunShot;
class BossBussunSequencer;
class ProjmapEffectMtxSetter;
class AnimScaleController;
class ActorCameraInfo;
class HitSensor;
class Nerve;

// SMG2-only boss "Bussun" family (no Petari counterpart). Layouts from the target asm.

class BossBussunStateSleep : public ActorStateBase< BossBussun > {
public:
    BossBussunStateSleep(BossBussun*);

    virtual ~BossBussunStateSleep();
    virtual void appear();

    bool tryWake();
    bool receiveOtherMsg(u32);
    void exeSleep();
    void exeSleepReaction();
};

class BossBussunStateRecover : public ActorStateBase< BossBussun > {
public:
    BossBussunStateRecover(BossBussun*);

    virtual ~BossBussunStateRecover();
    virtual void appear();

    void exeRecover();
};

class BossBussunStateDash : public ActorStateBase< BossBussun > {
public:
    BossBussunStateDash(BossBussun*);

    virtual ~BossBussunStateDash();
    virtual void appear();
    virtual void control();

    bool tryStartDash(LiveActor*);
    void exeStart();
    void exeDash();
    void exeDashEnd();

    /* 0x10 */ s32 mCoolTime;
};

class BossBussunStateDemo : public ActorStateBase< BossBussun > {
public:
    BossBussunStateDemo(BossBussun*, const char*, bool);

    virtual ~BossBussunStateDemo();
    virtual void appear();

    void exeDemo();
    void exeWaitEnd();

    /* 0x10 */ const char* mDemoName;
    /* 0x14 */ bool mIsPowerStarDemo;
};

class BossBussunStateDamage : public ActorStateBase< BossBussun > {
public:
    BossBussunStateDamage(BossBussun*);

    virtual ~BossBussunStateDamage();
    virtual void appear();

    void exeDamage();
    void updateDamagePos();

    /* 0x10 */ TVec3f mPosA;
    /* 0x1C */ TVec3f mPosB;
    /* 0x28 */ TVec3f mPosC;
};

class BossBussunStateShot : public ActorStateBase< BossBussun > {
public:
    BossBussunStateShot(BossBussun*);

    virtual ~BossBussunStateShot();
    virtual void appear();
    virtual void kill();

    void exeShotSign();
    void exeShot();
    void setShotCountMax(s32);
    void killAll();

    /* 0x10 */ LiveActorGroup* mShotGroup;
    /* 0x14 */ FixedPosition* mFixedPos;
    /* 0x18 */ s32 mShotCount;
    /* 0x1C */ s32 mShotCountMax;
};

class BossBussunStateChangeSide : public ActorStateBase< BossBussun > {
public:
    BossBussunStateChangeSide(BossBussun*);

    virtual ~BossBussunStateChangeSide();
    virtual void appear();

    /* 0x10 .. 0x47 : see BossBussunStateChangeSide.cpp */
    u8 _10[0x48 - 0x10];
};

class BossBussunShot : public LiveActor {
public:
    BossBussunShot(const char*);

    virtual ~BossBussunShot();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual void makeActorDead();

    void exeBound();
    void exeFly();
    void exeCrash();
    void exeHold();
    void exeKill();
    void shoot(LiveActor*, const TVec3f*);
    bool calcJointMtx(LiveActor*, MtxPtr);

    /* 0x90 */ AnimScaleController* mAnimScale;
    /* 0x94 */ LiveActor* mParent;
    /* 0x98 */ HitSensor* mSensor;
    /* 0x9C */ BossBussunStateSleep* mSleepState;
    /* 0xA0 */ TQuat4f mQuatA;
    /* 0xB0 */ TQuat4f mQuatB;
    /* 0xC0 */ f32 _C0;
    /* 0xC4 */ f32 _C4;
    /* 0xC8 */ f32 _C8;
    /* 0xD8 */ f32 _D8;
    /* 0xDC */ f32 _DC;
    /* 0xE0 */ f32 _E0;
    /* 0xE4 */ f32 _E4;
    /* 0xE8 */ f32 _E8;
    /* 0xEC */ f32 _EC;
    /* 0xF0 */ s32 _F0;
    /* 0xF4 */ s32 _F4;
    /* 0xF8 */ s32 _F8;
};

class BossBussunSequencer {
public:
    BossBussunSequencer(BossBussun*, const JMapInfoIter*);

    void startCamera();
    void endCamera(s32);
    void updateCamera();
    void initCameraState();
    void calcFrameAxes(const TVec3f&, const TVec3f&);
    f32 calcPlayerDot() const;
    bool isNearHeight(f32) const;

    /* 0x00 */ BossBussun* mHost;
    /* 0x04 */ u8 _04[0x0C];
    /* 0x10 */ TVec3f mVec10;
    /* 0x1C */ TVec3f mVec1C;
    /* 0x28 */ TVec3f mVec28;
    /* 0x34 */ TVec3f mVec34;
    /* 0x40 */ TVec3f mVec40;
    /* 0x4C */ u8 mIsNear;
};

class BossBussun : public LiveActor {
public:
    BossBussun(const char*);

    virtual ~BossBussun();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    // nerve executors (exe names only; nerves are NEW_NERVE in BossBussun.cpp)
    void exeStart();
    void exeWait();
    void exeTurn();
    void exeMove();
    void exeAttack();
    void exeStateRun();
    void exeAttackStart();
    void exeStateNext();
    void exeFinish();

    void startDefault();
    void calcNearestHomePos(TVec3f*);
    bool tryStartAttack();
    void storePlayerPosY();
    void resetBodyOffset();
    void resetAnimScale();
    void updateVelocityOnGround();
    bool isPlayerAttackable();
    bool isNotDamaged() const;
    bool isStateRunning() const;
    bool isNotDamagedSimple() const;
    bool isAttackingNerve() const;
    bool isAttackingNerveAny() const;
    f32 getDashDistanceLimit() const;
    f32 getStepParam() const;
    void resetHomePos();
    bool checkSensorPlayerInFront(HitSensor*);
    void setBinderExceptPlayer(u32);

    /* 0x090 */ ProjmapEffectMtxSetter* mProjmapEffect;
    /* 0x094 */ BossBussunSequencer* mSequencer;
    /* 0x098 */ ActorCameraInfo* mCameraInfo;
    /* 0x09C */ BossBussunStateSleep* mSleepState;
    /* 0x0A0 */ BossBussunStateShot* mShotState;
    /* 0x0A4 */ BossBussunStateDash* mDashState;
    /* 0x0A8 */ AnimScaleController* mAnimScale;
    /* 0x0AC */ TVec3f _AC;
    /* 0x0B8 */ TVec3f _B8;
    /* 0x0C4 */ TVec3f _C4;
    /* 0x0D0 */ TVec3f _D0;
    /* 0x0DC */ TVec3f _DC;
    /* 0x0E8 */ TVec3f _E8;
    /* 0x0F4 */ TVec3f mScale;
    /* 0x100 */ s32 mStateTimer;
    /* 0x104 */ f32 _104;
    /* 0x108 */ s32 mHitCount;
    /* 0x10C */ u8 _10C;
    /* 0x10D */ u8 _10D;
};


// Unnamed SMG2 helpers shared by the BossBussun family (still called by the state units; BossBussun.cpp
// holds the member versions of these, to be switched over once that file compiles).
extern "C" {
void fn_80098B00(BossBussun*);
bool fn_80098B50(BossBussun*);
void fn_80098CD0(BossBussun*);
f32 fn_80098420(BossBussun*);
void fn_80098D10(BossBussun*);
void fn_80098D20(BossBussun*);
void fn_80099CD0(BossBussunSequencer*);
void fn_80098FB0(BossBussun*);
void fn_80099D10(BossBussunSequencer*);
}
