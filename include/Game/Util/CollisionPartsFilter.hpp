#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/CollisionParts.hpp"

class CollisionPartsFilterBase {
public:
    inline CollisionPartsFilterBase() {
    }

    virtual bool isInvalidParts(const CollisionParts*) const = 0;
};

class CollisionPartsFilterSensor : public CollisionPartsFilterBase {
public:
    inline CollisionPartsFilterSensor(const HitSensor* pSensor) : mSensor(pSensor) {
    }

    virtual bool isInvalidParts(const CollisionParts* pParts) const;

    /* 0x04 */ const HitSensor* mSensor;
};

class CollisionPartsFilterActor : public CollisionPartsFilterBase {
public:
    inline CollisionPartsFilterActor(const LiveActor* pActor) : mActor(pActor) {
    }

    virtual bool isInvalidParts(const CollisionParts* pParts) const;

    /* 0x04 */ const LiveActor* mActor;
};

class CollisionPartsFilterSensorType : public CollisionPartsFilterBase {
public:
    inline CollisionPartsFilterSensorType(u32 type) : mSensorType(type) {
    }

    virtual bool isInvalidParts(const CollisionParts* pParts) const;

    /* 0x04 */ u32 mSensorType;
};

template < typename T >
class CollisionPartsFilterDelegator : public CollisionPartsFilterBase {
public:
    typedef bool (T::*DelegateFilter)(const CollisionParts*);

    CollisionPartsFilterDelegator(T* pParent, DelegateFilter filter) : mParent(pParent), mFunc(filter) {
    }

    virtual bool isInvalidParts(const CollisionParts* pParts) const {
        return (mParent->*mFunc)(pParts);
    }

    /* 0x04 */ T* mParent;
    /* 0x08 */ DelegateFilter mFunc;
};

namespace MR {
    void* createCollisionPartsFilterDelegator(bool (*)(const CollisionParts*));
};  // namespace MR
