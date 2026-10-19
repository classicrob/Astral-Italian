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






extern const char lbl_807CFE90[1];
extern const char lbl_807CFE94[1];
extern const char lbl_807CFE98[1];
extern const char lbl_807CFE9C[1];
static const char* const sMarioPosName = "\203}\203\212\203I\210\312\222u[\227X\225\326\211\256\202\263\202\361\203C\203x\203\223\203g]";

extern const volatile s32 gKinopioPostmanLetterStartDelay;




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
    NPCActorCaps caps("Kinopio");
    caps.setDefault();
    caps.mMessage = false;
    caps.mWaitNerve = GET_NERVE(KinopioPostman, KinopioPostmanNrvTalk);
    caps.mReactionNerve = GET_NERVE(KinopioPostman, KinopioPostmanNrvReaction);
    initialize(rIter, caps, "Kinopio", nullptr, nullptr);
    MR::invalidateHitSensor(this, "Pickel");
    setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear));

    TVec3f rot(mRotation);
    rot.x *= 0.017453292f;
    rot.y *= 0.017453292f;
    rot.z *= 0.017453292f;
    _A0.setEuler(rot);
    MR::setScale(this, 1.2f);
    bool isTalkReady = this->initTalkCtrl();

    _130 = cNPCSpinDefault;
    _134 = cNPCTrampledDefault;
    _138 = cNPCPointingDefault;
    _13C = cNPCReactionDefault;
    _12C = 450.0f;
    mParam._14 = cNPCWaitDefault;
    mParam._18 = cNPCTurnDefault;
    mParam._1C = "Talk";
    mParam._20 = "Talk";
    _11C = "Walk";
    _120 = "Walk";
    _124 = 1;
    MR::startBrk(this, "ColorChange");
    MR::setBrkFrameAndStop(this, 2.0f);

    NPCActorItem item;
    item.mActor = "Kinopio";
    item.mGoods0 = lbl_807CFE9C;
    item.mGoodsJoint0 = lbl_807CFE98;
    item.mGoods1 = lbl_807CFE94;
    item.mGoodsJoint1 = lbl_807CFE90;
    MR::getNPCItemData(&item, 3);
    equipment(item, false);
    MR::onCalcGravity(this);
    MR::initUseStageSwitchReadA(this, rIter);
    this->initChildObj(rIter);

    MR::FunctorV0M<KinopioPostman*, void (KinopioPostman::*)()> onSwitch(this, &KinopioPostman::onSwitchA);
    MR::listenStageSwitchOnA(this, onSwitch);

    const char* pNames[9] = {
        "\203t\203@\203C\203\213\203Z\203\214\203N\203g\202\251\202\347\202\314\212J\216n",
        "\227X\225\326\211\256\202\263\202\361\203C\203x\203\223\203g[\203^\203X\203N]",
        "\227X\225\326\211\256\202\263\202\361\203C\203x\203\223\203g[\203\215\203[\203b\203^]",
        "\203O\203\211\203\223\203h\203X\203^\201[\213A\212\322[\202Q\211\361\226\332\210\310\215~]",
        "\203p\203\217\201[\203X\203^\201[\213A\212\322",
        "\203p\203\217\201[\203X\203^\201[\213A\212\322[NPC\217\320\211\356]",
        "\203p\203\217\201[\203X\203^\201[\213A\212\322[\203A\203C\203e\203\200\230f\220\257]",
        "\203p\203\217\201[\203X\203^\201[\213A\212\322[\217\211\211\361]",
        "\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217I\227\271"
    };
    for (u32 i = 0; i < 9; ++i) {
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

// NONMATCHING: callback constant-table relocations differ despite equal offsets and member identities.
bool KinopioPostman::initTalkCtrl() {


    if (!PostmanEvent::isAvailable()) {
        return false;
    }
    for (TalkMessageCtrl** ctrl = mTalkCtrls; ctrl != mTalkCtrls + 7; ++ctrl) {
        *ctrl = PostmanEvent::createTalkCtrl(this, ctrl - mTalkCtrls);
    }

    {
        TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)> callback(this, &KinopioPostman::eventGive);
        MR::registerEventFunc(this->mTalkCtrls[3], callback);
    }
    {
        TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)> callback(this, &KinopioPostman::branchImportant);
        MR::registerBranchFunc(this->mTalkCtrls[3], callback);
    }
    {
        TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)> callback(this, &KinopioPostman::eventRead);
        MR::registerEventFunc(this->mTalkCtrls[2], callback);
    }
    {
        TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)> callback(this, &KinopioPostman::eventGive);
        MR::registerEventFunc(this->mTalkCtrls[4], callback);
    }
    {
        TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)> callback(this, &KinopioPostman::branchLuigi);
        MR::registerBranchFunc(this->mTalkCtrls[4], callback);
    }
    {
        TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)> callback(this, &KinopioPostman::eventGive);
        MR::registerEventFunc(this->mTalkCtrls[5], callback);
    }
    {
        TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)> callback(this, &KinopioPostman::eventReply);
        MR::registerEventFunc(this->mTalkCtrls[6], callback);
    }
    {
        TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)> callback(this, &KinopioPostman::eventGive);
        MR::registerEventFunc(this->mTalkCtrls[1], callback);
    }
    {
        TalkMessageFuncM<KinopioPostman*, bool (KinopioPostman::*)(u32)> callback(this, &KinopioPostman::eventRead);
        MR::registerEventFunc(this->mTalkCtrls[0], callback);
    }

    this->mMsgCtrl = this->mTalkCtrls[0];
    this->mLayout = PostmanEvent::createLetter();
    this->mStepNum = PostmanEvent::getTalkType(this->mLayout);
    this->mIsEventFlag = (this->mStepNum - 2) != 0;
    this->mMsgCtrl = this->mTalkCtrls[this->mStepNum];
    return true;
}

bool KinopioPostman::tryRegisterDemo(const char* pName, const JMapInfoIter& rIter) {


    bool hasLetter = this->mLayout != nullptr;
    if (MR::tryRegisterDemoCast(this, pName, rIter)) {
        if (hasLetter) {
            DemoFunction::registerDemoTalkMessageCtrlDirect(this, this->mMsgCtrl, pName);
        }
        MR::FunctorV0M<KinopioPostman*, void (KinopioPostman::*)()> action(this, &KinopioPostman::startDemo);
        MR::registerDemoActionFunctorDirect(this, action, pName, nullptr);
        return true;
    }
    return false;
}

void KinopioPostman::initChildObj(const JMapInfoIter& rIter) {


    for (s32 i = 0; i < MR::getChildObjNum(rIter); ++i) {
        const char* pChildName = nullptr;
        MR::getChildObjName(&pChildName, rIter, i);
        if (MR::isEqualString(pChildName, "Kinopio")) {
            this->mChildObj = static_cast<LiveActor*>(NameObjFactory::initChildObj(rIter, i));
            break;
        }
    }
    this->mChildObj->makeActorDead();
}

void KinopioPostman::onSwitchA() {


    if (!PostmanEvent::isAvailable()) {
        this->mChildObj->makeActorAppeared();
    }
}

bool KinopioPostman::eventRead(u32 step) {


    if (step == 0) {
        if (this->tryEndLetter()) {
            return true;
        }
        this->tryStartLetter();
        return false;
    }
    return true;
}

bool KinopioPostman::eventGive(u32 step) {


    if (step == 0) {
        if (this->tryEndLetter()) {
            return true;
        }
        this->tryStartLetter();
        return false;
    }
    if (step == 2) {
        PostmanEvent::receiveLetter(this, this->mLayout->getCurrentLetterId());
        this->mStepNum = 2;
        this->mMsgCtrl = this->mTalkCtrls[this->mStepNum];
        this->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvFinish));
        return true;
    }
    return true;
}

// NONMATCHING: equivalent false-return branch layout differs.
bool KinopioPostman::branchImportant(u32 step) {


    if (step == 0) {
        if (MR::isValidPostmanImportantEvent(this->mLayout->getCurrentLetterId())) {
            s32 count = MR::getPostmanImportantEventReward(this->mLayout->getCurrentLetterId());
            if (count > 0) {
                MR::setMessageArg(this->mTalkCtrls[3], count);
                return true;
            }
        }
        return false;
    }
    if (step == 1) {
        if (!MR::isPlayerLuigi()) {
            return false;
        } else {
            return (this->mLayout->getCurrentLetterId() - 1) == 0;
        }
    }
    return false;
}

#pragma global_optimizer off
bool KinopioPostman::branchLuigi(u32 step) {


    if (step == 0) {
        if (!MR::isPlayerLuigi()) {
            return false;
        } else {
            return (this->mLayout->getCurrentLetterId() - 2) == 0;
        }
    }
    return false;
}


#pragma global_optimizer reset
bool KinopioPostman::eventReply(u32 step) {


    if (step == 0) {
        if (this->tryEndLetter()) {
            return true;
        }
        this->tryStartLetter();
        return false;
    }
    if (step == 2) {
        PostmanEvent::receiveLetter(this, this->mLayout->getCurrentLetterId());
        MR::startActionSound(this, "AddMailStarPiece", -1, -1, -1);
        this->mStepNum = 2;
        this->mMsgCtrl = this->mTalkCtrls[this->mStepNum];
        this->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvFinish));
        return true;
    }
    return true;
}


bool KinopioPostman::trySelectDefaultTalk() {


    if (!this->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWait))) {
        return false;
    }
    if (!this->mIsEventFlag) {
        return false;
    }
    this->mMsgCtrl = this->mTalkCtrls[0];
    return true;
}

bool KinopioPostman::trySelectEventTalk() {


    if (!this->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWait))) {
        return false;
    }
    if (!this->mIsEventFlag) {
        return false;
    }
    this->mMsgCtrl = this->mTalkCtrls[this->mStepNum];
    return true;
}

bool KinopioPostman::tryEndLetter() {


    if (!MR::isDead(this->mLayout) && this->mLayout->isEnd()) {
        if (this->mWaitTimer < 0) {
            this->mWaitTimer = 0x23;
        }
        this->mWaitTimer -= 1;
        if (this->mWaitTimer < 0) {
            this->mIsEventFlag = false;
            this->mLayout->kill();
            return true;
        }
    }
    return false;
}

bool KinopioPostman::tryStartLetter() {


    if (MR::isDead(this->mLayout)) {
        if (this->mWaitTimer < 0) {
            this->mWaitTimer = gKinopioPostmanLetterStartDelay;
        }
        this->mWaitTimer -= 1;
        if (this->mWaitTimer < 0) {
            this->mLayout->appear();
            return true;
        }
    }
    return false;
}

// KinopioPostmanFunction::turnQuaternion: turns the quaternion's facing toward rTo around its up axis, then blends up/front.
// NONMATCHING: local floating-point pool relocations differ.
void KinopioPostmanFunction::turnQuaternion(TQuat4f* pQuat, const TVec3f& rUp, const TVec3f& rTo, f32 rate) {
    TVec3f sp2C(rTo);
    TVec3f sp20(rUp);
    TVec3f sp14;
    pQuat->getYDir(sp14);
    TVec3f sp8;
    pQuat->getZDir(sp8);
    MR::turnVecToVecCosOnPlane(&sp8, sp2C, sp14, rate);
    MR::blendQuatUpFront(pQuat, sp20, sp8, 1.0f, 1.0f);
}





void KinopioPostman::exeTalk() {


    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    TVec3f sp14 = *MR::getPlayerPos() - this->mPosition;
    TVec3f sp8 = -this->mGravity;
    KinopioPostmanFunction::turnQuaternion(&this->_A0, sp8, sp14, 0.997f);
    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(KinopioPostman, KinopioPostmanNrvReaction))) {
        MR::tryTalkNearPlayerAndStartMoveTalkAction(this);
    }
}

void KinopioPostman::exeReaction() {



    if (this->_D8 != 0) {
        MR::startActionSound(this, "SmNpcTrampled", -1, -1, -1);
        MR::startActionSound(this, "SvKinopioTrampled", -1, -1, -1);
    }
    if (this->isPointingSe()) {
        MR::startDPDHitSound();
        MR::startActionSound(this, "SvKinopioPointing", -1, -1, -1);
    }
    if (this->_DB != 0) {
        MR::startActionSound(this, "SvKinopioStarPieceHit", -1, -1, -1);
    }
    MR::tryStartReactionAndPopNerve(this);
}

void KinopioPostman::exeWait() {


    if (MR::isFirstStep(this)) {
        if (!MR::isExistRail(this)) {
            MR::onCalcShadowOneTime(this, nullptr);
        }
        this->trySelectDefaultTalk();
    }
    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(KinopioPostman, KinopioPostmanNrvReaction))) {
        if (MR::isMarioFaceShipNpcTalkAllowed()) {
            TalkMessageCtrl* pCtrl = this->mMsgCtrl;
            if (MR::tryTalkNearPlayerAtEnd(pCtrl)) {
                MR::startAction(this, "Notice");
            }
        }
        if (MR::isActionContinuous(this)) {
            if (this->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear))) {
                if (MR::isAnyAnimPlaying(this, "Notice")) {
                    TVec3f sp14 = *MR::getPlayerPos() - this->mPosition;
                    TVec3f sp8 = -this->mGravity;
                    KinopioPostmanFunction::turnQuaternion(&this->_A0, sp8, sp14, 0.997f);
                }
            }
        } else {
            if (this->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWait))) {
                if (MR::isNearPlayer(this, 200.0f)) {
                    MR::startAction(this, "Notice");
                    this->trySelectEventTalk();
                    this->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear));
                    return;
                }
                MR::tryStartMoveTalkAction(this);
                return;
            }
            if (this->isNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear))) {
                bool far = !MR::isNearPlayer(this, 300.0f);
                if (far) {
                    this->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWait));
                }
                MR::tryStartTalkAction(this);
            }
        }
    }
}

void KinopioPostman::exeFinish() {


    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Notice");
    }
    if (MR::isActionEnd(this)) {
        if (this->mIsReplyFlag) {
            this->mPostmanEvent->makeActorDead();
        }
        this->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear));
    }
}

// NONMATCHING: local vector stack offsets differ.
void KinopioPostman::exeDemo() {


    if (MR::isFirstStep(this)) {
        if (!MR::isExistRail(this)) {
            MR::onCalcShadowOneTime(this, nullptr);
        }
        this->trySelectDefaultTalk();
        TVec3f sp8;
        MR::findNamePos(sMarioPosName, &sp8, nullptr);
        TVec3f sp20 = sp8 - this->mPosition;
        TVec3f sp14 = -this->mGravity;
        KinopioPostmanFunction::turnQuaternion(&this->_A0, sp14, sp20, -1.0f);
        MR::startAction(this, "Talk");
    }
    if (!MR::isTimeKeepDemoActive()) {
        MR::startAction(this, "Notice");
        if (this->mIsReplyFlag) {
            MR::startPlayerNormalBva();
        }
        this->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvWaitNear));
    }
}

void KinopioPostman::startDemo() {


    MR::moveCoordAndTransToRailStartPoint(this);
    MR::followRailPoseOnGround(this, 1.0f);
    if (this->mIsReplyFlag) {
        this->mPostmanEvent->makeActorAppeared();
    }
    this->setNerve(GET_NERVE(KinopioPostman, KinopioPostmanNrvDemo));
}

KinopioPostman::~KinopioPostman() {}

extern const volatile s32 gKinopioPostmanLetterStartDelay = 0;
