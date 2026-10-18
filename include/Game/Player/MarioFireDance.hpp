#pragma once

#include "Game/Player/MarioState.hpp"

class MarioActor;

class MarioFireDance : public MarioState {
public:
    MarioFireDance(MarioActor*);

    virtual bool start();
    virtual bool close();
    virtual bool update();

    void impact();
    void impactEffect();

    // SMG2-only (original names unknown; named by address)
    void fn_803E5FA0();

    /* 0x14 */ TVec3f _14;
    /* 0x20 */ f32 _20;
    /* 0x24 */ u8 _24;
    /* 0x26 */ u16 _26;
    /* 0x28 */ u8 _28;
    /* 0x29 */ u8 _29;
    /* 0x2A */ u16 _2A;
    /* 0x2C */ u16 _2C;
};
