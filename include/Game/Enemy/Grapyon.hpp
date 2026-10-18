#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class Grapyon : public LiveActor {
public:
    Grapyon(const char*);
    virtual ~Grapyon();

    /* 0x90 */ void* mUnk90;
    /* 0x94 */ TPos3f mMtx;
    /* 0xC4 */ void* mUnkC4;
    /* 0xC8 */ void* mUnkC8;
    /* 0xF0 */ TVec3f mVec;
    /* 0xFC */ bool mUnkFC;
};
