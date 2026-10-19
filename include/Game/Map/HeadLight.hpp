#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/Color.hpp"

class HeadLight : public LiveActor {
public:
    HeadLight(const char*);
    virtual ~HeadLight();
    virtual void init(const JMapInfoIter&);
    virtual void control();

    void calcAndSetBaseMtx();

    /* 0x90 */ Color8 mColor;
    /* 0x94 */ s32 mBrightnessLevel;
};
