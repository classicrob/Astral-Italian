#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class DrillHole : public LiveActor {
public:
    DrillHole(const char*, const char*);

    virtual ~DrillHole();
    virtual void init(const JMapInfoIter&) override;
    virtual void appear() override;
    virtual void control() override;
    virtual void calcAndSetBaseMtx() override;

    /* 0x90 */ const char* mModelName;
    /* 0x94 */ s32 mCounter;
    /* 0x98 */ TPos3f mBaseMtx;
};

class DrillHoleHolder : public NameObj {
public:
    DrillHoleHolder(const char*);

    virtual ~DrillHoleHolder();
    virtual void init(const JMapInfoIter&) override;

    /* 0x14 */ DrillHole** mHoles1;
    /* 0x18 */ DrillHole** mHoles2;
};

void createDrillHoleHolderSceneObj();
void appearDrillHoleFromArea1(const TPos3f*);
void appearDrillHoleFromArea2(const TPos3f*);
