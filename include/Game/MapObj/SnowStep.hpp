#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class SnowStep : public LiveActor {
public:
    SnowStep(const char*);

    static s32 getSensorType();

    virtual ~SnowStep();
    virtual void init(const JMapInfoIter&) override;
    virtual void control() override;
    virtual void calcAndSetBaseMtx() override;
    void appearSnowStep(const TVec3f&, const TVec3f&, s32, s32, s32, f32);
    void exeExtend();
    void exeWait();
    void exeShrink();

    /* 0x90 */ TPos3f mBaseMtx;
    /* 0xC0 */ TQuat4f mQuat;
    /* 0xD0 */ s32 mExtendStep;
    /* 0xD4 */ s32 mWaitStep;
    /* 0xD8 */ s32 mShrinkStep;
    /* 0xDC */ f32 mSize;
    /* 0xE0 */ f32 mBaseScale;
};

class SnowStepHolder : public DeriveActorGroup< SnowStep > {
public:
    SnowStepHolder();

    virtual ~SnowStepHolder();
};

namespace MR {
    void createSnowStepHolder();
    bool tryAppearSnowStep(const TVec3f&, const TVec3f&, s32, s32, s32, f32);
};  // namespace MR
