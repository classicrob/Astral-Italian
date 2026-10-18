#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TQuat.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class WalkerStateBindStarPointer;
class VolumeDrawer;
class YoshiLockOnTarget;
class ActorStateParamScale;
class HitSensor;

/// @brief Peto (a balloon-like enemy that extends/shrinks and jumps at the player).
/// Layout from the SB4E01 ctor (LiveActor is 0x90 bytes in SMG2).
class Peto : public LiveActor {
public:
    Peto(const char*);
    virtual ~Peto();

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void movement();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    NO_INLINE void fn_801EB600();
    NO_INLINE void fn_801EB690();
    NO_INLINE void fn_801EB740();
    NO_INLINE void fn_801EB7C0();
    NO_INLINE void fn_801EB850();
    NO_INLINE void fn_801EB900();
    void exeShrink();
    NO_INLINE void fn_801EB9F0();
    NO_INLINE void fn_801EBA70();
    NO_INLINE void fn_801EBB30();
    NO_INLINE void fn_801EBCE0();
    NO_INLINE void fn_801EBD50();
    NO_INLINE void fn_801EBE00();
    void exeDown();
    NO_INLINE void fn_801EBF10();

    NO_INLINE bool fn_801EBF90() const;
    NO_INLINE bool fn_801EC000() const;
    NO_INLINE bool fn_801EC1D0() const;
    NO_INLINE bool fn_801EB4F0(HitSensor*, HitSensor*);
    NO_INLINE bool fn_801EB5A0();
    NO_INLINE void fn_801EC3B0(bool);
    bool updateScale(f32);
    NO_INLINE bool fn_801EC0D0() const;
    NO_INLINE void fn_801EC790();
    NO_INLINE void fn_801EC460();
    NO_INLINE bool fn_801EC150() const;
    NO_INLINE bool fn_801EC2A0() const;
    NO_INLINE void fn_801EC4F0();
    NO_INLINE bool fn_801EC5E0(TVec3f*);

    /* 0x90 */ AnimScaleController* mAnimScale;
    /* 0x94 */ WalkerStateBindStarPointer* mBindStar;
    /* 0x98 */ VolumeDrawer* mVolumeDrawer;
    /* 0x9C */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xA0 */ ActorStateParamScale* mStateParam;
    /* 0xA4 */ TMtx34f mMtx;
    /* 0xD4 */ TQuat4f mQuat;
    /* 0xE4 */ TVec3f mFront;
    /* 0xF0 */ TVec3f mJumpPos;
    /* 0xFC */ TVec3f mJumpStart;
    /* 0x108 */ f32 mScale;
    /* 0x10C */ TVec3f mRailPos;
    /* 0x118 */ TVec3f mWalkPos;
    /* 0x124 */ TVec3f mDustPos;
    /* 0x130 */ u8 mIsOnceFlag;
    /* 0x131 */ u8 mIsDustEmit;
};
