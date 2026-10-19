#pragma once

#include "Game/Effect/MultiEmitterParticleCallBack.hpp"
#include <revolution.h>

class SpinPullParticleCallBack : public MultiEmitterParticleCallBack {
public:
    SpinPullParticleCallBack();

    virtual void execute(JPABaseEmitter*, JPABaseParticle*);

    /* 0x04 */ f32 mPullScale;
};

class MerameraParticleCallBack : public SpinPullParticleCallBack {
public:
    virtual void execute(JPABaseEmitter*, JPABaseParticle*);
};