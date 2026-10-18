#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvTwoLegsStateWait {
    NEW_NERVE(TwoLegsStateWaitNrvWait, TwoLegsStateWait, Wait);
};  // namespace NrvTwoLegsStateWait

TwoLegsStateWait::TwoLegsStateWait(TwoLegs* pHost) : ActorStateBase< TwoLegs >("State[\x83""E\x83""F\x83""C\x83g]", pHost) {}

TwoLegsStateWait::~TwoLegsStateWait() {}

void TwoLegsStateWait::init() {
    initNerve(&NrvTwoLegsStateWait::TwoLegsStateWaitNrvWait::sInstance, 0);
}

void TwoLegsStateWait::appear() {
    mIsDead = false;
    if (mHost->isPhase0()) {
        mHost->jointTrackOn();
    } else {
        mHost->jointTrackOnReturn();
    }
    setNerve(&NrvTwoLegsStateWait::TwoLegsStateWaitNrvWait::sInstance);
}

void TwoLegsStateWait::exeWait() {
    bool phase0 = mHost->isPhase0();
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        MR::startAction(mHost, phase0 ? "WaitDown" : "WaitUp");
    }
    if (!mHost->tryKillStateIfPlayerBetweenFeet(this)) {
        if (MR::isStep(this, phase0 ? 120 : 30)) {
            kill();
        }
    }
}
