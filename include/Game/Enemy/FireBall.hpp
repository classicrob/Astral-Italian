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
    void fn_8017DA20(const TVec3f&, const TVec3f&);  // SMG2-only, by address (OtaRockTank throw)

    void exeThrow();
    void exeReflect();
    void exeFreeze();
    void fn_8017DF50();

    /* 0x8C */ LiveActor* mHost;
    /* 0x94 */ AnimScaleController* _94;
    /* 0x98 */ WalkerStateBindStarPointer* _98;
    /* 0x9C */ TVec3f _9C;
    /* 0xA8 */ TVec3f mUp;
    f32 _B4;
};
