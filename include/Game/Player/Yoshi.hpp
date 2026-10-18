#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Player/YoshiTongue.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include "Game/System/NerveExecutor.hpp"

class Mario;
class MarioActor;
class XanimePlayer;
class Yoshi;

class YoshiAnimationSync {
public:
    YoshiAnimationSync(Yoshi*);
    void update();
    void fn_80413440();
    void fn_804135A0(const char*);
    Yoshi* mActor;
    const char* mAnimation;
    bool _8;
    bool _9;
    bool _A;
};

class YoshiModelCtrl {
public:
    void fn_80417FE0();
    void fn_80418020();
    void fn_80418120();
};

class YoshiUpdateTask {
public:
    virtual void update() = 0;
};

class YoshiUpdateTaskList {
public:
    YoshiUpdateTaskList(s32);
    void add(YoshiUpdateTask*);
    void update();
    YoshiUpdateTask** mTasks;
    s32 mCapacity;
    s32 mCount;
};

class YoshiVelocityAdjustment {
public:
    YoshiVelocityAdjustment();
    void include(const TVec3f&);
    void apply(Yoshi*);
    void remove(Yoshi*) NO_INLINE;
    TVec3f mMinimum;
    TVec3f mMaximum;
};

// SMG2 Yoshi riding system. Unknown method names retain their target addresses.

class YoshiHeadCtrl {
public:
    bool fn_8041AD70(s32) const;
    void fn_8041AA70(s32, const TVec3f*);
    void fn_8041AB50(s32);
};

/// @brief The Yoshi actor (LiveActor subclass, units around 0x8040B000..0x80410000).
class Yoshi : public LiveActor {
public:
    Yoshi(const char*);
    virtual ~Yoshi();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual void calcAnim();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void fn_8040E2D0();
    void fn_8040CF50();
    void fn_8040CFC0();
    void fn_8040D130();
    void fn_8040D260();
    void fn_8040D380();
    void fn_8040D5B0();
    void fn_8040D6E0();
    void fn_8040D7F0();
    void fn_8040D8E0();
    void fn_8040D9F0();
    void fn_8040DB10();
    void fn_8040DC20();
    void fn_8040DDA0();
    void fn_8040DE20();
    void fn_8040DF10();
    void fn_8040DFB0();
    void fn_8040E080();
    void fn_8040E160();
    void fn_8040E230();
    void fn_8040E350();
    void fn_8040E660();
    void fn_8040E750();
    void fn_8040E820();
    void fn_8040E8F0();
    void fn_8040E9E0();
    void fn_8040EAE0();
    void fn_8040EB60();
    void fn_8040EBF0();
    void fn_80411ED0();
    void fn_80410890();
    void fn_80410D90();
    void fn_804109E0();
    void fn_80410AC0();
    void fn_80410BA0();
    void fn_804117F0();
    void fn_80411BF0();
    bool fn_80412270() const;
    bool fn_804113C0() const;
    bool fn_8040FEE0() const;
    bool fn_804100E0() const;
    bool fn_804101D0() const;
    bool fn_80411960() const;
    bool fn_80411D10() const;
    bool fn_80412310() const;
    void fn_804116B0();
    void fn_804103F0();
    void fn_80411A30();
    void fn_8040CC40();
    void fn_8040C960();
    void fn_8040CA10();
    void fn_8040CB30();
    u16 fn_8040F440();
    bool fn_8040F530() const;

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
    void fn_8040F320();
    bool fn_8040F5B0(const char*);
    f32 fn_8040F620() const;
    void fn_8040F630(const TPos3f&);
    u16 fn_8040F190() const;
    HitSensor* fn_8040F1C0(s32) const;
    const TVec3f& fn_8040F200() const;

    /* 0x90 */ s32 mRideCountdown;
    /* 0x94 */ void* _94;
    /* 0x98 */ XanimePlayer* mXanimePlayer;
    /* 0x9C */ XanimePlayer* mXanimePlayerUpper;
    /* 0xA0 */ u8 _A0[4];
    /* 0xA4 */ bool mUpperDefaultAnimationEnabled;
    /* 0xA5 */ bool mUpperAnimationActive;
    /* 0xA6 */ u8 _A6[2];
    /* 0xA8 */ TPos3f mBaseMtx;
    /* 0xD8 */ u8 _D8[0x10];
    /* 0xE8 */ TVec3f mBinderOffset;
    /* 0xF4 */ YoshiTongue* mTongue;
    /* 0xF8 */ YoshiModelCtrl* mYoshiModelCtrl;
    /* 0xFC */ YoshiHeadCtrl* mHeadCtrl;
    /* 0x100 */ u8 _100[8];
    /* 0x108 */ YoshiUpdateTaskList* mUpdateTasks;
    /* 0x10C */ u8 _10C[0x14];
    /* 0x120 */ YoshiVelocityAdjustment* mVelocityAdjustment;
    /* 0x124 */ u8 _124[8];
    /* 0x12C */ YoshiAnimationSync* mAnimationSync;
    /* 0x130 */ bool _130;
    /* 0x131 */ bool mMountedBySpin;
    /* 0x132 */ u8 _132[2];
    /* 0x134 */ s32 mDashTimer;
    /* 0x138 */ bool mDashEffectActive;
    /* 0x139 */ bool mDashTopWaterEffectActive;
    /* 0x13A */ u8 _13A[2];
    /* 0x13C */ s32 _13C;
    /* 0x140 */ s32 mSupportSpinTimer;
    /* 0x144 */ bool _144;
    /* 0x145 */ bool _145;
    /* 0x146 */ u8 _146[0x1E];
    /* 0x164 */ TVec3f mMovementVelocity;
    /* 0x170 */ TVec3f mPreviousVelocity;
    /* 0x17C */ TVec3f mAdditionalVelocity;
    /* 0x188 */ u8 _188[0x10];
    /* 0x198 */ bool _198;
    /* 0x199 */ u8 _199[3];
    /* 0x19C */ LiveActor* mRideModel;
    /* 0x1A0 */ bool mRideModelVisible;
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
    /* 0x14 */ u8 _14[4];
    /* 0x18 */ LiveActor* mRideModelActor;
    /* 0x1C */ u8 _1C[0x34];
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
