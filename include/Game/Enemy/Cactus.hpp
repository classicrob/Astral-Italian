#pragma once

#include "Game/LiveActor/LiveActor.hpp"

/// @brief Cactus (SMG2 only, no SMG1 source). Stationary hazard whose body hurts on contact while in its Wait nerve.
class Cactus : public LiveActor {
public:
    Cactus(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);

    bool isNerveWait() const;
};
