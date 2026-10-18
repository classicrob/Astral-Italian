#pragma once
#include "Game/Player/YoshiHolder.hpp"

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Player/YoshiTongue.hpp"
#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/Player/YoshiModelCtrl.hpp"
#include "Game/Player/MarioMtxPos.hpp"
#include "Game/Map/HitInfo.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include "Game/System/NerveExecutor.hpp"

class Mario;
class MarioActor;
class XanimePlayer;
class Yoshi;
class WaterInfo;
class XanimeResourceTable;
class BreakModel;
class YoshiAnimationModelCtrl {
public:
    YoshiAnimationModelCtrl(Yoshi*, XanimePlayer*, const char*);
    virtual ~YoshiAnimationModelCtrl();
    virtual void calcAnim();
    virtual void appear();
    virtual void kill();
    void fn_8040B6D0();
    u8 _4[0x28];
};
class YoshiFruitCtrl {
public:
    YoshiFruitCtrl(Yoshi*);
    void fn_80415630();
    void fn_804157E0();
    void fn_80415800();
    Yoshi* mActor;
    bool mLightFruitActive;
    bool _5;
    u8 _6[2];
    s32 _8;
    s32 _C;
    bool _10;
    bool _11;
    u8 _12[2];
    s32 _14;
    s32 _18;
};

class YoshiSilhouette : public LiveActor {
public:
    YoshiSilhouette(Yoshi*);
    u8 _90[8];
};
class YoshiModelVisibility {
public:
    YoshiModelVisibility(Yoshi*, YoshiAnimationModelCtrl*);
    void fn_804132C0();
    void fn_80413330();
    Yoshi* mActor;
    YoshiAnimationModelCtrl* mModelCtrl;
    bool mEnabled;
    bool mAppeared;
};

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

class YoshiGroundChecker : public YoshiUpdateTask {
public:
    YoshiGroundChecker(Yoshi*);
    virtual void update();
    bool isGrounded() const;
    bool isAirborne() const;
    Yoshi* mActor;
    s32 mAirborneFrames;
    bool mUpdated;
    bool mSlipping;
    HitInfo mGroundHit;
};

class YoshiAutoDisappearTask : public YoshiUpdateTask {
public:
    YoshiAutoDisappearTask(Yoshi*);
    virtual void update();
    Yoshi* mActor;
    s32 mTimer;
};

class YoshiRandomTurn {
public:
    YoshiRandomTurn();
    void reset();
    void update(const TPos3f&, TVec3f*);
    s32 mTimer;
    s32 mTurnTimer;
    f32 mTurnAngle;
};

class YoshiEscapeDirection {
public:
    YoshiEscapeDirection(TPos3f*);
    void reset();
    void update();
    void finish();
    TPos3f* mBaseMtx;
    s32 mStep;
    s32 mHitCount;
    TVec3f mDirection;
};

class YoshiPlayerFacingCtrl {
public:
    YoshiPlayerFacingCtrl(TPos3f*);
    bool fn_80413040();
    void fn_80413110();
    bool fn_804131C0(const TVec3f&);
    TPos3f* mBaseMtx;
    s32 mTimer;
    s32 mTurnTimer;
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
    YoshiHeadCtrl(YoshiModelCtrl*);
    void fn_8041A9F0();
    bool fn_8041AD70(s32) const;
    void fn_8041AA70(s32, const TVec3f*);
    void fn_8041AB50(s32);
    void fn_8041AD20(s32);
    void fn_8041AC40(s32);
    YoshiModelCtrl* mModelCtrl;
    s32 _4;
    s32 mTargetCount;
    bool mEnabled[2];
    bool _E[2];
    const TVec3f* mTargets[2];
};

/// @brief The Yoshi actor (LiveActor subclass, units around 0x8040B000..0x80410000).
class Yoshi : public LiveActor {
public:
    bool fn_8040F5F0() const;
    bool fn_8040F070() const;
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
    void fn_80410280();
    void fn_804103F0();
    void fn_80411A30();
    void fn_8040CBD0();
    void fn_8040CBF0();
    void fn_8040CC00();
    void fn_8040CC20();
    void fn_8040CC30();
    void fn_8040CC40();
    void fn_8040EF20();
    void fn_8040EF30();
    void fn_8040FCD0();
    void fn_8040FD80();
    void fn_8040FE30();
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
    void fn_8040F030();
    void fn_8040F050();
    void fn_8040F080();
    void fn_8040F020();
    void fn_8040F0A0();
    void fn_8040F0B0();
    void fn_8040F0D0();
    void fn_8040F110();
    void fn_8040F310();
    void fn_8040F230(const JMapInfoIter&);
    MtxPtr fn_8040F300();
    void fn_8040EF40();
    void fn_8040EFB0();
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
    void fn_80410570(f32);
    bool fn_80410810(TVec3f*) const;
    void fn_80411630();
    void fn_804121A0(const TVec3f&, const TVec3f&);
    bool fn_80412590(f32) const;
    void fn_804124D0(f32);
    bool fn_804112A0() const;
    s32 fn_8040F190() const;
    HitSensor* fn_8040F1C0(s32) const;
    const TVec3f& fn_8040F200() const;

    /* 0x90 */ s32 mRideCountdown;
    /* 0x94 */ XanimeResourceTable* mAnimationResources;
    /* 0x98 */ XanimePlayer* mXanimePlayer;
    /* 0x9C */ XanimePlayer* mXanimePlayerUpper;
    /* 0xA0 */ void* _A0;
    /* 0xA4 */ bool mUpperDefaultAnimationEnabled;
    /* 0xA5 */ bool mUpperAnimationActive;
    /* 0xA6 */ u8 _A6[2];
    /* 0xA8 */ TPos3f mBaseMtx;
    /* 0xD8 */ u8 _D8[0x10];
    /* 0xE8 */ TVec3f mBinderOffset;
    /* 0xF4 */ YoshiTongue* mTongue;
    /* 0xF8 */ YoshiModelCtrl* mYoshiModelCtrl;
    /* 0xFC */ YoshiHeadCtrl* mHeadCtrl;
    /* 0x100 */ YoshiAnimationModelCtrl* mAnimationModelCtrl;
    /* 0x104 */ YoshiFruitCtrl* mFruitCtrl;
    /* 0x108 */ YoshiUpdateTaskList* mUpdateTasks;
    /* 0x10C */ YoshiGroundChecker* mGroundChecker;
    /* 0x110 */ YoshiAutoDisappearTask* mAutoDisappearTask;
    /* 0x114 */ YoshiRandomTurn* mRandomTurn;
    /* 0x118 */ bool* mJumpRequested;
    /* 0x11C */ YoshiEscapeDirection* mEscapeDirection;
    /* 0x120 */ YoshiVelocityAdjustment* mVelocityAdjustment;
    /* 0x124 */ YoshiPlayerFacingCtrl* mPlayerFacingCtrl;
    /* 0x128 */ YoshiModelVisibility* mModelVisibility;
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
    /* 0x146 */ u8 _146[2];
    /* 0x148 */ WaterInfo* mWaterInfo;
    /* 0x14C */ TVec3f _14C;
    /* 0x158 */ s32 _158;
    /* 0x15C */ f32 _15C;
    /* 0x160 */ s32 _160;
    /* 0x164 */ TVec3f mMovementVelocity;
    /* 0x170 */ TVec3f mPreviousVelocity;
    /* 0x17C */ TVec3f mAdditionalVelocity;
    /* 0x188 */ s32 _188;
    /* 0x18C */ TVec3f _18C;
    /* 0x198 */ bool _198;
    /* 0x199 */ u8 _199[3];
    /* 0x19C */ LiveActor* mRideModel;
    /* 0x1A0 */ bool mRideModelVisible;
    /* 0x1A1 */ bool _1A1;
    /* 0x1A2 */ bool _1A2;
    /* 0x1A4 */ BreakModel* mBreakModel;
    /* 0x1A8 */ MarioMtxPos mBreakModelMtx;
};

/// @brief The object owned by MarioActor that bundles the Yoshi actor, tongue action and tongue (size 0x50).


/// @brief Static queries of Yoshi's tuning values (they ignore their object).
namespace YoshiParam {
    void fn_8041BE00(TVec3f*);
    s32 fn_8041BD40();
    s32 fn_8041BD60();
    f32 fn_8041BD80();
    f32 fn_8041BD90();
    f32 fn_8041BDC0();
    f32 fn_8041BDD0();
    f32 fn_8041BDE0();
};  // namespace YoshiParam
