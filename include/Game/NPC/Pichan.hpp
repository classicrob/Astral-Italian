#pragma once

#include "Game/NPC/NPCActor.hpp"
#include "Game/Map/RaceManager.hpp"

class TakeOutStar;

class Pichan : public NPCActor, public AbstractAudience, public AbstractRaceJudge {
public:
    Pichan(const char*);
    virtual ~Pichan();
    virtual void init(const JMapInfoIter&);
    virtual void prepAudience();
    virtual void startAudience();
    virtual void resetAudience();
    virtual bool isJudgeReady() const;
    virtual void exitAudience();

    bool branchFunc(u32);
    bool eventFunc(u32);
    bool animeFunc(u32);
    void exeWait();
    void exeTalk();

    TakeOutStar* mTakeOutStar;
    bool mIsTutorial;
};
