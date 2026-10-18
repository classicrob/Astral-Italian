#include "Game/Boss/PoltaDemo.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/Polta.hpp"
#include "Game/Boss/PoltaActionBase.hpp"
#include "Game/Boss/PoltaArm.hpp"
#include "Game/Boss/PoltaFunction.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const s32 sBgmStartStep = 60;
    static const s32 sOpeningBlurStart = 300;
    static const s32 sOpeningBlurTime = 70;
    static const f32 sOpeningBlurOffset = 18.0f;
    static const u8 sOpeningBlurAlpha = 80;
    static const s32 sOpeningBlurFadeIn = 5;
    static const s32 sOpeningBlurFadeOut = 30;
    static const s32 sPowerUpBlurStart = 530;
    static const s32 sPowerUpBlurTime = 80;
    static const f32 sPowerUpBlurOffset = 20.0f;
    static const u8 sPowerUpBlurAlpha = 80;
    static const s32 sPowerUpBlurFadeIn = 5;
    static const s32 sPowerUpBlurFadeOut = 40;
};  // namespace

namespace NrvPoltaDemo {
    NEW_NERVE(PoltaDemoNrvTryStartDemo, PoltaDemo, TryStartDemo);
    NEW_NERVE(PoltaDemoNrvOpeningDemo, PoltaDemo, OpeningDemo);
    NEW_NERVE(PoltaDemoNrvPowerUpDemo, PoltaDemo, PowerUpDemo);
    NEW_NERVE(PoltaDemoNrvDownDemo, PoltaDemo, DownDemo);
    NEW_NERVE(PoltaDemoNrvCameraEndDemo, PoltaDemo, CameraEndDemo);
};  // namespace NrvPoltaDemo

PoltaDemo::PoltaDemo(Polta* pPolta) : PoltaActionBase("ポルタデモ", pPolta), mCurDemoNerve(), mCurDemoName(), mCameraName() {
}

void PoltaDemo::init() {
    initNerve(GET_NERVE(PoltaDemo, PoltaDemoNrvOpeningDemo));
}

void PoltaDemo::control() {
}

void PoltaDemo::startOpeningDemo() {
    startTryDemo("ポルタ開始デモ", GET_NERVE(PoltaDemo, PoltaDemoNrvOpeningDemo));
}

void PoltaDemo::startPowerUpDemo() {
    startTryDemo("ポルタパワーアップデモ", GET_NERVE(PoltaDemo, PoltaDemoNrvPowerUpDemo));
}

void PoltaDemo::startDownDemo() {
    startTryDemo("ポルタダウンデモ", GET_NERVE(PoltaDemo, PoltaDemoNrvDownDemo));
}

void PoltaDemo::startDemoSetting() {
    MR::overlayWithPreviousScreen(2);
    setNerve(mCurDemoNerve);
    MR::resetPlayerEffect();
    mCurDemoName = nullptr;
    mCurDemoNerve = nullptr;
}

void PoltaDemo::startCameraEndDemo(const char* pCameraName) {
    mCameraName = pCameraName;
    MR::tryForceDeleteEffectAll(getHost());
    if (!MR::isActionEnd(getHost())) {
        MR::setBckFrameEndAndStop(getHost());
    }
    PoltaArm* left = getHost()->mLeftArm;
    if (!MR::isDead(left) && !MR::isActionEnd(left)) {
        MR::setBckFrameEndAndStop(left);
    }
    PoltaArm* right = getHost()->mRightArm;
    if (!MR::isDead(right) && !MR::isActionEnd(right)) {
        MR::setBckFrameEndAndStop(right);
    }
    MR::setAnimCameraLastFrame(getHost(), getHost()->mCameraInfo, pCameraName);
    MR::overlayWithPreviousScreen(2);
    setNerve(GET_NERVE(PoltaDemo, PoltaDemoNrvCameraEndDemo));
}

#pragma dont_inline on
void PoltaDemo::startTryDemo(const char* pDemoName, const Nerve* pDemoNerve) {
    appear();
    mCurDemoNerve = pDemoNerve;
    mCurDemoName = pDemoName;
    setNerve(GET_NERVE(PoltaDemo, PoltaDemoNrvTryStartDemo));
}
#pragma dont_inline off

void PoltaDemo::exeTryStartDemo() {
    if (MR::tryStartDemoMarioPuppetable(getHost(), mCurDemoName)) {
        startDemoSetting();
    }
}

void PoltaDemo::exeOpeningDemo() {
    if (MR::isFirstStep(this)) {
        MR::showModel(getHost());
        PoltaFunction::killGroundRock(getHost());
        getHost()->setStartPose();
        MR::stopStageBGMImmediately();
        MR::stopStageBGM(60);
        MR::startAnimCameraTargetSelf(getHost(), getHost()->mCameraInfo, "OpeningDemo", 0, false, 1.0f);
        MR::startAction(getHost(), "OpeningDemo");
        MR::startBckPlayer("BattleWait");
        PoltaFunction::onMovement(getHost());
        MR::setPlayerPos("ポルタ開始デモプレイヤー位置");
        getHost()->mFormationModel->appear();
        MR::startAction(getHost()->mFormationModel, "OpeningDemo");
    }

    if (MR::isStep(this, ::sOpeningBlurStart)) {
        MR::startCenterScreenBlur(::sOpeningBlurTime, ::sOpeningBlurOffset, ::sOpeningBlurAlpha, ::sOpeningBlurFadeIn, ::sOpeningBlurFadeOut);
    }

    if (MR::isStep(this, 60)) {
        MR::startStageBGMIfNotPlaying("MBGM_SMG_BOSS_09A", false);
    }

    Polta* host = getHost();
    if (MR::tryEndDemo(host, "ポルタ開始デモ", MR::isActionEnd(host))) {
        getHost()->mFormationModel->kill();
        if (MR::isDemoEndWaitCameraInterpolating()) {
            MR::startStageBGMIfNotPlaying("MBGM_SMG_BOSS_09A", false);
            startCameraEndDemo("OpeningDemo");
            return;
        }
        MR::endAnimCamera(getHost(), getHost()->mCameraInfo, "OpeningDemo", -1, true);
        kill();
    }
}

void PoltaDemo::exePowerUpDemo() {
    if (MR::isFirstStep(this)) {
        PoltaFunction::killGroundRock(getHost());
        getHost()->setStartPose();
        MR::invalidateShadowAll(getHost()->mLeftArm);
        MR::invalidateShadowAll(getHost()->mRightArm);
        MR::stopStageBGM(60);
        PoltaFunction::onMovement(getHost());
        PoltaFunction::startArm(getHost());
        MR::startAnimCameraTargetSelf(getHost(), getHost()->mCameraInfo, "PowerUpDemo", 0, false, 1.0f);
        MR::startAction(getHost(), "PowerUpDemo");
        MR::startAction(PoltaFunction::getLeftArmActor(getHost()), "PowerUpDemo");
        MR::startAction(PoltaFunction::getRightArmActor(getHost()), "PowerUpDemo");
        MR::startBckPlayer("BattleWait");
        MR::setPlayerPos("ポルタデモプレイヤー位置");
        getHost()->mFormationModel->appear();
        getHost()->mLeftArm->mFormationModel->appear();
        getHost()->mRightArm->mFormationModel->appear();
        MR::startAction(getHost()->mFormationModel, "PowerUpDemo");
        MR::startAction(getHost()->mLeftArm->mFormationModel, "PowerUpDemoArmL");
        MR::startAction(getHost()->mRightArm->mFormationModel, "PowerUpDemoArmR");
    }

    if (MR::isStep(this, ::sPowerUpBlurStart)) {
        MR::startCenterScreenBlur(::sPowerUpBlurTime, ::sPowerUpBlurOffset, ::sPowerUpBlurAlpha, ::sPowerUpBlurFadeIn, ::sPowerUpBlurFadeOut);
    }
    Polta* host = getHost();
    if (MR::tryEndDemo(host, "ポルタパワーアップデモ", MR::isActionEnd(host))) {
        MR::startStageBGM("MBGM_SMG_BOSS_09B", false);
        MR::validateShadowAll(getHost()->mLeftArm);
        MR::validateShadowAll(getHost()->mRightArm);
        getHost()->mFormationModel->kill();
        getHost()->mLeftArm->mFormationModel->kill();
        getHost()->mRightArm->mFormationModel->kill();
        if (MR::isDemoEndWaitCameraInterpolating()) {
            startCameraEndDemo("PowerUpDemo");
            return;
        }
        MR::endAnimCamera(getHost(), getHost()->mCameraInfo, "PowerUpDemo", -1, true);
        kill();
    }
}

void PoltaDemo::exeDownDemo() {
    if (MR::isFirstStep(this)) {
        PoltaFunction::killGroundRock(getHost());
        getHost()->setStartPose();
        MR::stopStageBGM(30);
        MR::startAnimCameraTargetSelf(getHost(), getHost()->mCameraInfo, "DownDemo", 0, false, 1.0f);
        MR::startAction(getHost(), "DownDemo");
        MR::startBckPlayer("BattleWait");
        PoltaFunction::onMovement(getHost());
        MR::setPlayerPos("ポルタデモプレイヤー位置");
    }
    Polta* host = getHost();
    if (MR::tryEndDemo(host, "ポルタダウンデモ", MR::isActionEnd(host))) {
        if (MR::isDemoEndWaitCameraInterpolating()) {
            startCameraEndDemo("DownDemo");
            return;
        }
        MR::endAnimCamera(getHost(), getHost()->mCameraInfo, "DownDemo", -1, true);
        kill();
    }
}

void PoltaDemo::exeCameraEndDemo() {
    if (MR::isFirstStep(this)) {
        MR::endAnimCamera(getHost(), getHost()->mCameraInfo, mCameraName, -1, true);
        kill();
    }
}

PoltaDemo::~PoltaDemo() {
}
