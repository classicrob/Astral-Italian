#pragma once
#include "Game/Screen/LayoutActor.hpp"
class CounterLayoutAppearer;
class CountUpPaneRumbler;
class DestroyCounter : public LayoutActor {
public:
    DestroyCounter(s32 maximum);
    virtual ~DestroyCounter();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    void activate();
    void updateCount();
    void exeAppear();
    void exeWait();
    void exeDisappear();
    s32 mMaximum;
    s32 mDisplayedCount;
    s32 mCount;
    s32 mCountDelay;
    CounterLayoutAppearer* mAppearer;
    CountUpPaneRumbler* mRumbler;
    bool mIsActive;
};
