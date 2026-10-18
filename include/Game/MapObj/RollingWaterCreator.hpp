#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JMapInfoIter;
class LiveActorGroup;
class RollingWater;

/// @brief Spawner for RollingWater (SMG2-only). Owns a LiveActorGroup of RollingWater actors.
class RollingWaterCreator : public LiveActor {
public:
    RollingWaterCreator(const char*);

    virtual ~RollingWaterCreator();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait() NO_INLINE;

    /* 0x90 */ LiveActorGroup* mRollingWaterGroup;
    /* 0x94 */ s32 mInterval;
    /* 0x98 */ s32 mDelay;
    /* 0x9C */ s32 mCounter;
};
