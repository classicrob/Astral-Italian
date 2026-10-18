#include "Game/Util/PlayerUtil.hpp"
#include "Game/NPC/PostmanEvent.hpp"
#include "Game/Screen/PostManLetter.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/System/MarioFaceShipEventDataHolder.hpp"
#include "Game/Demo/DemoFunction.hpp"
#pragma auto_inline off

#include "Game/NPC/KinopioPostman.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Util.hpp"

// Data of this unit (strings and nerve instances in the target's .data/.sbss).
extern "C" const char lbl_806B97E0[];
extern "C" const char lbl_806B8EE0[];
extern "C" const char lbl_806B8EE8[];
extern "C" const char lbl_806B7AEC[];
extern "C" const char lbl_806B7AE0[];
extern "C" const char lbl_806B7AD4[];
extern "C" const char lbl_806B7AC8[];
extern "C" const char lbl_806B99AC[];
extern "C" const char lbl_806B99C0[];
extern "C" const char lbl_806B9918[];
extern "C" const char lbl_806B9908[];
extern "C" const char lbl_806B9A18[];
extern "C" const char lbl_807CFE90[];
extern "C" const char lbl_807CFE94[];
extern "C" const char lbl_807CFE98[];
extern "C" const char lbl_807CFE9C[];
extern "C" const char* const lbl_8064B9C0[9];
extern "C" s32 lbl_807E3100;

#define KP_STR(ofs) (lbl_806B97E0 + (ofs))


namespace NrvKinopioPostman {
    NEW_NERVE(KinopioPostmanNrvTalk, KinopioPostman, Talk);
    NEW_NERVE(KinopioPostmanNrvReaction, KinopioPostman, Reaction);
    NEW_NERVE(KinopioPostmanNrvDemo, KinopioPostman, Demo);
    NEW_NERVE(KinopioPostmanNrvFinish, KinopioPostman, Finish);
    NEW_NERVE(KinopioPostmanNrvWait, KinopioPostman, Wait);
    NEW_NERVE(KinopioPostmanNrvWaitNear, KinopioPostman, Wait);
}

KinopioPostman::KinopioPostman(const char* pName) : NPCActor(pName) {
    mLayout = nullptr;
    mIsEventFlag = false;
    mIsReplyFlag = false;
    mStepNum = 7;
    mChildObj = nullptr;
    mPostmanEvent = nullptr;
    mWaitTimer = -1;
}

// NONMATCHING: callback member-pointer copies, item construction and stack/register allocation differ.
void KinopioPostman::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps(KP_STR(0x128));
    caps.setDefault();
    caps.mMessage = false;
    caps.mWaitNerve = GET_NERVE(KinopioPostman, KinopioPostmanNrvTalk);
    caps.mReactionNerve = GET_NERVE(KinopioPostman, KinopioPostmanNrvReaction);
    initialize(rIter, caps, KP_STR(0x128), nullptr, nullptr);
    MR::invalidateHitSensor(this, KP_STR(0x130));
    setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear));

    TVec3f rot(mRotation);
    rot.x *= 0.017453292f;
    rot.y *= 0.017453292f;
    rot.z *= 0.017453292f;
    _A0.setEuler(rot);
    MR::setScale(this, 1.2f);
    bool isTalkReady = this->initTalkCtrl();

    _12C = 450.0f;
    _130 = lbl_806B7AEC;
    _134 = lbl_806B7AE0;
    _138 = lbl_806B7AD4;
    _13C = lbl_806B7AC8;
    mParam._14 = lbl_806B8EE8;
    mParam._18 = lbl_806B8EE0;
    mParam._1C = lbl_806B97E0 + 0x138;
    mParam._20 = lbl_806B97E0 + 0x138;
    _11C = lbl_806B97E0 + 0x140;
    _120 = lbl_806B97E0 + 0x140;
    _124 = 1;
    MR::startBrk(this, KP_STR(0x148));
    MR::setBrkFrameAndStop(this, 2.0f);

    NPCActorItem item("Kinopio");
    MR::getNPCItemData(&item, 3);
    equipment(item, false);
    MR::onCalcGravity(this);
    MR::initUseStageSwitchReadA(this, rIter);
    this->initChildObj(rIter);

    MR::listenStageSwitchOnA(this, MR::FunctorV0M<KinopioPostman*, void (KinopioPostman::*)()>(this, &KinopioPostman::onSwitchA));

    const char* const* pNames = lbl_8064B9C0;
    for (s32 i = 0; i < 9; ++i) {
        this->tryRegisterDemo(pNames[i], rIter);
    }
    mIsReplyFlag = PostmanEvent::isCurrentReplyEvent();
    mPostmanEvent = PostmanEvent::createPlayerDemo(rIter);

    if (isTalkReady) {
        makeActorAppeared();
        return;
    }
    makeActorDead();
}

void KinopioPostman::initAfterPlacement() {
    MR::moveCoordAndTransToRailStartPoint(this);
}

void KinopioPostman::control() {
    NPCActor::control();
    if (mLayout != nullptr) {
        mLayout->movement();
    }
}

// fn_8034FAB0
bool KinopioPostman::eventRead(u32 step) {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (step == 0) {
        if (pThis->tryEndLetter()) {
            return true;
        }
        pThis->tryStartLetter();
        return false;
    }
    return true;
}

// fn_8034FB10
// NONMATCHING: equivalent return branches and store scheduling differ.
bool KinopioPostman::eventGive(u32 step) {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (step == 0) {
        if (pThis->tryEndLetter()) {
            return true;
        }
        pThis->tryStartLetter();
        return false;
    }
    if (step == 2) {
        PostmanEvent::receiveLetter(pThis, pThis->mLayout->getCurrentLetterId());
        pThis->mStepNum = 2;
        pThis->mMsgCtrl = pThis->mTalkCtrls[2];
        pThis->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvFinish));
        return true;
    }
    return true;
}

// fn_8034FBB0
// NONMATCHING: equivalent false-return branch layout differs.
bool KinopioPostman::branchImportant(u32 step) {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (step == 0) {
        if (MR::isValidPostmanImportantEvent(pThis->mLayout->getCurrentLetterId())) {
            s32 count = MR::getPostmanImportantEventReward(pThis->mLayout->getCurrentLetterId());
            if (count > 0) {
                MR::setMessageArg(pThis->mTalkCtrls[3], count);
                return true;
            }
        }
        return false;
    }
    if (step == 1) {
        if (!MR::isPlayerLuigi()) {
            return false;
        }
        return (pThis->mLayout->getCurrentLetterId() - 1) == 0;
    }
    return false;
}

// fn_8034FC60
// NONMATCHING: equivalent false-return branch layout differs.
bool KinopioPostman::branchLuigi(u32 step) {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (step == 0) {
        if (!MR::isPlayerLuigi()) {
            return false;
        }
        return (pThis->mLayout->getCurrentLetterId() - 2) == 0;
    }
    return false;
}

// fn_8034FCC0
// NONMATCHING: equivalent return branches and store scheduling differ.
bool KinopioPostman::eventReply(u32 step) {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (step == 0) {
        if (pThis->tryEndLetter()) {
            return true;
        }
        pThis->tryStartLetter();
        return false;
    }
    if (step == 2) {
        PostmanEvent::receiveLetter(pThis, pThis->mLayout->getCurrentLetterId());
        MR::startActionSound(pThis, lbl_806B99AC, -1, -1, -1);
        pThis->mStepNum = 2;
        pThis->mMsgCtrl = pThis->mTalkCtrls[2];
        pThis->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvFinish));
        return true;
    }
    return true;
}


// fn_8034F600
// NONMATCHING: callback table addressing and pointer/index loop allocation differ.
bool KinopioPostman::initTalkCtrl() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (!PostmanEvent::isAvailable()) {
        return false;
    }
    for (s32 i = 0; i < 7; ++i) {
        pThis->mTalkCtrls[i] = PostmanEvent::createTalkCtrl(pThis, i);
    }

    MR::registerEventFunc(pThis->mTalkCtrls[3], TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)>(pThis, &KinopioPostman::eventGive));
    MR::registerBranchFunc(pThis->mTalkCtrls[3], TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)>(pThis, &KinopioPostman::branchImportant));
    MR::registerEventFunc(pThis->mTalkCtrls[2], TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)>(pThis, &KinopioPostman::eventRead));
    MR::registerEventFunc(pThis->mTalkCtrls[4], TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)>(pThis, &KinopioPostman::eventGive));
    MR::registerBranchFunc(pThis->mTalkCtrls[4], TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)>(pThis, &KinopioPostman::branchLuigi));
    MR::registerEventFunc(pThis->mTalkCtrls[5], TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)>(pThis, &KinopioPostman::eventGive));
    MR::registerEventFunc(pThis->mTalkCtrls[6], TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)>(pThis, &KinopioPostman::eventReply));
    MR::registerEventFunc(pThis->mTalkCtrls[1], TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)>(pThis, &KinopioPostman::eventGive));
    MR::registerEventFunc(pThis->mTalkCtrls[0], TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)>(pThis, &KinopioPostman::eventRead));

    pThis->mMsgCtrl = pThis->mTalkCtrls[0];
    pThis->mLayout = PostmanEvent::createLetter();
    pThis->mStepNum = PostmanEvent::getTalkType(pThis->mLayout);
    pThis->mIsEventFlag = (pThis->mStepNum - 2) != 0;
    pThis->mMsgCtrl = pThis->mTalkCtrls[pThis->mStepNum];
    return true;
}

// fn_8034F900
bool KinopioPostman::tryRegisterDemo(const char* pName, const JMapInfoIter& rIter) {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    bool hasLetter = pThis->mLayout != nullptr;
    if (MR::tryRegisterDemoCast(pThis, pName, rIter)) {
        if (hasLetter) {
            DemoFunction::registerDemoTalkMessageCtrlDirect(pThis, pThis->mMsgCtrl, pName);
        }
        MR::registerDemoActionFunctorDirect(this, MR::FunctorV0M<KinopioPostman*, void (KinopioPostman::*)()>(this, &KinopioPostman::startDemo), pName, nullptr);
        return true;
    }
    return false;
}

// fn_8034F9B0
void KinopioPostman::initChildObj(const JMapInfoIter& rIter) {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    for (s32 i = 0; i < MR::getChildObjNum(rIter); ++i) {
        const char* pChildName = nullptr;
        MR::getChildObjName(&pChildName, rIter, i);
        if (MR::isEqualString(pChildName, lbl_806B9908)) {
            pThis->mChildObj = static_cast<LiveActor*>(NameObjFactory::initChildObj(rIter, i));
            break;
        }
    }
    pThis->mChildObj->makeActorDead();
}

// fn_8034FA60
void KinopioPostman::onSwitchA() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (!PostmanEvent::isAvailable()) {
        pThis->mChildObj->makeActorAppeared();
    }
}

// fn_8034FDE0
bool KinopioPostman::trySelectDefaultTalk() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (!pActor->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWait))) {
        return false;
    }
    if (!pThis->mIsEventFlag) {
        return false;
    }
    pThis->mMsgCtrl = pThis->mTalkCtrls[0];
    return true;
}

// fn_8034FE40
bool KinopioPostman::trySelectEventTalk() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (!pActor->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWait))) {
        return false;
    }
    if (!pThis->mIsEventFlag) {
        return false;
    }
    pThis->mMsgCtrl = pThis->mTalkCtrls[pThis->mStepNum];
    return true;
}

// fn_8034FEB0
bool KinopioPostman::tryEndLetter() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (!MR::isDead(pThis->mLayout) && pThis->mLayout->isEnd()) {
        if (pThis->mWaitTimer < 0) {
            pThis->mWaitTimer = 0x23;
        }
        pThis->mWaitTimer -= 1;
        if (pThis->mWaitTimer < 0) {
            pThis->mIsEventFlag = false;
            pThis->mLayout->kill();
            return true;
        }
    }
    return false;
}

// fn_8034FF50
bool KinopioPostman::tryStartLetter() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (MR::isDead(pThis->mLayout)) {
        if (pThis->mWaitTimer < 0) {
            pThis->mWaitTimer = lbl_807E3100;
        }
        pThis->mWaitTimer -= 1;
        if (pThis->mWaitTimer < 0) {
            pThis->mLayout->appear();
            return true;
        }
    }
    return false;
}

// KinopioPostmanFunction::turnQuaternion: turns the quaternion's facing toward rTo around its up axis, then blends up/front.
// NONMATCHING: quaternion component reloads and float/register scheduling differ.
void KinopioPostmanFunction::turnQuaternion(TQuat4f* pQuat, const TVec3f& rUp, const TVec3f& rTo, f32 rate) {
    TVec3f sp2C(rTo);
    TVec3f sp20(rUp);
    f32 x = pQuat->x;
    f32 y = pQuat->y;
    f32 z = pQuat->z;
    f32 w = pQuat->w;
    TVec3f sp14;
    sp14.set<f32>(2.0f * (x * y) - 2.0f * (w * z), (1.0f - 2.0f * (x * x)) - 2.0f * (z * z), 2.0f * (y * z) + 2.0f * (w * x));
    TVec3f sp8;
    sp8.set<f32>(2.0f * (x * z) + 2.0f * (w * y), 2.0f * (y * z) - 2.0f * (w * x), (1.0f - 2.0f * (x * x)) - 2.0f * (y * y));
    MR::turnVecToVecCosOnPlane(&sp8, sp2C, sp14, rate);
    MR::blendQuatUpFront(pQuat, sp20, sp8, 1.0f, 1.0f);
}





// fn_80350120
void KinopioPostman::exeTalk() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    (void)pActor;
    if (MR::isFirstStep(pThis)) {
        MR::startAction(pThis, lbl_806B99C0);
    }
    TVec3f sp14 = *MR::getPlayerPos() - pThis->mPosition;
    TVec3f sp8 = -pThis->mGravity;
    KinopioPostmanFunction::turnQuaternion(&pThis->_A0, sp8, sp14, 0.997f);
    if (!MR::tryStartReactionAndPushNerve(pThis, GET_NERVE(KinopioPostman, KinopioPostmanNrvReaction))) {
        MR::tryTalkNearPlayerAndStartMoveTalkAction(pThis);
    }
}

// fn_803501C0
void KinopioPostman::exeReaction() {
    const char* data = lbl_806B97E0;
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;

    if (pThis->_D8 != 0) {
        MR::startActionSound(pThis, data + 0x1E8, -1, -1, -1);
        MR::startActionSound(pThis, data + 0x1F8, -1, -1, -1);
    }
    if (pThis->isPointingSe()) {
        MR::startDPDHitSound();
        MR::startActionSound(pThis, data + 0x20C, -1, -1, -1);
    }
    if (pThis->_DB != 0) {
        MR::startActionSound(pThis, data + 0x220, -1, -1, -1);
    }
    MR::tryStartReactionAndPopNerve(pThis);
}

// fn_80350290
void KinopioPostman::exeWait() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (MR::isFirstStep(pActor)) {
        if (!MR::isExistRail(pActor)) {
            MR::onCalcShadowOneTime(pActor, nullptr);
        }
        pThis->trySelectDefaultTalk();
    }
    if (!MR::tryStartReactionAndPushNerve(pThis, GET_NERVE(KinopioPostman, KinopioPostmanNrvReaction))) {
        if (MR::isMarioFaceShipNpcTalkAllowed()) {
            TalkMessageCtrl* pCtrl = pThis->mMsgCtrl;
            if (MR::tryTalkNearPlayerAtEnd(pCtrl)) {
                MR::startAction(pThis, lbl_806B9A18);
            }
        }
        if (MR::isActionContinuous(pThis)) {
            if (pThis->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear))) {
                if (MR::isAnyAnimPlaying(pThis, lbl_806B9A18)) {
                    TVec3f sp14 = *MR::getPlayerPos() - pThis->mPosition;
                    TVec3f sp8 = -pThis->mGravity;
                    KinopioPostmanFunction::turnQuaternion(&pThis->_A0, sp8, sp14, 0.997f);
                }
            }
        } else {
            if (pThis->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWait))) {
                if (MR::isNearPlayer(pThis, 200.0f)) {
                    MR::startAction(pThis, lbl_806B9A18);
                    pThis->trySelectEventTalk();
                    pThis->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear));
                    return;
                }
                MR::tryStartMoveTalkAction(pThis);
                return;
            }
            if (pThis->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear))) {
                if (!MR::isNearPlayer(pThis, 300.0f)) {
                    pThis->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWait));
                }
                MR::tryStartTalkAction(pThis);
            }
        }
    }
}

// fn_80350440
void KinopioPostman::exeFinish() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pThis, lbl_806B9A18);
    }
    if (MR::isActionEnd(pThis)) {
        if (pThis->mIsReplyFlag) {
            pThis->mPostmanEvent->makeActorDead();
        }
        pThis->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear));
    }
}

// fn_803504C0
// NONMATCHING: local vector stack offsets differ.
void KinopioPostman::exeDemo() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    if (MR::isFirstStep(pActor)) {
        if (!MR::isExistRail(pActor)) {
            MR::onCalcShadowOneTime(pActor, nullptr);
        }
        pThis->trySelectDefaultTalk();
        TVec3f sp8;
        MR::findNamePos(lbl_806B97E0, &sp8, nullptr);
        TVec3f sp20 = sp8 - pThis->mPosition;
        TVec3f sp14 = -pThis->mGravity;
        KinopioPostmanFunction::turnQuaternion(&pThis->_A0, sp14, sp20, -1.0f);
        MR::startAction(pThis, lbl_806B9918);
    }
    if (!MR::isTimeKeepDemoActive()) {
        MR::startAction(pThis, lbl_806B9A18);
        if (pThis->mIsReplyFlag) {
            MR::startPlayerNormalBva();
        }
        pThis->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear));
    }
}

// fn_803505A0
void KinopioPostman::startDemo() {
    KinopioPostman* pThis = this;
    LiveActor* pActor = this;
    MR::moveCoordAndTransToRailStartPoint(pActor);
    MR::followRailPoseOnGround(pThis, 1.0f);
    if (pThis->mIsReplyFlag) {
        pThis->mPostmanEvent->makeActorAppeared();
    }
    pThis->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvDemo));
}

KinopioPostman::~KinopioPostman() {}
