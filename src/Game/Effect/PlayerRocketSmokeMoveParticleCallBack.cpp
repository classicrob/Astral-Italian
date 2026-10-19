#include "Game/Effect/PlayerRocketSmokeMoveParticleCallBack.hpp"

PlayerRocketSmokeMoveParticleCallBack::PlayerRocketSmokeMoveParticleCallBack(LiveActor* host, const char*, s32)
    : SoundParticleCallBack(host, "dummy", 0), mState(0) {
}

void PlayerRocketSmokeMoveParticleCallBack::execute(JPABaseEmitter*, JPABaseParticle*) {
}
