#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "JSystem/JGeometry.hpp"

class BallRailPoint {
public:
    BallRailPoint();

    /* 0x00 */ TVec3f _0;
    /* 0x0C */ TVec3f _C;
    /* 0x18 */ TVec3f _18;
    /* 0x24 */ TVec3f _24;
};

class BallRail : public LiveActor {
public:
    BallRail(const char*);

    virtual ~BallRail();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initRailPoints();
    void exeWait();
    void exeSetUp();
    void exeRun();
    bool trySetNerveRun();
    inline void exeNoBind();

    /* 0x90 */ BallRailPoint* mRailPoints;
    /* 0x94 */ HitSensor* _90;
    /* 0x98 */ TPos3f mBaseMtx;
    /* 0xC8 */ TVec3f _C8;
    /* 0xD4 */ s32 mNumPoints;
    /* 0xD8 */ f32 mAcceleration;
    /* 0xDC */ f32 mDeceleration;
    /* 0xE0 */ f32 _AC;
};
