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

namespace NrvSandyStateDemoDown {
    NEW_NERVE(SandyStateDemoDownNrvStart, SandyStateDemoDown, Start);
    NEW_NERVE(SandyStateDemoDownNrvDemo, SandyStateDemoDown, Demo);
};  // namespace NrvSandyStateDemoDown

// 80100EA0
SandyStateDemoDown::SandyStateDemoDown(Sandy* pSandy) : ActorStateBase< Sandy >("State:\203_\203E\203\223", pSandy), mPowerStar(nullptr) {
    initNerve(&NrvSandyStateDemoDown::SandyStateDemoDownNrvStart::sInstance, 0);
    Sandy* host = mHost;
    mPowerStar = MR::createPowerStarDemoModel(host, "\203f\203\202\227p\203p\203\217\201[\203X\203^\201[", host->getBaseMtx());
    MR::registerDemoSimpleCastAll(mPowerStar);
    MR::invalidateClipping(mPowerStar);
    mPowerStar->makeActorDead();
}

// 80100F70
void SandyStateDemoDown::kill() {
    mPowerStar->makeActorDead();
    mHost->getRightHand()->kill();
    mHost->getLeftHand()->kill();
    mIsDead = true;
}

// 80100FF0
void SandyStateDemoDown::exeStart() {
    mHost->updateTarget();
    if (MR::tryStartDemo(mHost, "\203T\203\223\203f\203B\201[\203_\203E\203\223\203f\203\202")) {
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, "DemoSandyDown", 0, false, 1.0f);
        setNerve(&NrvSandyStateDemoDown::SandyStateDemoDownNrvDemo::sInstance);
    }
}

// 80101070
void SandyStateDemoDown::exeDemo() {
    if (MR::isFirstStep(this)) {
        mHost->copyQuat();
        mHost->forceWaitHands();
        mHost->getLeftMini()->kill();
        mHost->getRightMini()->kill();
        mHost->mHolder->killAll();
        MR::killPlayerFireBalls();
        mHost->killAll();
        MR::stopStageBGM(10);
        mHost->startActionAll("DemoSandyDown", nullptr);
        MR::startBrk(mHost, "Sandy");
        MR::startBrk(mHost->getLeftHand(), "SandyHand");
        MR::startBrk(mHost->getRightHand(), "SandyHand");
        mPowerStar->appear();
        MR::startAction(mPowerStar, "DemoSandyDown");
        MR::setPlayerPos("\203}\203\212\203I\203f\203\202\210\312\222u");
        MR::startBckPlayer("WatchUp", nullptr);
        MR::overlayWithPreviousScreen(2);
        MR::onSwitchA(mHost);
    }
    if (MR::isActionEnd(mHost)) {
        MR::endAnimCamera(mHost, mHost->mCameraInfo, "DemoSandyDown", 0, true);
        MR::endDemo(mHost, "\203T\203\223\203f\203B\201[\203_\203E\203\223\203f\203\202");
        kill();
    }
}

// 801011E0
SandyStateDemoDown::~SandyStateDemoDown() {
}
