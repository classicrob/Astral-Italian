#pragma once

#include "Game/Map/RaceManager.hpp"

class ActorCameraInfo;
class RaceManager;

class PlayerRacer : public LiveActor, public AbstractRacer {
public:
    PlayerRacer(const char*);

    virtual ~PlayerRacer();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void initRacer();
    virtual void prepRacer(const RaceManager*);
    virtual void startRacer();
    virtual bool updateRacer(const RaceManager*);
    virtual bool goalRacer();
    virtual void loseRacer();
    virtual void resetRacer(const RaceManager*);
    virtual void exitRacer();
    virtual void requestMovementOnRace();

    /* 0xB4 */ u32 _B4;
    /* 0xB8 */ u32 _B8;
    /* 0xBC */ ActorCameraInfo* mCameraInfo;
    /* 0xC0 */ f32 _C0;
    /* 0xC4 */ bool _C4;
};

class RaceRail : public NameObj {
public:
    RaceRail(const char*);

    virtual ~RaceRail();
    virtual void init(const JMapInfoIter&);

    /* 0x0C */ PlayerRacer* mPlayerRacer;
};
