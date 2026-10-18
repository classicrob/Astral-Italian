#pragma auto_inline off
#include "Game/NPC/SuperDreamer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Util/NPCUtil.hpp"


namespace NrvSuperDreamer {
    NEW_NERVE(SuperDreamerNrvAppear, SuperDreamer, Appear);
    NEW_NERVE(SuperDreamerNrvWait, SuperDreamer, Wait);
    NEW_NERVE(SuperDreamerNrvStartTransform, SuperDreamer, StartTransform);
    NEW_NERVE(SuperDreamerNrvDemoTransform, SuperDreamer, DemoTransform);
    NEW_NERVE(SuperDreamerNrvInHelp, SuperDreamer, InHelp);
    NEW_NERVE(SuperDreamerNrvEndHelp, SuperDreamer, EndHelp);
    NEW_NERVE(SuperDreamerNrvTalk, SuperDreamer, Talk);
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

// SMG2 NPCActor fields reached by offset (NPCActor.hpp is off in this range).
#define SUPERDREAMER_FIELD(T, ofs) (*reinterpret_cast< T* >(reinterpret_cast< u8* >(this) + (ofs)))

class DreamerButtonGuidance : public LayoutActor {
public:
    DreamerButtonGuidance(const char*);
    void showSuperDreamer();
    void showExit();
    void hideExit();
    void disappear();
};

namespace GameSceneFunction {
    void requestStartSuperDreamerTalk();
    void requestEndSuperDreamerTalk();
}

namespace GameSequenceFunction {
    void setPadModeSuperDreamer(const NameObj*, s32);
    bool isPadMode();
    void clearPadModeSuperDreamer();
}

namespace MR {
    bool isPadRecorderStopped();
    bool isRequestedStopReplayPad();
    void stopReplayPad();
    void onDestroySceneKeepAllSound();
    void hideBubbleBeak(TalkMessageCtrl*);
}

extern "C" const char* lbl_807CFF18;
extern "C" const char* lbl_807CFF1C;
extern "C" const char* lbl_807CFF20;
extern "C" const char* lbl_807CFF24;

extern "C" s32 fn_800589B0(); // PLACEHOLDER(astra): inherited fn_800589B0 ABI; resolve when its target signature and canonical owner are verified.

void SuperDreamer::exeWait() {
    if (MR::isFirstStep(this)) {
        SUPERDREAMER_FIELD(TalkMessageCtrl*, 0x94) = mTalkCtrl;
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
    if (!MR::tryStartReactionAndPushNerve(this, SUPERDREAMER_FIELD(const Nerve*, 0x15C))) {
        if (SUPERDREAMER_FIELD(TalkMessageCtrl*, 0x94) != nullptr) {
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

void SuperDreamer::exeEndHelp() {
    if (MR::isFirstStep(this)) {
        MR::endToCaptureScreen("Camera");
    }
    if (MR::tryTalkForceWithoutDemoAtEnd(SUPERDREAMER_FIELD(TalkMessageCtrl*, 0x94)) || MR::isPlayerDead()) {
        setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvEndTalk));
    }
}

void SuperDreamer::exeInHelp() {
    MR::calcPlayerJointMtx(&mPlayerJointMtx, "Hip");
    if (MR::isFirstStep(this)) {
        MR::forceOpenWipeCircle();
        SUPERDREAMER_FIELD(TalkMessageCtrl*, 0x94) = mTalkCtrl;
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
        if (SUPERDREAMER_FIELD(TalkMessageCtrl*, 0x94) != nullptr) {
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

void SuperDreamer::initHelperWitchBall() {
    mHelperWitchBall = new ModelObj("\x82\xA8\x8F\x95\x82\xAF\x83\x45\x83\x42\x83\x62\x83\x60\x83\x7B\x81\x5B\x83\x8B",
                                    "HelperWitchBall", nullptr, -2, -2, -2, false);
    mHelperWitchBall->initWithoutIter();
    mHelperWitchBall->makeActorDead();
}

void SuperDreamer::initHelperWitchEffect() {
    MR::makeMtxTR(reinterpret_cast< MtxPtr >(&mHelperEffectMtx), this);
    mHelperWitchEffect = new ModelObj("\x83\x82\x83\x66\x83\x8B\x83\x47\x83\x74\x83\x46\x83\x4E\x83\x67",
                                      "HelperWitchEffect", reinterpret_cast< MtxPtr >(&mHelperEffectMtx), -2, -2, -2, false);
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

void SuperDreamer::attemptResetPadMode() const {
    if (MR::isDemoActive() || MR::isSystemTalking() || MR::isPlayerDead() ||
        (!MR::isPlayerInRush() && !MR::isOnGroundPlayer()) || fn_800589B0()) {
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

namespace DreamerFunction {
    bool isPadModeSuperDreamerAndStartId(s32);
    void initChildObj(const JMapInfoIter&);
}

namespace GameSequenceFunction {
    bool isAllowDreamerMissNum(s32);
    bool isPadMode();
}

extern "C" bool fn_800416F0(const char*, const JMapInfoIter&, bool); // PLACEHOLDER(astra): inherited fn_800416F0 ABI; resolve when its target signature and canonical owner are verified.

// NONMATCHING: string-literal stores hoisted/ordered differently (lis batch + different GPR for 0x13C/0x140); same calls and values.
void SuperDreamer::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("HelperWitch");
    caps.setDefault();
    caps.mMessage = false;
    SUPERDREAMER_FIELD(const char*, 0x134) = "Spin";
    SUPERDREAMER_FIELD(const char*, 0x138) = "Trampled";
    SUPERDREAMER_FIELD(const char*, 0x13C) = "Pointing";
    SUPERDREAMER_FIELD(const char*, 0x140) = "Reaction";
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

void SuperDreamer::initForPlaying(const JMapInfoIter& rIter) {
    setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvInHelp));
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    mTalkCtrl = createTalkCtrl(rIter, lbl_807CFF20, lbl_807CFF24);
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

// NONMATCHING: target copies the 12-byte member-pointer constant from lbl_806BD378+0xC0 into a stack temp and calls fn_803616C0 (the TalkMessageFuncM ctor); ours emits the ctor call directly.
void SuperDreamer::initForNpc(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg1NoInit(rIter, &mMissNum);
    setNerve(GET_NERVE(SuperDreamer, SuperDreamerNrvWait));
    mTalkCtrl = createTalkCtrl(rIter, lbl_807CFF18, lbl_807CFF1C);
    MR::registerEventFunc(mTalkCtrl, TalkMessageFuncM< SuperDreamer*, bool (SuperDreamer::*)(u32) >(this, &SuperDreamer::eventFunc));
    MR::registerKillFunc(mTalkCtrl, TalkMessageFuncM< SuperDreamer*, bool (SuperDreamer::*)(u32) >(this, &SuperDreamer::killFunc));
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
    TVec3f offs;
    if (pName1 != nullptr && MR::isExistMessageId(rIter) && fn_800416F0(pName1, rIter, true)) {
        offs.x = 0.0f;
        offs.y = 220.0f;
        offs.z = 0.0f;
        pCtrl = MR::createTalkCtrl(this, rIter, pName1, offs, nullptr);
    } else if (pName2 != nullptr) {
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

extern "C" f64 cos(f64);
extern "C" f64 sin(f64);

extern "C" TVec3f lbl_80729B58;
extern "C" TVec3f lbl_80729B64;

// NONMATCHING: target calls TPosition3::setTrans out of line; the shared TMatrix.hpp setTrans is inlined here. Float op scheduling also differs.
void SuperDreamer::updatePlayerBaseMtx() const {
    f32 cz = (f32)cos(lbl_80729B64.z);
    f32 cy = (f32)cos(lbl_80729B64.y);
    f32 cx = (f32)cos(lbl_80729B64.x);
    f32 sz = (f32)sin(lbl_80729B64.z);
    f32 sy = (f32)sin(lbl_80729B64.y);
    f32 sx = (f32)sin(lbl_80729B64.x);

    TPos3f mtx;
    mtx.mMtx[0][0] = cy * cz;
    mtx.mMtx[0][1] = (sx * sy) * cz - cx * sz;
    mtx.mMtx[0][2] = (cx * cz) * sy + sx * sz;
    mtx.mMtx[1][0] = cy * sz;
    mtx.mMtx[1][1] = cx * cz + (sx * sy) * sz;
    mtx.mMtx[1][2] = (cx * sz) * sy - sx * cz;
    mtx.mMtx[2][0] = -sy;
    mtx.mMtx[2][1] = sx * cy;
    mtx.mMtx[2][2] = cx * cy;
    mtx.setTrans(lbl_80729B58);

    TMtx34f base;
    MR::makeMtxTR(reinterpret_cast< MtxPtr >(&base), this);
    base.concat(mtx);
    MR::setPlayerBaseMtx(reinterpret_cast< MtxPtr >(&base));
}
