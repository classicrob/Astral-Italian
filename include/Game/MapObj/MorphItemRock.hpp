#pragma once
#include "Game/MapObj/MorphItemObjNeo.hpp"
class RollingRock;
class MorphItemRock : public MorphItemObjNeo {
public:
    MorphItemRock(const char*);
    virtual void init(const JMapInfoIter&);
    virtual int getPowerUp();
    virtual void emitAppearEffects();
    virtual void appearTransformation();
    virtual void killTransformation();
    RollingRock* mRock;
};
