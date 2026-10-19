#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include "Game/Player/YoshiLockOnTarget.hpp"

class LiveActor;
class HitSensor;
class YoshiLockOnManager;

class YoshiTongueTarget {
public:
    YoshiTongueTarget(YoshiLockOnManager*);
    void takeLockOnTargets();
    bool sendStartTongueMessage();
    void sendStartEatenMessage();
    void advanceTargetIfMore();
    void cancelTargetAndAdvance();
    bool hasNextTarget() const;
    bool hasCurrentTarget() const;
    bool hasMultipleRemainingTargets() const;
    bool fn_8041C120() const;
    bool fn_8041C140() const;
    bool fn_8041C170() const;
    bool fn_8041C1B0() const;
    bool fn_8041C1E0() const;
    bool fn_8041C210() const;
    void fn_8041C440(TVec3f*) const;
    void sendReleaseEatenMessage();
    bool sendLoopTongueMessage();
    void calcCurrentTargetPosition(TVec3f*) const;
    LiveActor* getCurrentTargetActor() const;
    MtxPtr getCurrentTargetMatrix() const;
    bool sendMessageToCurrentTarget(u32);
    u32 mCapacity;
    u32 mCount;
    u32 mIndex;
    YoshiLockOnTargetDesc* mTargets;
    YoshiLockOnManager* mManager;
    HitSensor* mSensor;
};
