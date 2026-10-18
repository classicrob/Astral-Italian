#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStateCommon.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaStatePowerUpDemo {
    NEW_NERVE(KoopaStatePowerUpDemoNrvWait, KoopaStatePowerUpDemo, Wait);
    NEW_NERVE(KoopaStatePowerUpDemoNrvDemo, KoopaStatePowerUpDemo, Demo);
};  // namespace NrvKoopaStatePowerUpDemo

KoopaStatePowerUpDemo::KoopaStatePowerUpDemo(KoopaBattle* pHost) : ActorStateBase< KoopaBattle >("クッパパワーアップデモ", pHost), mIsCameraInterpolating(false) {
    MR::initAnimCamera(pHost, pHost->mCameraInfo, "DemoPowerUp");
    initNerve(&NrvKoopaStatePowerUpDemo::KoopaStatePowerUpDemoNrvWait::sInstance, 0);
}

void KoopaStatePowerUpDemo::appear() {
    mIsDead = false;
    MR::appearKoopaBattlePlanet();
    fn_8001FEF0(this, mHost, "パワーアップ", &NrvKoopaStatePowerUpDemo::KoopaStatePowerUpDemoNrvDemo::sInstance,
                &NrvKoopaStatePowerUpDemo::KoopaStatePowerUpDemoNrvWait::sInstance);
    mIsCameraInterpolating = false;
}

void KoopaStatePowerUpDemo::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::resetKoopaBattleDemo(mHost);
        MR::resetKoopaBattleToPoseA(mHost);
        MR::startAction(mHost, "DemoPowerUp");
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, "DemoPowerUp", 0, false, 1.0f);
        MR::overlayWithPreviousScreen(2);
        MR::hidePlayer();
    }
    KoopaBattle* pHost = mHost;
    if (MR::tryEndDemo(pHost, "パワーアップ", MR::isActionEnd(pHost))) {
        mIsCameraInterpolating = MR::isDemoEndWaitCameraInterpolating();
        MR::endAnimCamera(mHost, mHost->mCameraInfo, "DemoPowerUp", 0, true);
        MR::showPlayer();
        kill();
    }
}

KoopaStatePowerUpDemo::~KoopaStatePowerUpDemo() {
}
