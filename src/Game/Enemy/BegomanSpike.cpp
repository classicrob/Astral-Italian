#include "Game/Enemy/BegomanSpike.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Enemy/BegomanSpringHead.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace NrvBegomanSpike {
    NEW_NERVE_ONEND(HostTypeNrvNoCalcWait, BegomanSpike, NoCalcWait, NoCalcWait);
    NEW_NERVE_ONEND(HostTypeNrvWait, BegomanSpike, Wait, Wait);
    NEW_NERVE_ONEND(HostTypeNrvSignAttack, BegomanSpike, SignAttack, HandBlur);
    NEW_NERVE_ONEND(HostTypeNrvPursue, BegomanSpike, Pursue, Pursue);
    NEW_NERVE_ONEND(HostTypeNrvTurn, BegomanSpike, Turn, Turn);
    NEW_NERVE(HostTypeNrvBrake, BegomanSpike, Brake);
    NEW_NERVE(HostTypeNrvStepBack, BegomanSpike, StepBack);
    NEW_NERVE(HostTypeNrvReturn, BegomanSpike, Return);
    NEW_NERVE_ONEND(HostTypeNrvProvoke, BegomanSpike, Provoke, HandBlur);
    NEW_NERVE_ONEND(HostTypeNrvTrampleReaction, BegomanSpike, TrampleReaction, HandBlur);
    NEW_NERVE_ONEND(HostTypeNrvHitReaction, BegomanSpike, HitReaction, HandBlur);
    NEW_NERVE(HostTypeNrvTired, BegomanSpike, Tired);
    NEW_NERVE(HostTypeNrvBlow, BegomanSpike, Blow);
    NEW_NERVE(HostTypeNrvElectricDeath, BegomanSpike, ElectricDeath);
    NEW_NERVE(HostTypeNrvBrokenPiece, BegomanSpike, BrokenPiece);
    NEW_NERVE(HostTypeNrvInWater, BegomanSpike, InWater);
    NEW_NERVE(HostTypeNrvKeepDistance, BegomanSpike, KeepDistance);
    NEW_NERVE(HostTypeNrvLaunch, BegomanSpike, Launch);
    NEW_NERVE(HostTypeNrvBindStarPointer, BegomanSpike, BindStarPointer);
};  // namespace NrvBegomanSpike

namespace {
    const MR::ActorMoveParam hWaitParam = {0.0f, 3.0f, 0.95f, 1.0f};
    const MR::ActorMoveParam hSignAttackParam = {0.0f, 3.0f, 0.8f, 3.0f};
    const MR::ActorMoveParam hPursueParam = {0.4f, 3.0f, 0.97f, 3.0f};
    const MR::ActorMoveParam hTurnParam = {0.4f, 3.0f, 0.95f, 0.0f};
    const MR::ActorMoveParam hStopParam = {0.0f, 3.0f, 0.95f, 0.0f};
    const MR::ActorMoveParam hHitReactionParam = {0.0f, 3.0f, 0.95f, 0.0f};
    const MR::ActorMoveParam hTiredParam = {0.2f, 1.0f, 0.95f, 3.0f};

    // hTurnStartDegree
    // hTurnEndDegree
    // hIsFaceToPlayerDegree
    // sDiscoverRange
    // sPursueRange
    // sCommonAirDamp
    // sCommonGravity
    // sReboundGroundRatio
    // sReboundRailRatio
    const f32 sReboundEnemyAddVel = 8.0f;
    const f32 sReboundPlayerAddVel = 2.0f;
    const f32 sPreventSwingbyDamp = 0.7f;
    // sHeadRotateRatio
    const f32 sCommonAddRotate = 0.5f;
    // sCameraLimitLength
    const f32 sCoinTransY = 80.0f;
    // sCoinVelocity
    const s32 sCoinGenerateNum = 1;
    // sCheckBodySize
    const f32 sNoCalcWaitRotate = 0.5f;
    // sWaitDampVel
    const f32 sWaitRotate = 0.15f;
    // sStopVelocitySquared
    // sSignRotateDampVel
    // sReadyToGoFrame
    // sSignRotateRotate
    // sHitChanceTime
    const f32 sSignAttackRotate = 0.5f;
    // sPursueLimitTime
    // sAnimFrame
    // sDashFrame
    // sRatio
    // sPursueDampVel
    // sAddVelocity
    // sMaxVelocity
    // sToTurnValue
    // sDashVelocity
    const f32 sPursueRotate = 0.5f;
    // sTurnTime
    // sTurnRatioStart
    // sTurnRatioEnd
    // sTurnDampVel
    // sTurnAddVelocity
    // sToPursueValue
    const f32 sTurnRotate = 0.4f;
    // sBrakeTime
    // sBrakeDampVel
    const f32 sBrakeRotate = 0.2f;
    // sProvokeTime
    const f32 sProvokeRotate = 0.3f;
    // sTrampleTime
    // sTrampleDampVel
    // sTrampleRotate
    const s32 sTrampleReactionTime = 120;
    // sTrampleReactionDampVel
    const f32 sTrampleReactionRotate = 0.25f;
    // sHitReactionDampVel
    const f32 sHitReactionRotate = 0.25f;
    const s32 sStopSceneTime = 2;
    const s32 sBlowFrame = 20;
    // sBlowDampVelAir
    // sBlowDampVelGround
    const f32 sBlowVel = 40.0f;
    const f32 sBlowVerticalVel = 10.0f;
    // sPlayerAddVelScale
    const s32 sDeathStopSceneTime = 5;
    const s32 sElectricDeathTime = 20;
    // sMoveCheckLineLength
    // sMoveCheckLineOffset
    // sHoleRebound
    const f32 sIsInWaterVel = 10.0f;
    const s32 sIsInWaterTime = 60;
    const f32 hKeepDistFar = 800.0f;
    const f32 hKeepDistNear = 600.0f;
};  // namespace

BegomanSpike::BegomanSpike(const char* pName) : BegomanBase(pName), mHead(), mBrokenModel(), mTurnAfterBlow() {
    mHeadMatrix.identity();
    mWaterColumnMatrix.identity();
}

void BegomanSpike::makeActorAppeared() {
    LiveActor::makeActorAppeared();

    if (mHead != nullptr) {
        mHead->makeActorAppeared();
    }
}

void BegomanSpike::makeActorDead() {
    BegomanBase::makeActorDead();

    if (mHead != nullptr) {
        mHead->makeActorDead();
    }
}

void BegomanSpike::init(const JMapInfoIter& rIter) {
    initCore(rIter, "BegomanSpike", false);
    initUseSwitchB(rIter, MR::FunctorV0M< BegomanSpike*, void (BegomanSpike::*)() >(this, &BegomanSpike::onSwitchB));
    MR::declareCoin(this, 1);
    initBinder(100.0f, 100.0f, 0);
    initNerve(GET_NERVE(BegomanSpike, HostTypeNrvWait), 1);
    initSensor(0, 70.0f, 140.0f, "Center");
    initSound(8, "BegomanSpike", nullptr, TVec3f(0.0f));
    initEffectAndStarPointerBind();
    MR::setEffectHostMtx(this, "WaterColumn", mWaterColumnMatrix);
    initShadow(70.0f, "Center");
    MR::initActorState(this, mStarPointBind, GET_NERVE(BegomanSpike, HostTypeNrvBindStarPointer), "SupportFreeze");
    mHead = new BegomanHead(this, "トゲ頭", "BegomanSpikeHead", mHeadMatrix, MR::DrawBufferType_Enemy, false);
    MR::initLightCtrl(mHead);
    mHead->mScaleSource = &mHeadScale;
    mHead->initWithoutIter();
    mBrokenModel = new ModelObj("ベーゴマン壊れモデル", "BegomanBrokenPiece", getBaseMtx(), MR::DrawBufferType_Enemy, -2, -2, false);
    MR::setScale(mBrokenModel, mScale);
    mBrokenModel->initWithoutIter();
    MR::invalidateClipping(mBrokenModel);
    mBrokenModel->makeActorDead();

    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::tryRegisterDemoCast(mHead, rIter);
    }

    MR::useStageSwitchAwake(this, rIter);

    if (MR::isDemoCast(this, nullptr) && MR::isRegisteredDemoActionAppear(this)) {
        makeActorDead();
    } else {
        MR::useStageSwitchSyncAppear(this, rIter);
    }
}

void BegomanSpike::endWait() {
    MR::invalidateClipping(this);
}

void BegomanSpike::endHandBlur() {
    MR::deleteEffect(this, "HandBlur1");
    MR::deleteEffect(this, "HandBlur2");
    MR::deleteEffect(this, "HandBlur3");
}

void BegomanSpike::onSwitchB() {
    setNerve(GET_NERVE(BegomanSpike, HostTypeNrvBrokenPiece));
}

void BegomanSpike::appear() {
    BegomanBase::appear();
    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmBegomanAppear");
    setNerve(GET_NERVE(BegomanSpike, HostTypeNrvWait));
}

void BegomanSpike::control() {
    if (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvPursue)) || isNerve(GET_NERVE(BegomanSpike, HostTypeNrvTurn))) {
        if (incAndCheckTiredCounter()) {
            setNerve(GET_NERVE(BegomanSpike, HostTypeNrvTired));
            return;
        }
    } else {
        mTiredCounter = 0;
    }

    if (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow)) || (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvTurn)) && mTurnAfterBlow)) {
        checkTouchElectricRail(true);
    } else if (!isNerve(GET_NERVE(BegomanSpike, HostTypeNrvElectricDeath)) && !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBrokenPiece))) {
        checkTouchElectricRail(false);
    }

    if (!isNerve(GET_NERVE(BegomanSpike, HostTypeNrvInWater))) {
        if (isInWaterAndSetWaterNerve(GET_NERVE(BegomanSpike, HostTypeNrvInWater), &mWaterColumnMatrix)) {
            return;
        }
    }

    if (MR::isBindedGroundDamageFire(this)) {
        kill();
        MR::emitEffect(this, "Fall");
        return;
    }

    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        kill();
        MR::emitEffect(this, "Death");
        MR::startActionSound(this, "EmExplodeS");
        return;
    }

    if (MR::isPressedRoofAndGround(this)) {
        kill();
        return;
    }

    if (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow))) {
        preventSwingby(::sPreventSwingbyDamp);
    }

    if ((!isNerve(GET_NERVE(BegomanSpike, HostTypeNrvNoCalcWait)) && !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvWait)) &&
         !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvSignAttack)) && !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvPursue)) &&
         !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvTurn)) && !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBrake)) &&
         !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvReturn)) && !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvProvoke)) &&
         !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvKeepDistance))) ||
        !tryAndSetStarPointerBind(GET_NERVE(BegomanSpike, HostTypeNrvBindStarPointer))) {
        BegomanBase::control();
    }
}

bool BegomanSpike::onTouchElectric(const TVec3f& rPos, const TVec3f& rDirection) {
    if (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvElectricDeath)) || isNerve(GET_NERVE(BegomanSpike, HostTypeNrvInWater)) ||
        isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBrokenPiece))) {
        return false;
    }

    if (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow)) || (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvTurn)) && mTurnAfterBlow)) {
        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvElectricDeath));
        return false;
    }

    if (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvStepBack)) && !MR::isOnGround(this)) {
        return false;
    }

    if (BegomanBase::onTouchElectric(rPos, rDirection)) {
        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvStepBack));
        return true;
    }

    return false;
}

bool BegomanSpike::setNerveReturn() {
    setNerve(GET_NERVE(BegomanSpike, HostTypeNrvReturn));
    return true;
}

const Nerve* BegomanSpike::getNerveWait() {
    return GET_NERVE(BegomanSpike, HostTypeNrvWait);
}

void BegomanSpike::setNerveLaunch() {
    setNerve(GET_NERVE(BegomanSpike, HostTypeNrvLaunch));
}

void BegomanSpike::exeNoCalcWait() {
    exeNoCalcWaitCore(::sNoCalcWaitRotate, GET_NERVE(BegomanSpike, HostTypeNrvWait));
}

void BegomanSpike::endNoCalcWait() {
    finishNoCalcWait();
}

void BegomanSpike::exeWait() {
    updateRotateY(::sWaitRotate, ::sCommonAddRotate);
    MR::startActionSound(this, "EmLvBegomanRotSlow");
    exeWaitCore(::hWaitParam, GET_NERVE(BegomanSpike, HostTypeNrvSignAttack), GET_NERVE(BegomanSpike, HostTypeNrvKeepDistance),
                GET_NERVE(BegomanSpike, HostTypeNrvNoCalcWait));
}

void BegomanSpike::exeSignAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SignAttack");
        MR::startActionSound(this, "EmBegomanPrePursue");
    }

    updateRotateY(::sSignAttackRotate, ::sCommonAddRotate);
    MR::startActionSound(this, "EmLvBegomanRotMiddle");
    exeSignAttackCore(::hSignAttackParam, GET_NERVE(BegomanSpike, HostTypeNrvPursue));
}

void BegomanSpike::exePursue() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "AttackBlur");
        MR::startActionSound(this, "EmBegomanPursueStart");
        MR::emitEffect(this, "HandBlur1");
        MR::emitEffect(this, "HandBlur2");
        MR::emitEffect(this, "HandBlur3");
    }

    updateRotateY(::sPursueRotate, ::sCommonAddRotate);
    MR::startActionSound(this, "EmLvBegomanPursue");
    exePursueCore(::hPursueParam, GET_NERVE(BegomanSpike, HostTypeNrvBrake), GET_NERVE(BegomanSpike, HostTypeNrvTurn), *getSoundNormal(), 1.0f);
}

void BegomanSpike::tearDownPursue() {
    MR::deleteEffect(this, "AttackBlur");
    MR::deleteEffect(this, "HandBlur1");
    MR::deleteEffect(this, "HandBlur2");
    MR::deleteEffect(this, "HandBlur3");
}

void BegomanSpike::endPursue() {
    tearDownPursue();
}

void BegomanSpike::exeTurn() {
    if (MR::isFirstStep(this)) {
    }

    updateRotateY(::sTurnRotate, ::sCommonAddRotate);
    MR::startActionSound(this, "EmLvBegomanRotMiddle");
    MR::startActionSound(this, "EmLvBegomanTurn");
    if (mTurnAfterBlow) {
        exeTurnCore(::hTurnParam, GET_NERVE(BegomanSpike, HostTypeNrvBrake), GET_NERVE(BegomanSpike, HostTypeNrvTired), mTurnAfterBlow);
    } else {
        exeTurnCore(::hTurnParam, GET_NERVE(BegomanSpike, HostTypeNrvBrake), GET_NERVE(BegomanSpike, HostTypeNrvPursue), mTurnAfterBlow);
    }
}

void BegomanSpike::endTurn() {
    mTurnAfterBlow = false;
}

void BegomanSpike::exeBrake() {
    updateRotateY(::sBrakeRotate, ::sCommonAddRotate);
    MR::startActionSound(this, "EmLvBegomanSpark");
    MR::startActionSound(this, "EmLvBegomanRotMiddle");
    exeBrakeCore(GET_NERVE(BegomanSpike, HostTypeNrvTurn));
}

void BegomanSpike::exeStepBack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "electricshock");
    }

    updateRotateY(0.2f, ::sCommonAddRotate);
    exeStepBackCore(::hWaitParam, GET_NERVE(BegomanSpike, HostTypeNrvWait));
}

void BegomanSpike::exeReturn() {
    updateRotateY(0.2f, ::sCommonAddRotate);
    exeReturnCore(GET_NERVE(BegomanSpike, HostTypeNrvWait));
}

void BegomanSpike::exeProvoke() {
    updateRotateY(::sProvokeRotate, ::sCommonAddRotate);
    MR::startActionSound(this, "EmLvBegomanRotMiddle");
    exeProvokeCore(::hWaitParam, GET_NERVE(BegomanSpike, HostTypeNrvSignAttack));
}

void BegomanSpike::exeTrampleReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "TrampleReaction");
    }

    updateRotateY(::sTrampleReactionRotate, ::sCommonAddRotate);
    MR::moveAndTurnToDirection(this, &mFaceVec, mTargetVec, ::hStopParam._0, ::hStopParam._4, ::hStopParam._8, ::hStopParam._C);
    reboundWallAndGround(&mFaceVec, false);
    if (MR::isGreaterStep(this, ::sTrampleReactionTime)) {
        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvProvoke));
    }
}

void BegomanSpike::exeHitReaction() {
    updateRotateY(::sHitReactionRotate, ::sCommonAddRotate);
    exeHitReactionCore(::hHitReactionParam, GET_NERVE(BegomanSpike, HostTypeNrvProvoke));
}

void BegomanSpike::exeTired() {
    if (MR::isFirstStep(this)) {
    }

    updateRotateY(0.2f, ::sCommonAddRotate);
    MR::startActionSound(this, "EmLvBegomanTurn");
    exeTiredCore(::hTiredParam, GET_NERVE(BegomanSpike, HostTypeNrvWait));
}

void BegomanSpike::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "EmBegomanRotStop");
        MR::startAction(this, "Damage");
        MR::stopScene(::sStopSceneTime);
    }

    if (MR::isInWater(mPosition)) {
        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvInWater));
        return;
    }

    MR::startActionSound(this, "EmLvBegomanSpark");
    MR::startActionSound(this, "EmLvBegomanRotMiddle");
    MR::moveAndTurnToDirection(this, &mFaceVec, mTargetVec, ::hHitReactionParam._0, ::hHitReactionParam._4, ::hHitReactionParam._8, ::hHitReactionParam._C);
    reboundWallAndGround(&mFaceVec, false);
    if (MR::isGreaterStep(this, ::sBlowFrame) && MR::isOnGround(this)) {
        MR::startAction(this, "Turn");
        mTurnAfterBlow = true;
        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvTurn));
    }
}

void BegomanSpike::exeElectricDeath() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "electricshock");
        mVelocity.zero();
        MR::invalidateHitSensor(this, "Body");
        MR::startActionSound(this, "EmBegomanElecDamage");
        MR::invalidateClipping(this);
    }

    if (MR::isGreaterStep(this, ::sElectricDeathTime)) {
        MR::stopScene(::sDeathStopSceneTime);
        MR::shakeCameraWeak();
        MR::startActionSound(this, "EmExplodeS");
        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvBrokenPiece));
        MR::emitEffect(this, "Death");
    }
}

void BegomanSpike::exeBrokenPiece() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::hideModel(this);
        mBrokenModel->appear();
        MR::startAction(mBrokenModel, "Broken01");
        generateItem();
    }

    if (MR::isActionEnd(mBrokenModel)) {
        MR::showModel(this);
        kill();
    }
}

void BegomanSpike::exeInWater() {
    if (MR::isFirstStep(this)) {
        mVelocity += mGravity;
        mVelocity.setLength(::sIsInWaterVel);
        MR::startActionSound(this, "EmFallIntoWaterS");
    }

    if (MR::isGreaterStep(this, ::sIsInWaterTime)) {
        kill();
        MR::emitEffect(this, "DeathWater");
        MR::startActionSound(this, "EmBegomanDeadInWater");
    }
}

void BegomanSpike::exeKeepDistance() {
    updateRotateY(0.2f, ::sCommonAddRotate);
    exeKeepDistanceCore(GET_NERVE(BegomanSpike, HostTypeNrvWait), GET_NERVE(BegomanSpike, HostTypeNrvSignAttack),
                        GET_NERVE(BegomanSpike, HostTypeNrvBrake), ::hKeepDistFar, ::hKeepDistNear);
}

void BegomanSpike::exeBindStarPointer() {
    BegomanBase::exeBindStarPointer();
}

void BegomanSpike::endBindStarPointer() {
    BegomanBase::finishBindStarPointer();
}

void BegomanSpike::generateItem() NO_INLINE {
    MR::appearCoinPop(this, mPosition - mGravity * ::sCoinTransY, ::sCoinGenerateNum);
}

bool BegomanSpike::receiveMsgPushEnemy(HitSensor* pSender, HitSensor* pReceiver) {
    TVec3f direction(*MR::getSensorPos(pReceiver));
    direction.sub(*MR::getSensorPos(pSender));
    MR::normalizeOrZero(&direction);

    if (MR::isNearZero(direction)) {
        return false;
    }

    bool reflected = reboundPlaneWithEffect(direction, 0.0f, 0.0f, "Spark");
    f32 receiverRadius = MR::getSensorRadius(pReceiver);
    f32 senderRadius = MR::getSensorRadius(pSender);
    f32 ratio = senderRadius / receiverRadius;

    MR::addVelocityLimit(this, direction * ::sReboundEnemyAddVel * ratio);

    if (reflected) {
        MR::startActionSound(this, "EmBegomanColliBegoman");
    }

    if (!isNerve(GET_NERVE(BegomanSpike, HostTypeNrvProvoke)) && !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvTrampleReaction)) &&
        !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow))) {
        MR::startAction(this, "Turn");
        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvTurn));
    }

    return true;
}

void BegomanSpike::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (pSender != getSensor("Body")) {
        return;
    }

    if (MR::isSensorEnemy(pReceiver)) {
        MR::sendMsgToEnemyAttackBlow(pReceiver, pSender);
        return;
    }

    if (!MR::isSensorPlayer(pReceiver) || isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow)) || MR::isPlayerSwingAction() || !MR::isOnGroundPlayer()) {
        return;
    }

    bool msgResult = MR::sendMsgEnemyAttackFlipRot(pReceiver, pSender);

    if (!msgResult) {
        msgResult = MR::sendMsgPush(pReceiver, pSender);
    }

    if (msgResult) {
        TVec3f dirFromSenderToReceiver((*MR::getSensorPos(pSender)));

        dirFromSenderToReceiver -= (*MR::getSensorPos(pReceiver));
        MR::normalizeOrZero(&dirFromSenderToReceiver);

        if (!MR::isNearZero(dirFromSenderToReceiver)) {
            bool reflected = reboundPlaneWithEffect(dirFromSenderToReceiver, 0.0f, 0.0f, "Spark");
            mVelocity += dirFromSenderToReceiver * ::sReboundPlayerAddVel;

            if (reflected) {
                MR::startActionSound(this, "EmBegomanColli");
            }
        }

        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvHitReaction));
    }
}

bool BegomanSpike::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorNpc(pSender)) {
        return receiveMsgPushEnemy(pSender, pReceiver);
    }

    return false;
}

bool BegomanSpike::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (pReceiver != getSensor("Body")) {
        return false;
    }

    if (MR::isMsgEnemyAttackElectric(msg)) {
        TVec3f direction(MR::getSensorHost(pSender)->mVelocity);
        MR::vecKillElement(direction, mGravity, &direction);
        MR::normalizeOrZero(&direction);
        return onTouchElectric(*MR::getSensorPos(pSender), direction);
    }

    if (MR::isMsgExplosionAttack(msg) && !isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow))) {
        calcBlowReaction(*MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver), 35.0f, 15.0f);
        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow));
        return true;
    }

    if (MR::isSensorEnemy(pSender)) {
        return receiveMsgPushEnemy(pSender, pReceiver);
    }

    return false;
}

inline const TVec3f& getBegomanSpikeGravity(const BegomanSpike& actor) {
    return actor.mGravity;
}

bool BegomanSpike::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        if (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow)) || isNerve(GET_NERVE(BegomanSpike, HostTypeNrvElectricDeath)) ||
            isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBrokenPiece))) {
            return false;
        }

        if (MR::isSensorType(pReceiver, 0x21 /* ATYPE_BEGOMAN */)) {
            MR::startActionSound(this, "EmTogegomanStick");
            MR::sendMsgEnemyAttack(pSender, pReceiver);
            if (!isNerve(GET_NERVE(BegomanSpike, HostTypeNrvTrampleReaction)) || !MR::isLessStep(this, 30)) {
                setNerve(GET_NERVE(BegomanSpike, HostTypeNrvTrampleReaction));
            }
        }

        return false;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::forceJumpPlayer(-mGravity);
        return true;
    }

    if (isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow)) || isNerve(GET_NERVE(BegomanSpike, HostTypeNrvElectricDeath)) ||
        isNerve(GET_NERVE(BegomanSpike, HostTypeNrvBrokenPiece))) {
        return false;
    }

    if (pReceiver == getSensor("Body") && (MR::isMsgPlayerHitAll(msg) || MR::isMsgStarPieceAttack(msg))) {
        TVec3f direction((*MR::getSensorPos(pReceiver)));
        direction.sub((*MR::getSensorPos(pSender)));
        TVec3f hitPos(-direction);
        f32 radius = MR::getSensorRadius(this, "Body");
        hitPos.setLength(radius);
        hitPos.add((*MR::getSensorPos(this, "Body")));
        MR::emitEffectHit(this, hitPos, "Hit");
        MR::startActionSound(this, "EmBegomanKnockSuccess");
        finishNoCalcWait();
        setNerve(GET_NERVE(BegomanSpike, HostTypeNrvBlow));
        MR::vecKillElement(direction, mGravity, &direction);
        MR::normalize(&direction);
        mFaceVec.set(direction);
        direction *= ::sBlowVel;
        direction.sub(getBegomanSpikeGravity(*this) * ::sBlowVerticalVel);
        mVelocity = direction;
        return true;
    }

    return false;
}

bool BegomanSpike::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return MR::isMsgHitmarkEmit(msg);
}

void BegomanSpike::calcAnim() {
    LiveActor::calcAnim();
    calcAnimCore(&mHeadMatrix);
}
