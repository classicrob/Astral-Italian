#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class StarPointerInput {
public:
    virtual const TVec2f& getScreenPosition() const = 0;
    virtual const TVec2f& getScreenVelocity() const = 0;
    virtual const TVec3f& getWorldVelocity() const = 0;
    virtual f32 getScreenSpeed() const = 0;
    virtual f32 getViewDistanceZ() const = 0;
    virtual s32 getPastPointCount() const = 0;
    virtual s32 getNextPastPointCount() const = 0;
    virtual const TVec2f& getPastPosition(s32) const = 0;
    virtual const TVec2f& getNextPastPosition(s32) const = 0;
    virtual const TVec3f& getWorldPosition() const = 0;
    virtual bool isInScreen() const = 0;
    virtual bool isOutScreenLong() const = 0;
};
