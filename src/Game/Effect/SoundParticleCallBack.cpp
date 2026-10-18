#include "Game/Effect/SoundParticleCallBack.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

SoundParticleCallBack::SoundParticleCallBack(LiveActor* pHost, const char* pName, s32 id)
    : MultiEmitterParticleCallBack() {
    mHost = pHost;
    mName = pName;
    mId = id;
}

void SoundParticleCallBack::execute(JPABaseEmitter*, JPABaseParticle* pParticle) {
    if (mId != pParticle->getAge()) {
        return;
    }
    MR::startActionSound(mHost, mName, -1, -1, -1);
}
