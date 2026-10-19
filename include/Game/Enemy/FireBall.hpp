#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class WalkerStateBindStarPointer;

class FireBall : public LiveActor {
public:
    FireBall(const char* = "ファイアーボール");

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void initHitSensors(const char*);
    void appearAndThrow(const TVec3f&, f32, f32);
    HitSensor* isBindedAny() const;
    void setVelocityToPlayer(f32);
    void calcReflectVelocity();
    bool tryToKill();
    void launchFireBallWithVelocity(const TVec3f&, const TVec3f&);

    void exeThrow();
    void exeReflect();
    void exeFreeze();
    void randomizeReflectVelocity();

    /* 0x8C */ LiveActor* mHost;
    /* 0x94 */ AnimScaleController* mScaleController;
    /* 0x98 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x9C */ TVec3f mReflectDir;
    /* 0xA8 */ TVec3f mUp;
    /* 0xB4 */ f32 mUnusedB4;
};
