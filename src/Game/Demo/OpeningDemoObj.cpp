#include "Game/Demo/OpeningDemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/UnnamedDemoCallee.hpp"

// Name tables in this unit's .data/.sdata (addresses from the target).
extern "C" char lbl_80680540[];
extern "C" char lbl_806805E4[];
extern "C" char lbl_806805EC[];
extern "C" char lbl_806805F8[];
extern "C" char lbl_80680604[];
extern "C" char lbl_80680610[];
extern "C" char lbl_80680628[];
extern "C" char lbl_80680630[];
extern "C" char lbl_807CF764[];
extern "C" char lbl_80647D70[];
extern "C" s32 lbl_80647D7C[];
extern "C" char lbl_80647DA0[];
extern "C" s32 lbl_807CF758;
extern "C" s32 lbl_807CF75C;
extern "C" s32 lbl_807CF760;
extern "C" f32 lbl_807D9808;


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

// NONMATCHING: zone placement matrix setup (getZonePlacementMtx/mult) omitted; model positions copied directly
void OpeningDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    MR::tryRegisterDemoCast(this, rIter);
    mModelA = MR::createModelObjMapObj(lbl_80680540, lbl_80680540 + 8, nullptr);
    MR::invalidateClipping(mModelA);
    MR::tryRegisterDemoCast(mModelA, rIter);
    mModelA->makeActorAppeared();
    mModelB = MR::createModelObjMapObj(lbl_80680540 + 0x1C, lbl_80680540 + 0x34, nullptr);
    mModelB->mPosition.set(mModelA->mPosition);
    mModelB->mRotation.set(mModelA->mRotation);
    MR::tryRegisterDemoCast(mModelB, rIter);
    mModelB->makeActorAppeared();
    mLayoutA = new SimpleLayout(lbl_80680540 + 0x48, lbl_80680540 + 0x5C, 1, -1);
    MR::registerDemoSimpleCastAll(mLayoutA);
    mLayoutA->appear();
    mLayoutB = new SimpleLayout(lbl_80680540 + 0x6C, lbl_80680540 + 0x80, 1, -1);
    MR::registerDemoSimpleCastAll(mLayoutB);
    mLayoutB->appear();
    mModelC = MR::createModelObjMapObj(lbl_80680540 + 0x90, lbl_80680540 + 0x98, nullptr);
    MR::setBrkFrameAndStop(mModelC, lbl_807D9808);
    mModelC->mPosition.set(mPosition);
    mModelC->mRotation.set(mRotation);
    MR::tryRegisterDemoCast(mModelC, rIter);
    mModelC->makeActorAppeared();
    initNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvStart), 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

// Nerve Start: wait for the puppet start, then go to WaitAnim.
void OpeningDemoObj::exeStart() {
    if (MR::isFirstStep(this)) {
        UnnamedDemoCallee::fn_80147990();
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
        MR::startAnim(mLayoutA, lbl_806805F8, 0);
    }
    if (MR::isAnimStopped(mLayoutA, 0)) {
        setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain1));
    }
}

// Nerves Main1 / Main2 / Main3 share this body.
void OpeningDemoObj::exeMain() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain3))) {
            MR::startAnim(mLayoutB, lbl_806805E4, 0);
        }
        UnnamedDemoCallee::fn_801477D0();
    }
    if (isNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain2))) {
        updateActionSound(getNerveStep());
    }
    if (!MR::isLessStep(this, lbl_807CF758) && MR::testCorePadTriggerA(0)) {
        UnnamedDemoCallee::fn_800085C0(mLayoutB, lbl_806805EC, -1, -1);
        UnnamedDemoCallee::fn_80147800();
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
        MR::startAnim(mLayoutA, lbl_80680604, 0);
        UnnamedDemoCallee::fn_801478C0();
    }
    if (MR::isGreaterEqualStep(this, lbl_807CF75C)) {
        updateActionSound(getNerveStep() - lbl_807CF75C);
    }
    if (MR::isAnimStopped(mLayoutA, 0)) {
        UnnamedDemoCallee::fn_80147930();
        setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain2));
    }
}

// Nerve Finish: play the layout A animation, resume the timekeep at its end.
// NONMATCHING: not yet verified against target branch layout
void OpeningDemoObj::exeFinish() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mLayoutA, lbl_807CF764, 0);
    }
    if (MR::isStep(this, 0x28)) {
        MR::startStageBGM(lbl_80680610, false);
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
        MR::startAnim(mLayoutB, lbl_80680628, 0);
    }
    if (MR::isStep(this, 0x64)) {
        UnnamedDemoCallee::fn_800085C0(mLayoutB, lbl_80680628, -1, -1);
    }
    UnnamedDemoCallee::fn_8005AC70(3, 0x1E);
    if (MR::isAnimStopped(mLayoutB, 0)) {
        setNerve(GET_NERVE(OpeningDemoObj, OpeningDemoObjNrvMain3));
    }
}

// Nerve End: layout B animation, then resume and go to the empty Wait nerve.
// NONMATCHING: not yet verified against target branch layout
void OpeningDemoObj::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mLayoutB, lbl_807CF764, 0);
    }
    if (MR::isAnimStopped(mLayoutB, 0)) {
        MR::resumeTimeKeepDemo(this);
        UnnamedDemoCallee::fn_80147830();
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
// NONMATCHING: action name built with memcpy instead of the target's struct copies
void OpeningDemoObj::updateActionSound(s32 step) {
    if (MR::isDead(mModelA) && (step % lbl_807CF760) == 0) {
        mModelA->appear();
        s32 action = lbl_80647D7C[((step / lbl_807CF760) % 9)];
        if (action != -1) {
            char name[0x10];
            memcpy(name, lbl_80647DA0, sizeof(name));
            s32 len = strlen(name);
            name[len] += static_cast< s8 >(action);
            MR::startAction(mModelA, name);
            MR::startActionSound(mModelA, lbl_80680630, -1, -1, -1);
        }
    }
    if (!MR::isDead(mModelA) && MR::isActionEnd(mModelA)) {
        mModelA->kill();
    }
}
