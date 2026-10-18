#pragma once

#include "Game/LiveActor/LiveActor.hpp"

/// @brief Takobo ink blob (SMG2 only, no SMG1 source). Flies out of a TakoboInkHolder, sticks to the ground, and breaks when hit.
class TakoboInk : public LiveActor {
public:
    TakoboInk(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void start(const TVec3f& rPos, const TVec3f& rVel, bool isBindOff);
    void exeWait();
};
