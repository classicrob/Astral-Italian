#include "Game/Boss/BossStinkBugFinishDemo.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/BossStinkBug.hpp"
#include "Game/Demo/DemoPositionController.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"

extern "C" void fn_80139930(DemoPositionController*, const char*);
extern "C" void fn_800AAAB0(BossStinkBug*, DemoPositionController*);

namespace NrvBossStinkBugFinishDemo {
    NEW_NERVE(BossStinkBugFinishDemoNrvTryStart, BossStinkBugFinishDemo, TryStart);
    NEW_NERVE(BossStinkBugFinishDemoNrvDemo, BossStinkBugFinishDemo, Demo);
    NEW_NERVE(BossStinkBugFinishDemoNrvAppearPowerStar, BossStinkBugFinishDemo, AppearPowerStar);
};  // namespace NrvBossStinkBugFinishDemo

BossStinkBugFinishDemo::BossStinkBugFinishDemo(BossStinkBug* pHost, const JMapInfoIter& rIter)
    : BossStinkBugActionBase("終了デモ", pHost), mDemoPositionController(nullptr) {
    initNerve(GET_NERVE(BossStinkBugFinishDemo, BossStinkBugFinishDemoNrvDemo));
    mDemoPositionController = new DemoPositionController("BossStinkBugDemo", rIter);
    mDemoPositionController->initAnimCamera("FinishDemo");
}

void BossStinkBugFinishDemo::appear() {
    ActorStateBase::appear();
    setNerve(GET_NERVE(BossStinkBugFinishDemo, BossStinkBugFinishDemoNrvTryStart));
    MR::requestStartDemoMarioPuppetable(this, getHost(), "ボスカメムシ終了デモ", GET_NERVE(BossStinkBugFinishDemo, BossStinkBugFinishDemoNrvDemo),
                                        nullptr);
}

void BossStinkBugFinishDemo::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::overlayWithPreviousScreen(2);
        MR::stopStageBGM(30);
        MR::startBckPlayer("BattleWait", "BattleWaitNoInter");
        getHost()->reuestMovementOnParts();
        fn_80139930(mDemoPositionController, "FinishDemo");
        MR::startAction(getHost(), "FinishDemo");
        MR::tryStartAllAnim(getHost()->getWingModel(), "FinishDemo");
    }

    fn_800AAAB0(getHost(), mDemoPositionController);

    if (MR::isActionEnd(getHost())) {
        setNerve(GET_NERVE(BossStinkBugFinishDemo, BossStinkBugFinishDemoNrvAppearPowerStar));
    }
}

void BossStinkBugFinishDemo::exeAppearPowerStar() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(getHost(), "Death");
        MR::startActionSound(getHost(), "BmBossBugExplode");
        MR::hideModel(getHost());
        MR::tryRumblePadVeryStrong(getHost(), WPAD_CHAN0);
        MR::appearPowerStarContinueCurrentDemo(getHost(), getHost()->mPosition);
    }

    if (MR::isEndPowerStarAppearDemo(getHost())) {
        mDemoPositionController->endDemo("FinishDemo");
        MR::endDemo(getHost(), "ボスカメムシ終了デモ");
        MR::overlayWithPreviousScreen(2);
        kill();
        getHost()->kill();
    }
}

void BossStinkBugFinishDemo::exeTryStart() {
}
