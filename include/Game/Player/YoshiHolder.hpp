#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class Mario;
class Yoshi;
class LiveActor;
class YoshiTongueAction;
class YoshiLockOnManager;
class YoshiTongueTarget;
class YoshiArrow;
class YoshiFruitCounter;
class YoshiEgg;
#include "Game/Player/YoshiDashStep.hpp"
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
    bool fn_8041BBD0() const;
    void fn_8041BD30();
    void fn_8041BBF0(const TVec3f&, const TVec3f&, const TVec3f&);

    /* 0x00 */ Yoshi* mYoshi;
    /* 0x04 */ u8 _4[4];
    /* 0x08 */ YoshiTongueAction* mTongueAction;
    /* 0x0C */ YoshiLockOnManager* _C;
    /* 0x10 */ YoshiTongueTarget* _10;
    /* 0x14 */ YoshiArrow* mArrow;
    /* 0x18 */ LiveActor* mRideModelActor;
    /* 0x1C */ YoshiHistory15C50* mHistory;
    YoshiFruitCounter* mFruitCounter;
    YoshiEgg** mEggs;
    s32 mCapacity;
    s32 mCount;
    YoshiEgg* mCurrentEgg;
    /* 0x34 */ bool _34;
    /* 0x35 */ bool _35;
    s32 _38;
    u8 _3C;
    s32 mCountdown;
    s32 mTimer;
    u8 _48;
    Mario* mMario;
};
