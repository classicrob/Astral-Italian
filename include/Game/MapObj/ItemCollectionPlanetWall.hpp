#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class ItemCollectionPlanetWall : public LiveActor {
public:
    ItemCollectionPlanetWall(const char*);
    virtual ~ItemCollectionPlanetWall();
    virtual void init(const JMapInfoIter&);
    void listenDemoAction();
    void exeFadeIn();
};
