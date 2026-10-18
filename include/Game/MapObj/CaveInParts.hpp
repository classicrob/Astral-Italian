#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"

class CaveInCube;
class WalkerStateBindStarPointer;

class CaveInParts : public LiveActor {
public:
    CaveInParts(CaveInCube*, const char*, const char*);

    virtual ~CaveInParts();
    virtual void init(const JMapInfoIter&) override;
    virtual void appear() override;
    virtual void control() override;
    virtual void attackSensor(HitSensor*, HitSensor*) override;
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*) override;
    void setLifeAndSpeed(s32, f32, f32, f32);
    void exeFall();

    /* 0x90 */ const char* mModelName;
    /* 0x94 */ CaveInCube* mCube;
    /* 0x98 */ s32 mLife;
    /* 0x9C */ f32 mMaxFallSpeed;
    /* 0xA0 */ f32 mGravityAccel;
    /* 0xA4 */ TVec3f mRotAxis;
    /* 0xB0 */ f32 mRotSpeed;
    /* 0xB4 */ WalkerStateBindStarPointer* mBindState;
};
