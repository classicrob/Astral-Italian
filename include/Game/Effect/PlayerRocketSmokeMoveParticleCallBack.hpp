#pragma once

#include "Game/Effect/SoundParticleCallBack.hpp"

class PlayerRocketSmokeMoveParticleCallBack : public SoundParticleCallBack {
public:
    PlayerRocketSmokeMoveParticleCallBack(LiveActor* pHost);

    virtual void execute(JPABaseEmitter* pEmitter, JPABaseParticle* pParticle);

    u8 mState;
};
