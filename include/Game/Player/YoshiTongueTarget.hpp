#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class LiveActor;
class HitSensor;
class YoshiLockOnManager;

class YoshiTongueTarget {
public:
    YoshiTongueTarget(YoshiLockOnManager*);
    void fn_8041BFD0();
    bool fn_8041C0E0() const;
    void fn_8041C050();
    void fn_8041C240(TVec3f*) const;
    LiveActor* fn_8041C4A0() const;
    u8 _0[0x14];
    HitSensor* mSensor;
};
