#pragma once

#include "Game/Player/MarioState.hpp"

class MarioActor;

/// @brief The SMG2 Yoshi pull state (MarioState status 0x26).
class MarioYoshiPull : public MarioState {
public:
    MarioYoshiPull(MarioActor*);

    virtual bool start();
    virtual bool close();
    virtual bool update();

    // SMG2-only (original names unknown; named by address)
    void fn_804090C0();
    bool exePull();
    s32 mPhase;
    s32 mTimer;
};
