#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"

class AnimScaleController;
class ClipAreaDrop;
class KeySwitch;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;
class PartsModel;
class Triangle;

class Teresa : public LiveActor {
public:
    Teresa(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initDummyModel(const JMapInfoIter&);
    void initFromJMapParam(const JMapInfoIter&);
    bool filterBind(const Triangle*);
    bool requestAttack(HitSensor*, HitSensor*);
    bool requestDrift();
    bool requestLoveHit(HitSensor*, HitSensor*);
    bool requestSearchLightDead();
    void setStartNerve();
    bool tryAppearFromWallEnd();
    bool tryAppearFromGroundEnd();
    bool tryRailTurn();
    bool tryRailTurnEnd();
    bool tryWalk();
    bool tryWalkEnd();
    bool tryChase();
    bool tryCheseEnd();
    bool tryFreeze();
    bool tryShay();
    bool tryShayEnd();
    bool tryLoveFind();
    bool tryLoveEnd();
    bool tryLoveFindEnd();
    bool tryLoveChaseEnd();
    bool tryLoveHitEnd();
    bool tryAggressiveEnd();
    bool tryAttackSuccessEnd();
    bool tryDriftEnd();
    bool tryHideWater();
    bool tryHideWall();
    bool tryHideWallEnd();
    bool tryAscension();
    void endAppear();
    void exeAppearFromWall();
    void exeAppearFromGround();
    void exeWait();
    void exeWalk();
    void exeRailWalk();
    void exeRailTurn();
    void exeLoveFind();
    void exeLoveChase();
    void exeLoveHit();
    void exeLoveEnd();
    void exeChase();
    void exeShay();
    void exeAggressive();
    void exeAttackSuccess();
    void exeDrift();
    void exeHideWall();
    void exeHideWater();
    void exeAscension();
    void exeStop();
    void exeFreeze();
    void exeEaten();
    void updateNormalVelocity();
    void addDriftVelocity();
    void updateDriftAnimScale();
    void updateDriftTransparency();
    void updateNormalTransparency();
    void setTransparency(f32);
    bool canAttack() const;
    bool canDrift() const;
    bool canSearchLightDead() const;
    bool isEnableStarPieceStop() const;
    bool isCheckWater() const;
    bool isEatable() const;
    bool isFreeze() const NO_INLINE;
    bool isShay() const NO_INLINE;
    void endDrift();
    void endAppearFromGround();
    void endAppearFromWall();

    inline bool isNotNear() const {
        bool isNearPlayer = MR::isNearPlayer(this, 2000.0f) == false;
        return isNearPlayer;
    }

    /* 0x90 */ PartsModel* mDisplayModel;
    /* 0x94 */ KeySwitch* mKeySwitch;
    /* 0x98 */ ClipAreaDrop* mClipAreaDrop;
    /* 0x9C */ AnimScaleController* mScaleController;
    /* 0xA0 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xA4 */ TQuat4f _94;
    /* 0xB4 */ TVec3f mWallNormal;
    /* 0xC0 */ TVec3f mWallHitPos;
    /* 0xCC */ TVec3f _BC;
    /* 0xD8 */ TVec3f _C8;
    /* 0xE4 */ TVec3f _D4;
    /* 0xF0 */ TVec3f _E0;
    /* 0xFC */ f32 _EC;
    /* 0x100 */ f32 _F0;
    /* 0x104 */ f32 _F4;
    /* 0x108 */ s32 mAppearanceType;
    /* 0x10C */ u8 _FC;
    /* 0x10D */ u8 _FD;
    /* 0x10E */ u8 _FE;
    /* 0x110 */ YoshiLockOnTarget* mLockOnTarget;
};
