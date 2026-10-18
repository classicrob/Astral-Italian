#pragma once
#include "Game/Player/MarioState.hpp"
class MarioYoshiDash : public MarioState {
public:
    MarioYoshiDash(MarioActor*);
    virtual bool start();
    virtual bool close();
    virtual bool update();
};
