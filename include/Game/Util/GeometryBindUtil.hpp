#pragma once

#include <JSystem/JGeometry.hpp>
#include <revolution.h>

class BindSphere {
public:
    BindSphere(const TVec3f& rPosition, f32 radius);

    f32 getRadius() const {
        return mRadius;
    }

    /* 0x0 */ TVec3f mPosition;
    /* 0xC */ f32 mRadius;
};

class BindCone {
public:
    BindCone(const TVec3f& rPosition, const TVec3f& rDirection, f32 height, f32 radius);

    bool isInPoint(const TVec3f& rPoint) const;
    void calcVertexPosition();
    void calcGeneratrixLength();
    void setPosition(const TVec3f& rPosition);
    void setDirection(const TVec3f& rDirection);

    f32 getRadius() const {
        return mRadius;
    }

    f32 getGeneratrixLength() const {
        return mGeneratrixLength;
    }

    /* 0x00 */ TVec3f mPosition;
    /* 0x0C */ TVec3f mVertex;
    /* 0x18 */ TVec3f mDirection;
    /* 0x24 */ f32 mHeight;
    /* 0x28 */ f32 mRadius;
    /* 0x2C */ f32 mGeneratrixLength;
};

class BindResult {
public:
    BindResult() : mPosition(0, 0, 0), mNormal(0, 0, 0), mContactPosition(0, 0, 0), mIsBound() {
    }

    /* 0x00 */ TVec3f mPosition;
    /* 0x0C */ TVec3f mNormal;
    /* 0x18 */ TVec3f mContactPosition;
    /* 0x24 */ bool mIsBound;
};

/// Cubic Hermite spline through two points (SMG2-only; fn_80066D40 lives in the BindCone unit, named by address).
/// Used by BoomerangBrosBoomerang. Evaluated as P0*h00 + P1*h01 + M0*h10 + M1*h11 at parameter t.
class HermiteSpline {
public:
    HermiteSpline(const TVec3f& rP0, const TVec3f& rM0, const TVec3f& rP1, const TVec3f& rM1) {
        mP0 = rP0;
        mM0 = rM0;
        mP1 = rP1;
        mM1 = rM1;
    }
    virtual ~HermiteSpline() {}
    void fn_80066D40(TVec3f* pOut, f32 t) const;

    /* 0x04 */ TVec3f mP0;
    /* 0x10 */ TVec3f mP1;
    /* 0x1C */ TVec3f mM0;
    /* 0x28 */ TVec3f mM1;
};

namespace MR {
    bool bindSpereToCone(BindResult* pResult, const TVec3f& rMovement, const BindSphere& rSphere, const BindCone& rCone);
    void updateBindPositionAndVelocity(TVec3f* pPosition, TVec3f* pVelocity, const BindResult& rResult, f32 factor);
};  // namespace MR
