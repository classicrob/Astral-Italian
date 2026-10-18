#include "Game/Boss/Sandy.hpp"
// SMG2: MR::appearCoinPop returns bool (ObjUtil.hpp declares it void), declare the real one here.
#define appearCoinPop appearCoinPopUnused
#include "Game/Util.hpp"
#undef appearCoinPop
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

static const char* const sDemoPowerUpName = "\203T\203\223\203f\203B\201[\203p\203\217\201[\203A\203b\203v\203f\203\202";
static const char* const sDemoPowerUpCamera = "DemoSandyPowerUp";
static const char* const sDemoPowerUpAction = "DemoSandyPowerUp";

namespace NrvSandyStateDemoPowerUp {
    NEW_NERVE(SandyStateDemoPowerUpNrvStart, SandyStateDemoPowerUp, Start);
    NEW_NERVE(SandyStateDemoPowerUpNrvDemo, SandyStateDemoPowerUp, Demo);
};  // namespace NrvSandyStateDemoPowerUp

// 801012B0
// NONMATCHING: ActorStateBase<Sandy> ctor is expanded inline in the target for this class.
SandyStateDemoPowerUp::SandyStateDemoPowerUp(Sandy* pSandy) : ActorStateBase< Sandy >("\217\363\221\324:\203f\203\202[\203p\203\217\201[\203A\203b\203v]", pSandy) {
    initNerve(&NrvSandyStateDemoPowerUp::SandyStateDemoPowerUpNrvStart::sInstance, 0);
}

// 80101320
void SandyStateDemoPowerUp::kill() {
    if (MR::isDemoEndWaitCameraInterpolating()) {
        mHost->setWaitHands();
        MR::startAction(mHost, "WaitNoInterpole");
        mHost->getLeftHand()->startWaitAndSetNerve(false);
        mHost->getRightHand()->startWaitAndSetNerve(false);
        mHost->getLeftMini()->startWait();
        mHost->getRightMini()->startWait();
    }
    mIsDead = true;
}

// 801013B0
void SandyStateDemoPowerUp::exeStart() {
    mHost->updateTarget();
    if (MR::tryStartDemo(mHost, sDemoPowerUpName)) {
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, sDemoPowerUpCamera, 0, false, 1.0f);
        setNerve(&NrvSandyStateDemoPowerUp::SandyStateDemoPowerUpNrvDemo::sInstance);
    }
}

// 80101420
void SandyStateDemoPowerUp::exeDemo() {
    if (MR::isFirstStep(this)) {
        turnToPlayer(&mHost->mTargetQuat, mHost, 180.0f);
        MR::hidePlayer();
        MR::killPlayerFireBalls();
        mHost->startActionAll(sDemoPowerUpAction, nullptr);
        MR::overlayWithPreviousScreen(2);
        mHost->getLeftMini()->appear();
        mHost->getRightMini()->appear();
        SandyMini* pLeftMini = mHost->getLeftMini();
        pLeftMini->mWaitStep = 90;
        SandyMini* pRightMini = mHost->getRightMini();
        pRightMini->mWaitStep = 180;
    }
    if (MR::checkPassBckFrame(mHost->getLeftMini(), 133.0f)) {
        MR::startActionSound(mHost->getLeftMini(), "DemoSandyPowerUpL", -1, -1, -1);
    }
    if (MR::checkPassBckFrame(mHost->getRightMini(), 128.0f)) {
        MR::startActionSound(mHost->getRightMini(), "DemoSandyPowerUpR", -1, -1, -1);
    }
    mHost->updateTarget();
    mHost->mMtx.setQT(mHost->mTargetQuat, mHost->mPosition);
    mHost->getLeftMini()->calcPosFromFixedPos();
    mHost->getRightMini()->calcPosFromFixedPos();
    Sandy* pHost = mHost;
    if (MR::tryEndDemo(pHost, sDemoPowerUpName, MR::isActionEnd(pHost))) {
        MR::endAnimCamera(mHost, mHost->mCameraInfo, sDemoPowerUpCamera, 0, true);
        MR::showPlayer();
        kill();
    }
}

// 80101680
SandyStateDemoPowerUp::~SandyStateDemoPowerUp() {
}
