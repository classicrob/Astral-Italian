#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "Game/Player/YoshiLockOnTarget.hpp"

class LiveActor;
class Yoshi;
class YoshiTargetMark;

class YoshiLockOnManager : public NameObj {
public:
    YoshiLockOnManager(Yoshi* yoshi);
    virtual ~YoshiLockOnManager();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual void draw() const;

    void loadLockOnParams(Yoshi* yoshi);
    bool tryLockOnEatTarget(YoshiLockOnTarget* target);
    bool tryLockOnLickTarget(YoshiLockOnTarget* target);
    bool tryLockOnCaptureTarget(YoshiLockOnTarget* target, bool alternateCapture);
    bool tryLockOnPullTarget(YoshiLockOnTarget* target);
    bool tryLockOnProjectileTarget(YoshiLockOnTarget* target);
    static u32 getTargetCapacity();
    // Copies active descriptors, then moves those controls to fading. The capacity argument is unused.
    u32 takeActiveTargets(YoshiLockOnTargetDesc* output, u32 capacity);
    void clearActiveTargets();

    bool tryLockOnMultipleTargets(YoshiLockOnTarget* target, s32 targetType, s32 selectionMode, u32 maxActiveTargets, f32 maxDistance);
    bool tryLockOnSingleTarget(YoshiLockOnTarget* target, s32 targetType, f32 maxDistance);
    bool tryActivateTarget(YoshiLockOnTarget* target, s32 framesUntilExpiry, s32 targetType);
    YoshiLockOnTargetControl* findActiveTarget(YoshiLockOnTarget* target);
    YoshiTargetMark* findUnusedMark();
    void updateFadingTargets();
    void updateActiveTargets();
    bool tryRefreshActiveTarget(YoshiLockOnTarget* target);
    bool tryReactivateFadingTarget(YoshiLockOnTarget* target);
    void startLockOn(LiveActor* contextActor);
    void endLockOn();

    /* 0x14 */ u32 mSelectionMode;  // 0: single-target offers; 1: Eat/Lick multiple-target offers.
    /* 0x18 */ LiveActor* mContextActor;
    /* 0x1C */ bool mContextActive;
    /* 0x1D */ bool mSuspended;
    /* 0x1E */ bool mHasUpdated;
    /* 0x20 */ MR::BothDirList<YoshiLockOnTargetControl> mFree;
    /* 0x2C */ MR::BothDirList<YoshiLockOnTargetControl> mFading;
    /* 0x38 */ MR::BothDirList<YoshiLockOnTargetControl> mActive;
    /* 0x44 */ YoshiTargetMark** mMarks;
    /* 0x48 */ s32 mMarkCount;
    /* 0x4C */ bool mSoundLatch;
    /* 0x4D */ u8 _4D[3];
};
