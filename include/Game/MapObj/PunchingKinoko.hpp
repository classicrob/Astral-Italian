#pragma once

#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/GroundChecker.hpp"
#include "Game/Util/JointController.hpp"

class WalkerStateBindStarPointer;

class PunchingKinoko : public LiveActor {
public:
    PunchingKinoko(const char*);

    virtual ~PunchingKinoko();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool requestPunch(HitSensor*, HitSensor*);
    bool requestEnemyBlow(HitSensor*, HitSensor*);
    bool requestTrample(HitSensor*, HitSensor*);
    bool requestCrush();
    bool requestCrushedShrink();
    bool requestCrushedRecover();

    void exeWait();
    void exeSwing();
    void exePointSnaped();
    void exePunched();
    void exePunchedBrake();
    void exeHitted();
    void exeCrushed();
    void exeCrushedEnd();
    void exeCrushedShrink();
    void exeCrushedFlat();
    void exeCrushedRecover();

    bool ballMtxCallBack(TPos3f*, const JointControllerInfo&);

    // some callers will only match if these are marked NO_INLINE, even though they aren't being inlined (???)
    bool isEnablePunched() const;
    bool isEnableHitPlayer() const NO_INLINE;
    bool isEnableEnemyAttack() const;
    bool isEnableCrushed() const;
    bool isEnableTrample() const;
    bool isEnableBlowed() const NO_INLINE;
    bool isEnablePush() const;
    bool isCrushed() const;

    void addVelocityKeepHeight();
    void addVelocityToUpPos(f32, f32, f32);

private:
    /* 0x90 */ GroundChecker* mGroundChecker;
    /* 0x94 */ AnimScaleController* mScaleController;
    /* 0x98 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x9C */ JointControlDelegator< PunchingKinoko >* mDelegator;
    /* 0xA0 */ s32 mCameraNo;
    /* 0xA4 */ TVec3f mCameraPos;
    /* 0xB0 */ TVec3f mUpDir;
    /* 0xBC */ s32 mStarPointerHitCoolDown;
    /* 0xC0 */ bool _C0;
    /* 0xC4 */ s32 mInvincibleHitCoolDown;
};
