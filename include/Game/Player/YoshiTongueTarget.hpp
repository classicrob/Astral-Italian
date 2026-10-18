#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include "Game/Player/YoshiLockOnTarget.hpp"

class LiveActor;
class HitSensor;
class YoshiLockOnManager;

class YoshiTongueTarget {
public:
    YoshiTongueTarget(YoshiLockOnManager*);
    void fn_8041BFD0();
    bool fn_8041C020();
    void fn_8041C040();
    void fn_8041C060();
    void fn_8041C080();
    bool fn_8041C100() const;
    bool fn_8041C0E0() const;
    bool fn_8041C0C0() const;
    bool fn_8041C120() const;
    bool fn_8041C140() const;
    bool fn_8041C170() const;
    bool fn_8041C1B0() const;
    bool fn_8041C1E0() const;
    bool fn_8041C210() const;
    void fn_8041C440(TVec3f*) const;
    void fn_8041C050();
    bool fn_8041C030();
    void fn_8041C240(TVec3f*) const;
    LiveActor* fn_8041C4A0() const;
    MtxPtr fn_8041C4C0() const;
    bool fn_8041C4E0(u32);
    u32 mCapacity;
    u32 mCount;
    u32 mIndex;
    YoshiLockOnTargetDesc* mTargets;
    YoshiLockOnManager* mManager;
    HitSensor* mSensor;
};
