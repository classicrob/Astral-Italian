#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class HitSensor;
class JMapInfoIter;
class LiveActorGroup;
class SupportTicoExecutor;
class SupportTicoStateSpin;
class SupportTicoStateJump;
class SupportTicoStateDemoHide;
class SupportTicoStateFollowPlayer;
class SupportTicoStateChase;
class SupportTicoStateBack;

// SMG2-only unit (SupportTico.cpp, SupportTicoExecutor.cpp, SupportTicoState*.cpp and
// SupportTicoStateFunction.cpp are merged into one split in this decomp).

class SupportTico : public LiveActor {
public:
    SupportTico(const char*);

    virtual ~SupportTico();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeDisappear();
    bool tryAttachToTarget(const LiveActor*);
    bool calcTargetPos(TVec3f*);
    LiveActor* findNearestTarget();
    void clearTarget();
    void updateTarget();
    bool isTargeting() const;
    void offControl();
    void onControl();
    void setFollowPlayerYDir();
    void resetFollowPlayerYDir();
    void requestHide();
    void requestShow();
    void releaseHoldSensor();
    bool tryHold(HitSensor*, HitSensor*);
    void resetTarget();

    TVec3f* getFollowPos() const;
    void setAttachRange(f32);

    /* 0x90 */ void* mPose;  // 0x1C bytes: TQuat4f + TVec3f (position at +0x10)
    /* 0x94 */ LiveActorGroup* mAttachGroup;
    /* 0x98 */ const LiveActor* mTarget;
    /* 0x9C */ const LiveActor* mNearTarget;
    /* 0xA0 */ HitSensor* mHoldSensor;
    /* 0xA4 */ HitSensor* mReleasedSensor;
    /* 0xA8 */ SupportTicoExecutor* mExecutor;
    /* 0xAC */ s32 mTargetTimer;
    /* 0xB0 */ s32 _B0;
    /* 0xB4 */ f32 mAttachRange;
    /* 0xB8 */ bool mControlOn;
    /* 0xB9 */ bool _B9;
    /* 0xBA */ bool mInForbidArea;
    /* 0xBB */ bool mFollowPlayerYDir;
    /* 0xBC */ bool mShow;
};

// Every state is its own TU in the original build, so the base constructor has exactly one call site there and gets
// inlined. In this merged unit it would have 8 call sites (not inlined with -O4,s); a per-state tag makes each call
// site a separate instantiation.
template < int N >
struct SupportTicoStateTag {};

class SupportTicoStateBase : public ActorStateBaseInterface {
public:
    template < int N >
    SupportTicoStateBase(const char* pName, SupportTico* pHost, SupportTicoStateTag< N >) : ActorStateBaseInterface(pName), mHost(pHost) {
    }

    virtual ~SupportTicoStateBase() {
    }

    /* 0xC */ SupportTico* mHost;
};

class SupportTicoExecutor : public SupportTicoStateBase {
public:
    SupportTicoExecutor(SupportTico*);

    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMessage(u32, HitSensor*, HitSensor*);

    void exeAppear();
    void exeBack();
    bool isActive() const;

    /* 0x10 */ SupportTicoStateDemoHide* mStateDemoHide;
    /* 0x14 */ SupportTicoStateFollowPlayer* mStateFollowPlayer;
    /* 0x18 */ SupportTicoStateChase* mStateChase;
    /* 0x1C */ SupportTicoStateBack* mStateBack;
    /* 0x20 */ bool mIsFirstAppear;
};

class SupportTicoStateBack : public SupportTicoStateBase {
public:
    SupportTicoStateBack(SupportTico*);

    virtual void appear();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void exeBack();
    void exeSpin();

    /* 0x10 */ SupportTicoStateSpin* mStateSpin;
    /* 0x14 */ TVec3f mStartPos;
    /* 0x20 */ f32 mTimer;
};

class SupportTicoStateChase : public SupportTicoStateBase {
public:
    SupportTicoStateChase(SupportTico*);

    virtual void appear();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void exeChase();
    void exeSpin();
    void exeItemGet();

    /* 0x10 */ SupportTicoStateSpin* mStateSpin;
    /* 0x14 */ TVec3f mStartPos;
};

class SupportTicoStateDemoHide : public SupportTicoStateBase {
public:
    SupportTicoStateDemoHide(SupportTico*);

    virtual void appear();
    virtual void kill();

    void setSilentHide(bool);
    void exeHide();
    void exeStop();

    /* 0x10 */ s32 mCounter;
    /* 0x14 */ bool mSilentHide;
};

class SupportTicoStateFollowPlayer : public SupportTicoStateBase {
public:
    SupportTicoStateFollowPlayer(SupportTico*);

    virtual void appear();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void requestJump();
    void exeFollow();
    void exeWait();
    void exeBackEnd();
    void exeJump();
    void exeSpin();
    void exeItemDrop();
    bool tryJumpStart();
    bool tryItemDrop();
    bool trySpin();
    void updateFollow();

    /* 0x10 */ SupportTicoStateJump* mStateJump;
    /* 0x14 */ SupportTicoStateSpin* mStateSpin;
    /* 0x18 */ TVec3f mFollowDir;
    /* 0x24 */ TVec3f mJumpOffset;
    /* 0x30 */ bool mRequestJump;
};

class SupportTicoStateJump : public SupportTicoStateBase {
public:
    SupportTicoStateJump(SupportTico*, TVec3f*);

    virtual void appear();

    void exeJump();
    void exeReturn();

    /* 0x10 */ TVec3f* mOffset;
};

class SupportTicoStateSpin : public SupportTicoStateBase {
public:
    SupportTicoStateSpin(SupportTico*);

    virtual void appear();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void exeSpin();
    void exeStop();
};

namespace SupportTicoStateFunction {
    bool isEnable();
    f32 getFollowScale();
    bool isSwing();
    bool isReleaseA();
    bool isPressA();
    bool isTriggerA();
    void startSupportTicoCSSound();
    bool isHideDemo();
    bool isHolding(const SupportTico*);
    void hideHoldingObj(SupportTico*);
    void showHoldingObj(SupportTico*);
    void calcFollowOffset(TVec3f*);
    void calcFollowOffset(SupportTico*);
    bool trySpinAttack(SupportTico*);
};  // namespace SupportTicoStateFunction
