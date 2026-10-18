#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/WaterInfo.hpp"

class AnimScaleController;
class HitSensor;
class PartsModel;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

// SMG2-only unit (Menbo, frozen-ice walker that swoons / breaks). LiveActor = 0x90 bytes in SMG2.
class Menbo : public LiveActor {
public:
    Menbo(const char* pName);

    virtual ~Menbo();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual void startClipped();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    void exeMove();
    void exeTurn();
    NO_INLINE void fn_801C43C0();
    void exeFind();
    void exeAttack();
    NO_INLINE void fn_801C4720();
    void exeSwoonStart();
    void exeSwoon();
    void exeSwoonEnd();
    void exeFreeze();
    void exePressDown();
    void exeBlownDown();
    void exeBreak();

    // Helpers (unnamed in the SMG2 symbol table; named by role).
    NO_INLINE bool fn_801C53A0() const;
    NO_INLINE bool fn_801C5430() const;
    NO_INLINE bool fn_801C54D0() const;
    NO_INLINE bool fn_801C55B0(HitSensor*, HitSensor*);
    NO_INLINE bool fn_801C5680(HitSensor* pSender, HitSensor* pReceiver);
    void updateWaterInfo();
    void blendUpVec();
    void updateGravityWater();
    NO_INLINE void fn_801C58E0(f32 rate);
    NO_INLINE void fn_801C5A50();
    NO_INLINE void fn_801C5AB0();
    NO_INLINE void fn_801C5B10();
    NO_INLINE bool fn_801C5BD0() const;
    NO_INLINE bool fn_801C5C30(f32 angle);
    bool isGroundIceEdge();
    bool isPlayerInSightWet();
    NO_INLINE f32 fn_801C5E60(f32 rate);
    NO_INLINE f32 fn_801C5E90();
    NO_INLINE void fn_801C5EF0(TPos3f* pMtx, const LiveActor* actor, const TVec3f& rUp, const TVec3f& rFront);

public:
    /* 0x90 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x94 */ WalkerStateBindStarPointer* mStarPointBind;
    /* 0x98 */ PartsModel* mPartsModel;      // ice model (PartsModel)
    /* 0x9C */ WaterInfo mWaterInfo;         // 0x54 bytes
    /* 0xF0 */ TVec3f mSpawnPos;
    /* 0xFC */ TVec3f mSpawnRot;
    /* 0x108 */ TVec3f mBasePos;
    /* 0x114 */ TVec3f mFront;
    /* 0x120 */ TVec3f mUp;
    /* 0x12C */ s32 mCooldown;
    /* 0x130 */ f32 mTurnDir;
    /* 0x134 */ f32 mTurnRate;
    /* 0x138 */ f32 mTurnTimer;
    /* 0x13C */ s32 mFreezeStep;
    /* 0x140 */ s32 mRetryCount;
    /* 0x144 */ f32 mGroundDist;
    /* 0x148 */ bool mIsInWater;
    /* 0x149 */ bool mIsPlayerWet;
    /* 0x14A */ bool mIsTurning;
    /* 0x14C */ YoshiLockOnTarget* mYoshiLockOn;
};
