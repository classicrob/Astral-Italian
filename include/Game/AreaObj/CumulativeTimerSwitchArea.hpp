#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class CumulativeTimerSwitchArea : public AreaObj {
public:
    CumulativeTimerSwitchArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();

    /* 0x48 */ s32 mTimer;
};
