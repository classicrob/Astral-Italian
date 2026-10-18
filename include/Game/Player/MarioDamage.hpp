#pragma once

#include "Game/Player/MarioState.hpp"

class MarioActor;
class HorrorDamageExecutor;

class MarioDamage : public MarioState {
public:
    MarioDamage(MarioActor*);

    virtual bool start();
    virtual bool close();
    virtual bool update();
    virtual bool notice();

    void setVec(const TVec3f&);
    void setVecSize(f32, f32);

    void stopHead(const TVec3f&);
    bool updateHorrorDamage();

    inline void setStrings(const char* pChar1, const char* pChar2) {
        _28 = pChar1;
        _2C = pChar2;
    }

    /* 0x11 */ u8 _11;
    /* 0x12 */ bool _12;
    /* 0x14 */ u16 _14;
    /* 0x16 */ u16 _16;
    /* 0x18 */ u16 _18;
    /* 0x1C */ TVec3f _1C;
    /* 0x28 */ const char* _28;
    /* 0x2C */ const char* _2C;
    /* 0x30 */ bool _30;
    /* 0x34 */ TVec3f _34;
    /* 0x40 */ HorrorDamageExecutor* mHorrorDamageExecutor;
    /* 0x44 */ u16 _44;
};
