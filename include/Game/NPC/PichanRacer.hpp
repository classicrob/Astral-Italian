#pragma once

#include "Game/NPC/NPCActor.hpp"
#include "Game/Map/RaceManager.hpp"

class TakeOutStar;
class RaceDataReader;

class PichanRacer : public NPCActor, public AbstractRacer {
public:
    PichanRacer(const char*);
    virtual ~PichanRacer();
    virtual void init(const JMapInfoIter&);
    virtual void prepRacer(const RaceManager*);
    virtual void startRacer();
    virtual bool updateRacer(const RaceManager*);
    virtual bool goalRacer();
    virtual void loseRacer();
    virtual void resetRacer(const RaceManager*);
    virtual void exitRacer();
    virtual void requestMovementOnRace();

    bool branchFunc(u32);
    bool eventFunc(u32);
    void exeWait();
    void exeTalk();
    void exeStart();
    void exeStartEnd();
    void exeGoal();

    s32 mColor;
    RaceDataReader* mRaceDataReader;
    TakeOutStar* mTakeOutStar;
    u32 mUnused194;
};
