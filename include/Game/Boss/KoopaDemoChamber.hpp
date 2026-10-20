#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/KoopaBattlePlanet.hpp"
#include "Game/Map/KoopaFinalStarPlanet.hpp"

class LiveActorGroup;
class DemoObj;
class FixedPosition;
class KoopaDemoChamber : public LiveActor {
public:
    KoopaDemoChamber(const char*);
    virtual ~KoopaDemoChamber();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    void startBreak();
    void exeBreak();
    void exeWait();
    void getDemoEndPlayerPos(MtxPtr) const;
    LiveActorGroup* mChildGroup;
    DemoObj* mBreakObj;
    FixedPosition* mPlayerPosition;
};


