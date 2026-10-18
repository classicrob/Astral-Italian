#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class CollisionParts;
class PhantomTorch;
class RisePictureBookPlanet : public LiveActor {
public:
    RisePictureBookPlanet(const char*);
    virtual ~RisePictureBookPlanet();
    virtual void init(const JMapInfoIter&);
    virtual void calcAnim();
    void exeWait();
    void exeMove();
    void exeReverse();
    void calcFirePos();
    CollisionParts** mParts;
    PhantomTorch** mTorches;
};
