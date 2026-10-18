#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

class ElectricPressureBullet : public LiveActor {
public:
    ElectricPressureBullet(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void shotElectricBullet(LiveActor*, const TPos3f&, const f32&, bool);
    void exeFly();

    /* 0x90 */ TVec3f _8C;
    /* 0x9C */ LiveActor* _98;
    /* 0xA0 */ f32 _9C;
    /* 0xA4 */ bool _A4;
};
