// NONMATCHING: init callback lifetime/register allocation and scalar versus paired-single stores; faceToPlayer register allocation; other residuals are literal aliases.
#pragma auto_inline off

#include "Game/NPC/Kinopio.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"



extern const char lbl_807CFE90[4], lbl_807CFE94[4], lbl_807CFE98[4], lbl_807CFE9C[4];

namespace NrvKinopio {
    NEW_NERVE(KinopioNrvReaction, Kinopio, Reaction);
    NEW_NERVE(KinopioNrvTakeOutStar, Kinopio, TakeOutStar);
    NEW_NERVE(KinopioNrvFar, Kinopio, Jump);
    NEW_NERVE(KinopioNrvNear, Kinopio, Jump);
    NEW_NERVE(KinopioNrvWait, Kinopio, Wait);
    NEW_NERVE(KinopioNrvStop0, Kinopio, Stop);
    NEW_NERVE(KinopioNrvStop1, Kinopio, Stop);
    NEW_NERVE(KinopioNrvAction, Kinopio, Action);
    NEW_NERVE(KinopioNrvAppear, Kinopio, Appear);
    NEW_NERVE(KinopioNrvEscape, Kinopio, Escape);
    NEW_NERVE(KinopioNrvStarWait, Kinopio, StarWait);
    NEW_NERVE(KinopioNrvMount, Kinopio, Mount);
    NEW_NERVE(KinopioNrvEvent, Kinopio, Event);
    NEW_NERVE(KinopioNrvEventAndTalk, Kinopio, EventAndTalk);
    NEW_NERVE(KinopioNrvEventTalking, Kinopio, EventAndTalk);
    NEW_NERVE(KinopioNrvEventFall, Kinopio, EventFall);
};  // namespace NrvKinopio

namespace {
    class Param : public AnimScaleParam {
    public:
        Param();
    };

    Param::Param() {
        mDpdVibrationTime = 15.0f;
        mDpdVibrationBase = 1.4f;
        mHitReactionVelocityY = 0.3f;
    }

    static Param sParam;

    static const f32 sDistancePlayerIsNear = 200.0f;
    static const f32 sDistancePlayerIsFar = 300.0f;
    static const f32 sDistancePlayerIsEvent = 450.0f;
    static const f32 sTurnLimit = 0.997f;
    static const f32 sUpVecBlendRate = 1.0f;
    static const f32 sDistanceEventPlayerIsNear = 600.0f;
    static const s32 sTimeToCancelEvent = 180;
    static const f32 sDistanceLodMiddle = 1500.0f;
    static const f32 sDistanceLodLow = 2500.0f;
    static const f32 sFallGravity = 0.6f;
    static const f32 sFallFrictionRate = 0.98f;
    static const s32 sLandingTime = 3;
    static const s32 sPickelIndex = 2;
    static const f32 sDefaultScale = 1.2f;
    static const f32 sWalkSpeed = 0.83f;
    static const f32 sRunSpeed = 3.32f;
};  // namespace

Kinopio::Kinopio(const char* pName)
    : NPCActor(pName), mObjArg0(), mBehavior(-1), mAppearBehavior(-1), mGoodsIndex(-1), mMapObjConnector(), mTakeOutStar(), mIsSpawnPowerStarMode(), mHasStartedStarTalk() {
    mMountRotation.set(0.0f, 0.0f, 0.0f, 1.0f);
}

void Kinopio::init(const JMapInfoIter& rIter) {
    MR::initDefaultPosAndQuat(this, rIter);

    TVec3f rotation(mRotation);
    rotation.scale(MR::toRadian(1.0f));
    _A0.setEuler(rotation);

    MR::getJMapInfoArg1NoInit(rIter, &mObjArg0);
    MR::getJMapInfoArg2NoInit(rIter, &mBehavior);
    MR::getJMapInfoArg3NoInit(rIter, &mIsSpawnPowerStarMode);
    MR::getJMapInfoArg4NoInit(rIter, &mAppearBehavior);
    MR::getJMapInfoArg7NoInit(rIter, &mGoodsIndex);
    MR::initActor(this,rIter,"Kinopio",false);
    TVec3f scale(::sDefaultScale);
    mScale.x=scale.x; mScale.y=scale.y; mScale.z=scale.z;
    MR::invalidateHitSensor(this, "Pickel");
    s32 isInvalidateSensor = -1;
    MR::getJMapInfoArg6NoInit(rIter, &isInvalidateSensor);
    if (isInvalidateSensor != -1) {
        MR::invalidateHitSensor(this, "Body");
    }

    MR::offBind(this);
    initNerve(GET_NERVE(Kinopio,KinopioNrvWait));
    mReactionNerve=GET_NERVE(Kinopio,KinopioNrvReaction);
    mMsgCtrl = MR::createTalkCtrl(this, rIter, "Kinopio", TVec3f(0.0f, 122.0f * ::sDefaultScale, 0.0f), nullptr);
    MR::setDistanceToTalk(mMsgCtrl, ::sDistancePlayerIsNear);
    MR::onRootNodeAutomatic(mMsgCtrl);
    MR::registerEventFunc(mMsgCtrl, TalkMessageFunc_Inline(this, &Kinopio::eventFunc));
    MR::registerBranchFunc(mMsgCtrl, TalkMessageFunc_Inline(this, &Kinopio::branchFunc));
    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToRailStartPoint(this);
        MR::onCalcShadow(this, nullptr);
    }

    const TVec3f& targetOffset=MR::getStarPointerTargetOffset(this);
    mYoshiTarget=MR::createYoshiLockOnTarget(this,getBaseMtx(),targetOffset);
    MR::addToAttributeGroupSearchTurtle(this);
    MR::addEffect(this,"YoshiTongueHit");
    s32 isStrictLodDistances = -1;
    MR::getJMapInfoArg5NoInit(rIter, &isStrictLodDistances);
    mLodCtrl = MR::createLodCtrlNPC(this, rIter);
    if (isStrictLodDistances != -1) {
        mLodCtrl->setDistanceToMiddleAndLow(::sDistanceLodMiddle, ::sDistanceLodLow);
    }

    NPCActorItem item;
    item.mActor="Kinopio"; item.mGoods0=lbl_807CFE9C; item.mGoodsJoint0=lbl_807CFE98;
    item.mGoods1=lbl_807CFE94; item.mGoodsJoint1=lbl_807CFE90;
    MR::getNPCItemData(&item, mGoodsIndex);
    equipment(item, false);
    mScaleController = new AnimScaleController(&::sParam);
    mDelegator = MR::createJointDelegatorWithNullChildFunc< Kinopio >(this, &Kinopio::calcJointScale, "body");

    MR::tryRegisterDemoCast(this, rIter);
    _130=cNPCSpinDefault; _134=cNPCTrampledDefault; _138=cNPCPointingDefault; _13C=cNPCReactionDefault;
    _12C = ::sDistancePlayerIsEvent;
    mParam._14=cNPCWaitDefault; mParam._18=cNPCTurnDefault;
    mParam.setTalkAction("Talk", "Talk");
    setTalkAction("Walk");
    _124 = true;
    _10C = ::sWalkSpeed;
    switch (mBehavior) {
    case -1:
        if (mGoodsIndex == ::sPickelIndex) {
            mParam.setMoveAction("WaitPickel", "WaitPickel");
            mParam.setTalkAction("TalkPickel", "TalkPickel");
            setTalkAction("WalkPickel");
            break;
        }

        setNerve(GET_NERVE(Kinopio, KinopioNrvFar));
        break;
    case 0:
        mParam.setNoTurnAction("SpinWait1");
        break;
    case 1:
        mParam.setNoTurnAction("SpinWait2");
        break;
    case 2:
        mParam.setNoTurnAction("SpinWait3");
        break;
    case 3:
        _128 = false;
        MR::setDistanceToTalk(mMsgCtrl, 400.0f);
        break;
    case 5:
        _128 = false;
        mParam.setSingleAction("SwimWait");
        break;
    case 7:
        _128 = false;
        mParam.setNoTurnAction("Sleep");
        break;
    case 9:
        mParam.setMoveAction("KinopioGoodsWeapon", "KinopioGoodsWeaponTurn");
        mParam.setTalkAction("KinopioGoodsWeapon", "KinopioGoodsWeaponTurn");
        setTalkAction("KinopioGoodsWeaponWalk");
        break;
    case 10:
        mParam.setSingleAction("Joy");
        break;
    case 11:
        mParam.setNoTurnAction("Rightened");
        break;
    case 14:
        mParam.setMoveAction("Rightened","TurnRightened");
        mParam.setTalkAction("Rightened","TurnRightened");
        break;
    case 12: {
        mParam.setSingleAction("StarPieceWait");
        const char* pWaitAction = "StarPieceWait";
        _11C = "KinopioGoodsStarPieceWalk";
        _120 = pWaitAction;
        break;
    }
    case 13:
        setTalkAction("Getaway");
        _10C = ::sRunSpeed;
        break;
    case 8:
        mIsSpawnPowerStarMode = true;
        break;
    case 6:
        mParam.setNoTurnAction("Pickel");
        MR::validateHitSensor(this, "Pickel");
        break;
    case 4:
        mMapObjConnector = new MapObjConnector(this);
        MR::invalidateClipping(this);
        mParam._0 = false;
        mParam._1 = false;
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        setNerve(GET_NERVE(Kinopio, KinopioNrvMount));
        break;
    }

    if (mIsSpawnPowerStarMode) {
        mTakeOutStar = new TakeOutStar(this, "TakeOutStar", "TakeOutStar", GET_NERVE(Kinopio, KinopioNrvTakeOutStar), nullptr);
        MR::declarePowerStar(this);
        setNerve(GET_NERVE(Kinopio, KinopioNrvStarWait));
    }

    MR::useStageSwitchAwake(this,rIter);
    if(MR::useStageSwitchSyncAppear(this,rIter) && mBehavior!=4) {
        if(mAppearBehavior==0) pushNerve(GET_NERVE(Kinopio,KinopioNrvEscape));
        else pushNerve(GET_NERVE(Kinopio,KinopioNrvAppear));
    }

    MR::startBrk(this, "ColorChange");
    MR::setBrkFrameAndStop(this, mObjArg0);
    MR::registerMarioFaceShipNpc(this,rIter);
}

void Kinopio::exeReaction() {
    if (mBehavior == 7) {
        if (_D8) {
            MR::startActionSound(this, "SmNpcTrampled");
            MR::startActionSound(this, "SvKinopioPointingSleep");
        }

        if (isPointingSe()) {
            MR::startDPDHitSound();
            MR::startActionSound(this, "SvKinopioPointingSleep");
        }

        if (_D9) {
            MR::startActionSound(this, "SmNpcTrampled");
            MR::startActionSound(this, "SvKinopioPointingSleep");
        }

        if (_DB) {
            MR::startActionSound(this, "SmNpcTrampled");
            MR::startActionSound(this, "SvKinopioPointingSleep");
        }
    } else {
        if (_D8) {
            MR::startActionSound(this, "SmNpcTrampled");
            MR::startActionSound(this, "SvKinopioTrampled");
        }

        if (isPointingSe()) {
            MR::startDPDHitSound();
            MR::startActionSound(this, "SvKinopioPointing");
        }

        if (_DB) {
            MR::startActionSound(this, "SvKinopioStarPieceHit");
        }
    }

    if (MR::tryStartReactionAndPopNerve(this)) {
        return;
    }
}

bool Kinopio::isPlayerNotNearKinopio() {
    return !MR::isNearPlayer(this, ::sDistancePlayerIsFar);
}

void Kinopio::exeJump() {
    s32 nextRailPoint;
    if (MR::isExistRail(this)) {
        nextRailPoint = MR::getNextRailPointNo(this);
    } else {
        nextRailPoint = 0;
    }

    if (MR::isFirstStep(this) && !MR::isExistRail(this)) {
        MR::onCalcShadowOneTime(this, nullptr);
    }

    if (MR::tryStartReactionAndPushNerve(this, GET_NERVE(Kinopio, KinopioNrvReaction))) {
        return;
    }

    if (MR::tryTalkNearPlayerAtEnd(mMsgCtrl)) {
        MR::startAction(this, "Notice");
    }

    if (MR::isActionContinuous(this)) {
        if (!isNerve(GET_NERVE(Kinopio, KinopioNrvNear))) {
            return;
        }

        if (MR::isAnyAnimPlaying(this, "Notice")) {
            turnToPlayer(mParam._8);
        }

        return;
    }

    if (isNerve(GET_NERVE(Kinopio, KinopioNrvFar))) {
        if (MR::isNearPlayer(this, ::sDistancePlayerIsNear)) {
            MR::startAction(this, "Notice");
            setNerve(GET_NERVE(Kinopio, KinopioNrvNear));
            return;
        }

        MR::tryStartMoveTalkAction(this);
    } else {
        if (isPlayerNotNearKinopio()) {
            setNerve(GET_NERVE(Kinopio, KinopioNrvFar));
        }

        MR::tryStartTalkAction(this);
    }

    if (MR::isExistRail(this)) {
        if (MR::getNextRailPointNo(this) != nextRailPoint) {
            tryStartArgs();
        }
    }
}

void Kinopio::exeStop() {
    if (MR::isFirstStep(this)) {
        _10C = 0.0f;
    }

    if (!(MR::tryStartReactionAndPushNerve(this, GET_NERVE(Kinopio, KinopioNrvReaction)) || MR::tryStartMoveTalkAction(this))) {
        if (isNerve(GET_NERVE(Kinopio, KinopioNrvStop0))) {
            if (MR::isGreaterEqualStep(this, 60)) {
                setDefaultNerve();
            }
        } else if (MR::isGreaterEqualStep(this, 120)) {
            setDefaultNerve();
        }
    }
}

void Kinopio::exeAction() {
    if (MR::isFirstStep(this)) {
        _10C = 0.0f;
    }

    MR::startMoveAction(this);
    if (!MR::isActionContinuous(this)) {
        if (MR::isEqualStringCase(MR::getPlayingBckName(this), "Tumble")) {
            MR::startAction(this, "Getup");
            setNerve(GET_NERVE(Kinopio, KinopioNrvAction));
        } else {
            setDefaultNerve();
        }
    }
}

void Kinopio::exeWait() {
    s32 nextRailPoint;
    if (MR::isExistRail(this)) {
        nextRailPoint = MR::getNextRailPointNo(this);
    } else {
        nextRailPoint = 0;
    }

    if (MR::isFirstStep(this) && !MR::isExistRail(this)) {
        MR::onCalcShadowOneTime(this, nullptr);
    }

    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(Kinopio, KinopioNrvReaction))) {
        MR::tryTalkNearPlayerAndStartMoveTalkAction(this);
        if (MR::isExistRail(this) && MR::getNextRailPointNo(this) != nextRailPoint) {
            tryStartArgs();
        }
    }
}

void Kinopio::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Change");
        MR::invalidateClipping(this);
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
    }

    MR::calcGravity(this);

    TVec3f fallSpeed(mGravity);
    fallSpeed.scale(::sFallGravity);
    mVelocity.add(fallSpeed);
    mVelocity.mult(::sFallFrictionRate);

    faceToPlayer(::sTurnLimit);

    if (MR::isBinded(this)) {
        mVelocity.zero();
        if (MR::isActionEnd(this)) {
            MR::validateClipping(this);
            popNerve();
        }
    }
}

void Kinopio::exeEscape() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Escape");
        MR::startActionSound(this, "SvKinopioPuha");
        MR::startActionSound(this, "SmNpcFlyOut");
        MR::invalidateClipping(this);
    }

    if (MR::isAnyAnimOneTimeAndStopped(this, "Escape")) {
        MR::validateClipping(this);
        popNerve();
    }
}

void Kinopio::exeMount() {
    if (MR::isFirstStep(this) && mMapObjConnector->mParts == nullptr) {
        MR::calcGravity(this);
        MR::setBaseTRMtx(this, _A0);
        mMapObjConnector->attachToUnder();
        _B0.set(0.0f, 0.0f, 0.0f, 1.0f);
        _A0.set(0.0f, 0.0f, 0.0f, 1.0f);
    }

    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(Kinopio, KinopioNrvReaction))) {
        if (MR::tryTalkNearPlayerAndStartTalkAction(this)) {
            faceToPlayer(::sTurnLimit);
        } else {
            faceToDefault(::sTurnLimit);
        }
    }
}

void Kinopio::exeStarWait() {
    exeWait();
    if (MR::isTalkTalking(mMsgCtrl) && !mHasStartedStarTalk) {
        mHasStartedStarTalk = true;
    }
}

void Kinopio::exeEventAndTalk() {
    MR::tryTalkRequest(mMsgCtrl);
    if (MR::isNearPlayer(mMsgCtrl, ::sDistanceEventPlayerIsNear)) {
        MR::tryTalkNearPlayer(mMsgCtrl);
    }
}

void Kinopio::exeEventFall() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
    }

    if (MR::isBindedWall(this) || MR::isBindedRoof(this)) {
        mVelocity.zero();
    }

    MR::calcGravity(this);

    TVec3f fallSpeed(mGravity);
    fallSpeed.scale(::sFallGravity);
    mVelocity.add(fallSpeed);
    mVelocity.mult(::sFallFrictionRate);

    faceToPlayer(::sTurnLimit);

    if (!tryCancelEvent() && MR::isBindedGround(this) && MR::isGreaterStep(this, ::sLandingTime)) {
        mVelocity.zero();
        MR::validateClipping(this);
        MR::turnQuatYDirRate(&_A0, _A0, -mGravity, ::sUpVecBlendRate);
        setInitPose();
        MR::tryForwardNode(mMsgCtrl);
        setNerve(GET_NERVE(Kinopio, KinopioNrvWait));
    }
}

void Kinopio::exeEvent() {
}

void Kinopio::exeTakeOutStar() {
}

void Kinopio::calcAndSetBaseMtx() {
    if (isNerve(GET_NERVE(Kinopio, KinopioNrvEvent)) || isNerve(GET_NERVE(Kinopio, KinopioNrvEventAndTalk)) ||
        isNerve(GET_NERVE(Kinopio, KinopioNrvEventTalking))) {
        return;
    }

    if (mMapObjConnector != nullptr) {
        mMapObjConnector->connect();
        TPos3f mtx(getBaseMtx());
        mtx.getQuat(mMountRotation);

        TQuat4f rotation;
        rotation.mult(mMountRotation, _A0);
        rotation.normalize();
        MR::setBaseTRMtx(this, rotation);
    } else {
        NPCActor::calcAndSetBaseMtx();
    }
}

bool Kinopio::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Pickel")) {
        return false;
    }

    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

bool Kinopio::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xDD) {
        MR::offRootNodeAutomatic(mMsgCtrl);
        MR::onCalcAnim(this);
        setNerve(GET_NERVE(Kinopio, KinopioNrvEvent));
        return true;
    }

    if (msg == 0xDE) {
        setNerve(GET_NERVE(Kinopio, KinopioNrvEventFall));
        return true;
    }

    if (msg == 0xDF) {
        if (!isNerve(GET_NERVE(Kinopio, KinopioNrvEvent))) {
            return false;
        }

        setNerve(GET_NERVE(Kinopio, KinopioNrvEventAndTalk));
        return true;
    }

    if (msg == 0xE0) {
        if (!(isNerve(GET_NERVE(Kinopio, KinopioNrvEventAndTalk)) || isNerve(GET_NERVE(Kinopio, KinopioNrvEventTalking)))) {
            return false;
        }

        setNerve(GET_NERVE(Kinopio, KinopioNrvEvent));
        return true;
    }

    if (msg == 0xE1) {
        MR::tryForwardNode(mMsgCtrl);
        return true;
    }

    return NPCActor::receiveOtherMsg(msg,pSender,pReceiver);
}

void Kinopio::faceToVector(const TVec3f& rDirection, f32 turnLimit) {
    TVec3f direction(rDirection);
    TVec3f up(-mGravity);
    TVec3f currentUp;
    TVec3f currentFront;
    TQuat4f inverse;
    f32 length = mMountRotation.squared();

    if (length <= JGeometry::TUtil< f32 >::epsilon()) {
        inverse.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
    } else {
        inverse.x = -mMountRotation.x / length;
        inverse.y = -mMountRotation.y / length;
        inverse.z = -mMountRotation.z / length;
        inverse.w = mMountRotation.w / length;
    }

    _A0.getYDir(currentUp);
    _A0.getZDir(currentFront);
    MR::normalize(&direction);
    inverse.transform(up);
    inverse.transform(direction);
    MR::turnVecToVecCosOnPlane(&currentFront, direction, currentUp, turnLimit);
    MR::blendQuatUpFront(&_A0, up, currentFront, ::sUpVecBlendRate, 1.0f);
}

void Kinopio::faceToPlayer(f32 turnLimit) {
    TVec3f direction(*MR::getPlayerPos());
    direction.sub(mPosition);
    faceToVector(direction, turnLimit);
}

void Kinopio::faceToDefault(f32 turnLimit) {
    TVec3f defaultUp;
    TVec3f defaultFront;
    TVec3f front;
    _B0.getYDir(defaultUp);
    _B0.getZDir(defaultFront);
    _A0.getZDir(front);
    MR::turnVecToVecCosOnPlane(&front, defaultFront, defaultUp, turnLimit);
    MR::blendQuatUpFront(&_A0, defaultUp, front, ::sUpVecBlendRate, 1.0f);
}

void Kinopio::setDefaultNerve() {
    if (mBehavior == 13) {
        _10C = ::sRunSpeed;
    } else {
        _10C = ::sWalkSpeed;
    }

    if (mBehavior == -1) {
        if (MR::isNearPlayer(this, ::sDistancePlayerIsNear)) {
            setNerve(GET_NERVE(Kinopio, KinopioNrvNear));
        } else {
            setNerve(GET_NERVE(Kinopio, KinopioNrvFar));
        }
    } else {
        setNerve(GET_NERVE(Kinopio, KinopioNrvWait));
    }
}

bool Kinopio::tryStartArgs() {
    s32 pointArg0;
    MR::getCurrentRailPointArg0WithInit(this, &pointArg0);
    switch (pointArg0) {
    case 0:
        setNerve(GET_NERVE(Kinopio, KinopioNrvStop0));
        break;
    case 1:
        setNerve(GET_NERVE(Kinopio, KinopioNrvStop1));
        break;
    case 2:
        MR::startAction(this, "Notice");
        setNerve(GET_NERVE(Kinopio, KinopioNrvAction));
        break;
    case 3:
        MR::startAction(this, "Tumble");
        setNerve(GET_NERVE(Kinopio, KinopioNrvAction));
        break;
    default:
        return false;
    }

    return true;
}

bool Kinopio::tryCancelEvent() {
    if (MR::isGreaterStep(this, ::sTimeToCancelEvent)) {
        kill();
        return true;
    }

    return false;
}

bool Kinopio::eventFunc(u32) {
    return mTakeOutStar->takeOut();
}

bool Kinopio::branchFunc(u32 id) { return MR::branchFuncGameProgress(id); }
