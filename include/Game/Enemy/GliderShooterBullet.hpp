#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;

/// @brief Glider shooter bullet (SMG2 only, no SMG1 source). Flies toward its target, can be frozen by a star pointer bind.
class GliderShooterBullet : public LiveActor {
public:
    GliderShooterBullet();

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void startClipped();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeFly();
    void exeFreeze();

    /* 0x90 */ TVec3f mUnk90;
    /* 0x9C */ TVec3f mUnk9C;
    /* 0xA8 */ AnimScaleController* mAnimScaleController;
    /* 0xAC */ WalkerStateBindStarPointer* mBindState;
};

