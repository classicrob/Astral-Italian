#include "Game/Map/CollisionHitInfoList.hpp"
#include "Game/Util/MapUtil.hpp"

namespace {
    struct CollisionQuery {
        s32 (*query)(const TVec3f&, const TVec3f&, s32, const CollisionPartsFilterBase*, const TriangleFilterBase*);
        const HitInfo* (*getHit)(u32);
    };

    CollisionQuery sMapQuery = {Collision::checkStrikeLineToMap, Collision::getStrikeInfoMap};
    CollisionQuery sDashYoshiQuery = {Collision::checkStrikeLineToDashYoshiStep, Collision::getStrikeInfoDashYoshiStep};

    // NONMATCHING: paired-single temporary registers differ; calls, branches and literals agree.
    u32 sortHits(const CollisionQuery* query, u32* count, HitInfo* hits, const TVec3f& origin,
                 const TVec3f& start, const TVec3f& direction, const HitSensor* excludedSensor) {
        const u32 found = query->query(start, direction, 0, 0, 0);
        if (found == 0) {
            return 0;
        }

        const HitInfo* remaining[32];
        u32 excluded = 0;
        for (u32 i = 0; i < found; ++i) {
            remaining[i] = query->getHit(i);
            if (excludedSensor && remaining[i]->mParentTriangle.mSensor == excludedSensor) {
                remaining[i] = 0;
                ++excluded;
            }
        }
        *count = found - excluded;

        for (u32 output = 0; output < *count; ++output) {
            f32 nearestDistance = 1000000.0f;
            u32 nearest = 0;
            for (u32 i = 0; i < found; ++i) {
                if (remaining[i]) {
                    const HitInfo* candidate = query->getHit(i);
                    TVec3f delta(origin);
                    delta.sub(candidate->mHitPos);
                    const f32 distance = PSVECMag(&delta);
                    if (nearestDistance > distance) {
                        nearest = i;
                        nearestDistance = distance;
                    }
                }
            }
            const HitInfo* selected = query->getHit(nearest);
            HitInfo& dest = hits[output];
            dest.mParentTriangle = selected->mParentTriangle;
            dest.mDistance = selected->mDistance;
            dest.mHitPos = selected->mHitPos;
            dest.mQueryPos = selected->mQueryPos;
            dest.mNormalMovement = selected->mNormalMovement;
            dest.mCollisionType = selected->mCollisionType;
            remaining[nearest] = 0;
        }
        return *count;
    }
}

CollisionHitInfoList::CollisionHitInfoList() : mCount(0) {}

u32 CollisionHitInfoList::getNearPolyOnLineSort(const TVec3f& origin, const TVec3f& start, const TVec3f& direction,
                                              const HitSensor* excludedSensor) {
    return sortHits(&sMapQuery, &mCount, mHits, origin, start, direction, excludedSensor);
}

u32 CollisionHitInfoList::getNearPolyOnLineSortDashYoshiStep(const TVec3f& origin, const TVec3f& start,
                                                           const TVec3f& direction, const HitSensor* excludedSensor) {
    return sortHits(&sDashYoshiQuery, &mCount, mHits, origin, start, direction, excludedSensor);
}

bool CollisionHitInfoList::getSortedPoly(TVec3f* position, Triangle* triangle, u32 index) const {
    if (mCount <= index) {
        return false;
    }
    const HitInfo& hit = mHits[index];
    if (triangle) {
        *triangle = hit.mParentTriangle;
    }
    if (position) {
        *position = hit.mHitPos;
    }
    return true;
}

const Triangle* CollisionHitInfoList::getSortedPoly(u32 index) const {
    if (mCount <= index) {
        return 0;
    }
    return &mHits[index].mParentTriangle;
}
