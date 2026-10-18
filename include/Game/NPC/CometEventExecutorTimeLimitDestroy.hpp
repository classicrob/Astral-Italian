#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/NPC/CometEventExecutor.hpp"
class BombTimerLayout;
class CometEventExecutorTimeLimitDestroy : public LiveActor, public CometEventExecutor {
public:
    CometEventExecutorTimeLimitDestroy();
    virtual ~CometEventExecutorTimeLimitDestroy();
    virtual void init(const JMapInfoIter&);
    virtual void init(s32);
    virtual void start();
    virtual void stop();
    virtual void pauseCometTimer();
    virtual void resumeCometTimer();
    virtual void showCometTimer();
    virtual void hideCometTimer();
    virtual void addCometTimer(s32);
    virtual void readdCometTimer();
    virtual void decCometTimer(s32);
    void exeStartEvent();
    u32 getTimeLimit() const;
    BombTimerLayout* mTimerLayout;
    s32 mTimeLimit;
};
