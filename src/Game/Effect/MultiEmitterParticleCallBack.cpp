#include "Game/Effect/MultiEmitterParticleCallBack.hpp"

MultiEmitterParticleCallBack::MultiEmitterParticleCallBack() {
}

// PLACEHOLDER(enemies): forces weak MultiEmitterParticleCallBack destructor emission; resolve when the real user is sourced
void MultiEmitterParticleCallBack_FORCE_MATCH(MultiEmitterParticleCallBack* pCallBack) {
    pCallBack->MultiEmitterParticleCallBack::~MultiEmitterParticleCallBack();
}
