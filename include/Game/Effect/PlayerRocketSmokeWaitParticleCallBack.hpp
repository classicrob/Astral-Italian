#pragma once

#include "Game/Effect/SoundParticleCallBack.hpp"

class PlayerRocketSmokeWaitParticleCallBack : public SoundParticleCallBack {
public:
    PlayerRocketSmokeWaitParticleCallBack(LiveActor* pHost, const char* pName, s32 id);

    virtual void execute(JPABaseEmitter* pEmitter, JPABaseParticle* pParticle);

    u8 mState;
};
