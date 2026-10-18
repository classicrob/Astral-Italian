#include "Game/Boss/BossStinkBugOpeningDemo.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/BossStinkBugFunction.hpp"
#include "Game/Boss/BossStinkBug.hpp"
#include "Game/Boss/BossStinkBugActionBase.hpp"
#include "Game/Demo/DemoPositionController.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include <revolution/types.h>


namespace NrvBossStinkBugOpeningDemo {
    // Demo is declared before TryStart so that exeTryStart (inlined into TryStart::execute) can reference it;
    // the static instances are still constructed in the order TryStart, Demo, DemoEnd.
    class BossStinkBugOpeningDemoNrvDemo : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BossStinkBugOpeningDemo* actor = reinterpret_cast< BossStinkBugOpeningDemo* >(pSpine->mExecutor);
            actor->exeDemo();
        };
        static BossStinkBugOpeningDemoNrvDemo sInstance;
    };
};  // namespace NrvBossStinkBugOpeningDemo

BossStinkBugOpeningDemo::BossStinkBugOpeningDemo(BossStinkBug* pStinkBug, const JMapInfoIter& rIter)
    : BossStinkBugActionBase("オープニングデモ", pStinkBug), mDemoPositionController(nullptr) {
    initNerve(GET_NERVE(BossStinkBugOpeningDemo, BossStinkBugOpeningDemoNrvDemo));
    mDemoPositionController = new DemoPositionController("BossStinkBugDemo", rIter);
    mDemoPositionController->initAnimCamera("OpeningDemo");
}

void BossStinkBugOpeningDemo::exeTryStart() {
    if (MR::tryStartDemoMarioPuppetable(getHost(), "ボスカメムシオープニングデモ")) {
        setNerve(GET_NERVE(BossStinkBugOpeningDemo, BossStinkBugOpeningDemoNrvDemo));
    }
}

namespace NrvBossStinkBugOpeningDemo {
    NEW_NERVE(BossStinkBugOpeningDemoNrvTryStart, BossStinkBugOpeningDemo, TryStart);
    BossStinkBugOpeningDemoNrvDemo BossStinkBugOpeningDemoNrvDemo::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BossStinkBugOpeningDemoNrvDemoEnd, BossStinkBugOpeningDemo, DemoEnd);
};  // namespace NrvBossStinkBugOpeningDemo

void BossStinkBugOpeningDemo::appear() {
    ActorStateBase::appear();
    setNerve(GET_NERVE(BossStinkBugOpeningDemo, BossStinkBugOpeningDemoNrvTryStart));
}

void BossStinkBugOpeningDemo::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::overlayWithPreviousScreen(2);
        MR::stopStageBGM(60);
        MR::startBckPlayer("BattleWait", nullptr);
        mDemoPositionController->startDemo("OpeningDemo");
        getHost()->reuestMovementOnParts();
        MR::startAction(getHost(), "OpeningDemo");
        MR::tryStartAllAnim(getHost()->getWingModel(), "OpeningDemo");
    }

    BossStinkBugFunction::syncDemoPose(getHost(), mDemoPositionController);

    if (MR::tryEndDemo(getHost(), "ボスカメムシオープニングデモ", MR::isActionEnd(getHost()))) {
        if (BossStinkBugFunction::tryEndDemoPose(getHost(), mDemoPositionController, "OpeningDemo")) {
            kill();
        } else {
            setNerve(GET_NERVE(BossStinkBugOpeningDemo, BossStinkBugOpeningDemoNrvDemoEnd));
        }
    }
}

void BossStinkBugOpeningDemo::exeDemoEnd() {
    if (MR::isFirstStep(this)) {
        mDemoPositionController->endDemo("OpeningDemo");
        kill();
    }
}
