#pragma once

#include "Game/Effect/MultiEmitterParticleCallBack.hpp"

class LiveActor;

class SoundParticleCallBack : public MultiEmitterParticleCallBack {
public:
    SoundParticleCallBack(LiveActor* pHost, const char* pName, s32 id);

    virtual ~SoundParticleCallBack();

    virtual void execute(JPABaseEmitter* pEmitter, JPABaseParticle* pParticle);

    LiveActor* mHost;
    const char* mName;
    s32 mId;
};
