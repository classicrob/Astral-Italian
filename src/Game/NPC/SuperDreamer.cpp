#define JGEOMETRY_QUAT_DIRECTION_NOINLINE
#pragma auto_inline off
#include "Game/NPC/SuperDreamer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Screen/DreamerButtonGuidance.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/NPC/DreamerFunction.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/Util/PadRecorder.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Util/NPCUtil.hpp"


    TVec3f gSuperDreamerPlayerOffset(0.0f, 0.0f, 0.0f);
    TVec3f gSuperDreamerPlayerRotation(0.0f, 0.0f, 0.0f);
    TVec3f gSuperDreamerEffectOffset(0.0f, 0.0f, 0.0f);

const char* gSuperDreamerMessageName = "SuperDreamer";
const char* gSuperDreamerFallbackMessage = "SuperDreamerCommon000";
const char* gSuperDreamerEndMessageName = "SuperDreamerEnd";
const char* gSuperDreamerEndFallbackMessage = "SuperDreamerEndCommon000";
namespace NrvSuperDreamer {
    NEW_NERVE(SuperDreamerNrvAppear, SuperDreamer, Appear);
    NEW_NERVE(SuperDreamerNrvWait, SuperDreamer, Wait);
    class SuperDreamerNrvTalk : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static SuperDreamerNrvTalk sInstance;
    };
    SuperDreamerNrvTalk SuperDreamerNrvTalk::sInstance;
    class SuperDreamerNrvStartTransform : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static SuperDreamerNrvStartTransform sInstance;
    };
    SuperDreamerNrvStartTransform SuperDreamerNrvStartTransform::sInstance;
    NEW_NERVE(SuperDreamerNrvDemoTransform, SuperDreamer, DemoTransform);
    NEW_NERVE(SuperDreamerNrvInHelp, SuperDreamer, InHelp);
    NEW_NERVE(SuperDreamerNrvEndHelp, SuperDreamer, EndHelp);
    NEW_NERVE(SuperDreamerNrvEndTalk, SuperDreamer, EndTalk);
    NEW_NERVE(SuperDreamerNrvEnd, SuperDreamer, End);
    NEW_NERVE(SuperDreamerNrvKill, SuperDreamer, Kill);
};  // namespace NrvSuperDreamer

SuperDreamer::SuperDreamer(const char* pName) : NPCActor(pName) {
    mButtonGuidance = nullptr;
    mStartId = -1;
    mMissNum = -1;
    mTalkCtrl = nullptr;
    mIsPadMode = false;
    mHelperWitchBall = nullptr;
    mHelperWitchEffect = nullptr;
    mPlayerJointMtx.identity();
    mHelperEffectMtx.identity();
}






void SuperDreamer::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("HelperWitch");
    caps.setDefault();
    caps.mMessage = false;
    _130 = cNPCSpinDefault;
    _134 = cNPCTrampledDefault;
    _138 = cNPCPointingDefault;
    _13C = cNPCReactionDefault;
    initialize(rIter, caps, "HelperWitch", nullptr, nullptr);
    MR::getJMapInfoArg0NoInit(rIter, &mStartId);

    if (MR::isPlayerLuigi() || (GameSequenceFunction::isPadMode() && !DreamerFunction::isPadModeSuperDreamerAndStartId(mStartId))) {
        mIsPadMode = true;
        makeActorDead();
        return;
    }
    if (DreamerFunction::isPadModeSuperDreamerAndStartId(mStartId)) {
        initForPlaying(rIter);
        return;
    }
    initForNpc(rIter);
}

void SuperDreamer::appear() {
    if (!mIsPadMode && MR::isDead(this)) {
        LiveActor::appear();
        setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvAppear));
    }
}

void SuperDreamer::makeActorAppeared() {
    if (mIsPadMode) {
        return;
    }
    NPCActor::makeActorAppeared();
}

void SuperDreamer::kill() {
    MR::validateClipping(this);
    NPCActor::kill();
}

void SuperDreamer::control() {
    NPCActor::control();
}

bool SuperDreamer::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

void SuperDreamer::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvWait));
        MR::startAction(this, "WaitA");
    }
}

bool SuperDreamer::eventFunc(u32 event) {
    if (event == 2) {
        setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvWait));
        return true;
    }
    return false;
}

bool SuperDreamer::killFunc(u32) {
    MR::forceCurePlayerElementMode();
    setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvStartTransform));
    return true;
}







void SuperDreamer::exeWait() {
    if (MR::isFirstStep(this)) {
        mMsgCtrl = mTalkCtrl;
        if (MR::getRandom() < 0.5f) {
            MR::startAction(this, "WaitA");
        } else {
            MR::startAction(this, "WaitB");
        }
    }
    if (MR::isCurrentRushTamakoro()) {
        kill();
        return;
    }
    if (!MR::tryStartReactionAndPushNerve(this, mReactionNerve)) {
        if (mMsgCtrl != nullptr) {
            if (MR::tryTalkNearPlayerAndStartTalkAction(this)) {
                setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvTalk));
            }
        } else {
            MR::tryStartTurnAction(this);
        }
    }
}

void SuperDreamer::exeKill() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        mHelperWitchEffect->kill();
    }
    if (MR::isDead(mButtonGuidance)) {
        makeActorDead();
    }
}

void SuperDreamer::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeCircle(-1);
    }
    if (MR::isWipeBlank()) {
        MR::hideModel(this);
        setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvKill));
    }
}

void SuperDreamer::exeEndTalk() {
    if (MR::isFirstStep(this)) {
        GameSceneFunction::requestEndSuperDreamerTalk();
        GameSequenceFunction::clearPadModeSuperDreamer();
        setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvKill));
    }
}





void NrvSuperDreamer::SuperDreamerNrvStartTransform::execute(Spine* spine) const {
    SuperDreamer* actor = static_cast<SuperDreamer*>(spine->mExecutor);
    if (MR::tryStartDemoMarioPuppetable(actor, "お助けウィッチ開始"))
        actor->setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvDemoTransform));
}



void SuperDreamer::exeDemoTransform() {
    if (MR::isFirstStep(this)) {
        MR::openWipeWhiteFade(-1);
        setToDefault();
        updatePlayerBaseMtx();
        MR::startAction(this, "DemoTransform");
        MR::startBckPlayer("DemoTransform");
        MR::setPosition(mHelperWitchBall, mPosition);
        MR::setRotation(mHelperWitchBall, mRotation);
        mHelperWitchBall->makeActorAppeared();
        MR::requestMovementOn(mHelperWitchBall);
        MR::startAction(mHelperWitchBall, "DemoTransform");
        ActorCameraInfo info(-1, 0);
        MR::startAnimCameraTargetSelf(this, &info, "DemoTransform", 0, false, 1.0f);
        MR::stopStageBGM(10);
    }
    if (MR::isActionEnd(this)) {
        setupPadMode();
        return;
    }
    if (MR::isGreaterStep(this, 60) && MR::testCorePadTriggerA(0)) {
        setupPadMode();
    }
}

void SuperDreamer::exeInHelp() {
    MR::calcPlayerJointMtx(&mPlayerJointMtx, "Hip");
    if (MR::isFirstStep(this)) {
        MR::forceOpenWipeCircle();
        mMsgCtrl = mTalkCtrl;
        if (mTalkCtrl != nullptr) {
            MR::offTalkCinemaFrame(mTalkCtrl);
        }
        mButtonGuidance->appear();
        mHelperWitchEffect->makeActorAppeared();
        MR::startAction(mHelperWitchEffect, "HelperWitchEffect");
    }
    MR::startActionSound(this, "InHelp", -1, -1, -1);
    attemptResetPadMode();
    updateHelperWitchEffectMtx();
    if (MR::isPadRecorderStopped()) {
        if (mMsgCtrl != nullptr) {
            mButtonGuidance->disappear();
            GameSceneFunction::requestStartSuperDreamerTalk();
            setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvEndHelp));
            return;
        }
        mButtonGuidance->disappear();
        GameSequenceFunction::clearPadModeSuperDreamer();
        setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvKill));
    }
}

void SuperDreamer::exeEndHelp() {
    if (MR::isFirstStep(this)) {
        MR::endToCaptureScreen("Camera");
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(mMsgCtrl) || MR::isPlayerDead()) {
        setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvEndTalk));
    }
}

void SuperDreamer::initForNpc(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg1NoInit(rIter, &mMissNum);
    setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvWait));
    mTalkCtrl = createTalkCtrl(rIter, gSuperDreamerMessageName, gSuperDreamerFallbackMessage);
    TalkMessageFuncM< SuperDreamer*, bool (SuperDreamer::*)(u32) > event = TalkMessageFunc_Inline(this, &SuperDreamer::eventFunc);
    MR::registerEventFunc(mTalkCtrl, event);
    TalkMessageFuncM< SuperDreamer*, bool (SuperDreamer::*)(u32) > kill = TalkMessageFunc_Inline(this, &SuperDreamer::killFunc);
    MR::registerKillFunc(mTalkCtrl, kill);
    if (mTalkCtrl != nullptr) {
        MR::onRootNodeAutomatic(mTalkCtrl);
        MR::onStartOnlyFront(mTalkCtrl);
    }
    MR::declareEventCameraProgrammable("\x83\x58\x81\x5B\x83\x70\x81\x5B\x83\x68\x83\x8A\x81\x5B\x83\x7D\x81\x5B\x83\x4A\x83\x81\x83\x89");
    ActorCameraInfo info(-1, 0);
    MR::initAnimCamera(this, &info, "DemoTransform");
    if (isMakeAppear()) {
        makeActorAppeared();
    } else {
        makeActorDead();
    }
    mScale.x = 1.35f;
    mScale.y = 1.35f;
    mScale.z = 1.35f;
    initHelperWitchBall();
    initHelperWitchEffect();
    initButtonGuidance();
    DreamerFunction::initChildObj(rIter);
}

void SuperDreamer::initHelperWitchBall() {
    mHelperWitchBall = new ModelObj("\x82\xA8\x8F\x95\x82\xAF\x83\x45\x83\x42\x83\x62\x83\x60\x83\x7B\x81\x5B\x83\x8B",
                                    "HelperWitchBall", nullptr, -2, -2, -2, false);
    mHelperWitchBall->initWithoutIter();
    mHelperWitchBall->makeActorDead();
}

void SuperDreamer::initHelperWitchEffect() {
    MR::makeMtxTR(mHelperEffectMtx.toMtxPtr(), this);
    mHelperWitchEffect = new ModelObj("\x83\x82\x83\x66\x83\x8B\x83\x47\x83\x74\x83\x46\x83\x4E\x83\x67",
                                      "HelperWitchEffect", mHelperEffectMtx.toMtxPtr(), -2, -2, -2, false);
    mHelperWitchEffect->initWithoutIter();
    MR::invalidateClipping(mHelperWitchEffect);
    MR::registerDemoSimpleCastAll(mHelperWitchEffect);
    mHelperWitchEffect->makeActorDead();
}

void SuperDreamer::initButtonGuidance() {
    mButtonGuidance = new DreamerButtonGuidance("\x83\x68\x83\x8A\x81\x5B\x83\x7D\x81\x5B\x97\x70\x83\x8C\x83\x43\x83\x41\x83\x45\x83\x67");
    mButtonGuidance->initWithoutIter();
    mButtonGuidance->showSuperDreamer();
}



void SuperDreamer::attemptResetPadMode() const {
    if (MR::isDemoActive() || MR::isSystemTalking() || MR::isPlayerDead() ||
        (!MR::isPlayerInRush() && !MR::isOnGroundPlayer()) || MR::isExistCaptureScreenTexture()) {
        mButtonGuidance->hideExit();
        return;
    }
    mButtonGuidance->showExit();
    if (MR::isGreaterEqualStep(this, 120)) {
        if (MR::isRequestedStopReplayPad()) {
            MR::stopReplayPad();
        }
    }
}






void SuperDreamer::initForPlaying(const JMapInfoIter& rIter) {
    setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvInHelp));
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    mTalkCtrl = createTalkCtrl(rIter, gSuperDreamerEndMessageName, gSuperDreamerEndFallbackMessage);
    if (mTalkCtrl != nullptr) {
        MR::offTalkCinemaFrame(mTalkCtrl);
        MR::hideBubbleBeak(mTalkCtrl);
    }
    makeActorAppeared();
    mIsPadMode = true;
    MR::hideModel(this);
    MR::invalidateHitSensors(this);
    MR::invalidateShadowAll(this);
    initHelperWitchEffect();
    initButtonGuidance();
}

// NONMATCHING: callback and local literal relocations differ.


bool SuperDreamer::isMakeAppear() const {
    if (DreamerFunction::isPadModeSuperDreamerAndStartId(mStartId)) {
        return true;
    }
    if (GameSequenceFunction::isPadMode2()) {
        return false;
    }
    bool result;
    if (mMissNum < 0) {
        result = false;
        if (MR::isValidSwitchAppear(this)) {
            if (MR::isOnSwitchAppear(this)) {
                result = true;
            }
        }
    } else {
        result = false;
        if ((MR::isValidSwitchAppear(this) && MR::isOnSwitchAppear(this)) || GameSequenceFunction::isAllowDreamerMissNum(mMissNum)) {
            result = true;
        }
    }
    return result;
}

TalkMessageCtrl* SuperDreamer::createTalkCtrl(const JMapInfoIter& rIter, const char* pName1, const char* pName2) {
    TalkMessageCtrl* pCtrl = nullptr;
    if (pName1 != nullptr && MR::isExistMessageId(rIter) && MR::isExistSceneMessage(pName1, rIter, true)) {
        TVec3f offs;
        offs.x = 0.0f;
        offs.y = 220.0f;
        offs.z = 0.0f;
        pCtrl = MR::createTalkCtrl(this, rIter, pName1, offs, nullptr);
    } else if (pName2 != nullptr) {
        TVec3f offs;
        offs.x = 0.0f;
        offs.y = 220.0f;
        offs.z = 0.0f;
        pCtrl = MR::createTalkCtrlDirect(this, rIter, pName2, offs, nullptr);
    }
    if (pCtrl != nullptr) {
        MR::setDistanceToTalk(pCtrl, 280.0f);
    }
    return pCtrl;
}






// NONMATCHING: rotation matches; angle lifetime changes saved FPR allocation.
void SuperDreamer::updatePlayerBaseMtx() const {
    TPos3f mtx;
    mtx.setRotate(gSuperDreamerPlayerRotation.x, gSuperDreamerPlayerRotation.y, gSuperDreamerPlayerRotation.z);
    mtx.setTrans(gSuperDreamerPlayerOffset);

    TMtx34f base;
    MR::makeMtxTR(base.toMtxPtr(), this);
    base.concat(mtx);
    MR::setPlayerBaseMtx(base.toMtxPtr());
}



void SuperDreamer::updateHelperWitchEffectMtx() const {
    TVec3f up(-*MR::getPlayerGravity());
    MR::normalizeOrZero(&up);
    TVec3f oldUp;
    mHelperEffectMtx.getYDir(oldUp);
    MR::normalizeOrZero(&oldUp);
    TQuat4f oldRotation;
    mHelperEffectMtx.getQuat(oldRotation);
    TQuat4f rotation;
    rotation.setRotate(oldUp, up);
    PSQUATMultiply(&rotation, &oldRotation, &rotation);
    TVec3f x, y, z;
    rotation.getXDir(x);
    MR::normalizeOrZero(&x);
    rotation.getYDir(y);
    MR::normalizeOrZero(&y);
    PSVECCrossProduct(&x, &y, &z);
    MR::normalizeOrZero(&z);
    PSVECCrossProduct(&y, &z, &x);
    mHelperEffectMtx.setXDir(x);
    mHelperEffectMtx.setYDir(y);
    mHelperEffectMtx.setZDir(z);
    TPos3f joint;
    MR::calcPlayerJointMtx(&joint, "Head");
    TVec3f position;
    joint.getTrans(position);
    TPos3f base;
    base.set(MR::getPlayerBaseMtx());
    TVec3f baseX, baseY, baseZ;
    base.getXDir(baseX);
    base.getYDir(baseY);
    base.getZDir(baseZ);
    position.add(baseX * gSuperDreamerEffectOffset.x);
    position.add(baseY * gSuperDreamerEffectOffset.y);
    position.add(baseZ * gSuperDreamerEffectOffset.z);
    mHelperEffectMtx.setTrans(position);
    MR::setPosition(mHelperWitchEffect, position);
    if (MR::isPlayerVisible()) {
        if (MR::isDead(mHelperWitchEffect))
            mHelperWitchEffect->appear();
    } else if (!MR::isDead(mHelperWitchEffect)) {
        MR::forceDeleteEffectAll(mHelperWitchEffect);
        mHelperWitchEffect->kill();
    }
}



void SuperDreamer::setupPadMode() {
    if (mStartId >= 0) {
        MR::onDestroySceneKeepAllSound();
        MR::startActionSound(this, "WipeIn", -1, -1, -1);
        MR::stopStageBGM(30);
        GameSequenceFunction::setPadModeSuperDreamer(this, mStartId);
        return;
    }
    setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvWait));
}

void NrvSuperDreamer::SuperDreamerNrvTalk::execute(Spine* spine) const {
    SuperDreamer* actor = static_cast<SuperDreamer*>(spine->mExecutor);
    if (!MR::tryStartReactionAndPushNerve(actor, actor->mReactionNerve))
        MR::tryTalkNearPlayerAtEndAndStartMoveTalkAction(actor);
}

SuperDreamer::~SuperDreamer() {}
