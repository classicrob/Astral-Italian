#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class Triangle;
class AnimScaleController;
class WalkerStateBindStarPointer;
class ParabolicPath;

class Meramera : public LiveActor {
public:
    Meramera(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    static s32 initElementType(const JMapInfoIter&);
    static void initPlayerResource(const JMapInfoIter&);
    void appearAtPos(const TVec3f&, const TVec3f&);
    void initEffect();
    void initAppearState(const JMapInfoIter&);
    bool requestDamage(HitSensor*, HitSensor*);
    bool requestFire(HitSensor*, HitSensor*);
    bool requestForceRunaway();
    bool requestDownForce(HitSensor*, HitSensor*);
    bool requestDown(HitSensor*, HitSensor*);
    bool requestAttack(HitSensor*, HitSensor*);
    bool requestFlatDown(HitSensor*, HitSensor*);
    bool sendMsgElementAttack(HitSensor*, HitSensor*);
    bool tryWalk();
    bool tryWalkEnd();
    bool tryChase();
    bool tryEndChase();
    bool tryStartDiving();
    bool tryForceSink();
    bool tryRecovery();
    bool tryPointBind();

    void exeWait();
    void exeWalk();
    void exeChaseStart();
    void exeChaseDash();
    void exeChaseTurn();
    void exeDamage();
    void exeRunaway();
    void endRunaway();
    void exeStartDiving();
    void endStartDiving();
    void exeDiving();
    void exeSink();
    void exeFloat();
    void exeIgnition();
    void exeIgnitionForce();
    void exeAttackSuccess();
    void exeDown();
    void exeFlatDown();

    void exeBindStarPointer();
    void endBindStarPointer();
    void exeBindStarPointerNoFire();
    void endBindStarPointerNoFire();
    void exeReadyRestart();

    bool isAffectedSpin() const;
    bool isEnableStarPieceReflect() const;
    bool isEnableAttack() const;
    bool isPushMoved() const;
    bool isEnableFireball() const;
    bool isEnableChase() const;
    void addToTargetMovingAccel(const TVec3f&, f32, f32) NO_INLINE;
    void addMovingAccel(const TVec3f&, f32, f32);
    void addRunawayJumpPower();
    void addOverWallAccel(const TVec3f&);
    void updatePose();
    void updateExtinguishMtx();
    bool checkDirectDivingPoint();
    bool checkDivingPoint(f32, f32);
    bool checkDivingPointMore(f32);
    bool isHitRecover() const;
    bool findDivingPoint(TVec3f, const TVec3f&);
    void resetAppear();
    void turnFireDirectionToSpin(f32);
    void startWaitLevelSound();
    void startChaseLevelSound();

    inline bool isSensor(const HitSensor*, const char*) const;
    inline bool isActionDisabled() const;
    inline TVec3f getDistanceToPlayer() const;

    /* 0x08C */ AnimScaleController* mAnimScaleController;
    /* 0x090 */ WalkerStateBindStarPointer* mWalkerStateBindStarPointer;
    /* 0x094 */ ParabolicPath* mParabolicPath;
    /* 0x098 */ TPos3f mEffectMtx;
    /* 0x0C8 */ TPos3f mExtinguishMtx;
    /* 0x0F8 */ TPos3f mFallEffectMtx;
    /* 0x128 */ TQuat4f mPoseQuat;
    /* 0x138 */ TVec3f mFront;
    /* 0x144 */ TVec3f mUp;
    /* 0x150 */ TVec3f mHomePosition;
    /* 0x15C */ TVec3f mChaseAccelDir;
    /* 0x168 */ TVec3f mPlayerPosition;
    /* 0x174 */ TVec3f mWanderTarget;
    /* 0x180 */ TVec3f mJumpTarget;
    /* 0x18C */ TVec3f mJumpDir;
    /* 0x198 */ TVec3f mJumpUp;
    /* 0x1A4 */ f32 mTargetDistance;
    /* 0x1A8 */ f32 mChaseDistance;
    /* 0x1AC */ s32 mAppearStatus;  // Obj_arg0
    /* 0x1B0 */ s32 mElementType;
    /* 0x1B4 */ s32 mRunawayTimer;
    /* 0x1BC */ s32 mAppearTimer;
    /* 0x1C0 */ bool mCanDive;
    /* 0x1C1 */ bool mIsValidRestart;  // Obj_arg2
    /* 0x1C2 */ bool mIsBindFire;
};
