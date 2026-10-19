#define JGEOMETRY_VEC3_COPY_NOINLINE
#include "Game/Map/CollisionParts.hpp"
#include "Game/Camera/CameraPolygonCodeUtil.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/CollisionCategorizedKeeper.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Map/KCollision.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/TriangleFilter.hpp"

CollisionParts::CollisionParts()
    : mFollowMtx(), mHitSensor(), mIsRegistered(), mIsUpdateEnabled(true), mForceUpdateOnce(), mUseAutoScale(), mIgnoreScale(), mStationaryFrames(), mBoundingSphereRadius(-1.0f), mEqualScale(1.0f), mKeeperIndex(-1), mZone() {
    mServer = new KCollisionServer();

    mPrevBaseMatrix.identity();
    mBaseMatrix.identity();
    mMatrix.identity();
    PSMTXInverse(mBaseMatrix.toMtxPtr(), mInvBaseMatrix.toMtxPtr());
}

void CollisionParts::resetBelongZone() {
    CollisionCategorizedKeeper* keeper = MR::getCollisionDirector()->getCategoryKeeper(mKeeperIndex);
    if (mZone->contains(this)) removeFromBelongZone();
    mZone = keeper->getZone(0);
    if (mIsRegistered) addToBelongZone();
}

#ifdef GALAXY_NATIVE
void CollisionParts::init(const TPos3f& rMatrix, HitSensor* pHitSensor, const void* pKclData, const void* pMapInfo, s32 keeperIndex,
                          bool useAutoScale) {
    mServer->init(const_cast< void* >(pKclData), pMapInfo);
    initAfterServerData(rMatrix, pHitSensor, keeperIndex, useAutoScale);
}

void CollisionParts::init(const TPos3f& rMatrix, HitSensor* pHitSensor, KCLFile* pFile, const void* pMapInfo, s32 keeperIndex, bool useAutoScale) {
    mServer->initWithFile(pFile, pMapInfo);
    initAfterServerData(rMatrix, pHitSensor, keeperIndex, useAutoScale);
}

void CollisionParts::initAfterServerData(const TPos3f& rMatrix, HitSensor* pHitSensor, s32 keeperIndex, bool useAutoScale) {
    mHitSensor = pHitSensor;
#else
void CollisionParts::init(const TPos3f& rMatrix, HitSensor* pHitSensor, const void* pKclData, const void* pMapInfo, s32 keeperIndex,
                          bool useAutoScale) {
    mServer->init(const_cast< void* >(pKclData), pMapInfo);
    mHitSensor = pHitSensor;
#endif

    resetAllMtx(rMatrix);

    TVec3f scale;
    mBaseMatrix.getScale(scale);

    mZone = MR::getCollisionDirector()->getCategoryKeeper(keeperIndex)->getZone(MR::getCurrentPlacementZoneId());

    s32 zoneId = mZone->mZoneID;
    MR::initCameraCodeCollection(MR::getSensorHost(pHitSensor)->mName, zoneId);
    mServer->calcFarthestVertexDistance();
    MR::termCameraCodeCollection();

    updateBoundingSphereRange(scale);
    mKeeperIndex = keeperIndex;
}

void CollisionParts::addToBelongZone() {
    s32 zoneID = mZone->mZoneID;

    MR::getCollisionDirector()->getCategoryKeeper(mKeeperIndex)->addToZone(this, zoneID);
}

void CollisionParts::removeFromBelongZone() {
    s32 zoneID = mZone->mZoneID;

    MR::getCollisionDirector()->getCategoryKeeper(mKeeperIndex)->removeFromZone(this, zoneID);
}

void CollisionParts::initWithAutoEqualScale(const TPos3f& rMatrix, HitSensor* pHitSensor, const void* pKclData, const void* pMapInfo, s32 keeperIndex,
                                            bool useAutoScale) {
    mUseAutoScale = true;
    mIgnoreScale = false;

    init(rMatrix, pHitSensor, pKclData, pMapInfo, keeperIndex, useAutoScale);
}

void CollisionParts::initWithNotUsingScale(const TPos3f& rMatrix, HitSensor* pHitSensor, const void* pKclData, const void* pMapInfo, s32 keeperIndex,
                                           bool useAutoScale) {
    mUseAutoScale = false;
    mIgnoreScale = true;

    init(rMatrix, pHitSensor, pKclData, pMapInfo, keeperIndex, useAutoScale);
}

void CollisionParts::resetAllMtx(const TPos3f& rMatrix) {
    bool reset = false;

    if (mIsUpdateEnabled || mForceUpdateOnce) {
        reset = true;
    }

    if (!reset) {
        return;
    }

    resetAllMtxPrivate(rMatrix);
}

void CollisionParts::resetAllMtx() {
    bool reset = false;

    if (mIsUpdateEnabled || mForceUpdateOnce) {
        reset = true;
    }

    if (reset) {
        TPos3f matrix(mFollowMtx);
        makeEqualScale(matrix.toMtxPtr());

        resetAllMtxPrivate(matrix);
    }
}


void CollisionParts::resetAllMtxPrivate(const TPos3f& rMatrix) {
    mPrevBaseMatrix.setInline(rMatrix);
    mBaseMatrix.setInline(rMatrix);
    mMatrix.setInline(rMatrix);
    PSMTXInverse(mBaseMatrix.toMtxPtr(), mInvBaseMatrix.toMtxPtr());
}

void CollisionParts::setMtx(const TPos3f& rMatrix) {
    mMatrix.setInline(rMatrix);
}

void CollisionParts::setMtx() {
    mMatrix.setInline(mFollowMtx);
}

void CollisionParts::updateMtx() {
    bool update = false;

    if (mIsUpdateEnabled || mForceUpdateOnce) {
        update = true;
    }

    if (!update) {
        if (MR::isSameMtx(mMatrix.toMtxPtr(), mBaseMatrix.toMtxPtr())) {
            mStationaryFrames++;
        }
    } else {
        if (MR::isSameMtx(mMatrix.toMtxPtr(), mBaseMatrix.toMtxPtr())) {
            mStationaryFrames++;
        } else {
            if (mForceUpdateOnce) {
                mStationaryFrames = 1;
            } else {
                mStationaryFrames = 0;
            }

            f32 scale = makeEqualScale(mMatrix.toMtxPtr());
            mScaleX = scale;
            f32 scaleChange = scale - mEqualScale;
            mScaleY = scale;
            mScaleZ = scale;

            if (!MR::isNearZero(scaleChange)) {
                updateBoundingSphereRangePrivate(scale);
            }
        }

        mForceUpdateOnce = false;

        if (mStationaryFrames < 2) {
            mPrevBaseMatrix.setInline(mBaseMatrix);
            mBaseMatrix.setInline(mMatrix);
            PSMTXInverse(mBaseMatrix.toMtxPtr(), mInvBaseMatrix.toMtxPtr());
        }
    }
}

f32 CollisionParts::makeEqualScale(MtxPtr pMatrix) {
    TPos3f& mtx = *reinterpret_cast< TPos3f* >(pMatrix);

    TVec3f scale;
    mtx.getScale(scale);

    TVec3f scaleDiff(scale.x - scale.y, scale.y - scale.z, scale.z - scale.x);

    if (MR::isNearZero(scaleDiff.x) && MR::isNearZero(scaleDiff.y) && MR::isNearZero(scaleDiff.z)) {
        return scale.x;
    }

    f32 uniformScale = 1.0f;
    TVec3f invScale;

    if (mIgnoreScale) {
        invScale.set< f32 >(uniformScale / scale.x, uniformScale / scale.y, uniformScale / scale.z);
        uniformScale = 1.0f;
    } else if (mUseAutoScale) {
        uniformScale = (scale.x + scale.y + scale.z) / 3.0f;
        invScale.set< f32 >(uniformScale / scale.x, uniformScale / scale.y, uniformScale / scale.z);
    }

    mtx.mMtx[0][0] *= invScale.x;
    mtx.mMtx[1][0] *= invScale.x;
    mtx.mMtx[2][0] *= invScale.x;

    mtx.mMtx[0][1] *= invScale.y;
    mtx.mMtx[1][1] *= invScale.y;
    mtx.mMtx[2][1] *= invScale.y;

    mtx.mMtx[0][2] *= invScale.z;
    mtx.mMtx[1][2] *= invScale.z;
    mtx.mMtx[2][2] *= invScale.z;

    return uniformScale;
}

void CollisionParts::updateBoundingSphereRange() {
    TPos3f matrix(mFollowMtx);
    f32 scale = makeEqualScale(matrix.toMtxPtr());
    updateBoundingSphereRangePrivate(scale);
}

void CollisionParts::updateBoundingSphereRange(TVec3f scale) {
    f32 range = (scale.x + scale.y + scale.z) / 3.0f;
    updateBoundingSphereRangePrivate(range);
}

void CollisionParts::updateBoundingSphereRangePrivate(f32 scale) {
    mEqualScale = scale;
    mBoundingSphereRadius = scale * mServer->mMaxVertexDistance;
}

const char* CollisionParts::getHostName() const {
    if (mHitSensor == nullptr) {
        return nullptr;
    }

    LiveActor* actor = mHitSensor->mHost;

    if (actor == nullptr) {
        return nullptr;
    }

    return actor->mName;
}

s32 CollisionParts::getPlacementZoneID() const {
    return mZone->mZoneID;
}

bool CollisionParts::checkStrikePoint(HitInfo* pHitInfo, const TVec3f& rPos, const TriangleFilterBase* filter) {
    TVec3f localPos;
    mInvBaseMatrix.mult(rPos, localPos);
    TVec3f scale;
    mInvBaseMatrix.getScale(scale);
    f32 inverseScale = 3.0f / (MR::abs(scale.x) + MR::abs(scale.y) + MR::abs(scale.z));
    Fxyz point;
    point.x = localPos.x;
    point.y = localPos.y;
    point.z = localPos.z;
    f32 distance;
    KC_PrismData* prism = mServer->checkPoint(&point, inverseScale, &distance);
    if (prism == nullptr) return false;
    if (filter != nullptr) {
        Triangle triangle;
        triangle.fillData(this, mServer->toIndex(prism), mHitSensor);
        if (filter != nullptr && filter->isInvalidTriangle(&triangle)) return false;
    }
    if (pHitInfo != nullptr) {
        pHitInfo->mParentTriangle.fillData(this, mServer->toIndex(prism), mHitSensor);
        f32 worldDistance = distance * inverseScale;
        pHitInfo->mDistance = worldDistance;
        pHitInfo->mHitPos = rPos + *pHitInfo->mParentTriangle.getNormal(0) * worldDistance;
    }
    return true;
}

u32 CollisionParts::checkStrikeBall(HitInfo* pHitInfo, u32 maxHits, const TVec3f& rPos, f32 radius, bool checkMovement,
                                    const TriangleFilterBase* pFilter) {
    KC_PrismData* prisms[64];
    f32 distances[64];
    u8 classifications[64];
    TPos3f inversePrev;
    TVec3f localPos;
    mInvBaseMatrix.mult(rPos, localPos);
    TVec3f scale;
    mInvBaseMatrix.getScale(scale);
    f32 averageScale = (scale.x + scale.y + scale.z) / 3.0f;
    f32 inverseScale = 1.0f / averageScale;
    radius *= averageScale;
    TVec3f movement(0, 0, 0);

    if (checkMovement && mStationaryFrames == 0) {
        PSMTXInverse(mPrevBaseMatrix.toMtxPtr(), inversePrev.toMtxPtr());
        TVec3f prevPos;
        inversePrev.mult(rPos, prevPos);
        TVec3f localMovement(localPos - prevPos);
        TVec3f worldMovement(localMovement);
        mBaseMatrix.mult33(worldMovement, worldMovement);
        s32 steps = static_cast< s32 >((1.0f / 35.0f) * localMovement.length()) + 1;
        TVec3f step(localMovement);

        if (steps > 1) {
            step /= steps;
        }

        TVec3f offset;
        offset.zero();

        for (s32 i = 0; i <= steps; i++) {
            movement.set(-(localMovement - offset));
            mBaseMatrix.mult33(movement, movement);
            const TVec3f* direction = i == steps ? nullptr : &worldMovement;

            u32 count = checkStrikeBallCore(pHitInfo, maxHits, prevPos + offset, movement, radius, averageScale, inverseScale, prisms, distances,
                                            classifications, pFilter, direction);
            if (count != 0) {
                return count;
            }

            offset += step;
        }

        return 0;
    } else {
        return checkStrikeBallCore(pHitInfo, maxHits, localPos, TVec3f(0, 0, 0), radius, averageScale, inverseScale, prisms, distances,
                                   classifications, pFilter, nullptr);
    }
}

u32 CollisionParts::checkStrikeBallCore(HitInfo* pHitInfo, u32 maxHits, const TVec3f& rPos, const TVec3f& rMovement, f32 radius, f32 scale,
                                        f32 inverseScale, KC_PrismData** pPrisms, f32* pDistances, u8* pClassifications,
                                        const TriangleFilterBase* pFilter, const TVec3f* pDirection) {
    const TVec3f* normal;
    u32 count =
        mServer->checkSphere(reinterpret_cast< Fxyz* >(const_cast< TVec3f* >(&rPos)), radius, scale, maxHits, pPrisms, pDistances, pClassifications);
    u32 hitCount = 0;

    for (u32 i = 0; i < count; i++) {
        HitInfo& hit = pHitInfo[hitCount];
        TVec3f position(rPos);
        calcCollidePosition(&position, *pPrisms[i], pClassifications[i]);
        mBaseMatrix.mult(position, hit.mHitPos);
        hit.mParentTriangle.fillData(this, mServer->toIndex(pPrisms[i]), mHitSensor);

        if (pFilter != nullptr && pFilter->isInvalidTriangle(&hit.mParentTriangle)) {
            continue;
        }

        if (pDirection != nullptr && 0.0f < pDirection->dot(*hit.mParentTriangle.getFaceNormal())) {
            continue;
        }

        hit.mDistance = inverseScale * pDistances[i];
        hit.mCollisionType = pClassifications[i];
        hit.mNormalMovement.set(rMovement);
        normal = hit.mParentTriangle.getFaceNormal();
        hit.mNormalMovement.scale(normal->dot(hit.mNormalMovement), *normal);
        hitCount++;
    }

    return hitCount;
}

u32 CollisionParts::checkStrikeBallWithThickness(HitInfo* pHitInfo, u32 maxHits, const TVec3f& rPos, f32 radius, f32 thickness,
                                                 const TriangleFilterBase* pFilter) {
    KC_PrismData* prisms[64];
    f32 distances[64];
    u8 classifications[64];
    TVec3f localPos;
    mInvBaseMatrix.mult(rPos, localPos);
    TVec3f scale;
    mInvBaseMatrix.getScale(scale);
    f32 averageScale = (scale.x + scale.y + scale.z) / 3.0f;
    Fxyz point;
    point.x = localPos.x;
    point.y = localPos.y;
    point.z = localPos.z;
    f32 scaledRadius = radius * averageScale;
    u32 count = mServer->checkSphereWithThickness(&point, scaledRadius, averageScale, maxHits, prisms, distances, classifications, thickness);
    f32 inverseScale = 1.0f / averageScale;
    u32 hitCount = 0;

    for (u32 i = 0; i < count; i++) {
        HitInfo& hit = pHitInfo[hitCount];
        TVec3f position(localPos);
        calcCollidePosition(&position, *prisms[i], classifications[i]);
        mBaseMatrix.mult(position, hit.mHitPos);
        hit.mParentTriangle.fillData(this, mServer->toIndex(prisms[i]), mHitSensor);

        if (pFilter != nullptr && pFilter->isInvalidTriangle(&hit.mParentTriangle)) {
            continue;
        }

        hit.mDistance = inverseScale * distances[i];
        hit.mCollisionType = classifications[i];
        hitCount++;
    }

    return hitCount;
}

void CollisionParts::calcCollidePosition(TVec3f* pPos, const KC_PrismData& rPrism, u8 classification) {
    TVec3f offset;
    TVec3f normal;

    switch (classification) {
    // face, project into the triangle's plane
    case 1:
        projectToPlane(pPos, *pPos, mServer->getPos(&rPrism, 0), *mServer->getNormal(rPrism.mNormalIndex));
        break;
    // edge 0, between verts 0-2
    case 2:
        projectToPlane(pPos, *pPos, mServer->getPos(&rPrism, 0), *mServer->getNormal(rPrism.mNormalIndex));
        normal.set(*mServer->getNormal(rPrism.mEdgeIndices[0]));
        offset.set(*pPos);
        offset -= mServer->getPos(&rPrism, 0);
        *pPos += -normal * offset.dot(normal);
        break;
    // edge 1, between verts 0-1
    case 3:
        projectToPlane(pPos, *pPos, mServer->getPos(&rPrism, 0), *mServer->getNormal(rPrism.mNormalIndex));
        normal.set(*mServer->getNormal(rPrism.mEdgeIndices[1]));
        offset.set(*pPos);
        offset -= mServer->getPos(&rPrism, 0);
        *pPos += -normal * offset.dot(normal);
        break;
    // edge 2, between verts 1-2
    case 4:
        projectToPlane(pPos, *pPos, mServer->getPos(&rPrism, 0), *mServer->getNormal(rPrism.mNormalIndex));
        normal.set(*mServer->getNormal(rPrism.mEdgeIndices[2]));
        offset.set(*pPos);
        offset -= mServer->getPos(&rPrism, 1);
        *pPos += -normal * offset.dot(normal);
        break;
    // vertex 0
    case 5:
        pPos->set(mServer->getPos(&rPrism, 0));
        break;
    // vertex 1
    case 6:
        pPos->set(mServer->getPos(&rPrism, 1));
        break;
    // vertex 2
    case 7:
        pPos->set(mServer->getPos(&rPrism, 2));
        break;
    }
}

void CollisionParts::projectToPlane(TVec3f* pProjected, const TVec3f& rPos, const TVec3f& rOrigin, const TVec3f& rNormal) {
    TVec3f projected = rPos;

    f32 distance = (rPos - rOrigin).dot(rNormal);

    projected.add(-rNormal * distance);
    pProjected->set(projected);
}

u32 CollisionParts::checkStrikeLine(HitInfo* pHitInfo, u32 maxHits, const TVec3f& rPos, const TVec3f& rDirection, const TriangleFilterBase* pFilter) {
    f32 distances[64];
    KC_PrismData* prisms[64];
    u8 classifications[64];
    f32 length = rDirection.length();
    TVec3f localPos;
    TVec3f localDirection;
    mInvBaseMatrix.mult(rPos, localPos);
    mInvBaseMatrix.mult(rPos + rDirection, localDirection);
    localDirection = localDirection - localPos;
    u32 count = 0;
    mServer->checkArrow(localPos, localDirection, distances, classifications, &count, prisms, maxHits);
    u32 hitCount = 0;

    for (u32 i = 0; i < count; i++) {
        HitInfo& hit = pHitInfo[hitCount];
        TVec3f position(localPos + localDirection * distances[i]);
        mBaseMatrix.mult(position, position);
        hit.mParentTriangle.fillData(this, mServer->toIndex(prisms[i]), mHitSensor);

        if (pFilter != nullptr && pFilter->isInvalidTriangle(&hit.mParentTriangle)) {
            continue;
        }

        hit.mDistance = length * distances[i];
        hit.mHitPos = position;
        hit.mCollisionType = classifications[i];
        hitCount++;
    }

    return hitCount;
}

u32 CollisionParts::createAreaPolygonList(Triangle* pTriangles, u32 maxTriangles, const TVec3f& rMin, const TVec3f& rMax) {
    KC_PrismData* prisms[512];
    TPos3f rotation;
    PSMTXCopy(mInvBaseMatrix.toMtxPtr(), rotation.toMtxPtr());
    rotation.zeroTrans();
    TVec3f min;
    TVec3f max;
    mInvBaseMatrix.mult(rMin, min);
    mInvBaseMatrix.mult(rMax, max);
    u32 count = mServer->checkArea3D(reinterpret_cast< Fxyz* >(&min), reinterpret_cast< Fxyz* >(&max), prisms, maxTriangles);
    if (count == 0) {
        return 0;
    }

    for (u32 i = 0; i < count; i++) {
        pTriangles[i].fillData(this, mServer->toIndex(prisms[i]), mHitSensor);
    }

    return count;
}

u32 CollisionParts::createAreaPolygonListArray(Triangle* pTriangles, u32 maxTriangles, TVec3f* pPoints, u32 numPoints) {
    KC_PrismData* prisms[512];
    TVec3f localPoints[32];
    TPos3f rotation;
    PSMTXCopy(mInvBaseMatrix.toMtxPtr(), rotation.toMtxPtr());
    rotation.zeroTrans();

    for (u32 i = 0; i < numPoints; i++) {
        mInvBaseMatrix.mult(pPoints[i], localPoints[i]);
    }

    TVec3f min;
    TVec3f max;
    MR::createBoundingBox(localPoints, numPoints, &min, &max);
    u32 count = mServer->checkArea3D(reinterpret_cast< Fxyz* >(&min), reinterpret_cast< Fxyz* >(&max), prisms, maxTriangles);
    if (count == 0) {
        return 0;
    }

    for (u32 i = 0; i < count; i++) {
        pTriangles[i].fillData(this, mServer->toIndex(prisms[i]), mHitSensor);
    }

    return count;
}

void CollisionParts::calcForceMovePower(TVec3f* pMovement, const TVec3f& rPos) const {
    TVec3f position(rPos);
    TMtx34f inversePrev;
    PSMTXInverse(mPrevBaseMatrix, inversePrev);

    inversePrev.mult(position, position);
    mBaseMatrix.mult(position, position);

    position.sub(rPos);
    *pMovement = position;
}
