#pragma once

#include "Game/Gravity/PlanetGravity.hpp"

class BarrelGravity : public PlanetGravity {
public:
    BarrelGravity();

    virtual void updateMtx(const TPos3f& rMtx);
    virtual bool calcOwnGravityVector(TVec3f* pDest, f32* pScalar, const TVec3f& rPosition) const;

    void setLocalPosition(const TVec3f&);
    void setUpDirection(const TVec3f&);
    void setRadius(f32);
    void setUnusedArg0(f32);
    void setInwardsThreshold(f32);

    /* 0x28 */ TVec3f mLocalPosition;
    /* 0x34 */ TVec3f mPosition;
    /* 0x40 */ TVec3f mLocalUpDirection;
    /* 0x4C */ TVec3f mUpDirection;
    /* 0x58 */ f32 mRadius;
    /* 0x5C */ f32 mScaledRadius;
    /* 0x60 */ f32 mUnusedArg0;
    /* 0x64 */ f32 mInwardsThreshold;
};
