#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class CollisionParts;
class WireCoverCage : public MapObjActor {
public:
    WireCoverCage(const char* name) : MapObjActor(name),mDrillCollision(nullptr) {}
    virtual ~WireCoverCage();
    virtual void init(const JMapInfoIter&);
    CollisionParts* mDrillCollision;
};
