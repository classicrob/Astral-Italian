#include "Game/Effect/PlayerRocketSmokeWaitParticleCallBack.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

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
