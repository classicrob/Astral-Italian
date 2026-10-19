#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class CameraPadOrbitCtrl {
public:
    CameraPadOrbitCtrl() NO_INLINE;
    void reset() NO_INLINE;
    void update(const TVec3f&, const TVec3f&, const TVec3f&, TVec3f*) NO_INLINE;
    void updateRotation(const TVec3f&, const TVec3f&, const TVec3f&, TVec3f*) NO_INLINE;
    void updateInput(const TVec3f&, const TVec3f&, const TVec3f&, TVec3f*) NO_INLINE;
    /* 0x00 */ bool mRotating;
    /* 0x04 */ f32 mAngle;
    /* 0x08 */ f32 mTargetAngle;
};
