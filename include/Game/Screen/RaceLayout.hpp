#pragma once

#include "Game/Screen/LayoutActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvRaceLayout {
#define DECLARE_RACE_NERVE(State) \
    class RaceLayoutNrv##State : public Nerve { \
    public: RaceLayoutNrv##State(); virtual void execute(Spine*) const; \
    static RaceLayoutNrv##State sInstance; };
    DECLARE_RACE_NERVE(Hide)
    DECLARE_RACE_NERVE(Appear)
    DECLARE_RACE_NERVE(Wait)
#undef DECLARE_RACE_NERVE
}

class RaceLayout : public LayoutActor {
public:
    RaceLayout(const char*);
    virtual ~RaceLayout();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    void exeAppear();
    void exeWait();
    void playGoal();
    void playGoalWithTime(u32);
    void playLose();
    void playRank(u32);
    bool isAllAnimStopped();
    void hideAllPane();
    void setTime(u32);
    void setBestTime(u32);
};
