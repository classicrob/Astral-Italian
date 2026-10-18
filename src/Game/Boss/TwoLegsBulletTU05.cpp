#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvTwoLegsStateChangePos {
    NEW_NERVE(TwoLegsStateChangePosNrvStandUp, TwoLegsStateChangePos, StandUp);
    NEW_NERVE(TwoLegsStateChangePosNrvJump, TwoLegsStateChangePos, Jump);
    NEW_NERVE(TwoLegsStateChangePosNrvHide, TwoLegsStateChangePos, Hide);
    NEW_NERVE(TwoLegsStateChangePosNrvLand, TwoLegsStateChangePos, Land);
    NEW_NERVE(TwoLegsStateChangePosNrvSitDown, TwoLegsStateChangePos, SitDown);
};  // namespace NrvTwoLegsStateChangePos

TwoLegsStateChangePos::TwoLegsStateChangePos(TwoLegs* pHost) : ActorStateBase< TwoLegs >("State[\x97\xa7\x82\xbf\x88\xca\x92u\x95\xcf\x8dX]", pHost) {}

TwoLegsStateChangePos::~TwoLegsStateChangePos() {}

void TwoLegsStateChangePos::init() {
    initNerve(&NrvTwoLegsStateChangePos::TwoLegsStateChangePosNrvJump::sInstance, 0);
}

void TwoLegsStateChangePos::appear() {
    mIsDead = false;
    mHost->jointTrackOff();
    if (mHost->mIsUp) {
        setNerve(&NrvTwoLegsStateChangePos::TwoLegsStateChangePosNrvJump::sInstance);
        return;
    }
    setNerve(&NrvTwoLegsStateChangePos::TwoLegsStateChangePosNrvStandUp::sInstance);
}

bool TwoLegsStateChangePos::isRunningAway() {
    bool ret = false;
    if ((isNerve(&NrvTwoLegsStateChangePos::TwoLegsStateChangePosNrvJump::sInstance) && MR::isGreaterStep(this, 24)) ||
        isNerve(&NrvTwoLegsStateChangePos::TwoLegsStateChangePosNrvHide::sInstance)) {
        ret = true;
    }
    return ret;
}

void TwoLegsStateChangePos::exeStandUp() {
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        mHost->startActionUp();
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvTwoLegsStateChangePos::TwoLegsStateChangePosNrvJump::sInstance);
    }
}

void TwoLegsStateChangePos::exeJump() {
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        MR::startAction(mHost, "ChangeSideJump");
        mHost->disableCamera();
    }
    if (MR::isStep(this, 43)) {
        setNerve(&NrvTwoLegsStateChangePos::TwoLegsStateChangePosNrvHide::sInstance);
    }
}

void TwoLegsStateChangePos::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(mHost);
    }
    if (MR::isGreaterStep(this, 60) && !MR::isPlayerInRush()) {
        setNerve(&NrvTwoLegsStateChangePos::TwoLegsStateChangePosNrvLand::sInstance);
    }
}

void TwoLegsStateChangePos::exeLand() {
    if (MR::isFirstStep(this)) {
        mHost->resetCamera();
        const TVec3f* pPlayer = MR::getPlayerPos();
        TVec3f dir;
        dir.sub(mHost->getBodyDir()->mPos, *pPlayer);
        mHost->placeOnBodyPlane(dir);
        MR::showModel(mHost);
        MR::startAction(mHost, "RunAwayLand");
    }
    if (MR::isActionEnd(mHost)) {
        if (mHost->isPhase0()) {
            setNerve(&NrvTwoLegsStateChangePos::TwoLegsStateChangePosNrvSitDown::sInstance);
            return;
        }
        kill();
    }
}

void TwoLegsStateChangePos::exeSitDown() {
    if (MR::isFirstStep(this)) {
        mHost->startActionDown();
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}
