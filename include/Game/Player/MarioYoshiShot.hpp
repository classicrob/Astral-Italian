#pragma once
#include "Game/Player/MarioState.hpp"
class MarioYoshiShot : public MarioState {
public:
    MarioYoshiShot(MarioActor*);
    virtual bool start();
    virtual bool close();
    virtual bool update();
    s32 mTimer;
    TVec3f mShotVelocity;
    TVec3f mInheritedVelocity;
};
