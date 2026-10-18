#include "Game/Demo/StaffRollDemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Screen/StaffRoll.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Util.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Demo/UnnamedDemoCallee.hpp"
#include "Game/NameObj/NameObjFactory.hpp"

extern "C" int snprintf(char*, u32, const char*, ...);

extern "C" char lbl_80680B80[];
extern "C" char lbl_80680C40[];
extern "C" char lbl_807D1E78[];
extern "C" char lbl_807D1E7C[];
extern "C" char lbl_807D1E80[];
extern "C" char lbl_807D1E84[];
extern "C" char lbl_807D1E88[];
extern "C" char lbl_807D1E8C[];
extern "C" char lbl_807D1E90[];
extern "C" char lbl_807D1E94[];
extern "C" const char* lbl_807D98B0;
extern "C" const char* lbl_807D98B4;
extern "C" const char* lbl_807D98B8;

namespace NrvStaffRollDemoObj {
    NEW_NERVE(StaffRollDemoObjNrvStart, StaffRollDemoObj, Start);
    NEW_NERVE(StaffRollDemoObjNrvB, StaffRollDemoObj, B);
    NEW_NERVE(StaffRollDemoObjNrvC, StaffRollDemoObj, C);
    NEW_NERVE(StaffRollDemoObjNrvD, StaffRollDemoObj, D);
    class StaffRollDemoObjNrvH : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static StaffRollDemoObjNrvH sInstance;
    };
    NEW_NERVE(StaffRollDemoObjNrvE, StaffRollDemoObj, E);
    NEW_NERVE(StaffRollDemoObjNrvF, StaffRollDemoObj, F);
    NEW_NERVE(StaffRollDemoObjNrvG, StaffRollDemoObj, G);
};  // namespace NrvStaffRollDemoObj

StaffRollDemoObj::StaffRollDemoObj(const char* pName) : LiveActor(pName), mStaffRollObj(nullptr), mChildA(nullptr), mStep98(0), mStep9C(0), mCameraInfo(nullptr), mActors(nullptr), mActorCount(0), mActorFill(0), mChildB(nullptr), mChildC(nullptr), mRollA(nullptr), mRollB(nullptr) {
}

// NONMATCHING: roll allocation via fn_801440F0 and StaffRoll+0xE4 store not reproduced
void StaffRollDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvStart), 0);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    mRollA = UnnamedDemoCallee::fn_801440F0(this, mCameraInfo, 0);
    mRollB = UnnamedDemoCallee::fn_801440F0(this, mCameraInfo, 1);
    UnnamedDemoCallee::fn_80144280(mRollA);
    mStaffRollObj = new StaffRoll();
    mStaffRollObj->initWithoutIter();
    // NONMATCHING: target also stores mRollB at StaffRoll+0xE4 (no named field in StaffRoll.hpp yet).
    initObjects(rIter);
    MR::initUseStageSwitchWriteA(this, rIter);
    makeActorAppeared();
}

// NONMATCHING: bck/btp frame setter nesting differs
void StaffRollDemoObj::initObjects(const JMapInfoIter& rIter) {
    s32 childNum = MR::getChildObjNum(rIter);
    for (s32 i = 0; i < childNum; i++) {
        const char* pName = nullptr;
        MR::getChildObjName(&pName, rIter, i);
        if (MR::isEqualString(pName, lbl_80680B80 + 0x4C)) {
            mChildA = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
            break;
        }
    }
    mActorCount = 3;
    mActors = new LiveActor*[mActorCount];
    mActorFill = 0;
    s32 actorNum = MR::getChildObjNum(rIter);
    for (s32 i = 0; i < actorNum; i++) {
        const char* pName = nullptr;
        MR::getChildObjName(&pName, rIter, i);
        if (MR::isEqualString(pName, lbl_80680B80 + 0x5C)) {
            LiveActor* pActor = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
            f32 frame = static_cast< f32 >(mActorFill);
            // NONMATCHING: target nests the bck/btp setters; order of the two calls differs.
            MR::setBckFrameAndStop(pActor, frame);
            MR::setBtpFrameAndStop(pActor, frame);
            s32 idx = mActorFill;
            mActorFill = idx + 1;
            mActors[idx] = pActor;
        } else if (MR::isEqualString(pName, lbl_80680B80 + 0x78)) {
            mChildB = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
            const char* pAnim = lbl_80680B80 + 0x9C;
            if (MR::getCurrentScenarioNo() == 2) {
                pAnim = lbl_80680B80 + 0x90;
            }
            MR::startAction(mChildB, pAnim);
            mChildB->makeActorAppeared();
        } else if (MR::isEqualString(pName, lbl_80680B80 + 0xA8)) {
            mChildC = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
            mChildC->makeActorDead();
        }
    }
}

// NONMATCHING: LayoutActor vt slot 0x2C used for the roll (inferred appear)
void StaffRollDemoObj::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::openWipeFade(0xB4);
        MR::startStarPointerModeEnding(this);
        setAndShowPlayer();
    }
    UnnamedDemoCallee::fn_801442A0(mRollA, 0);
    if (MR::isGreaterEqualStep(this, 0xB4)) {
        MR::offPlayerControl();
        MR::hidePlayer();
        MR::endDemo(this, lbl_807D98B4);
        // Target calls the LayoutActor vtable slot 0x2C here (appear); see NOTES.
        static_cast< LayoutActor* >(mStaffRollObj)->appear();
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvB));
    }
}

// NONMATCHING: loop and string formatting simplified
void StaffRollDemoObj::setAndShowPlayer() {
    char name[0x100];
    s32 frameMax = static_cast< s32 >(MR::getBvaFrameMax(mChildA));
    s32 found = 0;
    for (s32 i = 0; i < frameMax; i++) {
        snprintf(name, 0x100, lbl_80680C40, mStep98, i);
        if (UnnamedDemoCallee::fn_8004E770(name, nullptr) != 0) {
            mStep9C = i;
            found = 1;
            break;
        }
    }
    if (!found) {
        mStep98 = 0;
        mStep9C = 0;
    }
    MR::setPlayerPos(name);
    MR::setPlayerStateWait();
    if (UnnamedDemoCallee::fn_80051080() == nullptr) {
        MR::onPlayerControl(true);
        MR::showPlayer();
    }
    s32 frame = mStep9C % static_cast< s32 >(MR::getBvaFrameMax(mChildA));
    MR::setBvaFrameAndStop(mChildA, static_cast< f32 >(frame));
}

// Shows the actors belonging to the current step; the others are hidden.
// NONMATCHING: unverified branch layout
void StaffRollDemoObj::updateActorVisibility() {
    s32 targetIdx;
    if (mStep98 == 2) {
        targetIdx = 2;
    } else if (mStep98 == 6) {
        targetIdx = 1;
    } else {
        targetIdx = 0;
    }
    for (s32 i = 0; i < mActorCount; i++) {
        if (i == targetIdx) {
            mActors[i]->makeActorAppeared();
        } else {
            mActors[i]->makeActorDead();
        }
    }
}

StaffRollDemoObj::~StaffRollDemoObj() {
}

// Nerve B: wait for the roll, then hand off to C.
// NONMATCHING: unverified nerve/slot details
void StaffRollDemoObj::exeB() {
    UnnamedDemoCallee::fn_801442A0(mRollA, 0);
    if (UnnamedDemoCallee::fn_80492640(mStaffRollObj) != 0) {
        MR::onSwitchA(this);
        UnnamedDemoCallee::fn_801442D0(mRollA);
        MR::tryStartDemo(this, lbl_807D98B0);
        for (s32 i = 0; i < mActorCount; i++) {
            MR::requestMovementOn(mActors[i]);
        }
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvD));
    }
}

// Nerve C: the roll is shown; loops back to B until the roll is done.
// NONMATCHING: unverified call order
void StaffRollDemoObj::exeC() {
    if (UnnamedDemoCallee::fn_804926A0(mStaffRollObj) != 0) {
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvE));
        return;
    }
    if (UnnamedDemoCallee::fn_80492640(mStaffRollObj) != 0) {
        UnnamedDemoCallee::fn_80422740();
        MR::tryStartDemo(this, lbl_807D98B0);
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvD));
    }
}

// Nerve D: the roll is running; advance the step counter on each entry.
// NONMATCHING: unverified call order
void StaffRollDemoObj::exeD() {
    if (MR::isFirstStep(this)) {
        MR::startCameraInterpolation(0);
        UnnamedDemoCallee::fn_80147830();
        mStep98 += 1;
        return;
    }
    if (UnnamedDemoCallee::fn_80147780() != 0) {
        setAndShowPlayer();
        updateActorVisibility();
        if (mStep98 == 1) {
            MR::endStarPointerMode(this);
        }
        if (mStep98 == 8) {
            mChildC->makeActorAppeared();
        }
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvH));
    }
}

// Nerve E: wait on the roll's end-of-part flag.
// NONMATCHING: unverified frame compare
void StaffRollDemoObj::exeE() {
    if (MR::isFirstStep(this)) {
        mChildB->makeActorAppeared();
    }
    s32 frame = UnnamedDemoCallee::fn_80492760(mStaffRollObj);
    if (frame >= 0x32C8) {
        MR::offPlayerControl();
    }
    if (UnnamedDemoCallee::fn_80492730(mStaffRollObj) != 0) {
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvF));
    }
}

// Nerve F: close the wipe, then move on to G.
void StaffRollDemoObj::exeF() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(0x78);
    }
    if (MR::isWipeBlank()) {
        MR::offSwitchA(this);
        mChildC->kill();
        MR::tryStartDemo(this, lbl_807D98B8);
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvG));
    }
    UnnamedDemoCallee::fn_8005AC70(0xD, 0);
}

// Nerve G: last step, request the next room.
void StaffRollDemoObj::exeG() {
    if (MR::isGreaterEqualStep(this, 0x78)) {
        MR::endDemo(this, lbl_807D98B8);
        GameSequenceFunction::requestChangeStageChildRoom();
    }
    UnnamedDemoCallee::fn_8005AC70(0xD, 0);
}

// Nerve H (custom execute body): when the wait flag clears, end the demo and return to C.
void NrvStaffRollDemoObj::StaffRollDemoObjNrvH::execute(Spine* pSpine) const {
    StaffRollDemoObj* pActor = reinterpret_cast< StaffRollDemoObj* >(pSpine->mExecutor);
    if (UnnamedDemoCallee::fn_801476E0() != 0) {
        MR::endDemo(pActor, lbl_807D98B4);
        pActor->setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvC));
    }
}

NrvStaffRollDemoObj::StaffRollDemoObjNrvH NrvStaffRollDemoObj::StaffRollDemoObjNrvH::sInstance ATTRIBUTE_WEAK;
