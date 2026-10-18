#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/NerveExecutor.hpp"

class Mario;
class MarioActor;
class XanimePlayer;

// SMG2 Yoshi riding system. Only the members the Player units call are declared; names are by address
// (original names unknown), layouts are not recovered.

/// @brief The Yoshi actor (LiveActor subclass, units around 0x8040B000..0x80410000).
class Yoshi : public LiveActor {
public:
    void fn_8040CA10();
    void fn_8040CB30();
    u16 fn_8040F440();

    void fn_8040CCD0(const TVec3f&);
    void fn_8040CD20(const TVec3f&);
    void fn_8040CDD0(const TVec3f&);
    void fn_8040CE80();
    void fn_8040CED0();
    void fn_8040CF20();
    bool fn_8040CF40() const;
    void fn_8040EDA0(const char*);
    void fn_8040EE30(const char*);
    void fn_8040EE70(const char*);
    void fn_8040ED90(const char*);
    void fn_8040EDE0(const char*);
    void fn_8040EE00(const char*);
    bool fn_8040F450(const char*);
    bool fn_8040F490(const char*);
    bool fn_8040F540(const char*);
    bool fn_8040F570();
    bool fn_8040F590(const char*);
    bool fn_8040F0E0() const;
    void fn_8040F370();

    /* 0x90 */ u8 _90[8];
    /* 0x98 */ XanimePlayer* mXanimePlayer;
    /* 0x9C */ XanimePlayer* mXanimePlayerUpper;
    /* 0xA0 */ u8 _A0[5];
    /* 0xA5 */ bool mUpperAnimationActive;
};

/// @brief The state machine that drives Yoshi's tongue (MarioActor holder + 0x8).
class YoshiTongueAction : public NerveExecutor {
public:
    void fn_8041DF40();
    bool fn_8041F8A0() const;
    bool fn_8041F910() const;
    bool fn_8041FCD0();
    bool fn_8041FD00();
    bool fn_8041FD80();
    void fn_8041FF70();
    void fn_8041FF80();
    void fn_80420000();
};

/// @brief The object owned by MarioActor that bundles the Yoshi actor, tongue action and tongue (size 0x50).
class YoshiHolder {
public:
    YoshiHolder(Mario*);

    void* fn_8041B6F0();
    void fn_8041B700(LiveActor*);
    void fn_8041BAE0();
    void fn_8041BAF0();
    void fn_8041BB00();
    bool fn_8041BB30() const;
    f32 fn_8041BB50() const;
    void fn_8041BB90();
    void fn_8041BBA0();
    void fn_8041BBB0();
    void fn_8041BBF0(const TVec3f&, const TVec3f&, const TVec3f&);

    /* 0x00 */ Yoshi* mYoshi;
    /* 0x04 */ u8 _4[4];
    /* 0x08 */ YoshiTongueAction* mTongueAction;
    /* 0x0C */ void* _C;
    /* 0x10 */ void* _10;
    /* 0x14 */ u8 _14[0x3C];
};

/// @brief Static queries of Yoshi's tuning values (they ignore their object).
namespace YoshiParam {
    s32 fn_8041BD40();
    s32 fn_8041BD60();
    f32 fn_8041BD80();
    f32 fn_8041BD90();
    f32 fn_8041BDC0();
    f32 fn_8041BDD0();
    f32 fn_8041BDE0();
};  // namespace YoshiParam
