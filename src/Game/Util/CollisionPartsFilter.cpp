#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"

bool CollisionPartsFilterSensorType::isInvalidParts(const CollisionParts* pParts) const {
    return pParts->mHitSensor->isType(mSensorType);
}

bool CollisionPartsFilterSensor::isInvalidParts(const CollisionParts* pParts) const {
    return pParts->mHitSensor == mSensor;
}

bool CollisionPartsFilterActor::isInvalidParts(const CollisionParts* pParts) const {
    return pParts->mHitSensor->mHost == mActor;
}
