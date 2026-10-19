#include "Game/Util/MapUtil.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/CollisionCategorizedKeeper.hpp"
#include "Game/Map/CollisionCode.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Map/CollisionHitInfoList.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/TriangleFilter.hpp"

namespace MR {
    bool isGroundCodeWaterIter(const JMapInfoIter&) NO_INLINE;
    bool isGroundCodeDamageElectric(const Triangle*) NO_INLINE;
    bool isGroundCodeWaterBottomL(const Triangle*) NO_INLINE;
    bool isGroundCodeSlip(const Triangle*) NO_INLINE;
    bool isGroundCodeSnowIter(const JMapInfoIter&) NO_INLINE;
}

static HitInfo mSortBuffer[32];
static u32 mSortCount;

namespace {
    u32 getStrikeInfoNumCategory(s32 category) NO_INLINE;
    bool getFirstPolyOnLineCategoryExceptActor(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, const LiveActor*, s32) NO_INLINE;
    bool getFirstPolyOnLineToMapAndMoveLimitWithFilter(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, const TriangleFilterBase*,
                                                       const CollisionPartsFilterBase*) NO_INLINE;
    bool getFirstPolyOnLineCategoryWithSensorType(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, u32, s32) NO_INLINE;

    u32 getStrikeInfoNumCategory(s32 category) {
        return MR::getCollisionDirector()->getCategoryKeeper(category)->_10;
    }

    const HitInfo* getStrikeInfoCategory(s32 category, u32 index) {
        return MR::getCollisionDirector()->getCategoryKeeper(category)->getStrikeInfo(index);
    }



    bool getFirstPolyOnLineCategory(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                    const TriangleFilterBase* pTriangleFilter, const CollisionPartsFilterBase* pPartsFilter, s32 category) {
        u32 count = MR::getCollisionDirector()->getCategoryKeeper(category)->checkStrikeLine(rStart, rDirection, 0, pPartsFilter, nullptr);
        const HitInfo* pHit;
        if (count == 0) {
            return false;
        }

        f32 minDistance = 1000000.0f;
        s32 nearest = -1;
        for (u32 i = 0; i < count; i++) {
            pHit = getStrikeInfoCategory(category, i);
            if (pTriangleFilter && pTriangleFilter->isInvalidTriangle(&pHit->mParentTriangle)) {
                continue;
            }

            if (minDistance > pHit->_60) {
                nearest = i;
                minDistance = pHit->_60;
            }
        }

        if (nearest == -1) {
            return false;
        }

        pHit = getStrikeInfoCategory(category, nearest);
        if (pPos) {
            *pPos = pHit->_64;
        }

        if (pTriangle) {
            *pTriangle = pHit->mParentTriangle;
        }

        return true;
    }

};  // namespace

Triangle& Triangle::operator=(const Triangle& rOther) {
    mParts = rOther.mParts;
    mIdx = rOther.mIdx;
    mSensor = rOther.mSensor;
    mNormals[0] = rOther.mNormals[0];
    mNormals[1] = rOther.mNormals[1];
    mNormals[2] = rOther.mNormals[2];
    mNormals[3] = rOther.mNormals[3];
    mPos[0] = rOther.mPos[0];
    mPos[1] = rOther.mPos[1];
    mPos[2] = rOther.mPos[2];
    return *this;
}

namespace {
    bool getFirstPolyOnLineToMapAndMoveLimitWithFilter(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                                       const TriangleFilterBase* pTriangleFilter, const CollisionPartsFilterBase* pPartsFilter) {
        Triangle mapTriangle;
        TVec3f mapPos;
        bool hitMap = getFirstPolyOnLineCategory(&mapPos, &mapTriangle, rStart, rDirection, pTriangleFilter, pPartsFilter, 0);
        Triangle limitTriangle;
        TVec3f limitPos;
        bool hitLimit = getFirstPolyOnLineCategory(&limitPos, &limitTriangle, rStart, rDirection, pTriangleFilter, pPartsFilter, 3);

        if (hitMap && hitLimit) {
            if (rStart.distance(mapPos) >= rStart.distance(limitPos)) {
                hitMap = false;
            }
        }

        if (hitMap) {
            if (pPos) {
                pPos->set(mapPos);
            }

            if (pTriangle) {
                *pTriangle = mapTriangle;
            }

            return true;
        }

        if (hitLimit) {
            if (pPos) {
                pPos->set(limitPos);
            }

            if (pTriangle) {
                *pTriangle = limitTriangle;
            }

            return true;
        }

        return false;
    }

    bool getFirstPolyOnLineCategoryExceptSensor(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                                const HitSensor* pSensor, s32 category) {
        CollisionPartsFilterSensor filter(pSensor);
        return getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, nullptr, &filter, category);
    }

    bool getFirstPolyOnLineCategoryWithSensorType(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                                  u32 sensorType, s32 category) {
        CollisionPartsFilterSensorType filter(sensorType);
        return getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, nullptr, &filter, category);
    }

    bool getFirstPolyOnLineCategoryExceptActor(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                               const LiveActor* pActor, s32 category) {
        CollisionPartsFilterActor filter(pActor);
        return getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, nullptr, &filter, category);
    }

};  // namespace

namespace MR {
    const TVec3f* getNormal(const Triangle* pTriangle) {
        return pTriangle->getNormal(0);
    }

    bool isWallPolygon(const TVec3f& rParam1, const TVec3f& rParam2) {
        if (isNearZero(rParam1)) {
            return false;
        }

        return isWallPolygon(rParam1.dot(rParam2));
    }

    bool isFloorPolygon(const TVec3f& rParam1, const TVec3f& rParam2) {
        if (isNearZero(rParam1)) {
            return false;
        }

        return isFloorPolygon(rParam1.dot(rParam2));
    }

    bool isFloorPolygonCos(const TVec3f& rParam1, const TVec3f& rParam2, f32 param3) {
        if (isNearZero(rParam1)) {
            return false;
        }

        if (-rParam1.dot(rParam2) < param3) {
            return false;
        }

        return isFloorPolygon(rParam1.dot(rParam2));
    }

    bool isWallPolygon(f32 param1) {
        return MR::abs(param1) < 0.34202015f;
    }

    bool isFloorPolygon(f32 param1) {
        if (isWallPolygon(param1)) {
            return false;
        }

        return param1 < 0.0f;
    }

    bool isCeilingPolygon(f32 param1) {
        if (isWallPolygon(param1) || isFloorPolygon(param1)) {
            return false;
        }

        return true;
    }

    bool isWaterPolygon(const Triangle* pTriangle) {
        const char* pFloorCodeString = getFloorCodeString(pTriangle);

        if (pFloorCodeString != nullptr) {
            if (strcmp(pFloorCodeString, "Water") == 0) {
                return true;
            }

            if (strcmp(pFloorCodeString, "Shallow") == 0) {
                return true;
            }
        }

        return false;
    }

    bool isThroughPolygon(const Triangle* pTriangle) {
        const char* pFloorCodeString = getFloorCodeString(pTriangle);

        if (pFloorCodeString != nullptr) {
            if (strcmp(pFloorCodeString, "Water") == 0) {
                return true;
            }

            if (strcmp(pFloorCodeString, "Shallow") == 0) {
                return true;
            }

            if (strcmp(pFloorCodeString, "PullBack") == 0) {
                return true;
            }
        }

        return false;
    }





    bool getFirstPolyOnLineToMap(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection) {
        return ::getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, nullptr, nullptr, 0);
    }

    bool fn_80037E40(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection) {
        return ::getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, nullptr, nullptr, 3);
    }

    bool getFirstPolyOnLineToMapAndMoveLimit(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection) {
        return ::getFirstPolyOnLineToMapAndMoveLimitWithFilter(pPos, pTriangle, rStart, rDirection, nullptr, nullptr);
    }

    bool getFirstPolyOnLineToWaterSurface(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection) {
        return ::getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, nullptr, nullptr, 2);
    }

    bool getFirstPolyOnLineToMapExceptSensor(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                             const HitSensor* pSensor) {
        return ::getFirstPolyOnLineCategoryExceptSensor(pPos, pTriangle, rStart, rDirection, pSensor, 0);
    }

    bool fn_80037E80(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection) {
        Triangle triangleA;
        Triangle triangleB;
        TVec3f posA;
        bool hitA = ::getFirstPolyOnLineCategory(&posA, &triangleA, rStart, rDirection, nullptr, nullptr, 0);
        TVec3f posB;
        bool hitB = ::getFirstPolyOnLineCategory(&posB, &triangleB, rStart, rDirection, nullptr, nullptr, 4);

        if (!hitA && !hitB) {
            return false;
        }

        if (hitA && hitB) {
            if (posB.distance(rStart) <= posA.distance(rStart)) {
                if (pPos) {
                    *pPos = posB;
                }

                if (pTriangle) {
                    *pTriangle = triangleB;
                }
            } else {
                if (pPos) {
                    *pPos = posA;
                }

                if (pTriangle) {
                    *pTriangle = triangleA;
                }
            }
        } else if (hitA) {
            if (pPos) {
                *pPos = posA;
            }

            if (pTriangle) {
                *pTriangle = triangleA;
            }
        } else {
            if (pPos) {
                *pPos = posB;
            }

            if (pTriangle) {
                *pTriangle = triangleB;
            }
        }

        return true;
    }

    // SMG2 fn_80038020 (address-named; symbol fn_80038020__2MRF..Ul): sensor-type line query via getFirstPolyOnLineCategoryWithSensorType
    bool fn_80038020(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection, u32 sensorType) {
        return ::getFirstPolyOnLineCategoryWithSensorType(pPos, pTriangle, rStart, rDirection, sensorType, 0);
    }

    bool getFirstPolyOnLineToMapExceptActor(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                            const LiveActor* pActor) {
        return ::getFirstPolyOnLineCategoryExceptActor(pPos, pTriangle, rStart, rDirection, pActor, 0);
    }







    bool getFirstPolyOnLineToMap(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                 const CollisionPartsFilterBase* pPartsFilter, const TriangleFilterBase* pTriangleFilter) {
        return ::getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, pTriangleFilter, pPartsFilter, 0);
    }

    bool fn_80038060(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection, const CollisionPartsFilterBase* pPartsFilter,
                     const TriangleFilterBase* pTriangleFilter) {
        Triangle triangleA;
        Triangle triangleB;
        TVec3f posA;
        bool hitA = ::getFirstPolyOnLineCategory(&posA, &triangleA, rStart, rDirection, pTriangleFilter, pPartsFilter, 0);
        TVec3f posB;
        bool hitB = ::getFirstPolyOnLineCategory(&posB, &triangleB, rStart, rDirection, pTriangleFilter, pPartsFilter, 4);

        if (!hitA && !hitB) {
            return false;
        }

        if (hitA && hitB) {
            if (posB.distance(rStart) <= posA.distance(rStart)) {
                *pPos = posB;
                *pTriangle = triangleB;
            } else {
                *pPos = posA;
                *pTriangle = triangleA;
            }
        } else if (hitA) {
            *pPos = posA;
            *pTriangle = triangleA;
        } else {
            *pPos = posB;
            *pTriangle = triangleB;
        }

        return true;
    }

    bool getFirstPolyOnLineToWaterSurface(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                          const CollisionPartsFilterBase* pPartsFilter, const TriangleFilterBase* pTriangleFilter) {
        return ::getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, pTriangleFilter, pPartsFilter, 2);
    }

    bool getFirstPolyNormalOnLineToMap(TVec3f* pNormal, const TVec3f& rStart, const TVec3f& rDirection, TVec3f* pPos, const HitSensor* pSensor) {
        Triangle triangle;
        if (!::getFirstPolyOnLineCategoryExceptSensor(pPos, &triangle, rStart, rDirection, pSensor, 0)) {
            return false;
        }

        pNormal->set(*triangle.getFaceNormal());
        return true;
    }

    bool isExistMapCollision(const TVec3f& rParam1, const TVec3f& rParam2) {
        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rParam1, rParam2, 1, nullptr, nullptr) != 0;
    }

    bool fn_800382E0(const TVec3f& rStart, const TVec3f& rDirection) {
        bool hit = getCollisionDirector()->getCategoryKeeper(4)->checkStrikeLine(rStart, rDirection, 1, nullptr, nullptr) != 0;
        if (hit) {
            return true;
        }

        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rStart, rDirection, 1, nullptr, nullptr) != 0;
    }

    bool isExistMoveLimitCollision(const TVec3f& rParam1, const TVec3f& rParam2) {
        return getCollisionDirector()->getCategoryKeeper(3)->checkStrikeLine(rParam1, rParam2, 1, nullptr, nullptr) != 0;
    }

    bool isExistMapCollisionExceptSensor(const TVec3f& rStart, const TVec3f& rDirection, const LiveActor* pActor) {
        CollisionPartsFilterSensor filter(reinterpret_cast< const HitSensor* >(pActor));
        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rStart, rDirection, 1, &filter, nullptr) != 0;
    }

    bool isExistMapCollisionExceptActor(const TVec3f& rStart, const TVec3f& rDirection, const LiveActor* pActor) {
        CollisionPartsFilterActor filter(pActor);
        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rStart, rDirection, 1, &filter, nullptr) != 0;
    }

    bool fn_800384C0(const TVec3f& rStart, const TVec3f& rDirection, const LiveActor* pActor) {
        CollisionPartsFilterActor filter(pActor);
        bool hit = getCollisionDirector()->getCategoryKeeper(4)->checkStrikeLine(rStart, rDirection, 1, &filter, nullptr) != 0;
        if (hit) {
            return true;
        }

        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rStart, rDirection, 1, &filter, nullptr) != 0;
    }

    bool fn_80038570(const TVec3f* pStart, const TVec3f* pDirection) {
        Triangle triangle;
        TVec3f pos(0.0f);
        if (::getFirstPolyOnLineCategory(&pos, &triangle, *pStart, *pDirection, nullptr, nullptr, 0)) {
            if (isGroundCodeIceIter(triangle.getAttributes())) {
                return true;
            }
        }

        return false;
    }

    bool checkStrikePointToMap(const TVec3f& rParam1, HitInfo* pParam2) {
        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikePoint(rParam1, pParam2, nullptr, nullptr) != 0;
    }

    bool checkStrikePointToMapExceptSensor(const TVec3f& rPos, const HitSensor* pSensor, HitInfo* pHit) {
        CollisionPartsFilterSensor filter(pSensor);
        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikePoint(rPos, pHit, &filter, nullptr) != 0;
    }

    bool fn_800386E0(const TVec3f& rPoint, HitInfo* pHit) {
        return Collision::checkStrikePointToDashYoshiStepAndMap(rPoint, pHit);
    }

    bool checkStrikeBallToMap(const TVec3f& rParam1, f32 param2) {
        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikeBall(rParam1, param2, false, nullptr, nullptr) != 0;
    }



    // SMG2 fn_80038770 (address-named; symbol fn_80038770__2MRF..PbPbf): ball-hit helper shared with Ride/GliBird.cpp
    s32 fn_80038770(TVec3f* pHitPos, TVec3f* pHitNormal, HitSensor** ppSensor, const TVec3f& rPos, bool* pIsWater, bool* pIsFire, f32 radius) {
        if (getCollisionDirector()->getCategoryKeeper(0)->checkStrikeBall(rPos, radius, false, nullptr, nullptr) == 0) {
            return 0;
        }

        HitInfo* pHit = getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(0);
        pHitPos->set(pHit->_64);
        pHitNormal->set(*pHit->mParentTriangle.getFaceNormal());
        *ppSensor = pHit->mParentTriangle.mSensor;
        if (pIsWater) {
            *pIsWater = isGroundCodeWaterIter(pHit->mParentTriangle.getAttributes());
        }

        if (pIsFire) {
            *pIsFire = isGroundCodeDamageFire(&pHit->mParentTriangle);
        }

        return 1;
    }

    // SMG2 fn_80038870 (address-named; symbol fn_80038870__2MRF..PbPbf): ray-cast helper (same decl as BellyDragonTU02.cpp)
    s32 fn_80038870(const TVec3f& rPos, TVec3f* pPos, const TVec3f& rDirection, TVec3f* pNormal, HitSensor** ppSensor, bool* pIsWater, bool* pIsFire,
                    f32 length) {
        TVec3f direction(rDirection);
        direction.scale(length);
        bool hit = getFirstPolyOnLineCategory(pPos, nullptr, rPos, direction, nullptr, nullptr, 0);
        if (hit) {
            HitInfo* pHit = getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(0);
            if (pNormal) {
                pNormal->set(*pHit->mParentTriangle.getFaceNormal());
            }

            if (ppSensor) {
                *ppSensor = pHit->mParentTriangle.mSensor;
            }
        }

        if (hit) {
            HitInfo* pHit = getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(0);
            if (pIsWater) {
                *pIsWater = isGroundCodeWaterIter(pHit->mParentTriangle.getAttributes());
            }

            if (pIsFire) {
                *pIsFire = isGroundCodeDamageFire(&pHit->mParentTriangle);
            }
        } else {
            if (pIsWater) {
                *pIsWater = false;
            }

            if (pIsFire) {
                *pIsFire = false;
            }
        }

        return hit;
    }

    bool calcMapGroundUpper(TVec3f* pOut, const LiveActor* pActor) {
        CollisionParts* pParts = getCollisionParts(const_cast<LiveActor*>(pActor));
        TVec3f gravity;
        calcGravityVector(pActor, pActor->mPosition, &gravity, nullptr, 0);
        f32 radius = 0.0f;
        calcModelBoundingRadius(&radius, pActor);
        f32 offsetScale = 2.5f * radius;
        f32 directionScale = 4.0f * radius;
        TVec3f offset(gravity);
        offset.scale(offsetScale);
        TVec3f start(pActor->mPosition);
        start.sub(offset);
        TVec3f direction(gravity);
        direction.scale(directionScale);
        HitInfo hit;
        pParts->checkStrikeLine(&hit, 1, start, direction, nullptr);
        CollisionPartsFilterSensor filter(pParts->mHitSensor);
        return getFirstPolyOnLineCategory(pOut, nullptr, hit._64, direction, nullptr, &filter, 0);
    }

    bool isFallNextMove(const LiveActor* pActor, f32 param2, f32 param3, f32 param4, const TriangleFilterBase* pParam5) {
        return isFallNextMove(pActor->mPosition, pActor->mVelocity, pActor->mGravity, param2, param3, param4, pParam5);
    }

    bool isFallNextMove(const TVec3f& rPos, const TVec3f& rVelocity, const TVec3f& rGravity, f32 forwardDistance, f32 upperDistance,
                        f32 lowerDistance, const TriangleFilterBase* pTriangleFilter) {
        if (isNearZero(rGravity)) {
            return false;
        }

        TVec3f forward;
        forward.scaleAdd(-rGravity.dot(rVelocity), rGravity, rVelocity);
        if (isNearZero(forward)) {
            return false;
        }

        normalize(&forward);
        forward *= forwardDistance;
        TVec3f start;
        start = forward + rPos - rGravity * upperDistance;
        bool hit = getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(start, rGravity * (upperDistance + lowerDistance), 1, nullptr,
                                                                                 pTriangleFilter) != 0;
        return !hit;
    }

    bool isFallOrDangerNextMove(const LiveActor* pActor, f32 forwardDistance, f32 upperDistance, f32 lowerDistance) {
        TriangleFilterDangerCode filter;
        return isFallNextMove(pActor, forwardDistance, upperDistance, lowerDistance, &filter);
    }

    bool isFallOrDangerNextMove(const TVec3f& rPos, const TVec3f& rVelocity, const TVec3f& rGravity, f32 forwardDistance, f32 upperDistance,
                                f32 lowerDistance) {
        TriangleFilterDangerCode filter;
        return isFallNextMove(rPos, rVelocity, rGravity, forwardDistance, upperDistance, lowerDistance, &filter);
    }

    HitSensor* fn_80038D20(const Triangle* pTriangle) {
        if (pTriangle && pTriangle->isValid()) {
            return pTriangle->mSensor;
        }
        return nullptr;
    }

    void calcVelocityMovingPoint(const Triangle* pTriangle, const TVec3f& rPoint, TVec3f* pVelocity) {
        if (isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            pVelocity->zero();
            return;
        }

        TVec3f localPoint;
        PSMTXMultVec(pTriangle->getBaseInvMtx()->toMtxPtr(), rPoint, &localPoint);
        TVec3f previousPoint;
        PSMTXMultVec(pTriangle->getPrevBaseMtx()->toMtxPtr(), &localPoint, &previousPoint);
        *pVelocity = rPoint - previousPoint;
    }

    u32 createAreaPolygonList(Triangle* pTriangle, u32 param2, const TVec3f& rParam3, const TVec3f& rParam4) {
        return getCollisionDirector()->getCategoryKeeper(0)->createAreaPolygonList(pTriangle, param2, rParam3, rParam4);
    }

    u32 createAreaPolygonListArray(Triangle* pTriangle, u32 param2, TVec3f* pParam3, u32 param4) {
        return getCollisionDirector()->getCategoryKeeper(0)->createAreaPolygonListArray(pTriangle, param2, pParam3, param4);
    }

    // NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited.
    CollisionParts* findMoveLimitCollision(const TVec3f& rPos, const TVec3f& rGravity) {
        TVec3f start(rPos);
        TVec3f direction(rGravity);
        Triangle triangle;
        start -= direction * 150.0f;
        direction *= 1000.0f;
        if (getFirstPolyOnLineCategory(nullptr, &triangle, start, direction, nullptr, nullptr, 3)) {
            return triangle.mParts;
        }
        if (getFirstPolyOnLineCategory(nullptr, &triangle, start, direction, nullptr, nullptr, 0)) {
            CollisionParts* sameParts = nullptr;
            getCollisionDirector()->getCategoryKeeper(3)->searchSameHostParts(&sameParts, triangle.mParts);
            return sameParts;
        }
        return nullptr;
    }

    bool trySetMoveLimitCollision(LiveActor* pActor) {
        CollisionParts* parts = findMoveLimitCollision(pActor->mPosition, pActor->mGravity);
        if (!parts) {
            return false;
        }
        pActor->mBinder->setExCollisionParts(parts);
        return true;
    }

    bool isBindedGroundIce(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeIce(&pActor->mBinder->mGroundInfo.mParentTriangle) ||
               isGroundCodeGlassIce(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundSand(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeSand(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundSnow(const LiveActor* pActor) {
        if (!pActor->mBinder) {
            return false;
        }
        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }
        return isGroundCodeSnowIter(pActor->mBinder->mGroundInfo.mParentTriangle.getAttributes());
    }

    bool isBindedGroundDamageFire(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeDamageFire(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundDamageElectric(const LiveActor* pActor) {
        if (!pActor->mBinder) {
            return false;
        }
        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }
        return isGroundCodeDamageElectric(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundWaterBottomH(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeWaterBottomH(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundWaterBottomM(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeWaterBottomM(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundWaterBottomL(const LiveActor* pActor) {
        if (!pActor->mBinder) {
            return false;
        }
        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }
        return isGroundCodeWaterBottomL(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundWater(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeWaterIter(pActor->mBinder->mGroundInfo.mParentTriangle.getAttributes());
    }

    bool isBindedGroundSinkDeath(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeSinkDeath(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundAreaMove(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeAreaMove(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundRailMove(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeRailMove(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundBrake(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeBrake(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    bool isBindedGroundSlip(const LiveActor* pActor) {
        if (!pActor->mBinder) {
            return false;
        }
        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }
        return isGroundCodeSlip(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }

    // NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited.
    bool isBindedWallNoActionWithoutGroundOrRoof(const LiveActor* pActor) {
        if (!pActor->mBinder) {
            return false;
        }
        if (isBindedGround(pActor) || isBindedRoof(pActor)) {
            return false;
        }
        return pActor->mBinder->isBindedWall() && isWallCodeNoAction(&pActor->mBinder->mWallInfo.mParentTriangle);
    }

    bool isBindedDamageFire(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (pActor->mBinder->isBindedGround() && isGroundCodeDamageFire(&pActor->mBinder->mGroundInfo.mParentTriangle)) {
            return true;
        }

        if (pActor->mBinder->isBindedWall() && isGroundCodeDamageFire(&pActor->mBinder->mWallInfo.mParentTriangle)) {
            return true;
        }

        if (pActor->mBinder->isBindedRoof() && isGroundCodeDamageFire(&pActor->mBinder->mRoofInfo.mParentTriangle)) {
            return true;
        }

        return false;
    }


    u32 getCameraID(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getCameraID(*pTriangle);
    }

    const char* getFloorCodeString(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getFloorCodeString(*pTriangle);
    }

    const char* getWallCodeString(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getWallCodeString(*pTriangle);
    }

    const char* getSoundCodeString(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getSoundCodeString(*pTriangle);
    }

    s32 getFloorCodeIndex(const JMapInfoIter& rIter) {
        return getCollisionDirector()->mCode->getFloorCode(rIter);
    }

    s32 getSoundCodeIndex(const JMapInfoIter& rIter) {
        return getCollisionDirector()->mCode->getSoundCode(rIter);
    }

    s32 getFloorCodeIndex(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getFloorCode(pTriangle->getAttributes());
    }

    s32 getWallCodeIndex(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getWallCode(pTriangle->getAttributes());
    }

    s32 getSoundCodeIndex(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getSoundCode(pTriangle->getAttributes());
    }























    bool isGroundCodeSnowIter(const JMapInfoIter& rIter) {
        return getFloorCodeIndex(rIter) == 0x1a || getFloorCodeIndex(rIter) == 0x2b;
    }

    bool isGroundCodePressIter(const JMapInfoIter& rIter) {
        return getFloorCodeIndex(rIter) == 0x1d || getFloorCodeIndex(rIter) == 0x27;
    }

    bool isGroundCodeWaterIter(const JMapInfoIter& rIter) {
        return static_cast<u32>(getFloorCodeIndex(rIter) - 0x14) <= 3;
    }

    bool isGroundCodeIceIter(const JMapInfoIter& rIter) {
        s32 code = getFloorCodeIndex(rIter);
        return code == 5 || code == 0x21;
    }

    bool isGroundCodeDeath(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x1;
    }

    bool isGroundCodeSlip(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x2;
    }

    bool isGroundCodeDamage(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x4;
    }

    bool isGroundCodeIce(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x5;
    }

    bool isGroundCodeDamageFire(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0xa;
    }

    bool isGroundCodeFireDance(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0xc;
    }

    bool isGroundCodeSand(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0xd;
    }

    bool isGroundCodeDamageElectric(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0xf;
    }

    bool isGroundCodeWaterBottomH(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x14;
    }

    bool isGroundCodeWaterBottomM(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x15;
    }

    bool isGroundCodeWaterBottomL(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x16;
    }

    bool isGroundCodeWet(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x17;
    }

    bool isGroundCodeSinkDeath(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x19;
    }

    bool isGroundCodeRailMove(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x1b;
    }

    bool isGroundCodeAreaMove(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x1c;
    }

    bool isGroundCodePress(const Triangle* pTriangle) {
        return isGroundCodePressIter(pTriangle->getAttributes());
    }

    bool isGroundCodeNoStampSand(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x1e;
    }

    bool isGroundCodeSinkDeathMud(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x1f;
    }

    bool isGroundCodeBrake(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x20;
    }

    bool isGroundCodeGlassIce(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x21;
    }

    bool isGroundCodeNoDig(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x24;
    }

    bool isGroundCodeForceDash(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == 0x28;
    }

    bool isGroundCodeWater(const Triangle* pTriangle) {
        return isGroundCodeWaterIter(pTriangle->getAttributes());
    }

    bool isWallCodeGhostThrough(const Triangle* pTriangle) {
        return getWallCodeIndex(pTriangle) == 0x4;
    }

    bool isWallCodeRebound(const Triangle* pTriangle) {
        return getWallCodeIndex(pTriangle) == 0x6;
    }

    bool isWallCodeNoAction(const Triangle* pTriangle) {
        return getWallCodeIndex(pTriangle) == 0x8;
    }

    bool isCameraCodeThrough(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getCameraCode(pTriangle->getAttributes()) == 0x1;
    }

    bool isCodeSand(const Triangle* pTriangle) {
        return getSoundCodeIndex(pTriangle) == 0xB || getSoundCodeIndex(pTriangle) == 0xC ||
               isGroundCodeSand(pTriangle) || isGroundCodeNoStampSand(pTriangle);
    }

    const Triangle* getCameraPolyFast(const TVec3f& rStart, const TVec3f& rDirection, const HitSensor* pSensor) {
        Triangle triangle;
        f32 remaining = rDirection.length();
        TVec3f direction(rDirection);
        direction.normalize();
        TVec3f start(rStart);
        TVec3f step(direction * 5000.0f);
        CollisionHitInfoList hits;
        do {
            f32 length = 5000.0f;
            if (remaining < length) {
                length = remaining;
                step = direction * remaining;
            }

            s32 count = hits.getNearPolyOnLineSort(start, start, step, pSensor);
            while (count != 0) {
                return hits.getSortedPoly(0);
            }

            remaining -= length;
            start += step;
        } while (!isNearZero(remaining));

        return nullptr;
    }

    bool getFirstPolyOnLineBFast(const TVec3f& rStart, const TVec3f& rDirection, TVec3f* pPos, Triangle* pTriangle) {
        Triangle triangle;
        f32 remaining = rDirection.length();
        TVec3f direction(rDirection);
        direction.normalize();
        TVec3f start(rStart);
        TVec3f step(direction * 5000.0f);
        CollisionHitInfoList hits;
        do {
            f32 length = 5000.0f;
            if (remaining < length) {
                length = remaining;
                step = direction * remaining;
            }

            u32 count = hits.getNearPolyOnLineSort(start, start, step, nullptr);
            for (u32 i = 0; i < count; i++) {
                Triangle hitTriangle;
                TVec3f hitPos;
                if (hits.getSortedPoly(&hitPos, &hitTriangle, i)) {
                    if (isWaterPolygon(&hitTriangle)) {
                        continue;
                    }

                    TVec3f backPos(hitPos - *getNormal(&hitTriangle) * 5.0f);
                    TVec3f normal(*getNormal(&hitTriangle) * 35.0f);
                    if (isExistMapCollision(backPos, normal)) {
                        continue;
                    }
                }

                if (pPos) {
                    *pPos = hitPos;
                }

                if (pTriangle) {
                    *pTriangle = hitTriangle;
                }

                return true;
            }

            remaining -= length;
            start += step;
        } while (!isNearZero(remaining));

        return false;
    }
    // NONMATCHING: register and temporary scheduling.
    bool fn_8003A720(const TVec3f& rStart, const TVec3f& rDirection, TVec3f* pPos, Triangle* pTriangle) {
        Triangle triangle;
        f32 remaining = rDirection.length();
        TVec3f direction(rDirection);
        direction.normalize();
        TVec3f start(rStart);
        TVec3f step(direction * 5000.0f);
        CollisionHitInfoList hits;
        do {
            f32 length = 5000.0f;
            if (remaining < length) {
                length = remaining;
                step = direction * remaining;
            }
            u32 count = hits.getNearPolyOnLineSortDashYoshiStep(start, start, step, nullptr);
            Triangle dashTriangle;
            TVec3f dashPos;
            bool hitDash = false;
            for (u32 i = 0; i < count; i++) {
                if (hits.getSortedPoly(&dashPos, &dashTriangle, i)) {
                    if (isWaterPolygon(&dashTriangle)) {
                        continue;
                    }
                    TVec3f backPos(dashPos - *dashTriangle.getNormal(0) * 5.0f);
                    TVec3f normal(*dashTriangle.getNormal(0) * 35.0f);
                    if (fn_800382E0(backPos, normal)) {
                        continue;
                    }
                }
                hitDash = true;
                break;
            }
            count = hits.getNearPolyOnLineSort(start, start, step, nullptr);
            Triangle mapTriangle;
            TVec3f mapPos;
            bool hitMap = false;
            for (u32 i = 0; i < count; i++) {
                if (hits.getSortedPoly(&mapPos, &mapTriangle, i)) {
                    if (isWaterPolygon(&mapTriangle)) {
                        continue;
                    }
                    TVec3f backPos(mapPos - *mapTriangle.getNormal(0) * 5.0f);
                    TVec3f normal(*mapTriangle.getNormal(0) * 35.0f);
                    if (fn_800382E0(backPos, normal)) {
                        continue;
                    }
                }
                hitMap = true;
                break;
            }
            if (hitMap && hitDash) {
                f32 mapDistance = mapPos.distance(rStart);
                if (dashPos.distance(rStart) < mapDistance) {
                    if (pPos) *pPos = dashPos;
                    if (pTriangle) *pTriangle = dashTriangle;
                } else {
                    if (pPos) *pPos = mapPos;
                    if (pTriangle) *pTriangle = mapTriangle;
                }
                return true;
            }
            if (hitMap) {
                if (pPos) *pPos = mapPos;
                if (pTriangle) *pTriangle = mapTriangle;
                return true;
            }
            if (hitDash) {
                if (pPos) *pPos = dashPos;
                if (pTriangle) *pTriangle = dashTriangle;
                return true;
            }
            remaining -= length;
            start += step;
        } while (!isNearZero(remaining));
        return false;
    }

};  // namespace MR

namespace Collision {
    s32 checkStrikePointToMap(const TVec3f& rPoint, HitInfo* pHit) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikePoint(rPoint, pHit);
    }

    s32 checkStrikeBallToMap(const TVec3f& rCenter, f32 radius, const CollisionPartsFilterBase* pPartsFilter,
                             const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeBall(rCenter, radius, false, pPartsFilter, pTriangleFilter);
    }

    s32 checkStrikeBallToMapWithMovingReaction(const TVec3f& rCenter, f32 radius, const CollisionPartsFilterBase* pPartsFilter,
                                               const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeBall(rCenter, radius, true, pPartsFilter, pTriangleFilter);
    }

    s32 checkStrikeBallToMapWithThickness(const TVec3f& rCenter, f32 radius, f32 thickness, const CollisionPartsFilterBase* pPartsFilter,
                                          const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeBallWithThickness(rCenter, radius, thickness, pPartsFilter,
                                                                                              pTriangleFilter);
    }

    s32 checkStrikeLineToMap(const TVec3f& rStart, const TVec3f& rDirection, s32 count, const CollisionPartsFilterBase* pPartsFilter,
                             const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rStart, rDirection, count, pPartsFilter, pTriangleFilter);
    }

    s32 checkStrikeLineToSunshade(const TVec3f& rStart, const TVec3f& rDirection, s32 count, const CollisionPartsFilterBase* pPartsFilter,
                                  const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(1)->checkStrikeLine(rStart, rDirection, count, pPartsFilter, pTriangleFilter);
    }

    const HitInfo* getStrikeInfoMap(u32 index) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(index);
    }

    u32 getStrikeInfoNumMap() {
        return ::getStrikeInfoNumCategory(0);
    }
};  // namespace Collision
