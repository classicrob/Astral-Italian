#pragma once

#include "Game/Player/MarioState.hpp"

class MarioActor;

class MarioFlow : public MarioState {
public:
    MarioFlow(MarioActor*);

    virtual bool start();
    virtual bool close();
    virtual bool update();

    enum Phase { Airborne = 0, Landed = 1 };

    /* 0x12 */ u16 mFrameCounter;
    /* 0x14 */ u16 mPhase;
    /* 0x18 */ TVec3f mVelocity;
};
