#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/MapObj/MapPartsFloatingForce.hpp"

class SimpleFloaterObj : public LiveActor {
public:
    SimpleFloaterObj(const char*);

    virtual ~SimpleFloaterObj();
    virtual void init(const JMapInfoIter&);

    void exeWait();
    inline bool isVelocityClose() {
        return mFloatingForce->mMoveVelocity.length() < 0.3f;
    }

    const char* mObjectName;
    MapPartsFloatingForce* mFloatingForce;
    TVec3f mEffectPos;
};
