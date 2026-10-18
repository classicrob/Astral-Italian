#include "Game/Effect/PeachCastleFireParticleCallBack.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

PeachCastleFireParticleCallBack::PeachCastleFireParticleCallBack(LiveActor* pHost)
    : SoundParticleCallBack(pHost, "Fire", 10) {
}

void PeachCastleFireParticleCallBack::execute(JPABaseEmitter*, JPABaseParticle* pParticle) {
    if (MR::isHiddenModel(mHost)) {
        return;
    }
    // 0x4: particle status bit (no named JPAPtclStts entry in our JPA headers yet)
    if (pParticle->checkStatus(0x4)) {
        return;
    }
    if (mId != pParticle->getAge()) {
        return;
    }
    MR::startActionSound(mHost, mName, -1, -1, -1);
}
