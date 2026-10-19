#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/BreakModel.hpp"

namespace NrvTwoLegsStateExplosion {
    NEW_NERVE(TwoLegsStateExplosionNrvBlurWait, TwoLegsStateExplosion, BlurWait);
    NEW_NERVE(TwoLegsStateExplosionNrvDemo, TwoLegsStateExplosion, Demo);
    NEW_NERVE(TwoLegsStateExplosionNrvExplosion, TwoLegsStateExplosion, Explosion);
    NEW_NERVE(TwoLegsStateExplosionNrvEnd, TwoLegsStateExplosion, End);
};  // namespace NrvTwoLegsStateExplosion

TwoLegsStateExplosion::TwoLegsStateExplosion(TwoLegs* pHost) : ActorStateBase< TwoLegs >("State[\x94\x9a\x94\xad\x83""f\x83\x82]", pHost) {}

TwoLegsStateExplosion::~TwoLegsStateExplosion() {}

void TwoLegsStateExplosion::init() {
    mBreakModel = MR::createBreakModel(mHost, nullptr);
    MR::registerDemoSimpleCastAll(mBreakModel);
    MR::createCenterScreenBlur();
    initNerve(&NrvTwoLegsStateExplosion::TwoLegsStateExplosionNrvBlurWait::sInstance, 0);
}

void TwoLegsStateExplosion::appear() {
    mIsDead = false;
    mHost->jointTrackOff();
    mHost->jointReset();
    setNerve(&NrvTwoLegsStateExplosion::TwoLegsStateExplosionNrvBlurWait::sInstance);
}

void TwoLegsStateExplosion::explode() {
    mHost->hideAndKillDummy();
    mBreakModel->appear();
    MR::deleteEffectAll(mHost);
    MR::emitEffect(mHost, "Explosion");
    MR::startActionSound(mHost, "Death", -1, -1, -1);
    MR::startCenterScreenBlur(120, 15.0f, 80, 5, 30);
}

void TwoLegsStateExplosion::exeBlurWait() {
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        MR::startAction(mHost, "Damage");
        MR::startActionSound(mHost, "DeathAS", -1, -1, -1);
        MR::stopStageBGM(10);
    }
    if (MR::isStep(this, 60)) {
        MR::requestStartDemoMarioPuppetable(this, mHost, "爆発", &NrvTwoLegsStateExplosion::TwoLegsStateExplosionNrvDemo::sInstance, nullptr);
    }
}

void TwoLegsStateExplosion::exeDemo() {
    if (MR::isFirstStep(this)) {
        mHost->resetToInitPos();
        MR::startAction(mHost, "Explosion");
#ifdef GALAXY_NATIVE
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, "Explosion", 0, 0, 1.0f);
#else
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, "Explosion", nullptr, 0, 1.0f);
#endif
        MR::overlayWithPreviousScreen(2);
        MR::hidePlayer();
        LiveActor* pCarry = MR::getPlayerCarrySensorActor();
        if (pCarry != nullptr) {
            MR::hideModelIfShown(pCarry);
        }
        MR::forceDeleteEffect(mHost, "Weaken1st");
        MR::forceDeleteEffect(mHost, "Weaken2nd");
        MR::setPlayerPosAndWait("二脚ボス爆発デモ後座標", false);
    }
    if (MR::isStep(this, 180)) {
        MR::deleteEffect(mHost, "Eye");
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvTwoLegsStateExplosion::TwoLegsStateExplosionNrvExplosion::sInstance);
    }
}

void TwoLegsStateExplosion::exeExplosion() {
    if (MR::isFirstStep(this)) {
        explode();
    }
    if (MR::isDead(mBreakModel)) {
        MR::requestAppearPowerStarFromBelow(mHost);
        MR::endDemo(mHost, "\x94\x9a\x94\xad");
        setNerve(&NrvTwoLegsStateExplosion::TwoLegsStateExplosionNrvEnd::sInstance);
    }
}

void TwoLegsStateExplosion::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::endAnimCamera(mHost, mHost->mCameraInfo, "Explosion", -1, true);
    }
    if (MR::isEndPowerStarAppearDemo(mHost)) {
        MR::overlayWithPreviousScreen(2);
        MR::showPlayer();
        LiveActor* pCarry = MR::getPlayerCarrySensorActor();
        if (pCarry != nullptr) {
            MR::showModelIfHidden(pCarry);
        }
        kill();
    }
}
