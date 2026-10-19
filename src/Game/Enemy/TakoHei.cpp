#include "Game/Enemy/TakoHei.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/TakoHeiInkHolder.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Player/YoshiFunction.hpp"



namespace MR {
    bool isMsgYoshiStartTongue(u32);
    bool isMsgYoshiLoopTongue(u32);
    bool isMsgYoshiCancelEaten(u32);
    bool isMsgYoshiReleaseEaten(u32);
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
}

namespace {
    static TVec3f sInkShotOffset(0.0f, 20.0f, 0.0f);
    static const f32 sBaseScale = 1.0f;
    static const f32 sBodyHitRadius = 70.0f;
    static const f32 sBodyHitYOffset = 70.0f;
    static const f32 sAttackHitRadius = 50.0f;
    static const f32 sAttackHitYOffset = 70.0f;
    static const f32 sCollisionRadius = 90.0f;
    static const f32 sShadowRadius = 60.0f;
    static const f32 sPointerHitSize = 70.0f;
    // static const f32 sFrontVecBlendRate;
    // static const f32 sUpVecBlendRate;
    static const f32 sPushAccel = 5.0f;
    static const f32 sPushScaleYSpeed = 0.1f;
    static const s32 sNoPushTime = 20;
    static const f32 sPushScaleYMarioSpeed = 0.15f;
    static const f32 sPushMarioAccel = 12.0f;
    static const s32 sNoPushMarioTime = 60;
    // static const f32 sFallLimitDistance;
    static const f32 sNormalGroundFrec = 0.85f;
    static const f32 sNormalAirFrec = 0.99f;
    static const f32 sNormalGravityAccel = 1.5f;
    static const f32 sActiveDistance = 3000.0f;
    static const s32 sWaitTime = 120;
    static const s32 sWalkMoveStartTime = 10;
    static const s32 sWalkEndTime = 120;
    static const f32 sWalkAccel = 0.5f;
    static const f32 sWalkGoalDistance = 500.0f;
    static const f32 sWalkTurnDegree = 6.0f;
    static const f32 sWalkTerritoryRadius = 400.0f;
    static const s32 sFindTurnTime = 20;
    static const s32 sFindTime = 30;
    static const f32 sFindDistance = 1600.0f;
    static const f32 sFindTurnDegree = 6.0f;
    static const s32 sPursueTime = 300;
    static const f32 sPursueAccel = 1.0f;
    static const f32 sPursueTurnDegree = 6.0f;
    static const s32 sCoolDownTime = 60;
    static const f32 sAttackSignDegree = 8.0f;
    static const f32 sAttackDistanceNear = 650.0f;
    static const f32 sAttackDistanceFar = 750.0f;
    static const s32 sForceFarAttackTime = 250;
    static const f32 sSpurtPowerH = 20.0f;
    static const f32 sSpurtPowerV = -15.0f;
    static const f32 sCheckFrontHeight = 40.0f;
    static const f32 sCheckFrontAttackSpace = 140.0f;
    static const f32 sBackAttackSpaceAccel = -0.1f;
    static const s32 sAttackSuccessTime = 60;
    static const f32 sAttackSuccessHitPower = 8.0f;
    static const s32 sSwoonTime = 180;
    static const s32 sRecoverTime = 20;
    static const f32 sSwoonGroundFrec = 0.85f;
    static const f32 sSwoonGravityAccel = 1.5f;
    static const s32 sPunchStopSceneTime = 2;
    static const s32 sPunchDownTime = 22;
    static const f32 sPunchHorizonPower = 20.0f;
    static const f32 sPunchVerticalPower = 35.0f;
    static const s32 sFlatDownTime = 20;
    static const s32 sHipDropDownTime = 40;
    static const s32 sPressDownTime = 180;
};  // namespace

namespace NrvTakoHei {
    NEW_NERVE(TakoHeiNrvNonActive, TakoHei, NonActive);
    NEW_NERVE(TakoHeiNrvWait, TakoHei, Wait);
    NEW_NERVE(TakoHeiNrvWalk, TakoHei, Walk);
    NEW_NERVE(TakoHeiNrvFindTurn, TakoHei, FindTurn);
    NEW_NERVE(TakoHeiNrvFind, TakoHei, Find);
    NEW_NERVE(TakoHeiNrvPursue, TakoHei, Pursue);
    NEW_NERVE(TakoHeiNrvCoolDown, TakoHei, CoolDown);
    NEW_NERVE(TakoHeiNrvAttackSign, TakoHei, AttackSign);
    NEW_NERVE(TakoHeiNrvAttack, TakoHei, Attack);
    NEW_NERVE(TakoHeiNrvAttackSuccess, TakoHei, AttackSuccess);
    NEW_NERVE(TakoHeiNrvBindStarPointer, TakoHei, BindStarPointer);
    NEW_NERVE(TakoHeiNrvSwoon, TakoHei, Swoon);
    NEW_NERVE(TakoHeiNrvRecover, TakoHei, Recover);
    NEW_NERVE(TakoHeiNrvPressDown, TakoHei, PressDown);
    NEW_NERVE(TakoHeiNrvHipDropDown, TakoHei, HipDropDown);
    NEW_NERVE(TakoHeiNrvFlatDown, TakoHei, FlatDown);
    NEW_NERVE(TakoHeiNrvPunchDown, TakoHei, PunchDown);

    class TakoHeiNrvDown : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static TakoHeiNrvDown sInstance;
    };
    TakoHeiNrvDown TakoHeiNrvDown::sInstance;
};  // namespace NrvTakoHei

TakoHei::TakoHei(const char* pName)
    : LiveActor(pName), mScaleController(0), mBindStarPointer(0), mYoshiLockOn(0), mPoseQuat(0.0f, 0.0f, 0.0f, 1.0f), mUnusedVecAC(0, 0, 0), mFront(0, 0, 1), mWalkTarget(0, 0, 0), mUnusedFront(0, 0, 1), mHomePos(0, 0, 0), mNoPushStep(0),
      mUnusedCounter(0), mAirStep(0), mUnusedFlag(0), mHasMoveLimitCollision(0) {
    MR::createTakoHeiInkHolder();
}

void TakoHei::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "TakoHei", false);
    mHomePos = mPosition;
    MR::makeQuatAndFrontFromRotate(&mPoseQuat, &mFront, this);
    MR::addToAttributeGroupSearchTurtle(this);
    initNerve(GET_NERVE(TakoHei, TakoHeiNrvWait), 1);
    mScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(TakoHei, TakoHeiNrvBindStarPointer), "SupportFreeze");
    MR::useStageSwitchAwake(this, rIter);
    MR::initUseStageSwitchWriteDead(this, rIter);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, MR::getJointMtx(this, "Center"), TVec3f(0.0f));
    MR::useStageSwitchSyncAppear(this, rIter);
}

void TakoHei::initAfterPlacement() {
    MR::turnQuatUpToGravity(&mPoseQuat, mPoseQuat, this);
    mHasMoveLimitCollision = MR::trySetMoveLimitCollision(this);
}

void TakoHei::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmExplodeS", -1, -1, -1);
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
}

void TakoHei::control() {
    if (isNerve(GET_NERVE(TakoHei, TakoHeiNrvNonActive))) {
        return;
    }

    mScaleController->updateNerve();
    if (mNoPushStep != 0) {
        mNoPushStep--;
    }

    if (MR::isBindedGround(this)) {
        mAirStep = 0;
    } else {
        mAirStep++;
    }

    if (mAirStep >= 5) {
        MR::invalidateClipping(this);
    } else {
        MR::validateClipping(this);
    }

    updatePose();
    tryPressed();
    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        kill();
    }
}

void TakoHei::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPoseQuat);
    MR::updateBaseScale(this, mScaleController);
}

void TakoHei::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvTakoHei::TakoHeiNrvDown::sInstance)) {
        return;
    }

    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayer(pReceiver) && canAttack()) {
            if (!MR::isPlayerHipDropFalling() && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                requestAttackSuccess(pSender, pReceiver);
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else if ((MR::isSensorPlayer(pReceiver) && !canAttack()) || MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) {
        if (MR::sendMsgPush(pReceiver, pSender) && isPushMovable()) {
            TVec3f sensorDirection;
            MR::calcSensorDirectionNormalize(&sensorDirection, pReceiver, pSender);
            if (mVelocity.dot(sensorDirection) < 0.0f) {
                mVelocity.orthogonalize(sensorDirection);
            }
        }
    }
}

bool TakoHei::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemyAttack(pReceiver)) {
        if ((MR::isSensorPlayer(pSender) && !canAttack()) || MR::isSensorEnemy(pSender)) {
            if (isPushMovable()) {
                MR::startBtp(this, "BlinkTwice");
                TVec3f sensorDirection;
                MR::calcSensorDirectionNormalize(&sensorDirection, pSender, pReceiver);
                mVelocity += sensorDirection * (MR::isSensorPlayer(pSender) ? ::sPushMarioAccel : ::sPushAccel);
                mScaleController->startAndAddScaleVelocityY(MR::isSensorPlayer(pSender) ? ::sPushScaleYMarioSpeed : ::sPushScaleYSpeed);
                mNoPushStep = MR::isSensorPlayer(pSender) ? ::sNoPushMarioTime : ::sNoPushTime;
            }
            return true;
        }
    }

    return false;
}

bool TakoHei::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pReceiver)) {
        return false;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        return requestSwoon();
    }

    if (MR::isMsgInvincibleAttack(msg)) {
        return requestPunch(msg, pReceiver, pSender);
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        if (requestHipDropDown()) {
            ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
            return true;
        }
        return false;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        if (requestFlatDown()) {
            ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
            return true;
        }
        return false;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        return requestPunch(msg, pReceiver, pSender);
    }

    return false;
}

bool TakoHei::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pReceiver)) {
        return false;
    }

    if (MR::isMsgToEnemyAttackTrample(msg)) {
        return requestFlatDown();
    }

    if (MR::isMsgToEnemyAttackBlow(msg)) {
        return requestPunch(msg, pReceiver, pSender);
    }

    return false;
}

bool TakoHei::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerKick(msg) && isEnableKick() && requestPunch(msg, pReceiver, pSender)) {
        return true;
    }

    if (MR::isMsgYoshiStartTongue(msg)) {
        setNerve(&NrvTakoHei::TakoHeiNrvDown::sInstance);
        return true;
    }

    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }

    if (MR::isMsgYoshiReleaseEaten(msg)) {
        if (MR::isValidSwitchDead(this)) {
            MR::onSwitchDead(this);
        }
        makeActorDead();
        return true;
    }

    if (MR::isMsgYoshiCancelEaten(msg)) {
        if (MR::isValidSwitchDead(this)) {
            MR::onSwitchDead(this);
        }
        makeActorDead();
        return true;
    }

    return false;
}

bool TakoHei::requestPressDown() {
    if (!isDown()) {
        MR::clearHitSensors(this);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvPressDown));
        return true;
    }

    return false;
}

bool TakoHei::requestHipDropDown() {
    if (!isDown()) {
        MR::clearHitSensors(this);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvHipDropDown));
        return true;
    }

    return false;
}

bool TakoHei::requestFlatDown() {
    if (!isDown()) {
        MR::clearHitSensors(this);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvFlatDown));
        return true;
    }

    return false;
}

bool TakoHei::requestSwoon() {
    if (!isDown()) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvSwoon));
        return true;
    }

    return false;
}

bool TakoHei::requestPunch(u32 msg, HitSensor* pReceiver, HitSensor* pSender) {
    if (!isDown()) {
        MR::clearHitSensors(this);
        MR::invalidateHitSensors(this);
        MR::addVelocitySeparateHV(this, pSender, pReceiver, ::sPunchHorizonPower, ::sPunchVerticalPower);
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvPunchDown));
        ScoreAttackFunction::scoreByAttack(this, msg, pSender);
        return true;
    }

    return false;
}

bool TakoHei::requestAttackSuccess(HitSensor* pSender, HitSensor* pReceiver) {
    if (canAttack()) {
        MR::addVelocityMoveToDirection(this, (*MR::getSensorPos(pSender)) - (*MR::getSensorPos(pReceiver)), ::sAttackSuccessHitPower);
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvAttackSuccess));
        return true;
    }

    return false;
}

bool TakoHei::tryActive() {
    if (MR::isNearPlayerAnyTime(this, ::sActiveDistance)) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvWait));
        return true;
    }

    return false;
}

bool TakoHei::tryNonActive() {
    bool isNotNearPlayer = MR::isNearPlayerAnyTime(this, ::sActiveDistance) ? false : true;
    if (isNotNearPlayer && MR::isBindedGround(this)) {
        MR::zeroVelocity(this);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvNonActive));
        return true;
    }

    return false;
}

bool TakoHei::tryWalk() {
    if (MR::isGreaterStep(this, ::sWaitTime)) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvWalk));
        return true;
    }

    return false;
}

bool TakoHei::tryWalkEnd() {
    TVec3f scaleAdd;
    scaleAdd.killElement(mWalkTarget - mPosition, mGravity);

    if (MR::isGreaterStep(this, ::sWalkEndTime) || scaleAdd.squared() < ::sWalkTerritoryRadius) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvWait));
        return true;
    }

    if (isFallNextMove()) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvWait));
        return true;
    }

    return false;
}

bool TakoHei::tryFindTurn() {
    if (isInSightMario()) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvFindTurn));
        return true;
    }

    return false;
}

bool TakoHei::tryFind() {
    if (MR::isGreaterStep(this, ::sFindTurnTime)) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvFind));
        return true;
    }

    return false;
}

bool TakoHei::tryPursue() {
    if (MR::isGreaterStep(this, ::sFindTime)) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvPursue));
        return true;
    }

    return false;
}

bool TakoHei::tryPursueEnd() {
    if (MR::isGreaterStep(this, ::sPursueTime)) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvCoolDown));
        return true;
    }

    if (isFallNextMove()) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvWait));
        return true;
    }

    return false;
}

bool TakoHei::tryAttack() {
    f32 distanceToPlayer = MR::calcDistanceToPlayer(this);
    if (::sAttackDistanceNear <= distanceToPlayer && (distanceToPlayer <= ::sAttackDistanceFar || MR::isGreaterStep(this, ::sForceFarAttackTime))) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvAttackSign));
        return true;
    }

    return false;
}

bool TakoHei::tryCoolDownEnd() {
    if (MR::isGreaterStep(this, ::sCoolDownTime)) {
        if (isInSightMario()) {
            setNerve(GET_NERVE(TakoHei, TakoHeiNrvPursue));
        } else {
            setNerve((GET_NERVE(TakoHei, TakoHeiNrvWait)));
        }
        return true;
    }

    return false;
}

bool TakoHei::tryAttackSuccessEnd() {
    if (MR::isGreaterStep(this, ::sAttackSuccessTime)) {
        setNerve((GET_NERVE(TakoHei, TakoHeiNrvWait)));
        return true;
    }

    return false;
}

bool TakoHei::tryPointBind() {
    if (mBindStarPointer->tryStartPointBind()) {
        setNerve((GET_NERVE(TakoHei, TakoHeiNrvBindStarPointer)));
        return true;
    }

    return false;
}

bool TakoHei::tryRecover() {
    if (MR::isGreaterStep(this, ::sSwoonTime)) {
        setNerve((GET_NERVE(TakoHei, TakoHeiNrvRecover)));
        return true;
    }

    return false;
}

bool TakoHei::tryRecoverEnd() {
    if (MR::isGreaterStep(this, ::sRecoverTime)) {
        setNerve((GET_NERVE(TakoHei, TakoHeiNrvWait)));
        return true;
    }

    return false;
}

bool TakoHei::tryPressed() {
    if (MR::isPressedRoofAndGround(this) && requestPressDown()) {
        MR::zeroVelocity(this);
        MR::offBind(this);
        return true;
    }

    return false;
}

void TakoHei::exeNonActive() {
    if (MR::isFirstStep(this)) {
        MR::startBtp(this, "Open");
        updatePose();
        MR::offBind(this);
        MR::offCalcShadow(this, nullptr);
        MR::offCalcGravity(this);
    }

    MR::zeroVelocity(this);

    if (tryActive() || tryPointBind()) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateHitSensors(this);
    }
}

void TakoHei::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startBtp(this, "Open");
    }

    if (mNoPushStep == 1) {
        MR::startBtp(this, "Open");
    }

    if (getNerveStep() % 60 == 59) {
        MR::startBtp(this, "Blink");
    }

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (tryPointBind() || tryFindTurn() || tryWalk() || tryNonActive()) {
        // needed to get unused comparison to generate
        return;
    }
}

void TakoHei::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
        MR::startBtp(this, "Open");
        decideNextTargetPos();
    }

    if (mNoPushStep == 1) {
        MR::startBtp(this, "Open");
    }

    if (getNerveStep() % 60 == 59) {
        MR::startBtp(this, "Blink");
    }

    MR::turnDirectionToTargetDegree(this, &mFront, mWalkTarget, ::sWalkTurnDegree);

    if (MR::isGreaterStep(this, ::sWalkMoveStartTime)) {
        MR::addVelocityMoveToTarget(this, mWalkTarget, ::sWalkAccel);
    }

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (tryPointBind() || tryFindTurn() || tryWalkEnd()) {
        return;
    }
}

void TakoHei::exeFindTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
        MR::startBtp(this, "Open");
        MR::startActionSound(this, "EmTakoheiFind");
    }

    if (mNoPushStep == 1) {
        MR::startBtp(this, "Open");
    }

    MR::turnDirectionToTargetDegree(this, &mFront, *MR::getPlayerPos(), ::sFindTurnDegree);

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (tryPointBind() || tryFind()) {
        return;
    }
}

void TakoHei::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Search");
        MR::startBtp(this, "Search");
    }

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (tryPointBind() || tryPursue()) {
        return;
    }
}

void TakoHei::exePursue() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
        MR::startBtp(this, "Blink");
    }

    if (mNoPushStep == 1) {
        MR::startBtp(this, "Blink");
    }

    bool isFarPlayer = false;

    if (MR::isNearPlayer(this, 700.0f)) {
        MR::addVelocityAwayFromTargetOnGround(this, *MR::getPlayerPos(), ::sPursueAccel);
    } else {
        MR::addVelocityMoveToTarget(this, *MR::getPlayerPos(), ::sPursueAccel);
        isFarPlayer = true;
    }

    MR::turnDirectionToTargetDegree(this, &mFront, *MR::getPlayerPos(), ::sPursueTurnDegree);

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (isFarPlayer) {
        if (checkFront()) {
            setNerve(GET_NERVE(TakoHei, TakoHeiNrvAttackSign));
            return;
        }
    }

    if (tryAttack() || tryPointBind() || tryPursueEnd()) {
        return;
    }
}

void TakoHei::exeCoolDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "CoolDown");
        MR::startBtp(this, "Cry");
    }

    if (mNoPushStep == 1) {
        MR::startBtp(this, "Cry");
    }

    MR::turnDirectionToTargetDegree(this, &mFront, *MR::getPlayerPos(), 0.0f);

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (tryPointBind() || tryCoolDownEnd()) {
        return;
    }
}

void TakoHei::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotStart");
        MR::startBtp(this, "Angry");
    }

    MR::startActionSound(this, "EmTakoheiLvPreSpitOut");

    MR::turnDirectionToTargetDegree(this, &mFront, *MR::getPlayerPos(), ::sAttackSignDegree);

    if (checkFront()) {
        MR::addVelocity(this, mFront * ::sBackAttackSpaceAccel);
    }

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (tryPointBind()) {
        return;
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvAttack));
    }
}

void TakoHei::exeAttack() {
    if (MR::isFirstStep(this)) {
        TPos3f mouthMtx = MR::getJointMtx(this, "Mouth");
        TVec3f mult;
        mouthMtx.mult(::sInkShotOffset, mult);
        MR::spurtTakoHeiInk(mult, mFront * ::sSpurtPowerH + mGravity * ::sSpurtPowerV);

        MR::startAction(this, "Shot");
        MR::startActionSound(this, "EmTakoheiSpitOut");
    }

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (tryPointBind()) {
        return;
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(TakoHei, TakoHeiNrvWait));
    }
}

void TakoHei::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::startBtp(this, "Angry");
    }

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (tryPointBind() || tryAttackSuccessEnd()) {
        return;
    }
}

void TakoHei::exeBindStarPointer() {
    MR::updateActorStateAndNextNerve(this, GET_NERVE(TakoHei, TakoHeiNrvWait));
}

void TakoHei::exeSwoon() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StunStart");
        MR::startBtp(this, "Cry");
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Stun");
    }

    if (MR::isBckPlaying(this, "Stun")) {
        MR::startActionSound(this, "EmLvSwoonS");
    }

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (MR::isGreaterStep(this, 60) && tryPointBind()) {
        return;
    }

    if (tryRecover()) {
        return;
    }
}

void TakoHei::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StunEnd");
        MR::startBtp(this, "BlinkTwice");
        MR::startActionSound(this, "EvTakoheiRecover");
    }

    updateNormalVelocity();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (tryRecoverEnd()) {
        return;
    }
}

void TakoHei::exePressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HipDropDown");
        MR::startBtp(this, "Cry");
        MR::startActionSound(this, "EmTakoheiStompedL");
        MR::zeroVelocity(this);
    }

    if (MR::isGreaterStep(this, ::sPressDownTime)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void TakoHei::exeHipDropDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HipDropDown");
        MR::startBtp(this, "Cry");
        MR::startActionSound(this, "EmTakoheiStompedL");
        MR::zeroVelocity(this);
    }

    if (MR::isGreaterStep(this, ::sHipDropDownTime)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void TakoHei::exeFlatDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FlatDown");
        MR::startBtp(this, "Cry");
        MR::startActionSound(this, "EmTakoheiStomped");
        MR::zeroVelocity(this);
    }

    if (MR::isGreaterStep(this, ::sFlatDownTime)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void TakoHei::exePunchDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PunchDown");
        MR::startBtp(this, "Cry");
        MR::turnDirectionToTarget(this, &mFront, *MR::getPlayerPos(), -1.0f);
        MR::stopScene(::sPunchStopSceneTime);
        MR::startActionSound(this, "EmTakoheiHitpunch");
        MR::startBlowHitSound(this);
    }

    mAirStep = 10;
    updateNormalVelocity();

    if (MR::isGreaterStep(this, ::sPunchDownTime) || (MR::isBinded(this) && MR::isGreaterStep(this, 5))) {
        MR::emitItemHit(this);
        kill();
    }
}

void TakoHei::updatePose() {
    MR::blendQuatUpFront(&mPoseQuat, MR::isBindedGround(this) ? MR::getGroundNormal(this) : -mGravity, mFront, 0.1f, 0.2f);
}

void TakoHei::updateNormalVelocity() {
    if (!MR::isBindedGround(this)) MR::addVelocityToGravity(this, 1.0f);
    MR::attenuateVelocity(this, mAirStep >= 5 ? ::sNormalAirFrec : ::sNormalGroundFrec);
    MR::reboundVelocityFromEachCollision(this, -1.0f, 0.0f, 0.0f, 0.0f);
}

void TakoHei::decideNextTargetPos() {
    MR::getRandomVector(&mWalkTarget, ::sWalkGoalDistance);

    mWalkTarget.orthogonalize(mGravity);

    mWalkTarget += mHomePos;
}

bool TakoHei::checkFront() const {
    TVec3f positionUpOffset;
    MR::calcPositionUpOffset(&positionUpOffset, this, ::sCheckFrontHeight);
    return MR::isExistMapCollision(positionUpOffset, mFront * ::sCheckFrontAttackSpace);
}

bool TakoHei::canAttack() const {
    if (isNerve(GET_NERVE(TakoHei, TakoHeiNrvWait)) || isNerve(GET_NERVE(TakoHei, TakoHeiNrvWalk)) ||
        isNerve(GET_NERVE(TakoHei, TakoHeiNrvFindTurn)) || isNerve(GET_NERVE(TakoHei, TakoHeiNrvFind)) ||
        isNerve(GET_NERVE(TakoHei, TakoHeiNrvPursue)) || isNerve(GET_NERVE(TakoHei, TakoHeiNrvCoolDown))) {
        return true;
    } else {
        return false;
    }
}

bool TakoHei::isPushMovable() const {
    if (mNoPushStep > 0) {
        return false;
    }

    if (isDown()) {
        return false;
    }

    if (isNerve(GET_NERVE(TakoHei, TakoHeiNrvNonActive)) || isNerve(GET_NERVE(TakoHei, TakoHeiNrvAttackSuccess)) ||
        isNerve(GET_NERVE(TakoHei, TakoHeiNrvBindStarPointer)) || isNerve(GET_NERVE(TakoHei, TakoHeiNrvSwoon)) || isNerve(&NrvTakoHei::TakoHeiNrvDown::sInstance)) {
        return false;
    } else {
        return true;
    }
}

bool TakoHei::isEnableKick() const {
    if (isNerve(GET_NERVE(TakoHei, TakoHeiNrvSwoon))) {
        return true;
    } else {
        return false;
    }
}

bool TakoHei::isDown() const {
    if (isNerve(GET_NERVE(TakoHei, TakoHeiNrvHipDropDown)) || isNerve(GET_NERVE(TakoHei, TakoHeiNrvFlatDown)) ||
        isNerve(GET_NERVE(TakoHei, TakoHeiNrvPressDown)) || isNerve(GET_NERVE(TakoHei, TakoHeiNrvPunchDown))) {
        return true;
    } else {
        return false;
    }
}

bool TakoHei::isFallNextMove() const {
    if (mHasMoveLimitCollision) {
        return false;
    }

    if (MR::isOnGround(this)) {
        return MR::isFallNextMove(mPosition, mVelocity, mGravity, 150.0f, mScale.x * 90.0f * 2.0f, 150.0f, nullptr);
    } else {
        return false;
    }
}

bool TakoHei::isInSightMario() const {
    TVec3f toPlayer(*MR::getPlayerPos() - mPosition);
    f32 scalarToPlayer;
    MR::separateScalarAndDirection(&scalarToPlayer, &toPlayer, toPlayer);

    TVec3f quatZDir;

    mPoseQuat.getZDir(quatZDir);

    if (scalarToPlayer < ::sFindDistance && toPlayer.dot(quatZDir) > 0.0f) {
        return true;
    } else {
        return false;
    }
}

TakoHei::~TakoHei(){};
