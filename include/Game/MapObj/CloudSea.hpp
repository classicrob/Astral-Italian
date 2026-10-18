#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class CloudSea : public MapObjActor {
public:
    CloudSea(const char*);
    virtual ~CloudSea();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    TVec3f mOffset;
};
