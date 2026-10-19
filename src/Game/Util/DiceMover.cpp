#pragma auto_inline off
#include "Game/Util/DiceMover.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"

template <>
template <>
void JGeometry::TVec3<f32>::setAll<f32>(f32 value) {
    x = value;
    y = value;
    z = value;
}

void DiceMoverUtil::normalizeComponents(f32* x, f32* y, f32* z) {
    f32 inverseLength = 1.0f / MR::sqrt<f32>((*x * *x + *y * *y) + *z * *z);
    *x *= inverseLength;
    *y *= inverseLength;
    *z *= inverseLength;
}

void DiceMoverUtil::crossComponents(f32* x, f32* y, f32* z, f32 ax, f32 ay, f32 az, f32 bx, f32 by, f32 bz) {
    *x = ay * bz - az * by;
    *y = az * bx - ax * bz;
    *z = ax * by - ay * bx;
}

f32 DiceMoverUtil::averageNormalComponents(f32* x, f32* y, f32* z, f32* nx, f32* ny, f32* nz) {
    f32 length = MR::sqrt<f32>((*nx * *nx + *ny * *ny) + *nz * *nz);
    f32 inverseLength = 1.0f / length;
    *x = 0.5f * (*x + inverseLength * *nx);
    *y = 0.5f * (*y + inverseLength * *ny);
    *z = 0.5f * (*z + inverseLength * *nz);
    return length;
}

namespace {
    typedef char DiceMoverSizeCheck[sizeof(DiceMover) == 0xF0 ? 1 : -1];
    typedef char DiceMoverAccumulatorSizeCheck[sizeof(DiceMoverAccumulator) == 0x60 ? 1 : -1];
    typedef char DiceMoverRotationBoundsSizeCheck[sizeof(DiceMoverRotationBounds) == 0x10 ? 1 : -1];
}

DiceMover::DiceMover(long count, long points) {
    init();
    initCollisionPoints(count, points);
}

DiceMoverPosition::DiceMoverPosition() {}

void DiceMover::addCollisionPoint(const TVec3f* point) {
    mCollisionPoints[mCollisionPointCount++].set(*point);
    f32 radius = PSVECMag(point);
    if (radius > mRadius) mRadius = radius;
}

void DiceMover::setMtx(MtxPtr mtx) {
    mPosition.set<f32>(mtx[0][3], mtx[1][3], mtx[2][3]);
    mRotation.set(mtx);
    mRotation.mMtx[0][3] = 0.0f;
    mRotation.mMtx[1][3] = 0.0f;
    mRotation.mMtx[2][3] = 0.0f;
    mVelocity.zero();
    mAngularVelocity[0].x = mAngularVelocity[0].y = 0.0f;
    mAngularVelocity[1].x = mAngularVelocity[1].y = 0.0f;
    mAngularVelocity[2].x = mAngularVelocity[2].y = 0.0f;
}
void DiceMover::collide(const TVec3f* point, const TVec3f* force) {
    mResponse->addCollision(&mPosition, point, force, mRadius);
}
void DiceMover::update() {
    TVec3f oldVelocity(mVelocity);
    mCollided = false;
    for (s32 i = 0; i < 4; ++i) {
        integrate();
        collideWithMap();
        updateCollision();
        addGravity();
        applyCollision();
    }
    checkVelocityChange(oldVelocity);
}
void DiceMover::updateCollision() {}
// NONMATCHING: regalloc
void DiceMover::calcMtx(TPos3f* out) {
    for (s32 i = 0; i < 3; ++i) {
        out->mMtx[i][0] = mRotation.mMtx[i][0];
        out->mMtx[i][1] = mRotation.mMtx[i][1];
        out->mMtx[i][2] = mRotation.mMtx[i][2];
    }
    f32 z = mPosition.z;
    f32 y = mPosition.y;
    f32 x = mPosition.x;
    out->mMtx[0][3] = x;
    out->mMtx[1][3] = y;
    out->mMtx[2][3] = z;
}
void DiceMover::calcRadius() {
    mRadius = 0.0f;
    for (s32 i = 0; i < mCollisionPointCount; ++i) {
        f32 radius = PSVECMag(&mCollisionPoints[i]);
        if (radius > mRadius) mRadius = radius;
    }
}
void DiceMover::setGravity(const TVec3f* gravity) { mGravity.set(*gravity); }
void DiceMover::setPointArray(void* data, long calc) {
    DiceMoverPointArray* points = static_cast<DiceMoverPointArray*>(data);
    mPointArray = points;
    if (calc) {
        for (u32 i = 0; i < points->mCount; ++i) {
            f32 radius = PSVECMag(&points->mPoints[i].mPosition);
            radius += points->mPoints[i].mRadius;
            if (mRadius < radius) mRadius = radius;
        }
    }
}
// NONMATCHING: regalloc
void DiceMover::initCollisionPoints(long count, long data) {
    if (!count) {
        mCollisionPoints = 0;
        mCollisionPointCapacity = 0;
        mCollisionPointCount = 0;
        mRadius = 0.0f;
        return;
    }
    mCollisionPoints = new TVec3f[count];
    mCollisionPointCapacity = count;
    mCollisionPointCount = 0;
    mRadius = 0.0f;
    if (data) {
        TVec3f* points = reinterpret_cast<TVec3f*>(data);
        for (s32 i = 0; i < count; ++i) {
            TVec3f* dst = &mCollisionPoints[i];
            dst->x = points[i].x;
            dst->y = points[i].y;
            dst->z = points[i].z;
        }
        mCollisionPointCount = count;
        calcRadius();
    }
}
void DiceMover::onMapCollision(const HitInfo*) {}
void DiceMover::onCollision(const TVec3f&, const TVec3f&, const TVec3f&) {}
void DiceMover::resolveCollision(const TVec3f* point, const TVec3f* normal, f32 depth) {
    TVec3f movement(*point);
    mPreviousTransform.mult(movement, movement);
    movement.sub(*point);
    f32 normalSpeed = movement.dot(*normal);
    TVec3f normalMove(*normal);
    normalMove.scale(normalSpeed);
    TVec3f tangent(movement);
    tangent.sub(normalMove);
    f32 ratio;
    if (normalSpeed < depth) {
        normalSpeed = depth;
        ratio = -1.0f;
        normalMove = *normal * depth;
    } else ratio = depth / normalSpeed;
    f32 friction = 0.999f;
    if (normalSpeed < 0.03f) friction *= normalSpeed / 0.03f;
    TVec3f response(tangent * friction + normalMove * mRestitution);
    if (ratio < 0.0f) {
        DiceMoverUtil::expandBounds(&mMapResponse->mPositionMin, &mMapResponse->mPositionMax, normalMove);
        onCollision(-normalMove, TVec3f(0, 0, 0), TVec3f(0, 0, 0));
        return;
    }
    mMapResponse->addCollision(&mPosition, point, &response, mRadius);
    TVec3f correction(((tangent + normalMove) * ratio) * 1.0f);
    DiceMoverUtil::expandBounds(&mMapResponse->mPositionMin, &mMapResponse->mPositionMax, correction);
    onCollision(-correction, *point, -response);
}
void DiceMover::init() {
    mCollisionPoints = 0;
    mPosition.x = 0.0f; mPosition.y = 0.0f; mPosition.z = 0.0f;
    mRotation.identity();
    mPreviousTransform.identity();
    mGravity.x = 0.0f; mGravity.y = -0.2f; mGravity.z = 0.0f;
    mRestitution = 2.0f;
    mVelocity.zero();
    mAngularVelocity[0].x = mAngularVelocity[0].y = 0.0f;
    mAngularVelocity[1].x = mAngularVelocity[1].y = 0.0f;
    mAngularVelocity[2].x = mAngularVelocity[2].y = 0.0f;
    mMaxSpeed = -1.0f;
    _B8 = false;
    _BC = 0.0f; _C0 = 0.0f; _C4 = 0.0f; _C8 = 0.0f; _CC = 0.0f; _D0 = 0.0f;
    mCollided = false;
    mVelocityChanged = false;
    mChangedSpeed = 0.0f;
    mExceptSensor = 0;
    mCheckSweptCollision = false;
    mMapResponse = new DiceMoverAccumulator;
    mResponse = new DiceMoverAccumulator;
    mPointArray = 0;
}
bool DiceMover::checkMapCollision(const TVec3f& point, HitInfo* info) {
    bool hit;
    if (mExceptSensor) hit = MR::checkStrikePointToMapExceptSensor(point, mExceptSensor, info);
    else hit = MR::checkStrikePointToMap(point, info);
    if (hit) return true;
    if (mCheckSweptCollision) {
        TVec3f previous(point);
        mPreviousTransform.mult(previous, previous);
        const HitSensor* except = mExceptSensor;
        TVec3f delta(point);
        delta.sub(previous);
        if (except) hit = MR::getFirstPolyOnLineToMapExceptSensor(&info->mHitPos, &info->mParentTriangle, previous, delta, except);
        else hit = MR::getFirstPolyOnLineToMap(&info->mHitPos, &info->mParentTriangle, previous, delta);
        if (hit) {
            info->mQueryPos = previous + delta;
            TVec3f penetration(info->mHitPos);
            penetration.sub(info->mQueryPos);
            info->mDistance = info->mParentTriangle.getNormal(0)->dot(penetration);
        }
        if (hit) return true;
    }
    return false;
}
// NONMATCHING: regalloc
void DiceMover::integrate() {
    TVec3f previousPosition(mPosition);
    TPos3f oldRotation = mRotation;
    mPosition.add(mVelocity);
    f32 a = mAngularVelocity[0].x, b = mAngularVelocity[0].y;
    f32 c = mAngularVelocity[1].x, d = mAngularVelocity[1].y;
    f32 e = mAngularVelocity[2].x, f = mAngularVelocity[2].y;
    mRotation.mMtx[0][0] += a * oldRotation.mMtx[0][1] + b * oldRotation.mMtx[0][2];
    mRotation.mMtx[1][0] += a * oldRotation.mMtx[1][1] + b * oldRotation.mMtx[1][2];
    mRotation.mMtx[2][0] += a * oldRotation.mMtx[2][1] + b * oldRotation.mMtx[2][2];
    mRotation.mMtx[0][1] += c * oldRotation.mMtx[0][2] + d * oldRotation.mMtx[0][0];
    mRotation.mMtx[1][1] += c * oldRotation.mMtx[1][2] + d * oldRotation.mMtx[1][0];
    mRotation.mMtx[2][1] += c * oldRotation.mMtx[2][2] + d * oldRotation.mMtx[2][0];
    mRotation.mMtx[0][2] += e * oldRotation.mMtx[0][0] + f * oldRotation.mMtx[0][1];
    mRotation.mMtx[1][2] += e * oldRotation.mMtx[1][0] + f * oldRotation.mMtx[1][1];
    mRotation.mMtx[2][2] += e * oldRotation.mMtx[2][0] + f * oldRotation.mMtx[2][1];
    f32 x[3], y[3], z[3];
    f32 sum;
    do {
        DiceMoverUtil::normalizeComponents(&mRotation.mMtx[0][0], &mRotation.mMtx[1][0], &mRotation.mMtx[2][0]);
        DiceMoverUtil::normalizeComponents(&mRotation.mMtx[0][1], &mRotation.mMtx[1][1], &mRotation.mMtx[2][1]);
        DiceMoverUtil::normalizeComponents(&mRotation.mMtx[0][2], &mRotation.mMtx[1][2], &mRotation.mMtx[2][2]);
        DiceMoverUtil::crossComponents(&x[0], &x[1], &x[2], mRotation.mMtx[0][1], mRotation.mMtx[1][1], mRotation.mMtx[2][1], mRotation.mMtx[0][2], mRotation.mMtx[1][2], mRotation.mMtx[2][2]);
        DiceMoverUtil::crossComponents(&y[0], &y[1], &y[2], mRotation.mMtx[0][2], mRotation.mMtx[1][2], mRotation.mMtx[2][2], mRotation.mMtx[0][0], mRotation.mMtx[1][0], mRotation.mMtx[2][0]);
        DiceMoverUtil::crossComponents(&z[0], &z[1], &z[2], mRotation.mMtx[0][0], mRotation.mMtx[1][0], mRotation.mMtx[2][0], mRotation.mMtx[0][1], mRotation.mMtx[1][1], mRotation.mMtx[2][1]);
        sum = DiceMoverUtil::averageNormalComponents(&mRotation.mMtx[0][0], &mRotation.mMtx[1][0], &mRotation.mMtx[2][0], &x[0], &x[1], &x[2]);
        sum += DiceMoverUtil::averageNormalComponents(&mRotation.mMtx[0][1], &mRotation.mMtx[1][1], &mRotation.mMtx[2][1], &y[0], &y[1], &y[2]);
        sum += DiceMoverUtil::averageNormalComponents(&mRotation.mMtx[0][2], &mRotation.mMtx[1][2], &mRotation.mMtx[2][2], &z[0], &z[1], &z[2]);
    } while (sum < 2.999f);
    TPos3f current = mRotation;
    current.setTrans(mPosition);
    current.invert(current);
    TPos3f previous = oldRotation;
    previous.setTrans(previousPosition);
    mPreviousTransform.concat(previous, current);
    mMapResponse->clear();
}
void DiceMover::collideWithMap() {
    HitInfo info;
    TVec3f point;
    for (s32 i = 0; i < mCollisionPointCount; ++i) {
        calcCollisionPoint(&point, i);
        if (checkMapCollision(point, &info)) {
            onMapCollision(&info);
            resolveCollision(&point, info.mParentTriangle.getNormal(0), info.mDistance);
            mCollided = true;
        }
    }
    if (mPointArray) {
        for (u32 i = 0; i < mPointArray->mCount; ++i) {
            TVec3f center(mPointArray->mPoints[i].mPosition);
            mRotation.mult33(center);
            center.add(mPosition);
            if (MR::checkStrikeBallToMap(center, mPointArray->mPoints[i].mRadius)) {
                for (u32 j = 0; j < Collision::getStrikeInfoNumMap(); ++j) {
                    const HitInfo* hit = Collision::getStrikeInfoMap(j);
                    TVec3f surface(hit->mHitPos - *hit->mParentTriangle.getNormal(0) * hit->mDistance);
                    resolveCollision(&surface, hit->mParentTriangle.getNormal(0), hit->mDistance);
                    mCollided = true;
                }
            }
        }
    }
}
void DiceMover::calcCollisionPoint(TVec3f* out, long index) {
    out->set(mCollisionPoints[index]);
    mRotation.mult33(*out);
    out->add(mPosition);
}
void DiceMover::addGravity() {
    TVec3f force(mGravity);
    force.scale(0.25f);
    DiceMoverUtil::expandBounds(&mMapResponse->mVelocityMin, &mMapResponse->mVelocityMax, force);
}
// NONMATCHING: regalloc
void DiceMover::applyCollision() {
    mPosition.add((mMapResponse->mPositionMin + mMapResponse->mPositionMax) + (mResponse->mPositionMin + mResponse->mPositionMax));
    mVelocity = (mMapResponse->getVelocity() + mResponse->getVelocity()) + mVelocity * 0.999f;
    if (mMaxSpeed > 0.0f) {
        f32 maxSpeed = mMaxSpeed;
        if (mVelocity.squared() > maxSpeed * maxSpeed) {
            f32 squared = mVelocity.squared();
            if (!(squared <= 0.000003814697265625f)) mVelocity.scale(JGeometry::TUtil<f32>::inv_sqrt(squared) * maxSpeed);
        }
    }
    mAngularVelocity[0] = ((mMapResponse->mRotation[0].mMin + mMapResponse->mRotation[0].mMax) + (mResponse->mRotation[0].mMin + mResponse->mRotation[0].mMax)) + mAngularVelocity[0] * 0.999f;
    mAngularVelocity[1] = ((mMapResponse->mRotation[1].mMin + mMapResponse->mRotation[1].mMax) + (mResponse->mRotation[1].mMin + mResponse->mRotation[1].mMax)) + mAngularVelocity[1] * 0.999f;
    mAngularVelocity[2] = ((mMapResponse->mRotation[2].mMin + mMapResponse->mRotation[2].mMax) + (mResponse->mRotation[2].mMin + mResponse->mRotation[2].mMax)) + mAngularVelocity[2] * 0.999f;
    mResponse->clear();
}
TVec3f DiceMoverAccumulator::getVelocity() const { return mVelocityMin + mVelocityMax; }
// NONMATCHING: regalloc
void DiceMover::checkVelocityChange(TVec3f previous) {
    f32 oldSpeed, newSpeed;
    TVec3f oldDirection, newDirection, difference;
    MR::separateScalarAndDirection(&oldSpeed, &oldDirection, previous);
    MR::separateScalarAndDirection(&newSpeed, &newDirection, mVelocity);
    difference = -oldDirection + newDirection;
    if (!(difference.squared() <= 0.000003814697265625f)) MR::normalize(&difference);
    f32 dot = newDirection.dot(difference);
    if (oldDirection.dot(difference) * dot < -0.1) {
        mVelocityChanged = true;
        mChangedSpeed = newSpeed;
    } else {
        mVelocityChanged = false;
        mChangedSpeed = 0.0f;
    }
}
void DiceMoverUtil::expandBounds(TVec3f* min, TVec3f* max, const TVec3f& value) {
    if (value.x < min->x) min->x = value.x; else if (value.x > max->x) max->x = value.x;
    if (value.y < min->y) min->y = value.y; else if (value.y > max->y) max->y = value.y;
    if (value.z < min->z) min->z = value.z; else if (value.z > max->z) max->z = value.z;
}
DiceMoverAccumulator::DiceMoverAccumulator() { clear(); }
void DiceMoverAccumulator::clear() {
    mPositionMin.zero(); mPositionMax.zero();
    mVelocityMin.zero(); mVelocityMax.zero();
    mRotation[0].clear(); mRotation[1].clear(); mRotation[2].clear();
}
void DiceMoverRotationBounds::clear() {
    mMin.x = mMin.y = 0.0f;
    mMax.x = mMax.y = 0.0f;
}
// NONMATCHING: regalloc
void DiceMoverAccumulator::addCollision(const TVec3f* center, const TVec3f* point, const TVec3f* force, f32 radius) {
    f32 inverseRadius = 1.0f / radius;
    f32 magnitude = PSVECMag(force);
    if (0.0f != magnitude) {
        TVec3f direction(*force);
        direction.scale(1.0f / magnitude);
        TVec3f offset(*point);
        offset.sub(*center);
        f32 squared = offset.squared();
        f32 projection = direction.dot(offset);
        f32 distance = MR::sqrt<f32>(squared - projection * projection);
        if (0.0f == distance) {
            DiceMoverUtil::expandBounds(&mVelocityMin, &mVelocityMax, *force);
            return;
        }
        f32 ratio = 1.0f - distance * inverseRadius;
        if (ratio > 1.0f) ratio = 1.0f;
        if (ratio < 0.0f) ratio = 0.01f;
        if (ratio > 0.0f) {
            TVec3f linear(*force);
            linear.scale(ratio * ratio);
            DiceMoverUtil::expandBounds(&mVelocityMin, &mVelocityMax, linear);
        }
        f32 rotationScale = 0.999f * inverseRadius;
        f32 inverseLength = 1.0f / MR::sqrt<f32>(squared);
        TVec3f radial(offset);
        radial.scale(inverseLength);
        f32 radialForce = radial.dot(*force);
        TVec3f tangent(*force - radial * radialForce);
        tangent.scale(rotationScale);
        addRotation(reinterpret_cast<const TPos3f*>(center), &radial, &tangent);
    }
}
// NONMATCHING: regalloc
void DiceMoverAccumulator::addRotation(const TPos3f* frame, const TVec3f* direction, const TVec3f* force) {
    TPos3f rotation;
    rotation.identity();
    TVec3f target(*direction + *force);
    DiceMoverUtil::normalizeComponents(&target.x, &target.y, &target.z);
    TQuat4f quaternion;
    quaternion.setRotate(*direction, target);
    rotation.setQuat(quaternion);
    TVec3f x, y, z;
    const f32* data = reinterpret_cast<const f32*>(frame);
    x.set<f32>(data[3], data[7], data[11]);
    y.set<f32>(data[4], data[8], data[12]);
    z.set<f32>(data[5], data[9], data[13]);
    TVec3f rx(x); rotation.mult(rx, rx);
    TVec3f dx(rx); dx.sub(x); dx.scale(1.0f / (1.0f + dx.dot(x)));
    f32 value = dx.dot(y);
    if (value < mRotation[0].mMin.x) mRotation[0].mMin.x = value; else if (value > mRotation[0].mMax.x) mRotation[0].mMax.x = value;
    value = dx.dot(z);
    if (value < mRotation[0].mMin.y) mRotation[0].mMin.y = value; else if (value > mRotation[0].mMax.y) mRotation[0].mMax.y = value;
    TVec3f ry(y); rotation.mult(ry, ry);
    TVec3f dy(ry); dy.sub(y); dy.scale(1.0f / (1.0f + dy.dot(y)));
    value = dy.dot(z);
    if (value < mRotation[1].mMin.x) mRotation[1].mMin.x = value; else if (value > mRotation[1].mMax.x) mRotation[1].mMax.x = value;
    value = dy.dot(x);
    if (value < mRotation[1].mMin.y) mRotation[1].mMin.y = value; else if (value > mRotation[1].mMax.y) mRotation[1].mMax.y = value;
    TVec3f rz(z); rotation.mult(rz, rz);
    TVec3f dz(rz); dz.sub(z); dz.scale(1.0f / (1.0f + dz.dot(z)));
    value = dz.dot(x);
    if (value < mRotation[2].mMin.x) mRotation[2].mMin.x = value; else if (value > mRotation[2].mMax.x) mRotation[2].mMax.x = value;
    value = dz.dot(y);
    if (value < mRotation[2].mMin.y) mRotation[2].mMin.y = value; else if (value > mRotation[2].mMax.y) mRotation[2].mMax.y = value;
}
DiceMover::~DiceMover() {}
