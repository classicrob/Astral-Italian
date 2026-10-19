#include "Game/Util/MapUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Map/CollisionCategorizedKeeper.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util/TriangleFilter.hpp"

namespace {
    NO_INLINE u32 getStrikeInfoNumCategory(s32 category) {
        return MR::getCollisionDirector()->getCategoryKeeper(category)->mHitCount;
    }
}  // namespace

namespace Collision {
    s32 checkStrikePointToDashYoshiStepAndMap(const TVec3f& rPoint, HitInfo* pHitInfo) {
        // NONMATCHING: target calls checkStrikePoint(vec, info, s32, s32); header only declares the 2-arg form (Map lane)
        s32 dashResult = MR::getCollisionDirector()->getCategoryKeeper(4)->checkStrikePoint(rPoint, pHitInfo);
        s32 mapResult = MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikePoint(rPoint, pHitInfo);
        return dashResult + mapResult;
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

    s32 checkStrikeBallToDashYoshiStepAndMap(const TVec3f& rCenter, f32 radius, const CollisionPartsFilterBase* pPartsFilter,
                                             const TriangleFilterBase* pTriangleFilter) {
        s32 dashResult = MR::getCollisionDirector()->getCategoryKeeper(4)->checkStrikeBall(rCenter, radius, false, pPartsFilter, pTriangleFilter);
        s32 mapResult = MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeBall(rCenter, radius, false, pPartsFilter, pTriangleFilter);
        return dashResult + mapResult;
    }

    s32 checkStrikeBallToDashYoshiStepAndMapWithThickness(const TVec3f& rCenter, f32 radius, f32 thickness,
                                                          const CollisionPartsFilterBase* pPartsFilter, const TriangleFilterBase* pTriangleFilter) {
        s32 dashResult = MR::getCollisionDirector()->getCategoryKeeper(4)->checkStrikeBallWithThickness(rCenter, radius, thickness, pPartsFilter, pTriangleFilter);
        s32 mapResult = MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeBallWithThickness(rCenter, radius, thickness, pPartsFilter, pTriangleFilter);
        return dashResult + mapResult;
    }

    s32 checkStrikeLineToMap(const TVec3f& rStart, const TVec3f& rDirection, s32 count, const CollisionPartsFilterBase* pPartsFilter,
                             const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rStart, rDirection, count, pPartsFilter, pTriangleFilter);
    }

    s32 checkStrikeLineToDashYoshiStep(const TVec3f& rStart, const TVec3f& rDirection, s32 count, const CollisionPartsFilterBase* pPartsFilter,
                                       const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(4)->checkStrikeLine(rStart, rDirection, count, pPartsFilter, pTriangleFilter);
    }

    s32 checkStrikeLineToDashYoshiStepAndMap(const TVec3f& rStart, const TVec3f& rDirection, s32 count, const CollisionPartsFilterBase* pPartsFilter,
                                             const TriangleFilterBase* pTriangleFilter) {
        s32 dashResult = MR::getCollisionDirector()->getCategoryKeeper(4)->checkStrikeLine(rStart, rDirection, count, pPartsFilter, pTriangleFilter);
        s32 mapResult = MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rStart, rDirection, count, pPartsFilter, pTriangleFilter);
        return dashResult + mapResult;
    }

    s32 checkStrikeLineToSunshade(const TVec3f& rStart, const TVec3f& rDirection, s32 count, const CollisionPartsFilterBase* pPartsFilter,
                                  const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(1)->checkStrikeLine(rStart, rDirection, count, pPartsFilter, pTriangleFilter);
    }

    void addToGlobal(CollisionParts* pParts) {
        MR::getCollisionDirector()->getCategoryKeeper(0)->addToGlobal(pParts);
    }

    void removeToGlobal(CollisionParts* pParts) {
        MR::getCollisionDirector()->getCategoryKeeper(0)->removeFromGlobal(pParts);
    }

    const HitInfo* getStrikeInfoMap(u32 index) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(index);
    }

    const HitInfo* getStrikeInfoDashYoshiStep(u32 index) {
        return MR::getCollisionDirector()->getCategoryKeeper(4)->getStrikeInfo(index);
    }

    const HitInfo* getStrikeInfoDashYoshiStepAndMap(u32 index) {
        if (getStrikeInfoNumCategory(4) > index) {
            return MR::getCollisionDirector()->getCategoryKeeper(4)->getStrikeInfo(index);
        }

        u32 dashNum = getStrikeInfoNumCategory(4);
        return MR::getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(index - dashNum);
    }

    u32 getStrikeInfoNumMap() {
        return getStrikeInfoNumCategory(0);
    }
};  // namespace Collision
