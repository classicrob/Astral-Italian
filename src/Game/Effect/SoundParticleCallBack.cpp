#include "Game/Effect/SoundParticleCallBack.hpp"
#include "Game/Effect/PlayerRocketSmokeMoveParticleCallBack.hpp"
#include "Game/Effect/PlayerRocketSmokeWaitParticleCallBack.hpp"
#include "Game/Effect/PeachCastleFireParticleCallBack.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

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

PlayerRocketSmokeMoveParticleCallBack::PlayerRocketSmokeMoveParticleCallBack(LiveActor* host, const char*, s32)
    : SoundParticleCallBack(host, "dummy", 0), mState(0) {
}

SoundParticleCallBack::~SoundParticleCallBack() {
}

void PlayerRocketSmokeMoveParticleCallBack::execute(JPABaseEmitter*, JPABaseParticle*) {
}

PlayerRocketSmokeWaitParticleCallBack::PlayerRocketSmokeWaitParticleCallBack(LiveActor* pHost, const char* pName,
                                                                              s32 id)
    : SoundParticleCallBack(pHost, "dummy", 0) {
    mState = 0;
}

void PlayerRocketSmokeWaitParticleCallBack::execute(JPABaseEmitter*, JPABaseParticle* pParticle) {
    if (mId != pParticle->getAge()) {
        return;
    }

    const char* pName = nullptr;
    switch (mState) {
    case 0:
        pName = "SteamWaitA";
        break;
    case 1:
        pName = "SteamWaitB";
        break;
    case 2:
        pName = "SteamWaitC";
        break;
    case 3:
        pName = "SteamWaitD";
        break;
    }

    mState = (mState == 3) ? 0 : mState + 1;

    if (pName == nullptr) {
        return;
    }
    MR::startActionSound(mHost, pName, -1, -1, -1);
}

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
