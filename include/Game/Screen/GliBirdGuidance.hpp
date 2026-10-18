#pragma once

#include "Game/Screen/LayoutActor.hpp"

class GliBirdGuidance : public LayoutActor {
public:
    GliBirdGuidance(const char*);
    virtual ~GliBirdGuidance();
    virtual void init(const JMapInfoIter&);

    void start(s32);
    void hold(s32);
    void success(s32);
    void exeAppear();
    void exeStart();
    void exeHold();
    void exeSuccess();

    /* 0x2C */ s32 mMode;
};
