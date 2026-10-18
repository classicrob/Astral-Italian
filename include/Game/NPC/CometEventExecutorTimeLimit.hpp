#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/NPC/CometEventExecutor.hpp"

class TimeLimitLayout;
class CometEventExecutorTimeLimit : public LiveActor, public CometEventExecutor {
public:
    CometEventExecutorTimeLimit();
    virtual ~CometEventExecutorTimeLimit();
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

    /* 0x94 */ TimeLimitLayout* mTimeLimitLayout;
    /* 0x98 */ s32 mTimeLimit;
};
