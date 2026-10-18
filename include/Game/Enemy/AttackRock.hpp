#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Boss/KoopaFinalBattlePos.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class AttackRock;
class AttackRockFinal;
class AttackRockTutorial;
class BreakModel;
class PartsModel;
class ProjmapEffectMtxSetter;
class VolumeDrawer;
class KoopaJrRobotShockWave;

// SMG2-only unit (Koopa battle rocks). Offsets are SMG2 (LiveActor = 0x90). Written from the target asm.

// Helper that lets the player ride the rock ("AutoRush" handling).
// ---- unit-local free helpers (AttackRock family; defined in the .cpp files) ----
void fn_801552A0(LiveActor* pActor) NO_INLINE;
f32 fn_801552E0(LiveActor* pActor, TQuat4f* pQuat) NO_INLINE;
void fn_80155360(TVec3f* pOut, TQuat4f* pQuat, HitSensor* pSender, HitSensor* pReceiver, f32 speed) NO_INLINE;
void fn_801553D0(TVec3f* pOut, TVec3f* pDir, HitSensor* pSender, HitSensor* pReceiver, f32 speed) NO_INLINE;
void fn_801554D0(TPos3f* pMtx, LiveActor* pActor) NO_INLINE;
void fn_80155540(ProjmapEffectMtxSetter* pSetter, TQuat4f* pQuat, TVec3f* pPos) NO_INLINE;
ProjmapEffectMtxSetter* fn_80155660(LiveActor* pActor) NO_INLINE;

class AttackRockRide {
public:
    // SMG2-only functions (original names unknown; named by address)
    AttackRockRide(LiveActor* pHost);
    void fn_801529E0(const TVec3f* pVec);
    void fn_80152A00();
    void fn_80152C00();
    bool fn_80152E20(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    void fn_80152F10() NO_INLINE;
    void fn_80152F20() NO_INLINE;
    // SMG2-only functions (original names unknown; named by address)

    /* 0x00 */ LiveActor* mHost;
    /* 0x04 */ TQuat4f mQuat0;
    /* 0x14 */ TQuat4f mQuat1;
    /* 0x24 */ TQuat4f mQuat2;
    /* 0x34 */ TVec3f _34;
    /* 0x40 */ TVec3f _40;
    /* 0x4C */ TVec3f _4C;
    /* 0x58 */ TVec3f _58;
    /* 0x64 */ TVec3f _64;
    /* 0x70 */ TVec3f _70;
    /* 0x7C */ TVec3f _7C;
    /* 0x88 */ f32 _88;
    /* 0x8C */ f32 _8C;
    /* 0x90 */ f32 _90;
    /* 0x94 */ s32 _94;
    /* 0x98 */ s32 _98;
    /* 0x9C */ bool mIsRush;
    /* 0x9D */ bool mIsEnable;
};

// State of the rock while it is held/floating by Koopa.
class AttackRockStateHold : public ActorStateBase< AttackRock > {
public:
    // SMG2-only functions (original names unknown; named by address)
    bool fn_80153400();
    bool fn_80153460();
    void fn_801534D0();
    void fn_80153530();
    void fn_80153620();
    void fn_80153720();
    void fn_801537D0();
    AttackRockStateHold(AttackRock*, s32);
    virtual ~AttackRockStateHold();

    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);


    /* 0x10 */ AttackRockRide* mRide;
    /* 0x14 */ BreakModel* mBreakModel;
    /* 0x18 */ TVec3f mStartPos;
    /* 0x24 */ TVec3f mUpVec;
    /* 0x30 */ f32 mRate;
    /* 0x34 */ s32 mRockNum;
};

// State of the rock while it falls and creates the shockwave.
class AttackRockStateFall : public ActorStateBase< AttackRock > {
public:
    // SMG2-only functions (original names unknown; named by address)
    void fn_80153DE0();
    void fn_80153EE0();
    void fn_80153FE0();
    void fn_80154060();
    void fn_80154280();
    void fn_80154400();
    AttackRockStateFall(AttackRock*, s32);
    virtual ~AttackRockStateFall();

    virtual void appear();
    virtual void kill();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);


    /* 0x10 */ KoopaJrRobotShockWave* mBreakModel;
    /* 0x14 */ VolumeDrawer* mVolumeDrawer;
    /* 0x18 */ TVec3f mFallPos;
    /* 0x24 */ TVec3f mStartPos;
    /* 0x30 */ TVec3f mLandPos;
    /* 0x3C */ TVec3f mLandNormal;
    /* 0x48 */ bool mIsNear;
};

class AttackRock : public LiveActor {
public:
    f32 fn_801510C0() const NO_INLINE;
    bool fn_801510D0() const NO_INLINE;
    bool fn_801517C0(s32) const;
    void fn_80151160();
    void fn_80151510();
    void fn_801516E0();
    AttackRock(const char* pName);

    virtual ~AttackRock();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);



    /* 0x90 */ TPos3f mMtx90;
    /* 0xC0 */ TPos3f mMtxC0;
    /* 0xF0 */ TQuat4f mQuatF0;
    /* 0x100 */ TQuat4f mQuat100;
    /* 0x110 */ BreakModel* mBreakModel;
    /* 0x114 */ LiveActor* mDemoObj;
    /* 0x118 */ ProjmapEffectMtxSetter* mProjmapEffectMtxSetter;
    /* 0x11C */ AttackRockStateFall* mStateFall;
    /* 0x120 */ AttackRockStateHold* mStateHold;
    /* 0x124 */ s32 mRockNum;
    /* 0x128 */ bool mIsOnPlayer;
};

class AttackRockFinal : public LiveActor {
public:
    bool fn_80152700(s32) const;
    void fn_801521F0(f32 a, f32 b);
    void fn_80152280();
    void fn_801522F0();
    void fn_80152400();
    void fn_801524F0();
    void fn_80152620();
    AttackRockFinal(const char*);

    virtual ~AttackRockFinal();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    /* 0x90 */ ProjmapEffectMtxSetter* mProjmapEffectMtxSetter;
    /* 0x94 */ BreakModel* mBreakModel;
    /* 0x98 */ AttackRockRide* mRide;
    /* 0x9C */ TPos3f mMtx9C;
    /* 0xCC */ TQuat4f mQuatCC;
    /* 0xDC */ TQuat4f mQuatDC;
    /* 0xEC */ TVec3f mStartPos;
    /* 0xF8 */ TVec3f mAxisF8;
    /* 0x104 */ TVec3f mAxis104;
    /* 0x110 */ s32 mWaitStep;
    /* 0x114 */ bool mIsOnPlayer;
};

class AttackRockTutorial : public LiveActor {
public:
    bool fn_80155090(s32) const;
    void fn_80154B60();
    void fn_80154BB0();
    void fn_80154C60();
    void fn_80154D40();
    void fn_80154E90();
    void fn_80154F90();
    AttackRockTutorial(const char*);

    virtual ~AttackRockTutorial();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    /* 0x90 */ TPos3f mMtx90;
    /* 0xC0 */ ProjmapEffectMtxSetter* mProjmapEffectMtxSetter;
    /* 0xC4 */ BreakModel* mBreakModel;
    /* 0xC8 */ PartsModel* mHillModel;
    /* 0xCC */ ActorCameraInfo* mCameraInfo;
    /* 0xD0 */ AttackRockRide* mRide;
    /* 0xD4 */ TQuat4f mQuatD4;
    /* 0xE4 */ TVec3f mStartPos;
    /* 0xF0 */ TVec3f mUpVec;
    /* 0xFC */ f32 mRate;
    /* 0x100 */ s32 mTouchCount;
    /* 0x104 */ bool mTouched;
    /* 0x105 */ bool mUseCamera;
};
