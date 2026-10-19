#pragma once
#include "JSystem/JGeometry/TMatrix.hpp"
class HitInfo;
class HitSensor;

namespace DiceMoverUtil {
    void normalizeComponents(f32*, f32*, f32*);
    void crossComponents(f32*, f32*, f32*, f32, f32, f32, f32, f32, f32);
    f32 averageNormalComponents(f32*, f32*, f32*, f32*, f32*, f32*);
    void expandBounds(TVec3f*, TVec3f*, const TVec3f&);
}
struct DiceMoverPoint { TVec3f mPosition; f32 mRadius; };
struct DiceMoverPointArray { u32 mCount; DiceMoverPoint* mPoints; };
struct DiceMoverRotationBounds {
    TVec2f mMin;
    TVec2f mMax;
    void clear();
};
class DiceMoverAccumulator {
public:
    DiceMoverAccumulator();
    void clear();
    TVec3f getVelocity() const NO_INLINE;
    void addCollision(const TVec3f*, const TVec3f*, const TVec3f*, f32);
    void addRotation(const TPos3f*, const TVec3f*, const TVec3f*);
    TVec3f mPositionMin;
    TVec3f mPositionMax;
    TVec3f mVelocityMin;
    TVec3f mVelocityMax;
    DiceMoverRotationBounds mRotation[3];
};
struct DiceMoverPosition : TVec3f { DiceMoverPosition() NO_INLINE; };
class DiceMover {
public:
    DiceMover(long, long);
    virtual ~DiceMover();
    virtual void updateCollision();
    virtual void onMapCollision(const HitInfo*);
    virtual void onCollision(const TVec3f&, const TVec3f&, const TVec3f&);
    void addCollisionPoint(const TVec3f*);
    void setMtx(MtxPtr);
    void collide(const TVec3f*, const TVec3f*);
    void update();
    void calcMtx(TPos3f*);
    void calcRadius();
    void setGravity(const TVec3f*);
    void setPointArray(void*, long);
    void initCollisionPoints(long, long);
    void resolveCollision(const TVec3f*, const TVec3f*, f32);
    void init();
    bool checkMapCollision(const TVec3f&, HitInfo*);
    void integrate();
    void collideWithMap();
    void calcCollisionPoint(TVec3f*, long) NO_INLINE;
    void addGravity();
    void applyCollision();
    void checkVelocityChange(TVec3f);
    DiceMoverPosition mPosition;        // 0x04
    TPos3f mRotation;                    // 0x10
    TPos3f mPreviousTransform;           // 0x40
    TVec3f* mCollisionPoints;            // 0x70
    long mCollisionPointCapacity;
    long mCollisionPointCount;
    f32 mRadius;
    TVec3f mGravity;                     // 0x80
    f32 mRestitution;
    TVec3f mVelocity;                    // 0x90
    TVec2f mAngularVelocity[3];          // 0x9C
    f32 mMaxSpeed;
    bool _B8;
    f32 _BC, _C0, _C4, _C8, _CC, _D0;
    bool mCollided;                      // 0xD4
    bool mVelocityChanged;
    f32 mChangedSpeed;
    HitSensor* mExceptSensor;
    bool mCheckSweptCollision;
    DiceMoverAccumulator* mMapResponse;  // 0xE4
    DiceMoverAccumulator* mResponse;
    DiceMoverPointArray* mPointArray;
};
