#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class SupportTicoStop;

/// @brief Spiny Cheep Cheep
class Balloonfish : public LiveActor {
public:
    Balloonfish(const char*);

    virtual void init(const JMapInfoIter& rIter);
    virtual void makeActorAppeared();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeDash();

    /* 0x90 */ AnimScaleController* mAnimScaleController;
    /* 0x94 */ SupportTicoStop* mSupportTicoStop;
    /* 0x98 */ TQuat4f mQuat;
    /* 0xA8 */ f32 mRotateAngle;
    /* 0xAC */ f32 mWaitScale;
    /* 0xB0 */ s32 mNotBoundStep;
};
