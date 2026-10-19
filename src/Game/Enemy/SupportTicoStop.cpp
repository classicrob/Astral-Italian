#include "Game/Util.hpp"
#include "Game/Enemy/SupportTicoStop.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"

namespace NrvSupportTicoStop {
    NEW_NERVE(SupportTicoStopNrvDoNothing, SupportTicoStop, DoNothing);
    NEW_NERVE(SupportTicoStopNrvTouch, SupportTicoStop, Touch);
    NEW_NERVE(SupportTicoStopNrvEndTouch, SupportTicoStop, EndTouch);
};  // namespace NrvSupportTicoStop

SupportTicoStop::SupportTicoStop(LiveActor* pHost, AnimScaleController* pScaleController) : NerveExecutor("スターポインタ停止") {
    mHost = pHost;
    mScaleController = pScaleController;
    mCounter = 0;
    mIsTouchStarted = false;
    mIsTouching = false;
    mHasPointerEffect = false;
    initNerve(&NrvSupportTicoStop::SupportTicoStopNrvDoNothing::sInstance, 0);
    if (MR::isExistEffectKeeper(pHost) && !MR::isRegisteredEffect(pHost, "Touch")) {
        mHasPointerEffect = true;
        MR::addEffect(pHost, "PointerTouch");
    }
}

bool SupportTicoStop::tryAttachTouch() {
    if (MR::attachSupportTicoToTarget(mHost)) {
        if (!isNerve(&NrvSupportTicoStop::SupportTicoStopNrvTouch::sInstance)) {
            mIsTouchStarted = true;
            setNerve(&NrvSupportTicoStop::SupportTicoStopNrvTouch::sInstance);
        }
        return true;
    }
    if (isNerve(&NrvSupportTicoStop::SupportTicoStopNrvTouch::sInstance)) {
        return true;
    }
    mIsTouching = true;
    return false;
}

bool SupportTicoStop::tryTouch() {
    if (isNerve(&NrvSupportTicoStop::SupportTicoStopNrvTouch::sInstance)) {
        setNerve(&NrvSupportTicoStop::SupportTicoStopNrvEndTouch::sInstance);
        return true;
    }
    return false;
}

bool SupportTicoStop::isTouching() const {
    return isNerve(&NrvSupportTicoStop::SupportTicoStopNrvTouch::sInstance);
}

// NONMATCHING: branch and store ordering (96%)
void SupportTicoStop::exeTouch() {
    if (getNerveStep() == 0) {
        MR::startDPDHitSound();
        mCounter = 0;
        if (MR::isExistEffectKeeper(mHost)) {
            MR::emitEffect(mHost, mHasPointerEffect ? "PointerTouch" : "Touch");
        }
        if (MR::isExistBck(mHost, nullptr)) {
            MR::setBckRate(mHost, 0.0f);
        }
        if (mScaleController != nullptr) {
            mScaleController->startDpdHitVibration();
        }
    }
    mIsTouchStarted = false;
    mIsTouching = false;
    MR::startDPDFreezeLevelSound(mHost);
    MR::zeroVelocity(mHost);
    if (MR::attachSupportTicoToTarget(mHost)) {
        mCounter = 0;
    } else {
        mCounter++;
    }
    if (mCounter > 0) {
        setNerve(&NrvSupportTicoStop::SupportTicoStopNrvEndTouch::sInstance);
    }
}

// NONMATCHING: branch and store ordering (95%)
void SupportTicoStop::exeEndTouch() {
    if (getNerveStep() == 0) {
        mIsTouching = true;
        if (mScaleController != nullptr) {
            mScaleController->startAnim();
        }
        if (MR::isExistEffectKeeper(mHost)) {
            MR::deleteEffect(mHost, mHasPointerEffect ? "PointerTouch" : "Touch");
        }
        if (MR::isExistBck(mHost, nullptr)) {
            MR::setBckRate(mHost, 1.0f);
        }
    } else {
        mIsTouchStarted = false;
        mIsTouching = false;
    }
    setNerve(&NrvSupportTicoStop::SupportTicoStopNrvDoNothing::sInstance);
}

void SupportTicoStop::exeDoNothing() {
    mIsTouchStarted = false;
    mIsTouching = false;
}

#pragma push
#pragma dont_inline on
void fn_80203850(TVec3f* pDst, const LiveActor* pTarget) {
    TVec3f sp8(*MR::getPlayerPos());
    sp8 -= pTarget->mPosition;
    if (PSVECMag(&sp8) < 800.0f) {
        MR::vecKillElement(sp8, pTarget->mGravity, &sp8);
        MR::normalizeOrZero(&sp8);
        MR::turnVecToVecDegree(pDst, *pDst, sp8, 2.865f, pTarget->mGravity);
    }
}
#pragma pop
