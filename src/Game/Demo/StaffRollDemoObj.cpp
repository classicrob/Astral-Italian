#include "Game/Demo/StaffRollDemoCamera.hpp"
#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Demo/StaffRollDemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Screen/StaffRoll.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Util.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/NameObj/NameObjFactory.hpp"

extern "C" int snprintf(char*, u32, const char*, ...);

const char* const sPageTurnDemoName = "絵本ページめくり";
const char* const sStartFadeDemoName = "スタッフロール開始フェード";
const char* const sEndFadeDemoName = "スタッフロール終了フェード";

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
    StaffRollDemoObjNrvH StaffRollDemoObjNrvH::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(StaffRollDemoObjNrvE, StaffRollDemoObj, E);
    NEW_NERVE(StaffRollDemoObjNrvF, StaffRollDemoObj, F);
    NEW_NERVE(StaffRollDemoObjNrvG, StaffRollDemoObj, G);
};  // namespace NrvStaffRollDemoObj

StaffRollDemoObj::StaffRollDemoObj(const char* pName) : LiveActor(pName), mStaffRollObj(nullptr), mChildA(nullptr), mStep98(0), mStep9C(0), mCameraInfo(nullptr), mActorArray(), mActorFill(0), mChildB(nullptr), mChildC(nullptr), mRollA(nullptr), mRollB(nullptr) {
}

void StaffRollDemoObj::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvStart), 0);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    mRollA = new StaffRollDemoCamera(this, mCameraInfo, 0);
    mRollB = new StaffRollDemoCamera(this, mCameraInfo, 1);
    mRollA->start();
    mStaffRollObj = new StaffRoll();
    mStaffRollObj->initWithoutIter();
    mStaffRollObj->mDemoCamera = mRollB;
        initObjects(rIter);
    MR::initUseStageSwitchWriteA(this, rIter);
    makeActorAppeared();
}

// NONMATCHING: regalloc and stack scheduling
void StaffRollDemoObj::initObjects(const JMapInfoIter& rIter) {
    LiveActor* sky = nullptr;
    s32 childNum = MR::getChildObjNum(rIter);
    for (s32 i = 0; i < childNum; i++) {
        const char* pName = nullptr;
        MR::getChildObjName(&pName, rIter, i);
        if (MR::isEqualString(pName, "StaffRollSky")) {
            sky = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
            break;
        }
    }
    mChildA = sky;
    mActorArray.mArr = new LiveActor*[3];
    mActorArray.mMaxSize = 3;
    s32 actorNum = MR::getChildObjNum(rIter);
    for (s32 i = 0; i < actorNum; i++) {
        const char* pName = nullptr;
        MR::getChildObjName(&pName, rIter, i);
        if (MR::isEqualString(pName, "StaffRollDecorateFaceShip")) {
            LiveActor* pActor = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
            pActor->makeActorDead();
            f32 frame = static_cast< f32 >(mActorFill);
                        MR::setBckFrameAndStop(pActor, frame);
            MR::setBtpFrameAndStop(pActor, frame);
            s32 idx = mActorFill;
            mActorFill = idx + 1;
            mActorArray.mArr[idx] = pActor;
        } else if (MR::isEqualString(pName, "StaffRollDecorateComet")) {
            mChildB = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
            const char* pAnim = "CometNormal";
            if (MR::getCurrentScenarioNo() == 2) {
                pAnim = "CometGreen";
            }
            MR::startAction(mChildB, pAnim);
            mChildB->makeActorDead();
        } else if (MR::isEqualString(pName, "PeachCastleNightPlanet")) {
            mChildC = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
            mChildC->makeActorDead();
        }
    }
}

void StaffRollDemoObj::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::openWipeFade(0xB4);
        MR::tryStartDemo(this, sStartFadeDemoName);
        MR::endStartPosCamera();
        MR::hidePlayer();
        MR::offPlayerControl();
        MR::startStarPointerModeEnding(this);
        setAndShowPlayer();
    }
    mRollA->update(false);
    if (MR::isGreaterEqualStep(this, 0xB4)) {
        MR::endDemo(this, sStartFadeDemoName);
        MR::hidePlayer();
        MR::offPlayerControl();
        // Target calls the LayoutActor vtable slot 0x2C here (appear); see NOTES.
        static_cast< LayoutActor* >(mStaffRollObj)->appear();
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvB));
    }
}

// NONMATCHING: equivalent search-result branch folding and regalloc
void StaffRollDemoObj::setAndShowPlayer() {
    char name[0x100];
    s32 page = mStep98;
    s32 frameMax = static_cast< s32 >(MR::getBvaFrameMax(mChildA));
    s32 found = 0;
    TPos3f position;
    for (s32 i = 0; i < frameMax; i++) {
        snprintf(name, 0x100, "マリオ位置%02d空%02d", page, i);
        if (MR::tryFindNamePos(name, position) != 0) {
            mStep9C = i;
            found = 1;
            break;
        }
    }
    if (!found) {
        snprintf(name, 0x100, "マリオ位置%02d空%02d", 0, 0);
        mStep9C = 0;
        mStep98 = 0;
        mStep9C = 0;
    }
    MR::setPlayerPos(name);
    MR::setPlayerStateWait();
    if (!MR::isPlayerVisible()) {
        MR::showPlayer();
        MR::onPlayerControl(true);
    }
    LiveActor* sky = mChildA;
    s32 index = mStep9C;
    s32 frame = index % static_cast< s32 >(MR::getBvaFrameMax(sky));
    MR::setBvaFrameAndStop(sky, static_cast< f32 >(frame));
}

// Shows the actors belonging to the current step; the others are hidden.
void StaffRollDemoObj::updateActorVisibility() {
    s32 targetIdx;
    if (mStep98 == 2) {
        targetIdx = 2;
    } else if (mStep98 == 6) {
        targetIdx = 1;
    } else {
        targetIdx = mStep98 == 8 ? 0 : -1;
    }
    for (LiveActor** actor = mActorArray.mArr; actor != mActorArray.mArr + mActorFill; actor++) {
        if (actor - mActorArray.mArr == targetIdx) {
            (*actor)->makeActorAppeared();
        } else {
            (*actor)->makeActorDead();
        }
    }
}

// NONMATCHING: equivalent redundant member null check omitted
StaffRollDemoObj::~StaffRollDemoObj() {
}

// Nerve B: wait for the roll, then hand off to C.
void StaffRollDemoObj::exeB() {
    mRollA->update(false);
    if (mStaffRollObj->isCurrentPartEnd()) {
        MR::onSwitchA(this);
        mRollA->end();
        MR::tryStartDemo(this, sPageTurnDemoName);
        for (LiveActor** actor = mActorArray.mArr; actor != mActorArray.mArr + mActorFill; actor++) {
            MR::requestMovementOn(*actor);
        }
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvD));
    }
}

// Nerve C: the roll is shown; loops back to B until the roll is done.
void StaffRollDemoObj::exeC() {
    if (mStaffRollObj->isLastPartEnd()) {
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvE));
        return;
    }
    if (mStaffRollObj->isCurrentPartEnd()) {
        MR::resetYoshiTongueForDemo();
        MR::tryStartDemo(this, sPageTurnDemoName);
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvD));
    }
}

// Nerve D: the roll is running; advance the step counter on each entry.
void StaffRollDemoObj::exeD() {
    if (MR::isFirstStep(this)) {
        MR::startCameraInterpolation(0);
        MR::startStoryBookTurnPage();
        mStep98 += 1;
        return;
    }
    if (MR::isStoryBookTurnPageStart() != 0) {
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
void StaffRollDemoObj::exeE() {
    if (MR::isFirstStep(this)) {
        mChildB->makeActorAppeared();
    }
    s32 frame = mStaffRollObj->getElapsedFrame();
    if (static_cast< u32 >(frame) >= 0x32C8) {
        MR::offPlayerControl();
    }
    if (mStaffRollObj->isEnd()) {
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
        mChildC->makeActorDead();
        MR::tryStartDemo(this, sEndFadeDemoName);
        setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvG));
    }
    MR::setSeVolumeSetLevel(0xD, 0);
}

// Nerve G: last step, request the next room.
void StaffRollDemoObj::exeG() {
    if (MR::isGreaterEqualStep(this, 0x78)) {
        MR::endDemo(this, sEndFadeDemoName);
        GameSequenceFunction::requestChangeStageChildRoom();
    }
    MR::setSeVolumeSetLevel(0xD, 0);
}

// Nerve H (custom execute body): when the wait flag clears, end the demo and return to C.
void NrvStaffRollDemoObj::StaffRollDemoObjNrvH::execute(Spine* pSpine) const {
    StaffRollDemoObj* pActor = reinterpret_cast< StaffRollDemoObj* >(pSpine->mExecutor);
    if (MR::isStoryBookTurnPageEnd() != 0) {
        MR::endDemo(pActor, sStartFadeDemoName);
        pActor->setNerve(GET_NERVE(StaffRollDemoObj, StaffRollDemoObjNrvC));
    }
}

