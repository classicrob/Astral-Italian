#pragma once

#include "Game/Screen/LayoutActor.hpp"
#include "Game/Screen/TimeUpLayout.hpp"

class ValueControl;

struct Timing {
    /* 0x0 */ u32 mScaleStartFrame;
    /* 0x4 */ u32 mScaleKeepFrame;
    /* 0x8 */ bool mIsScaleUp;
    /* 0x9 */ bool mIsScaleDown;
    /* 0xA */ bool _A;
};

class TimeLimitLayout : public LayoutActor {
public:
    TimeLimitLayout();

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    virtual void control();

    void setTimeLimit(u32);
    void pause();
    void resume();
    void addTimeLimit(s32);
    void setDisplayModeOnNormal(bool);
    bool isReadyToTimeUp() const;
    void resetFrame();
    void updateTextBox();
    void exeAppear();
    void exeCountDown();
    void exeScaleUp();
    void exeScaleKeep();
    void exeScaleDown();
    void exeFadeout();
    void exeTimeUpReady();
    const Timing* getCurrentTiming() const;
    bool updateNormal();

private:
    /* 0x2C */ u32 mRemainingFrames;
    /* 0x30 */ u32 mTimeLimit;
    /* 0x34 */ ValueControl* mScaleControl;
    /* 0x38 */ ValueControl* mFadeControl;
    /* 0x3C */ const Timing* mCurrentTiming;
    /* 0x40 */ bool mIsSuspend;
    /* 0x41 */ bool mDisplayOnNormal;
};
