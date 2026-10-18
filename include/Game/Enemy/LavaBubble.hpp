#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

// SMG2-only enemy: the lava bubble that rises out of the magma and falls back (Wait -> Launch nerves).
// LiveActor = 0x90 bytes; members start there. Offsets are the SMG2 binary offsets.
class LavaBubble : public LiveActor {
public:
    LavaBubble(const char*);

    virtual ~LavaBubble();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual void control();
    virtual void startClipped();

    void exeWait();
    void exeLaunch();
    void initPose();
    void resetPose();
    // SMG2-only helpers (original names unknown; named by address)
    void fn_801BF0A0();
    void fn_801BF1C0();
    bool fn_801BF290();
    TVec3f getFallDir() const;

public:
    /* 0x90 */ TPos3f mBaseMtx;
    /* 0xC0 */ TPos3f mEffectMtx;  // translation column is the last hit point (control)
    /* 0xF0 */ TVec3f mHomePos;
    /* 0xFC */ TVec3f mUpVec;
    /* 0x108 */ TVec3f mPrevGravity;
    /* 0x114 */ TVec3f mUp;
    /* 0x120 */ TVec3f mUnk120;  // zeroed in the ctor, otherwise unused so far
    /* 0x12C */ TVec3f mSavedVel;
    /* 0x138 */ f32 mRange;  // JMap arg0, default 1000
    /* 0x13C */ f32 mSpeed;  // default 1.0
    /* 0x140 */ u8 mUseSavedGravity;  // default 1, cleared when JMap arg1 is given
    /* 0x141 */ u8 mUseFlippedGravity;  // default 0, set when JMap arg2 is given
    /* 0x144 */ s32 mLaunchDuration;  // JMap arg3, default 100
    /* 0x148 */ s32 mBounceCooldown;
    /* 0x14C */ s32 mLaunchDelay;  // JMap arg4, default 90
    /* 0x150 */ s32 mLaunchCount;
};
