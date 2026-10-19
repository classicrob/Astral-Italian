#include "Game/Boss/DinoPackunStateAwake.hpp"
#include "Game/Boss/DinoPackun.hpp"
#include "Game/Boss/DinoPackunTail.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvDinoPackunStateAwake {
    NEW_NERVE(DinoPackunStateAwakeNrvFind, DinoPackunStateAwake, Find);
};  // namespace NrvDinoPackunStateAwake

DinoPackunStateAwake::DinoPackunStateAwake(DinoPackun* pHost) : ActorStateBase< DinoPackun >("ディノパックン気付く", pHost) {
    initNerve(GET_NERVE(DinoPackunStateAwake, DinoPackunStateAwakeNrvFind));
}

void DinoPackunStateAwake::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(DinoPackunStateAwake, DinoPackunStateAwakeNrvFind));
}

void DinoPackunStateAwake::kill() {
    mIsDead = true;
    getHost()->mTail->mAnimationFollowRate = 0.0f;
}

void DinoPackunStateAwake::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(getHost(), "Find");
        MR::startActionSound(getHost(), "BvDPakkunFind");
        MR::startActionSound(getHost(), "BmDPakkunSlaver");
    }

    f32 rate = MR::calcNerveEaseOutRate(this, 30);
    getHost()->mTail->mAnimationFollowRate = 0.015f * rate;
    MR::turnDirectionToPlayerDegree(getHost(), &getHost()->mFront, 4.0f);
    getHost()->updateNormalVelocity();

    if (MR::isActionEnd(getHost())) {
        kill();
    }
}

DinoPackunStateAwake::~DinoPackunStateAwake() {
}
