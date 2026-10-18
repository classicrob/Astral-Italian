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

// Unnamed SMG2 helpers outside this unit.
// PLACEHOLDER(objects): unnamed target symbol fn_8001BA80, declared extern "C" so the reloc name matches; resolve when fn_8001BA80 gets its real name in symbols.txt
extern "C" void fn_8001BA80();

// Demo/camera names stored as const pointers (they live in .sdata2 in the target).
static const char* const sDemoStartName = "\203T\203\223\203f\203B\201[\223o\217\352\203f\203\202";
static const char* const sDemoStartCamera = "DemoSandyStart";
static const char* const sDemoStartAction = "DemoSandyStart";

namespace NrvSandyStateDemoStart {
    NEW_NERVE(SandyStateDemoStartNrvStart, SandyStateDemoStart, Start);
    NEW_NERVE(SandyStateDemoStartNrvDemo, SandyStateDemoStart, Demo);
};  // namespace NrvSandyStateDemoStart

// 80100B90
// NONMATCHING: ActorStateBase<Sandy> ctor is expanded inline in the target for this class.
SandyStateDemoStart::SandyStateDemoStart(Sandy* pSandy) : ActorStateBase< Sandy >("\217\363\221\324:\203f\203\202[\212J\216n]", pSandy) {
    initNerve(&NrvSandyStateDemoStart::SandyStateDemoStartNrvStart::sInstance, 0);
}

// 80100C00
void SandyStateDemoStart::appear() {
    mIsDead = false;
    MR::invalidateClipping(mHost);
}

// 80100C10
void SandyStateDemoStart::kill() {
    mHost->setWaitHands();
    mHost->appearAll();
    if (MR::isDemoEndWaitCameraInterpolating()) {
        MR::startAction(mHost, "WaitNoInterpole");
        mHost->getLeftHand()->startWaitAndSetNerve(true);
        mHost->getRightHand()->startWaitAndSetNerve(true);
    }
    mIsDead = true;
}

// 80100C90
void SandyStateDemoStart::exeStart() {
    if (MR::tryStartDemoMarioPuppetable(mHost, sDemoStartName)) {
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, sDemoStartCamera, 0, false, 1.0f);
        setNerve(&NrvSandyStateDemoStart::SandyStateDemoStartNrvDemo::sInstance);
    }
}

// 80100D00
void SandyStateDemoStart::exeDemo() {
    if (MR::isFirstStep(this)) {
        mHost->showAll();
        mHost->startActionAll(sDemoStartAction, nullptr);
        MR::setPlayerPos("\203}\203\212\203I\203f\203\202\210\312\222u");
        MR::startBckPlayer("BattleWait", nullptr);
        MR::overlayWithPreviousScreen(2);
        fn_8001BA80();
    }
    Sandy* pHost = mHost;
    if (MR::tryEndDemo(pHost, sDemoStartName, MR::isActionEnd(pHost))) {
        MR::endAnimCamera(mHost, mHost->mCameraInfo, sDemoStartCamera, 0, true);
        kill();
    }
}

// 80100DD0
SandyStateDemoStart::~SandyStateDemoStart() {
}
