#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"

CollisionPartsFilterBase* MR::createCollisionPartsFilterDelegator(bool (*filter)(const CollisionParts*)) {
    return new CollisionPartsFilterFunction(filter);
}

bool CollisionPartsFilterFunction::isInvalidParts(const CollisionParts* pParts) const {
    return mFilter(pParts);
}

bool CollisionPartsFilterSensorType::isInvalidParts(const CollisionParts* pParts) const {
    return pParts->mHitSensor->isType(mSensorType);
}

bool CollisionPartsFilterSensor::isInvalidParts(const CollisionParts* pParts) const {
    return pParts->mHitSensor == mSensor;
}

bool CollisionPartsFilterActor::isInvalidParts(const CollisionParts* pParts) const {
    return pParts->mHitSensor->mHost == mActor;
}
