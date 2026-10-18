#pragma once

#include "Game/MapObj/MapObjActor.hpp"

class DrumTrampoline : public MapObjActor {
public:
    DrumTrampoline(const char*);

    virtual ~DrumTrampoline();
    virtual void init(const JMapInfoIter&) override;
    virtual void control() override;
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*) override;
    void exeBound();

    static bool tryGetGroundNormal(TVec3f*, LiveActor*, HitSensor*);

    /* 0xD0 */ bool mIsBounce;
};
