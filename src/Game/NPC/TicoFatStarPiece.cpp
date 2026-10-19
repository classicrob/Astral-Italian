#pragma auto_inline off
#include "Game/NPC/TicoFatStarPiece.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Screen/FullnessMeter.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/NPC/TicoFatStarPieceTransformation.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/System/GameSequenceFunction.hpp"
extern const volatile s32 sStarPieceTicoEatSpeedStartStep;
extern const volatile f32 sStarPieceTicoCameraEyeSide, sStarPieceTicoCameraTargetSide, sStarPieceTicoCameraTargetFront;

extern const char lbl_807CFE90[4], lbl_807CFE94[4], lbl_807CFE98[4], lbl_807CFE9C[4];
namespace { const char* sEventCamera = "デブチコカメラ"; }
namespace NrvTicoFatStarPiece {
    NEW_NERVE(TicoFatStarPieceNrvReaction, TicoFatStarPiece, Reaction);
    NEW_NERVE(TicoFatStarPieceNrvWait, TicoFatStarPiece, Wait);
    NEW_NERVE(TicoFatStarPieceNrvPoint, TicoFatStarPiece, Point);
    NEW_NERVE(TicoFatStarPieceNrvEatWait, TicoFatStarPiece, EatWait);
    NEW_NERVE(TicoFatStarPieceNrvEat, TicoFatStarPiece, Eat);
    NEW_NERVE(TicoFatStarPieceNrvChem, TicoFatStarPiece, Chem);
    class TicoFatStarPieceNrvTransform : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            TicoFatStarPiece* actor = static_cast<TicoFatStarPiece*>(spine->mExecutor);
            if (MR::updateActorState(actor)) actor->kill();
        }
        static TicoFatStarPieceNrvTransform sInstance;
    };
    TicoFatStarPieceNrvTransform TicoFatStarPieceNrvTransform::sInstance;
}

TicoFatStarPiece::TicoFatStarPiece(const char* name) : NPCActor(name), mFullnessMeter(0), mEatTalkCtrl(0), mStarPieceCount(0), mRemainingStarPieceCount(0), mPendingStarPieceCount(0), mNearPlayerTimer(0), mPointingTimer(0), mEventRequested(false), mTicoFatId(-1), mEatEndSoundTimer(0), mEatSoundTimer(0), mTransformation(0) {}


// NONMATCHING: register allocation, clamp instruction choice, and literal addressing differ.
void TicoFatStarPiece::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("TicoFatStarPiece");
    caps.setDefault();
    caps.mWaitNerve = GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvWait);
    caps.mNerveStep = 1;
    caps.mMessage = false;
    caps.mYoshiInteraction = false;
    initialize(iter, caps, "TicoFatStarPiece", nullptr, nullptr);
    if (!GameSequenceFunction::isTicoFatStarPieceActive()) {
        MR::tryRegisterDemoCast(this, "ハラペコスターピースチコ紹介", iter);
        makeActorDead();
        return;
    }
    MR::moveCoordToStartPos(this);
    MR::onCalcShadowAll(this);
    mTransformation = new TicoFatStarPieceTransformation(this, iter);
    MR::initActorState(this, mTransformation, GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvTransform), "Demo");
    if (GameSequenceFunction::isTicoFatStarPieceActive()) {
        mTicoFatId = GameSequenceFunction::getTicoFatStarPieceId();
        mStarPieceCount = GameSequenceFunction::getTicoFatStarPieceRequirement();
        s32 alreadyEaten = GameDataFunction::getTicoFatStarPieceAlms(mTicoFatId);
        mPendingStarPieceCount = 0;
        mRemainingStarPieceCount = (mStarPieceCount - alreadyEaten <= 0 ? 0 : mStarPieceCount - alreadyEaten);
        MR::declareStarPieceReceiver(this, mRemainingStarPieceCount);
    }
    TVec3f requestOffset;
    requestOffset.x = 0.0f; requestOffset.y = 220.0f; requestOffset.z = 0.0f;
    initTalkCtrlDirect(iter, "TicoFat_Request000", requestOffset, nullptr);
    TVec3f eatOffset;
    eatOffset.x = 0.0f; eatOffset.y = 220.0f; eatOffset.z = 0.0f;
    mEatTalkCtrl = MR::createTalkCtrlDirect(this, iter, "TicoFat_StarPiece000", eatOffset, nullptr);
    MR::setDistanceToTalk(mMsgCtrl, 280.0f);
    MR::setDistanceToTalk(mEatTalkCtrl, 280.0f);
    MR::onStartOnlyFront(mMsgCtrl);
    MR::onStartOnlyFront(mEatTalkCtrl);
    MR::onRootNodeAutomatic(mMsgCtrl);
    MR::registerEventFunc(mMsgCtrl, TalkMessageFunc_Inline(this, &TicoFatStarPiece::eventFunc));
    MR::setMessageArg(mMsgCtrl, mRemainingStarPieceCount);
    MR::declareEventCameraProgrammable(sEventCamera);
    AnimScaleParam param;
    param.mHitReactionVelocityY = 0.12f;
    param.mHitReactionSpringStrength = 0.12f;
    param.mHitReactionDamping = 0.9f;
    mScaleController = new AnimScaleController(nullptr);
    mSpinAction = cNPCSpinDefault; mTrampledAction = cNPCTrampledDefault; mPointingAction = cNPCPointingDefault; mReactionAction = cNPCReactionDefault;
    mSpinAttackRange = 1000.0f;
    mParam._14 = getActionName("Wait");
    mParam._18 = mParam._14;
    mParam._1C = getActionName("Talk");
    mParam._20 = mParam._1C;
    NPCActorItem item;
    item.mActor = "TicoFatStarPiece";
    item.mGoods0 = lbl_807CFE9C; item.mGoodsJoint0 = lbl_807CFE98;
    item.mGoods1 = lbl_807CFE94; item.mGoodsJoint1 = lbl_807CFE90;
    MR::getNPCItemData(&item, 0);
    equipment(item, false);
    MR::startAction(mGoods0, "TicoFatGoodsStarPiece");
    MR::startAction(mGoods1, "TicoFatGoodsStarPiece");
    MR::setBpkFrame(mGoods0, 0.0f);
    MR::setBpkFrame(mGoods1, 100.0f);
    MR::startBva(this, "Small0");
    MR::startBrk(this, "Normal");
    MR::setBrkFrameAndStop(this, 0.0f);
    updateScale(calcScale());
    mFullnessMeter = new FullnessMeter(this, mStarPieceCount, mStarPieceCount - mRemainingStarPieceCount);
    mFullnessMeter->initWithoutIter();
    mFullnessMeter->setNumber(mStarPieceCount - mRemainingStarPieceCount);
    MR::tryRegisterDemoCast(this, "ハラペコスターピースチコ紹介", iter);
    if (!GameSequenceFunction::isTicoFatStarPieceActive()) makeActorDead();
}


const char* TicoFatStarPiece::getActionName(const char* name) {
    f32 scale = calcScale();
    if (1.6f < scale) {
        if (MR::isEqualString(name, "Eat")) return "Eat2Normal";
        if (MR::isEqualString(name, "EatStart")) return "EatStart2";
        if (MR::isEqualString(name, "Wait")) return "Wait2";
        if (MR::isEqualString(name, "Excite")) return "Excite2";
        if (MR::isEqualString(name, "Joy")) return "Joy2";
    } else if (1.3f < scale) {
        if (MR::isEqualString(name, "Eat")) return "Eat1Normal";
        if (MR::isEqualString(name, "EatStart")) return "EatStart1";
        if (MR::isEqualString(name, "Wait")) return "Wait1";
        if (MR::isEqualString(name, "Excite")) return "Excite1";
        if (MR::isEqualString(name, "Joy")) return "Joy1";
    } else {
        if (MR::isEqualString(name, "Eat")) return "Eat0Normal";
        if (MR::isEqualString(name, "EatStart")) return "EatStart0";
        if (MR::isEqualString(name, "Wait")) return "Wait0";
        if (MR::isEqualString(name, "Excite")) return "Excite0";
        if (MR::isEqualString(name, "Joy")) return "Joy0";
    }
    return name;
}


void TicoFatStarPiece::selectEatMessage(s32 mode) {
    MR::resetNode(mEatTalkCtrl);
    s32 count = 0;
    if (mode == 1) {
        f32 ratio = static_cast<f32>(mRemainingStarPieceCount) / static_cast<f32>(mStarPieceCount);
        if (ratio == 1.0f) count = 1;
        else if (0.5f <= ratio) count = 2;
        else if (0.25f <= ratio) count = 3;
        else if (0.05f <= ratio) count = 4;
        else count = 5;
    } else if (mode == 2) count = 6;
    for (s32 i = 0; i < count; i++) MR::forwardNode(mEatTalkCtrl);
}


void TicoFatStarPiece::exeReaction() {
    if (MR::isFirstStep(this) && !mSpinAttackedStart) mScaleController->startHitReaction();
    if (mTrampledStart) {
        MR::startActionSound(this, "SmNpcTrampled", -1, -1, -1);
        MR::startActionSound(this, "SvTicofatTrampled", -1, -1, -1);
    }
    if (isPointingSe()) {
        MR::startDPDHitSound();
        MR::startActionSound(this, "SvTicofatPoint", -1, -1, -1);
    }
    if (mSpinAttackedStart) MR::startActionSound(this, "SmTicofatSpin", -1, -1, -1);
    if (mReactionStart) MR::startActionSound(this, "SvTicofatStarPieceHit", -1, -1, -1);
    MR::tryStartReactionAndPopNerve(this);
}


void TicoFatStarPiece::exeWait() {
    if (MR::isFirstStep(this)) MR::startAction(this, getActionName("Wait"));
    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvReaction))) {
        if (!MR::isPlayerElementModeInvincible()) MR::tryTalkNearPlayerAndStartTalkAction(this);
    }
}


void TicoFatStarPiece::exePoint() {
    if (MR::isFirstStep(this)) {
        const char* talk = getActionName("Excite");
        mParam._14 = getActionName("Wait");
        mParam._1C = talk;
        mParam._18 = nullptr;
        mParam._20 = nullptr;
        mParam._0 = false;
        mParam._1 = false;
        selectEatMessage(1);
    }
    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvReaction))) {
        if (mPointingTimer > 0) { setNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvEatWait)); return; }
        turnToDefault(mParam._8);
        MR::tryTalkNearPlayer(mEatTalkCtrl);
        MR::tryStartTalkAction(this);
    }
}


void TicoFatStarPiece::exeEatWait() {
    if (MR::isFirstStep(this)) { MR::startAction(this, getActionName("Excite")); selectEatMessage(0); }
    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvReaction))) {
        if (mPointingTimer <= 0) { setNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvPoint)); return; }
        turnToDefault(mParam._8);
        MR::tryTalkForce(mEatTalkCtrl);
    }
}


void TicoFatStarPiece::exeEat() {
    if (MR::isFirstStep(this)) MR::startAction(this, getActionName("EatStart"));
    if (MR::tryStartActionNotPlaying(this, getActionName("Eat"))) MR::setBckFrame(this, MR::getBckFrame(this));
    MR::startActionSound(this, "SmLvTicofatEating", calcEatSoundPitch(), -1, -1);
    if (mPendingStarPieceCount <= 0) setNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvChem));
}


void TicoFatStarPiece::exeChem() {
    if (MR::isFirstStep(this)) { mEatEndSoundTimer = 5; MR::startAction(this, getActionName("Chem")); }
    if (mEatEndSoundTimer > 0) { mEatEndSoundTimer--; MR::startActionSound(this, "SmLvTicofatEating", calcEatSoundPitch(), -1, -1); }
    if (MR::isBckLooped(this)) setNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvEatWait));
}


void TicoFatStarPiece::control() {
    if (isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvTransform))) { updatePointing(); return; }
    NPCActor::control();
    if (isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvReaction)) || isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvWait)) ||
        isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvPoint)) || isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvEatWait)) ||
        isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvEat)) || isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvChem))) {
        MR::startActionSound(this, "SmLvTicoWait", -1, -1, -1);
    }
    mEatSoundTimer--;
    if (mEventRequested) {
        updatePointing(); updateEventCamera(); updateStarPieceGift();
        if (mRemainingStarPieceCount == 0) setNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvTransform));
    }
}


bool TicoFatStarPiece::receiveMsgPlayerAttack(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (MR::isMsgStarPieceAttack(msg)) { mReacting = true; return true; }
    if (MR::isMsgStarPieceGift(msg)) { receiveStarPiece(MR::getStarPieceGiftCount(msg)); return true; }
    return NPCActor::receiveMsgPlayerAttack(msg, sender, receiver);
}


void TicoFatStarPiece::receiveStarPiece(s32 count) {
    s32 received = mStarPieceCount;
    if (received >= count) received = count;
    mPendingStarPieceCount -= received;
    mRemainingStarPieceCount -= received;
    GameDataFunction::addTicoFatStarPieceAlms(mTicoFatId, received);
    MR::tryRumblePadVeryWeak(this, 0);
    startEatSound();
    updateScale(calcScale());
    mFullnessMeter->setNumber(mStarPieceCount - mRemainingStarPieceCount);
    MR::setMessageArg(mMsgCtrl, mRemainingStarPieceCount);
    if (isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvEat))) {
        MR::setBckRate(this, MR::calcNerveValue(this, sStarPieceTicoEatSpeedStartStep, 180, 1.0f, 1.6f));
        return;
    }
    if (!isEmptyNerve()) { popNerve(); return; }
    setNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvEat));
}


bool TicoFatStarPiece::eventFunc(u32) {
    mEventRequested = true;
    setNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvPoint));
    return true;
}


// NONMATCHING: optimizer folds a redundant jump to the common epilogue.
void TicoFatStarPiece::updateStarPieceGift() {
    if (mPointingTimer <= 0) { mFullnessMeter->requestDisappear(); return; }
    mFullnessMeter->requestAppear();
    if (MR::isIntervalStep(this, 5) && MR::testCorePadButtonB(0) && !MR::isDemoActive() &&
        !isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvReaction)) && mRemainingStarPieceCount > 0) {
        if (MR::getStarPieceNum() <= 0) return;
        s32 count = 15;
        if (mStarPieceCount / 50 <= count) count = mStarPieceCount / 50;
        if (count >= mRemainingStarPieceCount) count = mRemainingStarPieceCount;
        s32 available = MR::getStarPieceNum();
        if (count >= available) count = available;
        if (MR::giftStarPieceToTarget(getSensor("Mouth"), count)) {
            mPendingStarPieceCount += count;
            MR::tryRumblePadVeryWeak(this, 0);
        }
    }
}


void TicoFatStarPiece::updatePointing() {
    if (mRemainingStarPieceCount == 0 || MR::isPlayerElementModeInvincible() || MR::isPlayerFlying()) {
        mNearPlayerTimer = 0; mPointingTimer = 0; return;
    }
    if (MR::isNearPlayer(mEatTalkCtrl, -1.0f)) {
        if (MR::requestTicoFatGuidance()) MR::requestCounterLayoutAppearanceForTicoEat(false);
        mNearPlayerTimer = 8;
    } else {
        mNearPlayerTimer = (mNearPlayerTimer - 1 <= 0 ? 0 : mNearPlayerTimer - 1);
    }
    if (mNearPlayerTimer <= 0) { mPointingTimer = 0; return; }
    bool pointing;
    if (isNerve(GET_NERVE(TicoFatStarPiece, TicoFatStarPieceNrvEat))) pointing = MR::isStarPointerPointing1PWithoutCheckZ(this, nullptr, false, false);
    else pointing = MR::isStarPointerPointing(this, 0, false, nullptr);
    if (pointing) { mPointingTimer = 4; return; }
    mPointingTimer = (mPointingTimer - 1 <= 0 ? 0 : mPointingTimer - 1);
}


// NONMATCHING: temporary vector stack allocation and literal addressing differ.
void TicoFatStarPiece::updateEventCamera() {
    if (mNearPlayerTimer <= 0 && mRemainingStarPieceCount != 0 && tryEndEventCamera()) return;
    bool start = false;
    if (mNearPlayerTimer > 0 && !MR::isGlobalEventCameraActive(sEventCamera)) start = true;
    if (!start) return;
    CameraTargetArg cameraTarget(this);
    MR::startGlobalEventCamera(sEventCamera, cameraTarget, 120);
    TVec3f x, y, z, position;
    MR::extractMtxXYZDir(getBaseMtx(), &x, &y, &z);
    MR::extractMtxTrans(getBaseMtx(), &position);
    x.set<f32>((1.0f - 2.0f * (mInitOrientationQuat.y * mInitOrientationQuat.y)) - 2.0f * (mInitOrientationQuat.z * mInitOrientationQuat.z), 2.0f * (mInitOrientationQuat.x * mInitOrientationQuat.y) + 2.0f * (mInitOrientationQuat.w * mInitOrientationQuat.z), 2.0f * (mInitOrientationQuat.x * mInitOrientationQuat.z) - 2.0f * (mInitOrientationQuat.w * mInitOrientationQuat.y));
    y.set<f32>(2.0f * (mInitOrientationQuat.x * mInitOrientationQuat.y) - 2.0f * (mInitOrientationQuat.w * mInitOrientationQuat.z), (1.0f - 2.0f * (mInitOrientationQuat.x * mInitOrientationQuat.x)) - 2.0f * (mInitOrientationQuat.z * mInitOrientationQuat.z), 2.0f * (mInitOrientationQuat.y * mInitOrientationQuat.z) + 2.0f * (mInitOrientationQuat.w * mInitOrientationQuat.x));
    z.set<f32>(2.0f * (mInitOrientationQuat.x * mInitOrientationQuat.z) + 2.0f * (mInitOrientationQuat.w * mInitOrientationQuat.y), 2.0f * (mInitOrientationQuat.y * mInitOrientationQuat.z) - 2.0f * (mInitOrientationQuat.w * mInitOrientationQuat.x), (1.0f - 2.0f * (mInitOrientationQuat.x * mInitOrientationQuat.x)) - 2.0f * (mInitOrientationQuat.y * mInitOrientationQuat.y));
    position = mInitPosition;
    TVec3f eye = ((position + x * sStarPieceTicoCameraEyeSide) + y * 100.0f) + z * MR::getLinerValueFromMinMax(calcScale(), 1.0f, 1.9f, 1300.0f, 1700.0f);
    TVec3f target = ((position + x * sStarPieceTicoCameraTargetSide) + y * 150.0f) + z * sStarPieceTicoCameraTargetFront;
    MR::setProgrammableCameraParam(sEventCamera, target, eye, y, false);
}


// NONMATCHING: floating arithmetic scheduling and register allocation differ.
f32 TicoFatStarPiece::calcScale() {
    f32 maxScale = 1.9f;
    f32 minScale = 1.0f;
    f32 scaleRange = maxScale - minScale;
    f32 eatenRatio = static_cast<f32>(mStarPieceCount - mRemainingStarPieceCount) / static_cast<f32>(mStarPieceCount);
    return minScale + scaleRange * eatenRatio;
}


// NONMATCHING: floating multiply operand register order differs.
void TicoFatStarPiece::updateScale(f32 scale) {
    f32 previous = mScale.x;
    mScale.setAll<f32>(scale);
    bool hitReaction = false;
    if ((previous <= 1.3f && 1.3f < scale) || (previous <= 1.6f && 1.6f < scale)) hitReaction = true;
    if (hitReaction) mScaleController->startHitReaction();
    MR::startBrk(this, "FatRed");
    f32 minScale = 1.0f, maxScale = 1.9f;
    f32 fraction = (scale - minScale) / (maxScale - minScale);
    fraction = 1.0f >= fraction ? fraction : 1.0f;
    MR::setBrkFrameAndStop(this, fraction * MR::getBrkFrameMax(this));
    MR::setSensorRadius(this, "Body", 100.0f * scale);
    MR::setStarPointerTargetRadius3d(this, 110.0f * scale);
    MR::setDistanceToTalk(mMsgCtrl, 280.0f * (scale >= 1.5f ? 1.5f : scale));
    MR::setDistanceToTalk(mEatTalkCtrl, 280.0f * (scale >= 1.5f ? 1.5f : scale));
    MR::setMessageBalloonFollowOffset(mMsgCtrl, TVec3f(0.0f, 220.0f * scale, 0.0f));
    MR::setMessageBalloonFollowOffset(mEatTalkCtrl, TVec3f(0.0f, 220.0f * scale, 0.0f));
}


s32 TicoFatStarPiece::calcEatSoundPitch() {
    s32 divisor = 50;
    s32 pitch = (mStarPieceCount - mRemainingStarPieceCount - 1) / divisor;
    if (pitch < 0) pitch = 0;
    if (pitch > 32) pitch = 32;
    return pitch;
}


void TicoFatStarPiece::startEatSound() {
    if (mEatSoundTimer <= 0) {
        MR::startActionSound(this, "SmTicofatAbsorb", -1, -1, -1);
        mEatSoundTimer = 5;
    }
}


void TicoFatStarPiece::startEatCompleteSound() {
    s32 pitch = calcEatSoundPitch();
    MR::startActionSound(this, "SmLvTicofatGlad", pitch, -1, -1);
}


bool TicoFatStarPiece::tryEndEventCamera() {
    if (MR::isGlobalEventCameraActive(sEventCamera)) {
        MR::endGlobalEventCamera(sEventCamera, 120, true);
        return true;
    }
    return false;
}


TicoFatStarPiece::~TicoFatStarPiece() {}

extern const volatile s32 sStarPieceTicoEatSpeedStartStep = 0;
extern const volatile f32 sStarPieceTicoCameraEyeSide = 0.0f;
extern const volatile f32 sStarPieceTicoCameraTargetSide = 0.0f;
extern const volatile f32 sStarPieceTicoCameraTargetFront = 0.0f;
