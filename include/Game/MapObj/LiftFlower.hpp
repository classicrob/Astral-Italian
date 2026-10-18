#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class DemoObj;

class LiftFlower : public LiveActor {
public:
    LiftFlower(const char*);

    virtual ~LiftFlower();
    virtual void init(const JMapInfoIter&) override;
    virtual void control() override;
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*) override;
    void exeWait();
    void exeMoveUp();
    void exeMoveStop();
    void exeDisappearSign();
    void exeDisappear();
    void exeHidden();
    void exeAppear();

    /* 0x90 */ TVec3f mHomePos;
    /* 0x9C */ f32 mMoveHeight;
    /* 0xA0 */ s32 mBounceTimer;
    /* 0xA4 */ s32 mStopStep;
    /* 0xA8 */ DemoObj* mDemoObj;
    /* 0xAC */ TPos3f mDemoMtx;
};
