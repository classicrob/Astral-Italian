#pragma once

#include "Game/Screen/LayoutActor.hpp"

class SubMeterLayout : public LayoutActor {
public:
    enum State {
        State_Hidden = 0,
        State_Appear = 1,
        State_Wait = 2,
        State_Danger = 3,
        State_FrameOut = 4,
        State_AutoHide = 5,
    };

    SubMeterLayout(const char*, const char*);

    virtual void init(const JMapInfoIter&);
    virtual void control();

    void setLifeRatio(f32);
    void requestDeactivate();
    void requestAppear();
    void requestFrameIn();
    void requestFrameOut();

    u8 mState;
    u8 _21;
    u16 mLife;
    u16 _24;
    u16 mFullLifeFrames;
};
