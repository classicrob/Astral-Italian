#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class CaveInParts;

class CaveInCube : public LiveActor {
public:
    CaveInCube(const char*);

    virtual ~CaveInCube();
    virtual void init(const JMapInfoIter&) override;
    void initRotate();
    void calcDebrisMtx(CaveInParts*);
    void initDebris();
    bool getUnusedDebris(u32, CaveInParts**);
    void exeGenerate();

    /* 0x90 */ TVec3f* mRotVecs;
    /* 0x94 */ TPos3f mRotMtx;  // translation column at 0xA0/0xB0/0xC0
    /* 0xC4 */ s32 mMaxNum;
    /* 0xC8 */ s32 mInterval;
    /* 0xCC */ s32 mRes;
    /* 0xD0 */ s32 mNumGenerated;
    /* 0xD4 */ s32 mTypeIdx;
    /* 0xD8 */ CaveInParts** mDebris0;
    /* 0xDC */ CaveInParts** mDebris1;
    /* 0xE0 */ CaveInParts** mDebris2;
};
