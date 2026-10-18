#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class CollisionObj;
class BendSwanpPlanet : public LiveActor {
public:
    BendSwanpPlanet(const char*);
    virtual ~BendSwanpPlanet();
    virtual void init(const JMapInfoIter&);
    void exeWaitA();
    void exeMoveA();
    void exeWaitB();
    void exeMoveB();
    CollisionObj** mParts;
};
