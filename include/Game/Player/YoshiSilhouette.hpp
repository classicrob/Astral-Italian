#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class Yoshi;
class YoshiSilhouette : public LiveActor {
public:
    YoshiSilhouette(Yoshi*);
    virtual ~YoshiSilhouette();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void movement();
    virtual void calcAnim();
    virtual void calcViewAndEntry();
    virtual void draw() const;
    bool checkHidden();
    void checkAppeared();
    Yoshi* mHost;
    bool mHidden;
};
