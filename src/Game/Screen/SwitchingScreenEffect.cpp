#include "Game/Screen/SwitchingScreenEffect.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneObjHolder.hpp"

using namespace NrvSwitchingScreenEffect;

SwitchingScreenEffect::SwitchingScreenEffect(const char* name) : LiveActor(name) {}

void SwitchingScreenEffect::init(const JMapInfoIter& iter) {
    MR::initDefaultPos(this, iter);
    MR::initActor(this, iter, "SwitchingScreenEffect", false);
    MR::connectToSceneSwitchingScreenEffect(this);
    initSound(2, "SwitchingScreenEffect", 0, TVec3f(0.0f));
    initNerve(&SwitchingScreenEffectNrvWait::sInstance);
    MR::invalidateClipping(this);
    MR::hideModel(this);
    makeActorDead();
}

void SwitchingScreenEffect::control() {}

void SwitchingScreenEffect::exeWaitCapture() {
    if (MR::isFirstStep(this)) {
        updateScreenPlacement();
        MR::pauseOnCameraDirector();
        MR::showModel(this);
        MR::startBrkAndSetFrameAndStop(this, "Change", 0.0f);
        MR::startToCaptureScreen("Camera");
    }
    if (MR::isGreaterStep(this, 0) && MR::tryStartDemoWithoutCinemaFrame(this, "スイッチング画面エフェクトデモ"))
        setNerve(&SwitchingScreenEffectNrvSwitch::sInstance);
}

void SwitchingScreenEffect::exeSwitch() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Change");
        MR::startActionSound(this, "SwitchOn", -1, -1, -1);
    }
    if (MR::isBrkStopped(this)) {
        MR::hideModel(this);
        MR::endDemo(this, "スイッチング画面エフェクトデモ");
        MR::pauseOffCameraDirector();
        MR::endToCaptureScreen("Camera");
        setNerve(&SwitchingScreenEffectNrvWait::sInstance);
    }
}

void SwitchingScreenEffect::exeDisplay() {
    if (MR::isFirstStep(this)) {
        updateScreenPlacement();
        MR::showModel(this);
        MR::startBrkAndSetFrameAndStop(this, "Change", 0.0f);
    }
}

void SwitchingScreenEffect::updateScreenPlacement() {
    mPosition.x = MR::getScreenWidth() / 2;
    mPosition.y = -MR::getScreenHeight() / 2;
    mScale.x = MR::getScreenWidth() / 1000.0f;
    mScale.y = MR::getScreenHeight() / 1000.0f;
}

SwitchingScreenEffect::~SwitchingScreenEffect() {}
namespace NrvSwitchingScreenEffect {
    SwitchingScreenEffectNrvWait::SwitchingScreenEffectNrvWait() {}
    SwitchingScreenEffectNrvWait SwitchingScreenEffectNrvWait::sInstance;
    SwitchingScreenEffectNrvWaitCapture::SwitchingScreenEffectNrvWaitCapture() {}
    SwitchingScreenEffectNrvWaitCapture SwitchingScreenEffectNrvWaitCapture::sInstance;
    SwitchingScreenEffectNrvSwitch::SwitchingScreenEffectNrvSwitch() {}
    SwitchingScreenEffectNrvSwitch SwitchingScreenEffectNrvSwitch::sInstance;
    SwitchingScreenEffectNrvDisplay::SwitchingScreenEffectNrvDisplay() {}
    SwitchingScreenEffectNrvDisplay SwitchingScreenEffectNrvDisplay::sInstance;
    void SwitchingScreenEffectNrvDisplay::execute(Spine* spine) const { static_cast<SwitchingScreenEffect*>(spine->mExecutor)->exeDisplay(); }
    void SwitchingScreenEffectNrvSwitch::execute(Spine* spine) const { static_cast<SwitchingScreenEffect*>(spine->mExecutor)->exeSwitch(); }
    void SwitchingScreenEffectNrvWaitCapture::execute(Spine* spine) const { static_cast<SwitchingScreenEffect*>(spine->mExecutor)->exeWaitCapture(); }
    void SwitchingScreenEffectNrvWait::execute(Spine* spine) const {  }
}
namespace MR {
    void createSwitchingScreenEffect() { createSceneObj(0x2F); }
    void onSwitchingScreenEffect() {
        SwitchingScreenEffect* effect = getSceneObj<SwitchingScreenEffect>(0x2F);
        effect->appear();
        effect->setNerve(&NrvSwitchingScreenEffect::SwitchingScreenEffectNrvWaitCapture::sInstance);
    }
    void offSwitchingScreenEffect() {
        SwitchingScreenEffect* effect = getSceneObj<SwitchingScreenEffect>(0x2F);
        effect->appear();
        effect->setNerve(&NrvSwitchingScreenEffect::SwitchingScreenEffectNrvWaitCapture::sInstance);
    }
}
