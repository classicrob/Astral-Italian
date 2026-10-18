#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class MapPartsRailMover;
class SpringValue;

class CloudStep : public LiveActor {
public:
    CloudStep(const char*);

    virtual ~CloudStep();
    virtual void init(const JMapInfoIter&) override;
    virtual void control() override;
    virtual void calcAndSetBaseMtx() override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*) override;
    void exeAppear();
    void exeDisappear();
    void startAppear();
    void startDisappear();

    /* 0x90 */ MapPartsRailMover* mRailMover;
    /* 0x94 */ SpringValue* mSpring;
    /* 0x98 */ TVec3f mClipPos;
    /* 0xA4 */ s32 mBounceTimer;
    /* 0xA8 */ bool mIsHeavy;
};
