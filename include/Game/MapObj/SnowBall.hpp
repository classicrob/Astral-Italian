#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class SnowBall : public LiveActor {
public:
    SnowBall(const char*);

    virtual ~SnowBall();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void setRadius(f32);
    void tryDropSnowStep(const TVec3f&);
    void launch(const TVec3f&, const TVec3f&, bool);
    void breakAndWait();
    void resetRoll();
    bool updateBindedGroundRoll(bool);
    void exeHide();
    void exeBreak();
    void exeThaw();
    void exeBlow();
    void exeRoll();
    void exeFall();
    void exeWait();

    bool isBlowOrThawNerve() const;
    bool isHideBreakOrThawNerve() const;

    /* 0x90 */ TPos3f mBaseMtx;
    /* 0xC0 */ TQuat4f mQuat;
    /* 0xD0 */ TVec3f mLastStepPos;
    /* 0xDC */ TVec3f mBasePos;
    /* 0xE8 */ TVec3f mPrevPos;
    /* 0xF4 */ s32 mGroundCount;
    /* 0xF8 */ s32 mStopCount;
    /* 0xFC */ f32 mSpeedTarget;
    /* 0x100 */ f32 mRadius;
    /* 0x104 */ f32 mBaseRadius;
    /* 0x108 */ s32 mStepCount;
    /* 0x10C */ bool mCanRespawn;
};
