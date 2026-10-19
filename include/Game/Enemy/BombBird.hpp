#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/Array.hpp"

class AnimScaleController;
class BombBirdBomb;
class FixedPosition;
class SpinHitController;
class WalkerStateBindStarPointer;

struct BombBirdParam {
    /* 0x00 */ f32 mMoveAccel;
    /* 0x04 */ f32 mGravityAccel;
    /* 0x08 */ f32 mVelocityDamping;
    /* 0x0C */ f32 mTurnMaxDegree;
};

class BombBird : public LiveActor {
public:
    BombBird(const char*);

    virtual ~BombBird() override;
    virtual void init(const JMapInfoIter&) override;
    virtual void makeActorDead() override;
    virtual void startClipped() override;
    virtual void control() override;
    virtual void calcAndSetBaseMtx() override;
    virtual void attackSensor(HitSensor*, HitSensor*) override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*) override;

    void exeBombBirdBindStarPointer();
    void exeBombBirdBindStarPointerGround();
    bool isFlyingNerve() const;
    bool isGroundSwoonNerve() const;
    void startSpinBlow(HitSensor*, HitSensor*);
    void exeFlyOnRail();
    void exeFlyOnRailSearch();
    void doFly();
    void addVelocityToUpFromGround(f32);
    void killAllBomb();
    void exeFlyWithAttack();
    void exeBlow();
    void exeEscape();
    void exeDamage();
    void exeFall();
    void exeSwoonStart();
    void exeSwoon();
    void exeSwoonEnd();
    void exeTrample();
    void exeSinkDown();

    /* 0x8C */ MR::FixedArray< BombBirdBomb*, 3 > mBombs;
    /* 0x98 */ BombBirdBomb* mHeldBomb;
    /* 0x9C */ TVec3f mFront;
    /* 0xA8 */ bool mSearchPlayer;
    /* 0xA9 */ bool mHoldBomb;
    /* 0xAC */ FixedPosition* mFixedPos;
    /* 0xB0 */ SpinHitController* mSpinHitController;
    /* 0xB4 */ AnimScaleController* mScaleController;
    /* 0xB8 */ WalkerStateBindStarPointer* mStarPointer;
    /* 0xC0 */ WalkerStateBindStarPointer* mStarPointer2;  // SMG2-only second star-pointer bind
};
