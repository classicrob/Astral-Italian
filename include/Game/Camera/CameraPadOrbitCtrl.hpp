#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class CameraPadOrbitCtrl {
public:
    CameraPadOrbitCtrl() NO_INLINE;
    void reset() NO_INLINE;
    void update(const TVec3f&, const TVec3f&, const TVec3f&, TVec3f*) NO_INLINE;
    void updateRotation(const TVec3f&, const TVec3f&, const TVec3f&, TVec3f*) NO_INLINE;
    void updateInput(const TVec3f&, const TVec3f&, const TVec3f&, TVec3f*) NO_INLINE;
    bool mRotating;
    f32 mAngle;
    f32 mTargetAngle;
};
