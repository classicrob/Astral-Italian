#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class HitSensor;
class PartsModel;
class WalkerStateBindStarPointer;

/// SMG2-only enemy. Offsets are SMG2 (LiveActor = 0x90). Written from the target asm.
class Grapyon : public LiveActor {
public:
    Grapyon(const char*);

    virtual ~Grapyon();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);

    void exeWait();
    void exeJump();
    void exeFall();
    void exeLand();
    void exeGuard();
    void exeAttackHit();
    void exeDown();
    void exeBlowDown();
    void exeDead();
    void exeSupportFreeze();

    void calcMove();
    bool isNormalNerve() const;
    bool isSwitchChanged() const;
    bool isJumpStartNerve() const;
    bool trySwitchFall();
    void updateHeadBtp();
    bool tryGuard(HitSensor*, HitSensor*);
    bool tryAttackHit();
    bool tryDown();
    bool tryElectric();
    bool tryBlowDown(HitSensor*, HitSensor*);

    /* 0x90 */ PartsModel* mHead;
    /* 0x94 */ TPos3f mMtx;
    /* 0xC4 */ AnimScaleController* mAnimScaleCtrl;
    /* 0xC8 */ WalkerStateBindStarPointer* mBindStar;
    /* 0xCC */ TVec3f mHomePos;
    /* 0xD8 */ TVec3f mFront;
    /* 0xE4 */ TVec3f mUp;
    /* 0xF0 */ f32 mScale;
    /* 0xF4 */ f32 mDirection;
    /* 0xF8 */ f32 mDirectionInit;
    /* 0xFC */ bool mSwitchState;
};
