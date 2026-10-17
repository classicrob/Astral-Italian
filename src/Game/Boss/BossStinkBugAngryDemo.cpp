#include "Game/Boss/BossStinkBugAngryDemo.hpp"
#include "Game/Boss/BossStinkBug.hpp"
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

namespace {
    static const s32 sFlySoundStartStep1 = 155;
    static const s32 sFlySoundStartStep2 = 200;
};  // namespace

extern "C" void fn_80139930(DemoPositionController*, const char*);
extern "C" void fn_800AAAB0(BossStinkBug*, DemoPositionController*);
extern "C" bool fn_800AAB30(BossStinkBug*, DemoPositionController*, const char*);
extern "C" bool fn_800203E0(LiveActor*, const char*, bool);
extern "C" bool fn_8001F950(LiveActor*, const char*);

namespace NrvBossStinkBugAngryDemo {
    // Demo is declared before TryStart so that exeTryStart (inlined into TryStart::execute) can reference it;
    // the static instances are still constructed in the order TryStart, Demo, DemoEnd.
    class BossStinkBugAngryDemoNrvDemo : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BossStinkBugAngryDemo* actor = reinterpret_cast< BossStinkBugAngryDemo* >(pSpine->mExecutor);
            actor->exeDemo();
        };
        static BossStinkBugAngryDemoNrvDemo sInstance;
    };
};  // namespace NrvBossStinkBugAngryDemo

BossStinkBugAngryDemo::BossStinkBugAngryDemo(BossStinkBug* pHost, const JMapInfoIter& rIter)
    : BossStinkBugActionBase("終了デモ", pHost), mDemoPositionController(nullptr) {
    initNerve(GET_NERVE(BossStinkBugAngryDemo, BossStinkBugAngryDemoNrvDemo));
    mDemoPositionController = new DemoPositionController("BossStinkBugDemo", rIter);
    mDemoPositionController->initAnimCamera("AngryDemo");
}

void BossStinkBugAngryDemo::exeTryStart() {
    if (fn_8001F950(getHost(), "ボスカメムシ怒りデモ")) {
        setNerve(GET_NERVE(BossStinkBugAngryDemo, BossStinkBugAngryDemoNrvDemo));
    }
}

namespace NrvBossStinkBugAngryDemo {
    NEW_NERVE(BossStinkBugAngryDemoNrvTryStart, BossStinkBugAngryDemo, TryStart);
    BossStinkBugAngryDemoNrvDemo BossStinkBugAngryDemoNrvDemo::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BossStinkBugAngryDemoNrvDemoEnd, BossStinkBugAngryDemo, DemoEnd);
};  // namespace NrvBossStinkBugAngryDemo

void BossStinkBugAngryDemo::appear() {
    ActorStateBase::appear();
    setNerve(GET_NERVE(BossStinkBugAngryDemo, BossStinkBugAngryDemoNrvTryStart));
}

void BossStinkBugAngryDemo::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::overlayWithPreviousScreen(2);
        MR::startBckPlayer("BattleWait", "BattleWaitNoInter");
        fn_80139930(mDemoPositionController, "AngryDemo");
        getHost()->reuestMovementOnParts();
        MR::startAction(getHost(), "AngryDemo");
        MR::startBrk(getHost(), "Angry");
        MR::tryStartAllAnim(getHost()->getWingModel(), "AngryDemo");
    }

    if (MR::isGreaterStep(this, ::sFlySoundStartStep1) && MR::isLessStep(this, ::sFlySoundStartStep2 + 1)) {
        MR::startActionSound(getHost(), "BmLvBossBugFlyAdd", 550, 120);
        MR::startActionSound(getHost(), "BmLvBossBugFlyAlarm", 250, 20);
    }

    if (MR::isGreaterStep(this, ::sFlySoundStartStep2)) {
        MR::startActionSound(getHost(), "BmLvBossBugFlyAdd", 550, 126);
        MR::startActionSound(getHost(), "BmLvBossBugFlyAlarm", 550, 126);
        MR::startActionSound(getHost(), "BmLvBossBugFlyBase2", 550, 126);
    }

    fn_800AAAB0(getHost(), mDemoPositionController);

    if (fn_800203E0(getHost(), "ボスカメムシ怒りデモ", MR::isActionEnd(getHost()))) {
        if (fn_800AAB30(getHost(), mDemoPositionController, "AngryDemo")) {
            kill();
        } else {
            setNerve(GET_NERVE(BossStinkBugAngryDemo, BossStinkBugAngryDemoNrvDemoEnd));
        }
    }
}

void BossStinkBugAngryDemo::exeDemoEnd() {
    if (MR::isFirstStep(this)) {
        mDemoPositionController->endDemo("AngryDemo");
        kill();
    }
}
