#include "Game/Boss/BossStinkBugFlyDemo.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/BossStinkBug.hpp"
#include "Game/Demo/DemoPositionController.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
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

extern "C" void fn_80139930(DemoPositionController*, const char*);
extern "C" void fn_800AAAB0(BossStinkBug*, DemoPositionController*);
extern "C" bool fn_800AAB30(BossStinkBug*, DemoPositionController*, const char*);
extern "C" bool fn_800203E0(LiveActor*, const char*, bool);
extern "C" bool fn_8001F950(LiveActor*, const char*);

namespace {
    static const s32 sFlySoundStartStep1 = 113;
    static const s32 sFlySoundStartStep2 = 260;
    static const s32 sShakeSoundStartStep = 48;
    static const s32 sShakeSoundEndStep = 90;
    static const s32 sCloseFeetSoundStartStep = 230;
    static const s32 sFlyStartSoundStartStep = 110;
    static const s32 sFlyStartSoundEndStep = 263;
};  // namespace

namespace NrvBossStinkBugFlyDemo {
    // Demo is declared before TryStart so that exeTryStart (inlined into TryStart::execute) can reference it;
    // the static instances are still constructed in the order TryStart, Demo, DemoEnd.
    class BossStinkBugFlyDemoNrvDemo : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BossStinkBugFlyDemo* actor = reinterpret_cast< BossStinkBugFlyDemo* >(pSpine->mExecutor);
            actor->exeDemo();
        };
        static BossStinkBugFlyDemoNrvDemo sInstance;
    };
};  // namespace NrvBossStinkBugFlyDemo

BossStinkBugFlyDemo::BossStinkBugFlyDemo(BossStinkBug* pStinkBug, const JMapInfoIter& rIter)
    : BossStinkBugActionBase("終了デモ", pStinkBug), mDemoPositionController(nullptr) {
    initNerve(GET_NERVE(BossStinkBugFlyDemo, BossStinkBugFlyDemoNrvDemo));
    mDemoPositionController = new DemoPositionController("BossStinkBugDemo", rIter);
    mDemoPositionController->initAnimCamera("FlyDemo");
}

void BossStinkBugFlyDemo::exeTryStart() {
    if (fn_8001F950(getHost(), "ボスカメムシ飛行デモ")) {
        setNerve(GET_NERVE(BossStinkBugFlyDemo, BossStinkBugFlyDemoNrvDemo));
    }
}

namespace NrvBossStinkBugFlyDemo {
    NEW_NERVE(BossStinkBugFlyDemoNrvTryStart, BossStinkBugFlyDemo, TryStart);
    BossStinkBugFlyDemoNrvDemo BossStinkBugFlyDemoNrvDemo::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BossStinkBugFlyDemoNrvDemoEnd, BossStinkBugFlyDemo, DemoEnd);
};  // namespace NrvBossStinkBugFlyDemo

void BossStinkBugFlyDemo::appear() {
    ActorStateBase::appear();
    setNerve(GET_NERVE(BossStinkBugFlyDemo, BossStinkBugFlyDemoNrvTryStart));
}

void BossStinkBugFlyDemo::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::overlayWithPreviousScreen(2);
        MR::stopStageBGM(30);
        getHost()->offBindLeg();
        getHost()->_98->endIKCtrlAll();
        MR::startBckPlayer("BattleWait", "BattleWaitNoInter");
        fn_80139930(mDemoPositionController, "FlyDemo");
        getHost()->reuestMovementOnParts();
        MR::startAction(getHost(), "FlyDemo");
        MR::startAction(getHost()->getBombLauncher(), "FlyDemo");
        MR::tryStartAllAnim(getHost()->getWingModel(), "FlyDemo");
    }

    if (MR::isGreaterStep(this, ::sFlySoundStartStep1)) {
        MR::startActionSound(getHost(), "BmLvBossBugFlyAdd", 500, 126);
    }

    if (MR::isGreaterStep(this, ::sFlySoundStartStep2)) {
        MR::startActionSound(getHost(), "BmLvBossBugFlyBase", 500, 126);
        MR::startActionSound(getHost(), "BmLvBossBugFlyAlarm", 500, 126);
    }

    if (MR::isGreaterStep(this, ::sShakeSoundStartStep) && MR::isLessStep(this, ::sShakeSoundEndStep)) {
        MR::startActionSound(getHost(), "BmLvBossBugShakeBody");
    }

    if (MR::isStep(this, ::sCloseFeetSoundStartStep)) {
        MR::startActionSound(getHost(), "BmBossBugCloseFeet");
    }

    if (MR::isGreaterStep(this, ::sFlyStartSoundStartStep) && MR::isLessStep(this, ::sFlyStartSoundEndStep)) {
        MR::startActionSound(getHost(), "BmLvBossBugFlyStart");
    }

    fn_800AAAB0(getHost(), mDemoPositionController);

    if (fn_800203E0(getHost(), "ボスカメムシ飛行デモ", MR::isActionEnd(getHost()))) {
        if (fn_800AAB30(getHost(), mDemoPositionController, "FlyDemo")) {
            kill();
        } else {
            setNerve(GET_NERVE(BossStinkBugFlyDemo, BossStinkBugFlyDemoNrvDemoEnd));
        }
    }
}

void BossStinkBugFlyDemo::exeDemoEnd() {
    if (MR::isFirstStep(this)) {
        mDemoPositionController->endDemo("FlyDemo");
        kill();
    }
}
