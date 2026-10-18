#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class Mario;
class Yoshi;
class LiveActor;
class YoshiTongueAction;
class YoshiHolder {
public:
    YoshiHolder(Mario*);

    void* fn_8041B6F0();
    void fn_8041B700(LiveActor*);
    void fn_8041B820(LiveActor*);
    void fn_8041B840(LiveActor*);
    void fn_8041B8B0(LiveActor*);
    void fn_8041B920();
    void fn_8041B970();
    void fn_8041B9C0();
    void fn_8041BA10();
    void fn_8041BA20(MtxPtr, LiveActor*);
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
    /* 0x14 */ u8 _14[4];
    /* 0x18 */ LiveActor* mRideModelActor;
    /* 0x1C */ u8 _1C[0x18];
    /* 0x34 */ bool _34;
    /* 0x35 */ u8 _35[0x1B];
};
