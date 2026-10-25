#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/Util/SoundUtil.hpp"
#include "Game/Demo/OpeningDemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/Util/SceneUtil.hpp"

static const Vec sStarLocalPosition = {-2500.0f, -6250.0f, 0.0f};

namespace {
    s32 sSkipEnableStep = 20;
    s32 sShootingStarStartStep = 60;
    s32 sShootingStarInterval = 90;

    // Indexed by beat; -1 means no shooting star on that beat.
    const s32 sShootingStarActions[9] = {1, 0, -1, 1, 2, -1, 0, -1, -1};
};  // namespace

namespace NrvOpeningDemoObj {
    NEW_NERVE(OpeningDemoObjNrvStart, OpeningDemoObj, Start);
    NEW_NERVE(OpeningDemoObjNrvWaitAnim, OpeningDemoObj, WaitAnim);
    NEW_NERVE(OpeningDemoObjNrvMain1, OpeningDemoObj, Main);
    NEW_NERVE(OpeningDemoObjNrvTalk, OpeningDemoObj, Talk);
    NEW_NERVE(OpeningDemoObjNrvMain2, OpeningDemoObj, Main);
    NEW_NERVE(OpeningDemoObjNrvFinish, OpeningDemoObj, Finish);
    NEW_NERVE(OpeningDemoObjNrvFall, OpeningDemoObj, Fall);
    NEW_NERVE(OpeningDemoObjNrvMain3, OpeningDemoObj, Main);
    NEW_NERVE(OpeningDemoObjNrvEnd, OpeningDemoObj, End);
    class OpeningDemoObjNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static OpeningDemoObjNrvWait sInstance;
    };
    OpeningDemoObjNrvWait OpeningDemoObjNrvWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvOpeningDemoObj

OpeningDemoObj::OpeningDemoObj(const char* pName) : LiveActor(pName), mModelA(nullptr), mModelB(nullptr), mLayoutA(nullptr), mLayoutB(nullptr), mModelC(nullptr) {
}

void OpeningDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    MR::tryRegisterDemoCast(this, rIter);
    mModelA = MR::createModelObjMapObj("流れ星", "PeachShootingStar", nullptr);
    TPos3f zone = *MR::getZonePlacementMtx(mLinkInfo.mZoneID);
    TVec3f starPosition(0.0f, 0.0f, 0.0f);
    zone.mult(TVec3f(*reinterpret_cast<const TVec3f*>(&sStarLocalPosition)), starPosition);
    mModelA->mPosition.set(starPosition);
    mModelA->mRotation.set(-90.0f, 0.0f, 0.0f);
    MR::tryRegisterDemoCast(mModelA, rIter);
    mModelA->makeActorDead();
    mModelB = MR::createModelObjMapObj("プロローグの空オブジェ", "PeachPrologueSky", nullptr);
    mModelB->mPosition.set(mModelA->mPosition);
    mModelB->mRotation.set(mModelA->mRotation);
    MR::tryRegisterDemoCast(mModelB, rIter);
    mModelB->makeActorAppeared();
    mLayoutA = new SimpleLayout("プロローグテキスト", "PrologueText", 1, -1);
    MR::registerDemoSimpleCastAll(mLayoutA);
    mLayoutA->kill();
    mLayoutB = new SimpleLayout("ピーチの手紙", "PeachLetterMini", 1, -1);
    MR::registerDemoSimpleCastAll(mLayoutB);
    mLayoutB->kill();
    mModelC = MR::createModelObjMapObj("土管", "EarthenPipe", nullptr);
    MR::setBrkFrameAndStop(mModelC, 0.0f);
    mModelC->mPosition.set(mPosition);
    mModelC->mRotation.set(mRotation);
    MR::tryRegisterDemoCast(mModelC, rIter);
    mModelC->makeActorDead();
    initNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvStart), 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

// Nerve Start: wait for the puppet start, then go to WaitAnim.
void OpeningDemoObj::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::resetStoryBookAppear();
    }
    if (MR::tryStartDemoRegisteredMarioPuppetable(this, nullptr)) {
        MR::endStartPosCamera();
        MR::setPlayerPos(mPosition);
        setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvWaitAnim));
    }
}

// Nerve WaitAnim: play the layout's first animation, then move on when it stops.
void OpeningDemoObj::exeWaitAnim() {
    if (MR::isFirstStep(this)) {
        MR::pauseTimeKeepDemo(this);
        mLayoutA->appear();
        MR::startAnim(mLayoutA, "Prologue01", 0);
    }
    if (MR::isAnimStopped(mLayoutA, 0)) {
        setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain1));
    }
}

// Nerves Main1 / Main2 / Main3 share this body.
void OpeningDemoObj::exeMain() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain3))) {
            MR::startAnim(mLayoutB, "Wait", 0);
        }
        MR::openStoryBookTextIcon();
    }
    if (isNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain2))) {
        updateActionSound(getNerveStep());
    }
    if (!MR::isLessStep(this, sSkipEnableStep) && MR::testCorePadTriggerA(0)) {
        MR::startActionSound(mLayoutB, "SyTalkOk", -1, -1);
        MR::termStoryBookTextIcon();
        if (isNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain1))) {
            setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvTalk));
            return;
        }
        if (isNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain2))) {
            setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvFinish));
            return;
        }
        if (isNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain3))) {
            setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvEnd));
        }
    }
}

// Nerve Talk: layout B animation, then go to Main2 when it stops.
void OpeningDemoObj::exeTalk() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mLayoutA, "Prologue02", 0);
        MR::startStoryBookAppear();
    }
    if (MR::isGreaterEqualStep(this, sShootingStarStartStep)) {
        updateActionSound(getNerveStep() - sShootingStarStartStep);
    }
    if (MR::isAnimStopped(mLayoutA, 0)) {
        MR::endStoryBookAppear();
        setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain2));
    }
}

// Nerve Finish: play the layout A animation, resume the timekeep at its end.
void OpeningDemoObj::exeFinish() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mLayoutA, "End", 0);
    }
    if (MR::isStep(this, 0x28)) {
        MR::startStageBGM("STM_SMG2_EV_PROLOGUE_02", false);
    }
    if (MR::isAnimStopped(mLayoutA, 0)) {
        MR::resumeTimeKeepDemo(this);
        setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvFall));
    }
}

// Nerve Fall: layout B animation with a pause, then to Main3.
void OpeningDemoObj::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::pauseTimeKeepDemo(this);
        mLayoutB->appear();
        MR::startAnim(mLayoutB, "Appear", 0);
    }
    if (MR::isStep(this, 0x64)) {
        MR::startActionSound(mLayoutB, "Appear", -1, -1);
    }
    MR::setSeVolumeSetLevel(3, 0x1E);
    if (MR::isAnimStopped(mLayoutB, 0)) {
        setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain3));
    }
}

// Nerve End: layout B animation, then resume and go to the empty Wait nerve.
void OpeningDemoObj::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mLayoutB, "End", 0);
    }
    if (MR::isAnimStopped(mLayoutB, 0)) {
        MR::resumeTimeKeepDemo(this);
        MR::startStoryBookTurnPage();
        setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvWait));
    }
}

void OpeningDemoObj::kill() {
    mModelB->kill();
    mLayoutA->kill();
    mLayoutB->kill();
    mModelC->kill();
    LiveActor::kill();
}

OpeningDemoObj::~OpeningDemoObj() {
}

// Plays the action sound / action animation for model A on a fixed beat.
void OpeningDemoObj::updateActionSound(s32 step) {
    if (MR::isDead(mModelA) && (step % sShootingStarInterval) == 0) {
        mModelA->appear();
        s32 action = sShootingStarActions[(static_cast<u32>(step / sShootingStarInterval) % 9)];
        if (action != -1) {
            char name[] = "PeachShootingStarA";
            s32 len = strlen(name);
            name[len - 1] += static_cast< s8 >(action);
            MR::startAction(mModelA, name);
            MR::startActionSound(mModelA, "ShootingStar", -1, -1, -1);
        }
    }
    if (!MR::isDead(mModelA) && MR::isActionEnd(mModelA)) {
        mModelA->kill();
    }
}
