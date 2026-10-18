#pragma once

#include "Game/Effect/SoundParticleCallBack.hpp"

class PeachCastleFireParticleCallBack : public SoundParticleCallBack {
public:
    PeachCastleFireParticleCallBack(LiveActor* pHost);

    virtual void execute(JPABaseEmitter* pEmitter, JPABaseParticle* pParticle);
};
