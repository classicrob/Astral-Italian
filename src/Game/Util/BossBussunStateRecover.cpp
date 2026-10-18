#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossBussunStateRecover {
    NEW_NERVE(BossBussunStateRecoverNrvRecover, BossBussunStateRecover, Recover);
};  // namespace NrvBossBussunStateRecover

BossBussunStateRecover::BossBussunStateRecover(BossBussun* pHost) : ActorStateBase< BossBussun >("ボスブッスン復活", pHost) {
    initNerve(GET_NERVE(BossBussunStateRecover, BossBussunStateRecoverNrvRecover), 0);
}

void BossBussunStateRecover::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(BossBussunStateRecover, BossBussunStateRecoverNrvRecover));
    fn_80098D20(mHost);
}

void BossBussunStateRecover::exeRecover() {
    if (MR::isFirstStep(this)) {
        if (mHost->mHitCount <= 1) {
            MR::startAction(mHost, "RecoverAngry");
        } else {
            MR::startAction(mHost, "Recover");
        }
        MR::zeroVelocity(mHost);
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

BossBussunStateRecover::~BossBussunStateRecover() {
}
