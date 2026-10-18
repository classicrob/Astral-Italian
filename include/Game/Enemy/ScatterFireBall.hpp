#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ScatterFireBall : public LiveActor {
public:
    ScatterFireBall(const char* = "拡散ファイアーボール");
    virtual ~ScatterFireBall();

    void fn_8017E490(const TVec3f&, f32, f32);
    void fn_8017E660(const TVec3f&, const TVec3f&);
    void fn_8017E6D0(const TVec3f&, const TVec3f&);
    bool fn_8017E750();
    void fn_8017E7B0();

    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ s32 _94;
    /* 0x98 */ s32 _98;
    /* 0x9C */ TVec3f _9C;
    /* 0xA8 */ TVec3f _A8;
    /* 0xB4 */ f32 _B4;
    /* 0xB8 */ u32 _B8;
    /* 0xBC */ void* _BC;
    /* 0xC0 */ bool _C0;
};
