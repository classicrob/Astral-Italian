#include "Game/Boss/BossStinkBugOpeningDemo.hpp"
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

extern "C" void fn_80139930(DemoPositionController*, const char*);
extern "C" void fn_800AAAB0(BossStinkBug*, DemoPositionController*);
extern "C" bool fn_800AAB30(BossStinkBug*, DemoPositionController*, const char*);
extern "C" bool fn_800203E0(LiveActor*, const char*, bool);
extern "C" bool fn_8001F950(LiveActor*, const char*);

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
    if (fn_8001F950(getHost(), "ボスカメムシオープニングデモ")) {
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
        fn_80139930(mDemoPositionController, "OpeningDemo");
        getHost()->reuestMovementOnParts();
        MR::startAction(getHost(), "OpeningDemo");
        MR::tryStartAllAnim(getHost()->getWingModel(), "OpeningDemo");
    }

    fn_800AAAB0(getHost(), mDemoPositionController);

    if (fn_800203E0(getHost(), "ボスカメムシオープニングデモ", MR::isActionEnd(getHost()))) {
        if (fn_800AAB30(getHost(), mDemoPositionController, "OpeningDemo")) {
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
