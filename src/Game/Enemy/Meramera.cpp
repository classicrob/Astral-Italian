#include "Game/Enemy/Meramera.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ParabolicPath.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

enum { ElementType_Fire, ElementType_Ice };
enum { EffectType_Null, EffectType_Wait, EffectType_Chase, EffectType_Escape };
enum { BodyEffectType_Heat = 0x1, BodyEffectType_CoolDown = 0x2, BodyEffectType_Cold = 0x3 };
enum { Status_Runaway, Status_Float };

namespace {
    const char* sElementArcName[] = {"Meramera", "IceMeramera", "ChildMeramera", "ChildIceMeramera"};
    s32 sElementList[] = {ElementType_Fire, ElementType_Ice, ElementType_Fire, ElementType_Ice};

    static const f32 sBodyHitRadius = 55.0f;
    static const f32 sBreakHitRadius = 10.0f;
    static const f32 sAttackHitRadius = 70.0f;
    static const f32 sBindRadius = 45.0f;
    static const f32 sShadowRadius = 50.0f;
    static const f32 sBaseScale = 150.0f;
    static f32 sUpVecBlendRate;
    static f32 sFrontVecBlendRate;
    static f32 sPushAccel;
    static const f32 sRecoveryDistance = 1500.0f;
    static const f32 sOverWallAccel = 0.9f;
    static const f32 sMaxOverWallHeight = 250.0f;
    static const s32 sWaitTime = 300;
    static f32 sNormalAccel;
    static f32 sGravityAccel;
    static f32 sFloatAccel;
    static f32 sAirFreq;
    static f32 sTurnLimit;
    static f32 sNormalHeight;
    static const s32 sWalkTime = 120;
    static f32 sTerritoryWalkRange;
    static const s32 sAttackSuccessTime = 180;
    static const f32 sTerritoryChaseRange = 3000.0f;
    static const s32 sChaseEndTime = 240;
    static f32 sChaseStartTime;
    static f32 sChaseStartFloatAccel;
    static f32 sChaseStartHeight;
    static f32 sChaseDashTime;
    static f32 sChaseDashFloatStartTime;
    static const f32 sChaseDashAccel = 0.98f;
    static f32 sChaseDashFreq;
    static f32 sChaseDashTurnLimit;
    static f32 sChaseDashFloatAccel;
    static f32 sChaseDashHeight;
    static f32 sChaseDashSecondFloatAccel;
    static f32 sChaseDashSecondHeight;
    static f32 sChaseTurnTime;
    static const f32 sChaseTurnAccel = 0.97f;
    static f32 sChaseTurnFreq;
    static f32 sChaseTurnTurnLimit;
    static const f32 sChaseTurnGravityAccel = 0.99f;
    static const f32 sChaseTurnFloatAccel = 0.05f;
    static f32 sChaseTurnHeight;
    static f32 sSinkTime;
    static const s32 sFloatTime = 30;
    static f32 sFloatUpPower;
    static f32 sFloatGravityAccel;
    static f32 sFloatFloatAccel;
    static f32 sFloatHeight;
    static f32 sFloatFloatRange;
    static f32 sFloatFreq;
    static const s32 sIgnitionTime = 20;
    static f32 sIgnitionUpDelayTime;
    static f32 sIgnitionUpPower;
    static f32 sIgnitionGravityAccel;
    static f32 sIgnitionFloatAccel;
    static f32 sIgnitionHeight;
    static f32 sIgnitionFloatRange;
    static f32 sIgnitionFreq;
    static const s32 sRunawayTime = 30;
    static f32 sRunawayStartJumpTime;
    static f32 sRunawayAnimFrameRate;
    static f32 sRunawayJumpStartVelocity;
    static f32 sRunawayJumpStartTime;
    static f32 sRunawayJumpStartTimeRange;
    static f32 sRunawayDirectionRandomRate;
    static f32 sRunawayAccel;
    static f32 sRunawayMinJumpPower;
    static f32 sRunawayMaxJumpPower;
    static f32 sRunawayGravityAccel;
    static f32 sRunawayGroundFreq;
    static f32 sRunawayAirFreq;
    static f32 sRunawayEmitLandEffectSpeed;
    static f32 sRunawayGroundReboundRate;
    static f32 sRunawayWallReboundRate;
    static f32 sRunawayGroundReboundStop;
    static f32 sRunawayFromPlayerDistance;
    static f32 sCheckDegree;
    static f32 sDivingMinPowerDis;
    static f32 sDivingMaxPowerDis;
    static f32 sCheckDivingFrontLength;
    static f32 sCheckDivingMoreFrontLength;
    static f32 sCheckDivingSideLength;
    static f32 sCheckDivingUpLength;
    static f32 sCheckDivingLength;
    static f32 sDivingHeight;
    static const s32 sDivingDelayTime = 35;
    static const s32 sDivingTime = 40;
    static f32 sDamageTime;
    static f32 sDelayExtinguishTime;
    static f32 sDamageRange;
    static f32 sDamageBlowPower;
    static f32 sDamageFloatAccel;
    static f32 sDamageHeight;
    static f32 sDamageAirFreq;
    static const s32 sDownStopFrame = 2;
    static f32 sDownScale;
    static f32 sDownUpPower;
    static const f32 sDownGravityAccel = 0.5f;
    static f32 sDownFreq;
    static f32 sDownReboundRate;
    static f32 sMinExtinguishDistance;
    static f32 sExtinguishSpeedRate;
    static f32 sTurnSmokeToGravityDegree;
};  // namespace


namespace NrvMeramera {
    NEW_NERVE(MerameraNrvWait, Meramera, Wait);
    NEW_NERVE(MerameraNrvWalk, Meramera, Walk);
    NEW_NERVE(MerameraNrvChaseStart, Meramera, ChaseStart);
    NEW_NERVE(MerameraNrvChaseDash, Meramera, ChaseDash);
    NEW_NERVE(MerameraNrvChaseTurn, Meramera, ChaseTurn);
    NEW_NERVE(MerameraNrvDamage, Meramera, Damage);
    NEW_NERVE_ONEND(MerameraNrvRunaway, Meramera, Runaway, Runaway);
    NEW_NERVE_ONEND(MerameraNrvStartDiving, Meramera, StartDiving, StartDiving);
    NEW_NERVE(MerameraNrvDiving, Meramera, Diving);
    NEW_NERVE(MerameraNrvSink, Meramera, Sink);
    NEW_NERVE(MerameraNrvFloat, Meramera, Float);
    NEW_NERVE(MerameraNrvIgnition, Meramera, Ignition);
    NEW_NERVE(MerameraNrvIgnitionForce, Meramera, IgnitionForce);
    NEW_NERVE(MerameraNrvAttackSuccess, Meramera, AttackSuccess);
    NEW_NERVE_ONEND(MerameraNrvBindStarPointer, Meramera, BindStarPointer, BindStarPointer);
    NEW_NERVE_ONEND(MerameraNrvBindStarPointerNoFire, Meramera, BindStarPointerNoFire, BindStarPointerNoFire);
    NEW_NERVE(MerameraNrvDown, Meramera, Down);
    NEW_NERVE(MerameraNrvFlatDown, Meramera, FlatDown);
    NEW_NERVE(MerameraNrvReadyRestart, Meramera, ReadyRestart);
};  // namespace NrvMeramera

f32 lerp(f32 start, f32 end, f32 t) {
    return start + (end - start) * t;
}

inline bool Meramera::isSensor(const HitSensor* pSensor, const char* pName) const {
    return getSensor(pName) == pSensor;
}

inline bool Meramera::isActionDisabled() const {
    return isNerve(GET_NERVE(Meramera, MerameraNrvRunaway)) || isNerve(GET_NERVE(Meramera, MerameraNrvStartDiving)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvDiving)) || isNerve(GET_NERVE(Meramera, MerameraNrvBindStarPointerNoFire));
}

inline TVec3f Meramera::getDistanceToPlayer() const {
    TVec3f playerPos(*MR::getPlayerPos());
    playerPos -= mPosition;
    return playerPos;
}

Meramera::Meramera(const char* pName)
    : LiveActor(pName), mAnimScaleController(), mWalkerStateBindStarPointer(), mPoseQuat(0.0f, 1.0f), mFront(0, 0, 1), mUp(0, 1, 0), mHomePosition(0, 0, 0),
      mChaseAccelDir(0, 0, 1), mWanderTarget(0, 0, 0), mJumpTarget(0, 0, 0), mJumpDir(0, 0, 1), mJumpUp(0, 1, 0), mTargetDistance(-1.0f), mChaseDistance(900.0f), mAppearStatus(-1),
      mElementType(), mAppearTimer(), mRunawayTimer(), mCanDive(), mIsValidRestart(), mIsBindFire(true) {
    mEffectMtx.identity();
    mExtinguishMtx.identity();
    mFallEffectMtx.identity();

    mParabolicPath = new ParabolicPath();
}

// NONMATCHING: initStarPointerTarget arg setup only differs in float register allocation (f0/f1/f2 order)
void Meramera::init(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        mElementType = initElementType(rIter);
    }

    MR::initActor(this, rIter, ::sElementArcName[mElementType], false);
    initEffect();
    f32 pointerRadius = 55.0f * mScale.x;
    TVec3f starPointerOffset(0.0f, 0.0f, 0.0f);
    MR::initStarPointerTarget(this, pointerRadius, starPointerOffset);
    mHomePosition = mPosition;
    mWalkerStateBindStarPointer = new WalkerStateBindStarPointer(this, mAnimScaleController);
    initNerve(GET_NERVE(Meramera, MerameraNrvWait), 2);
    MR::initActorState(this, mWalkerStateBindStarPointer, GET_NERVE(Meramera, MerameraNrvBindStarPointer), "SupportFreeze");
    MR::initActorState(this, mWalkerStateBindStarPointer, GET_NERVE(Meramera, MerameraNrvBindStarPointerNoFire), "SupportFreezeNoFire");
    MR::initUseStageSwitchWriteDead(this, rIter);
    mAnimScaleController = new AnimScaleController(nullptr);
    mAnimScaleController->setParamTight();
    initAppearState(rIter);
}

void Meramera::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

s32 Meramera::initElementType(const JMapInfoIter& rIter) {
    for (s32 idx = 0; idx < ARRAY_SIZE(::sElementArcName); idx++) {
        if (MR::isEqualObjectName(rIter, ::sElementArcName[idx])) {
            return ::sElementList[idx];
        }
    }

    return 0;
}

void Meramera::initEffect() {
    MR::setEffectHostMtx(this, "ExtinguishSide", mExtinguishMtx);
    MR::setEffectHostMtx(this, "ExtinguishSideLight", mExtinguishMtx);
    MR::setEffectHostMtx(this, "Wait", mEffectMtx);
    MR::setEffectHostMtx(this, "Escape", mEffectMtx);
    MR::setEffectHostMtx(this, "ExtinguishSmoke", mEffectMtx);
    MR::setEffectHostMtx(this, "ChaseStart", mEffectMtx);
    MR::setEffectHostMtx(this, "Chase", mEffectMtx);
    MR::setEffectHostMtx(this, "Attack", mEffectMtx);
    MR::setEffectHostMtx(this, "Fall", mFallEffectMtx);
}

void Meramera::initAppearState(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg0WithInit(rIter, &mAppearStatus);
    }

    resetAppear();

    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg2NoInit(rIter, &mIsValidRestart);
    }

    MR::useStageSwitchSyncAppear(this, rIter);
}

void Meramera::makeActorAppeared() {
    LiveActor::makeActorAppeared();
}

void Meramera::kill() {
    MR::emitEffect(this, "Break");

    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    if (mIsValidRestart) {
        MR::invalidateClipping(this);
        MR::hideModel(this);
        MR::deleteEffectAll(this);

        setNerve(GET_NERVE(Meramera, MerameraNrvReadyRestart));
    } else {
        LiveActor::kill();
    }
}


void Meramera::endClipped() {
    LiveActor::endClipped();

    setNerve(GET_NERVE(Meramera, MerameraNrvWait));
}

void Meramera::control() {
    mAnimScaleController->updateNerve();
    updatePose();

    TVec3f up = -mGravity;
    TVec3f sinPos;
    MR::turnVecToVecDegree(&mUp, mUp, up, 1.0f, mFront);

    MR::copyJointPos(this, "Sin", &sinPos);
    MR::makeMtxUpNoSupportPos(&mEffectMtx, mUp, sinPos);

    tryPointBind();

    if (mAppearTimer > 0) {
        mAppearTimer--;
    }
}

void Meramera::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPoseQuat);
    MR::updateBaseScale(this, mAnimScaleController);
}

void Meramera::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (mAppearTimer > 0) {
        return;
    }

    if (isSensor(pSender, "Body") && (MR::isSensorEnemy(pReceiver) || MR::isSensorPlayer(pReceiver) || MR::isSensorYoshi(pReceiver))) {
        MR::sendMsgPush(pReceiver, pSender);
    }

    if (requestAttack(pSender, pReceiver)) {
        return;
    }
}

bool Meramera::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isSensor(pReceiver, "Body")) {
        return false;
    }

    if (MR::isSensorEnemy(pSender) || MR::isSensorPlayer(pSender) || MR::isSensorYoshi(pSender)) {
        if (isPushMoved()) {
            TVec3f dir;
            MR::calcSensorDirectionNormalize(&dir, pSender, pReceiver);
            MR::vecScaleAdd(&mVelocity, &dir, 0.5f);
        }

        return true;
    }

    return false;
}

bool Meramera::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mAppearTimer > 0) {
        return false;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return isAffectedSpin();
    }

    if (MR::isMsgStarPieceReflect(msg) && isEnableStarPieceReflect()) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg) && requestForceRunaway()) {
        return true;
    }

    if (MR::isMsgFireBallAttack(msg)) {
        return requestFire(pReceiver, pSender);
    }

    if (MR::isMsgSnowBallAttack(msg)) {
        return requestDownForce(pReceiver, pSender);
    }

    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        return requestFlatDown(pReceiver, pSender);
    }

    if (MR::isMsgSupportTicoSpin(msg)) {
        if (isActionDisabled()) {
            return requestDownForce(pReceiver, pSender);
        }

        return requestDamage(pReceiver, pSender);
    }

    if (MR::isMsgPlayerHitProjectiles(msg)) {
        return requestDownForce(pReceiver, pSender);
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        return requestDown(pReceiver, pSender);
    }

    return false;
}

bool Meramera::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mAppearTimer > 0) {
        return false;
    }

    if (MR::isMsgExplosionAttack(msg)) {
        return requestDownForce(pReceiver, pSender);
    }

    if (MR::isMsgToEnemyAttackBlow(msg)) {
        return requestDownForce(pReceiver, pSender);
    }

    return false;
}

bool Meramera::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgSpinStormRange(msg)) {
        if (mAppearTimer > 0) {
            return false;
        }

        return requestDamage(pReceiver, pSender);
    }

    if (MR::isMsgPlayerKick(msg)) {
        return requestDown(pReceiver, pSender);
    }

    if (MR::isMsgStartPowerStarGet(msg)) {
        makeActorDead();

        return true;
    }

    return false;
}

bool Meramera::requestDamage(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isSensor(pSender, "Body")) {
        return false;
    }

    if (!isAffectedSpin()) {
        return false;
    }

    bool isFar = MR::isNear(pSender, pReceiver, 300.0f) == false;
    if (isFar) {
        turnFireDirectionToSpin(5.0f);

        return false;
    }

    if (MR::isSensorPlayer(pReceiver) && !MR::isPlayerSwingAction()) {
        return false;
    }

    MR::setVelocitySeparateHV(this, pReceiver, pSender, 35.0f, 0.0f);

    setNerve(GET_NERVE(Meramera, MerameraNrvDamage));

    return true;
}

bool Meramera::requestFire(HitSensor* pSender, HitSensor* pReceiver) {
    switch (mElementType) {
    case ElementType_Fire:
        if (isEnableFireball()) {
            setNerve(GET_NERVE(Meramera, MerameraNrvIgnitionForce));
            return true;
        }
        break;

    case ElementType_Ice:
        if (requestForceRunaway()) {
            return true;
        }

        if (requestDownForce(pSender, pReceiver)) {
            return true;
        }
        break;
    }

    return false;
}

bool Meramera::requestForceRunaway() {
    if (!isAffectedSpin()) {
        return false;
    }

    MR::startAction(this, "Damage");

    setNerve(GET_NERVE(Meramera, MerameraNrvRunaway));

    return true;
}

bool Meramera::requestDownForce(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Meramera, MerameraNrvDown))) {
        return false;
    }

    MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, "HitMark");

    TVec3f horizon;
    MR::calcSensorHorizonNormalize(&horizon, mGravity, pSender, pReceiver);

    if (!MR::isNearZero(horizon)) {
        mFront.set(horizon);
    }

    MR::zeroVelocity(this);

    setNerve(GET_NERVE(Meramera, MerameraNrvDown));

    return true;
}

bool Meramera::requestDown(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayer(pReceiver)) {
        return false;
    }

    if (isActionDisabled()) {
        return requestDownForce(pSender, pReceiver);
    }

    return false;
}

bool Meramera::requestAttack(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isSensor(pSender, "attack") || !isEnableAttack()) {
        return false;
    }

    if (MR::isSensorPlayer(pReceiver)) {
        if (sendMsgElementAttack(pReceiver, pSender)) {
            setNerve(GET_NERVE(Meramera, MerameraNrvAttackSuccess));
            return true;
        }
    } else {
        sendMsgElementAttack(pReceiver, pSender);
    }
    return false;
}

bool Meramera::requestFlatDown(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isSensor(pSender, "Body")) {
        return false;
    }

    if (isActionDisabled()) {
        MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, "HitMark");

        setNerve(GET_NERVE(Meramera, MerameraNrvFlatDown));

        return true;
    }

    return false;
}

bool Meramera::sendMsgElementAttack(HitSensor* pSender, HitSensor* pReceiver) {
    switch (mElementType) {
    case ElementType_Fire:
        return MR::sendMsgEnemyAttackFire(pSender, pReceiver);

    case ElementType_Ice:
        return MR::sendMsgEnemyAttackFreeze(pSender, pReceiver);

    default:
        return false;
    }
}

void Meramera::appearAtPos(const TVec3f& rPos, const TVec3f&) {
    if (MR::isDead(this)) {
        mHomePosition.set(rPos);
        resetAppear();
        appear();
    }
}

void Meramera::initPlayerResource(const JMapInfoIter& rIter) {
    if (initElementType(rIter) == ElementType_Ice) {
        MR::enablePlayerResourceFreezeIce();
    }
}

bool Meramera::tryWalk() {
    if (getNerveStep() > ::sWalkTime) {
        TVec3f randomVec;
        MR::getRandomVector(&randomVec, 1.0f);
        MR::normalizeOrZero(&randomVec);
        randomVec.killElement(randomVec, mGravity);
        randomVec *= 200.0f;
        mWanderTarget = randomVec + mHomePosition;

        setNerve(GET_NERVE(Meramera, MerameraNrvWalk));

        return true;
    }

    return false;
}

bool Meramera::tryWalkEnd() {
    if (getNerveStep() > ::sWaitTime || (0.0f <= mTargetDistance && mTargetDistance <= 100.0f)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvWait));

        return true;
    }

    return false;
}

bool Meramera::tryChase() {
    if (isEnableChase()) {
        setNerve(GET_NERVE(Meramera, MerameraNrvChaseStart));
        return true;
    }

    return false;
}

bool Meramera::tryEndChase() {
    bool isFarFromPlayer = MR::isNearPlayer(this, mChaseDistance) == false;
    if (isFarFromPlayer || MR::isGreaterStep(this, ::sChaseEndTime) || mHomePosition.distance(mPosition) > ::sTerritoryChaseRange) {
        setNerve(GET_NERVE(Meramera, MerameraNrvWait));
        return true;
    }

    return false;
}

bool Meramera::tryStartDiving() {
    if (mCanDive && MR::isBindedGround(this)) {
        MR::zeroVelocity(this);

        setNerve(GET_NERVE(Meramera, MerameraNrvStartDiving));

        return true;
    }

    return false;
}

bool Meramera::tryForceSink() {
    if (isHitRecover()) {
        mJumpTarget.set(mPosition);
        mJumpUp.set(-mGravity);

        MR::zeroVelocity(this);

        setNerve(GET_NERVE(Meramera, MerameraNrvSink));
        return true;
    }

    return false;
}

bool Meramera::tryRecovery() {
    f32 scale = mScale.x;
    bool isThisFarFromPlayer = MR::isNearPlayer(this, ::sRecoveryDistance) == false;
    if (isThisFarFromPlayer) {
        bool isHomeFarFromPlayer = MR::isNearPlayer(mHomePosition, ::sRecoveryDistance) == false;
        if (isHomeFarFromPlayer && MR::isJudgedToClipFrustum(mPosition, scale * 100.0f) && MR::isJudgedToClipFrustum(mHomePosition, scale * 100.0f)) {
            resetAppear();

            return true;
        }
    }

    return false;
}

bool Meramera::tryPointBind() {
    if (mAppearTimer > 0) {
        return false;
    }

    if ((isNerve(GET_NERVE(Meramera, MerameraNrvWait)) || isNerve(GET_NERVE(Meramera, MerameraNrvWalk)) ||
         isNerve(GET_NERVE(Meramera, MerameraNrvChaseStart)) || isNerve(GET_NERVE(Meramera, MerameraNrvChaseDash)) ||
         isNerve(GET_NERVE(Meramera, MerameraNrvChaseTurn)) || isNerve(GET_NERVE(Meramera, MerameraNrvIgnition)) ||
         isNerve(GET_NERVE(Meramera, MerameraNrvIgnitionForce)) || isNerve(GET_NERVE(Meramera, MerameraNrvAttackSuccess))) &&
        mWalkerStateBindStarPointer->tryStartPointBind()) {
        mIsBindFire = true;
        setNerve(GET_NERVE(Meramera, MerameraNrvBindStarPointer));
        return true;
    }

    if ((isNerve(GET_NERVE(Meramera, MerameraNrvRunaway)) || isNerve(GET_NERVE(Meramera, MerameraNrvStartDiving))) &&
        mWalkerStateBindStarPointer->tryStartPointBind()) {
        mIsBindFire = false;
        setNerve(GET_NERVE(Meramera, MerameraNrvBindStarPointerNoFire));
        return true;
    }

    return false;
}

void Meramera::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
    }

    startWaitLevelSound();

    MR::addVelocityKeepHeightUseShadow(this, 130.0f, 0.5f, 0.1f, 15.0f, nullptr);
    MR::attenuateVelocity(this, 0.98f);
    MR::reboundVelocityFromCollision(this);

    if (!tryChase() && tryWalk()) {
        return;
    }
}

void Meramera::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
    }

    startWaitLevelSound();

    MR::addVelocityKeepHeightUseShadow(this, 130.0f, 0.5f, 0.1f, 15.0f, nullptr);
    addToTargetMovingAccel(mWanderTarget, 0.05f, 0.995f);
    MR::attenuateVelocity(this, 0.98f);

    MR::reboundVelocityFromCollision(this);

    if (!tryChase() && tryWalkEnd()) {
        return;
    }
}

void Meramera::exeChaseStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ChaseStart");
        MR::startActionSound(this, "EmMerameraFind");

        if (mElementType == ElementType_Ice) {
            MR::startActionSound(this, "EmIcemeraPowerUp");
        } else {
            MR::startActionSound(this, "EmMerameraFirepowerUp");
        }

    }

    startWaitLevelSound();

    MR::addVelocityKeepHeightUseShadow(this, 300.0f, 0.5f, 0.2f, 15.0f, nullptr);
    addToTargetMovingAccel(*MR::getPlayerPos(), 0.05f, 0.99f);
    MR::attenuateVelocity(this, 0.97f);
    MR::reboundVelocityFromCollision(this);

    if (!tryEndChase() && MR::isGreaterStep(this, 60)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvChaseDash));
    }
}

void Meramera::exeChaseDash() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ChaseDash");

        MR::startActionSound(this, "EmMerameraDash");

        mPlayerPosition = *MR::getPlayerPos();
        mChaseAccelDir = mPlayerPosition - mPosition;

    }

    startChaseLevelSound();

    if (MR::isLessStep(this, 30)) {
        MR::addVelocityKeepHeightUseShadow(this, 150.0f, 0.5f, 0.1f, 15.0f, nullptr);
    } else {
        MR::addVelocityKeepHeightUseShadow(this, 190.0f, 0.5f, 0.2f, 20.0f, nullptr);
    }

    addMovingAccel(mChaseAccelDir, 0.175f, ::sChaseDashAccel);
    addOverWallAccel(mChaseAccelDir);
    MR::attenuateVelocity(this, ::sChaseDashAccel);
    MR::reboundVelocityFromCollision(this);

    if (MR::isGreaterStep(this, 45)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvChaseTurn));
    }
}

void Meramera::exeChaseTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ChaseTurn");

    }

    startChaseLevelSound();

    MR::addVelocityKeepHeightUseShadow(this, 190.0f, 0.3f, 0.3f, 20.0f, nullptr);
    addToTargetMovingAccel(*MR::getPlayerPos(), ::sChaseTurnFloatAccel, ::sChaseTurnGravityAccel);
    MR::attenuateVelocity(this, ::sChaseTurnAccel);
    MR::reboundVelocityFromCollision(this);

    if (!tryEndChase() && MR::isGreaterStep(this, 50)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvChaseDash));
    }
}

void Meramera::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::turnDirectionToPlayer(this, &mFront, -1.0f);

        if (mElementType == ElementType_Ice) {
            MR::startActionSound(this, "EmIcemeraBlowCold");
        } else {
            MR::startActionSound(this, "EmMerameraBlowFire");
        }

    }

    if (MR::isStep(this, 3)) {
        TVec3f sinPos;
        MR::copyJointPos(this, "Sin", &sinPos);
        mExtinguishMtx.makeTrans(sinPos);

        MR::startAction(this, "Damage");

        MR::startActionSound(this, "EmMerameraSmoke");
    }

    if (MR::isGreaterEqualStep(this, 3)) {
        updateExtinguishMtx();
    }

    MR::addVelocityKeepHeightUseShadow(this, 140.0f, 0.0f, 1.2f, 15.0f, nullptr);
    MR::attenuateVelocity(this, 0.9f);
    MR::reboundVelocityFromCollision(this);

    if (MR::isGreaterStep(this, 30)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvRunaway));
    }
}

void Meramera::exeRunaway() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Escape");

        mCanDive = false;
        mRunawayTimer = 0;
    }

    if (tryForceSink()) {
        return;
    }

    f32 dot = MR::calcGravitySpeed(this);

    if (MR::isBindedGround(this) && dot > 12.0f) {
        MR::emitEffect(this, "EscapeRock");
        MR::startActionSound(this, "EmMerameraBound");
    }

    if (tryStartDiving()) {
        return;
    }

    if (MR::isGreaterStep(this, 70)) {
        if (MR::isBindedGround(this)) {
            f32 scalar = MR::calcHorizontalSpeed(this);

            if (scalar < 3.0f) {
                mRunawayTimer--;
            }

            if (mRunawayTimer <= 0) {
                addRunawayJumpPower();

                MR::emitEffect(this, "EscapeRock");

                MR::startActionSound(this, "EmMerameraJump");

                mRunawayTimer = MR::getRandom(0L, 10L) + 30;
            }
        }

        if (!mCanDive) {
            f32 val = (getNerveStep() % 5 * 80.0f) / 5.0f;
            if (getNerveStep() % 2 == 1) {
                val = -val;
            }
            checkDivingPoint(250.0f, val);
        }
    }

    if (!MR::isBindedGround(this)) {
        MR::addVelocityToGravity(this, 2.0f);
    }

    MR::attenuateVelocity(this, MR::isBindedGround(this) ? 0.8f : 0.98f);
    MR::reboundVelocityFromEachCollision(this, 0.5f, 0.3f, 0.0f, 10.0f);

    if (tryRecovery()) {
        return;
    }
}

void Meramera::exeStartDiving() {
    if (MR::isFirstStep(this)) {

        MR::setBckRate(this, 0.0f);
    }

    if (MR::isLessStep(this, 35)) {
        checkDivingPointMore(1.0f / 0.35f);
    }

    if (MR::isGreaterStep(this, 35)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvDiving));
    }
}

void Meramera::exeDiving() {
    if (MR::isFirstStep(this)) {
        mParabolicPath->initFromUpVector(mPosition, mJumpTarget, mJumpUp, 300.0f);


        MR::turnDirectionToTarget(this, &mFront, mJumpTarget, -1.0f);

        MR::setBckRate(this, 1.4f);
        MR::emitEffect(this, "EscapeRock");
        MR::startActionSound(this, "EmMerameraJump");
    }

    TVec3f pos;
    mParabolicPath->calcPosition(&pos, MR::calcNerveRate(this, 40));
    const TVec3f& vel = pos - mPosition;
    MR::setVelocity(this, vel);

    MR::turnVecToVecDegree(&mUp, mUp, -mGravity, 180.0f, mFront);

    if (!tryForceSink() && MR::isGreaterStep(this, 40)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvRunaway));
    }
}

void Meramera::exeSink() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Fall");
        MR::onCalcAnim(this);
        MR::startActionSound(this, "EmMerameraLavaIn");
        MR::makeMtxUpNoSupportPos(&mFallEffectMtx, -mGravity, mPosition);
    }

    MR::turnDirectionToPlayer(this, &mFront, -1.0f);

    if (MR::isGreaterStep(this, 50)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvFloat));
    }
}

void Meramera::exeFloat() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Float");
        MR::startActionSound(this, "EmMerameraLavaOut");
        MR::setVelocity(this, mJumpUp * 50.0f);
    }

    MR::addVelocityKeepHeightUseShadow(this, 100.0f, 0.9f, 0.1f, 50.0f, nullptr);
    MR::attenuateVelocity(this, 0.94f);
    MR::reboundVelocityFromCollision(this);

    MR::turnDirectionToPlayer(this, &mFront, -1.0f);

    if (MR::isGreaterStep(this, 30)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvIgnition));
    }
}

void Meramera::exeIgnition() {
    if (MR::isStep(this, 15)) {
        MR::startAction(this, "FloatIgnition");

        if (mElementType == ElementType_Ice) {
            MR::startActionSound(this, "EmIcemeraIgnition");
        } else {
            MR::startActionSound(this, "EmMerameraIgnition");
        }

        MR::addVelocity(this, mJumpUp * 25.0f);
    }

    MR::addVelocityKeepHeightUseShadow(this, 100.0f, 0.5f, 0.1f, 50.0f, nullptr);
    MR::attenuateVelocity(this, 0.98f);
    MR::reboundVelocityFromCollision(this);

    if (MR::isGreaterStep(this, ::sIgnitionTime)) {
        if (isEnableChase()) {
            setNerve(GET_NERVE(Meramera, MerameraNrvChaseDash));
        } else {
            setNerve(GET_NERVE(Meramera, MerameraNrvWait));
        }
    }
}

void Meramera::exeIgnitionForce() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FloatIgnition");

        MR::setVelocityJump(this, 15.0f);

    }

    MR::addVelocityKeepHeightUseShadow(this, 130.0f, 0.5f, 0.1f, 15.0f, nullptr);
    MR::attenuateVelocity(this, 0.98f);
    MR::reboundVelocityFromCollision(this);

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvWait));
    }
}

void Meramera::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");

        MR::zeroVelocity(this);

    }

    MR::addVelocityKeepHeightUseShadow(this, 50.0f, 0.5f, 0.1f, 15.0f, nullptr);
    MR::attenuateVelocity(this, 0.98f);
    MR::reboundVelocityFromCollision(this);

    if (MR::isGreaterStep(this, ::sAttackSuccessTime)) {
        setNerve(GET_NERVE(Meramera, MerameraNrvWait));
    }
}

void Meramera::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
        MR::stopScene(::sDownStopFrame);
        MR::startActionSound(this, "EmMerameraStomped");
        MR::zeroVelocity(this);
        MR::addVelocityMoveToDirection3D(this, mFront, -0.0f);
        MR::addVelocityToGravity(this, -25.0f);
    }

    if (!MR::isBindedGround(this)) {
        MR::addVelocityToGravity(this, ::sDownGravityAccel);
    }

    MR::attenuateVelocity(this, 0.99f);
    MR::reboundVelocityFromEachCollision(this, 0.9f, 0.9f, 0.9f, 0.0f);

    if (MR::isBinded(this) || MR::isGreaterStep(this, 0)) {
        MR::startActionSound(this, "EmMerameraExplode");
        MR::emitItemHit(this);
        kill();
    }
}

void Meramera::exeFlatDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FlatDown");
        MR::stopScene(::sDownStopFrame);
        MR::startActionSound(this, "EmMerameraStomped");
        MR::zeroVelocity(this);
    }

    if (MR::isGreaterStep(this, 0)) {
        MR::startActionSound(this, "EmExplodeS");
        MR::emitItemStomp(this);
        kill();
    }
}

bool Meramera::isAffectedSpin() const {
    return isNerve(GET_NERVE(Meramera, MerameraNrvWait)) || isNerve(GET_NERVE(Meramera, MerameraNrvAttackSuccess)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvWalk)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvChaseStart)) || isNerve(GET_NERVE(Meramera, MerameraNrvChaseDash)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvChaseTurn)) || isNerve(GET_NERVE(Meramera, MerameraNrvIgnition)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvIgnitionForce)) || isNerve(GET_NERVE(Meramera, MerameraNrvBindStarPointer));
}

bool Meramera::isEnableStarPieceReflect() const {
    return isNerve(GET_NERVE(Meramera, MerameraNrvDamage)) || isNerve(GET_NERVE(Meramera, MerameraNrvRunaway)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvStartDiving)) || isNerve(GET_NERVE(Meramera, MerameraNrvDiving)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvBindStarPointerNoFire));
}

bool Meramera::isEnableAttack() const {
    return isNerve(GET_NERVE(Meramera, MerameraNrvWait)) || isNerve(GET_NERVE(Meramera, MerameraNrvWalk)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvChaseDash)) || isNerve(GET_NERVE(Meramera, MerameraNrvChaseTurn)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvBindStarPointer));
}

bool Meramera::isPushMoved() const {
    return isNerve(GET_NERVE(Meramera, MerameraNrvWait)) || isNerve(GET_NERVE(Meramera, MerameraNrvWalk)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvDamage)) || isNerve(GET_NERVE(Meramera, MerameraNrvChaseDash)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvChaseTurn)) || isNerve(GET_NERVE(Meramera, MerameraNrvAttackSuccess));
}

bool Meramera::isEnableFireball() const {
    return isNerve(GET_NERVE(Meramera, MerameraNrvWait)) || isNerve(GET_NERVE(Meramera, MerameraNrvWalk)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvChaseStart)) || isNerve(GET_NERVE(Meramera, MerameraNrvChaseDash)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvChaseTurn)) || isNerve(GET_NERVE(Meramera, MerameraNrvDamage)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvRunaway)) || isNerve(GET_NERVE(Meramera, MerameraNrvStartDiving)) ||
           isNerve(GET_NERVE(Meramera, MerameraNrvAttackSuccess));
}

bool Meramera::isEnableChase() const {
    return MR::isNearPlayer(this, mChaseDistance) && mHomePosition.distance(mPosition) <= 3000.0f;
}

void Meramera::addToTargetMovingAccel(const TVec3f& rVec, f32 velocityAccel, f32 gravityAccel) {
    addMovingAccel(rVec - mPosition, velocityAccel, gravityAccel);
}

void Meramera::addMovingAccel(const TVec3f& rVec, f32 velocityAccel, f32 gravityAccel) {
    TVec3f vec(mGravity);
    TVec3f vec2;
    vec2.killElement(rVec, vec);

    MR::separateScalarAndDirection(&mTargetDistance, &vec2, vec2);

    if (!MR::isNearZero(vec2)) {
        if (-1.0f < gravityAccel && gravityAccel < 1.0f) {
            MR::turnVecToVecCos(&mFront, mFront, vec2, gravityAccel, vec);
        } else {
            mFront = vec2;
        }
    }

    MR::addVelocityMoveToDirection3D(this, mFront, velocityAccel);
}

void Meramera::addRunawayJumpPower() {
    TVec3f vec;
    f32 val = 1.0f;
    if (mCanDive) {
        vec.set(mJumpTarget - mPosition);

        TVec3f vec2;
        vec2.killElement(vec, mGravity);

        val = MR::normalize(vec2.length(), 10.0f, 500.0f);
    } else if (MR::isNearPlayer(this, 1000.0f)) {
        vec.set(mPosition - *MR::getPlayerPos());
    } else {
        vec.set(mHomePosition - mPosition);
    }

    MR::normalizeOrZero(&vec);
    MR::addRandomVector(&vec, vec, 0.5f * val);

    f32 val2 = 20.0f * (0.1f + val * 0.9f);
    f32 randomVal = MR::getRandom(45.0f, 50.0f);
    val = (0.2f + val * 0.8f) * randomVal;
    addMovingAccel(vec, val2, -1.0f);
    MR::addVelocityToGravity(this, -val);
}

void Meramera::addOverWallAccel(const TVec3f& rVec) {
    const TVec3f& gravity = mGravity;

    TVec3f vec;
    vec.killElement(rVec, gravity);
    MR::normalizeOrZero(&vec);

    if (MR::getFirstPolyOnLineToMap(nullptr, nullptr, mPosition, vec * 200.0f) && MR::getShadowNearProjectionLength(this) < ::sMaxOverWallHeight) {
        MR::addVelocityToGravity(this, -::sOverWallAccel);
    }
}

void Meramera::updatePose() {
    if (isActionDisabled()) {
        MR::rotateQuatRollBall(&mPoseQuat, mVelocity, -mGravity, 50.0f);
    } else {
        MR::blendQuatUpFront(&mPoseQuat, -mGravity, mFront, 0.25f, 0.25f);
    }
}

void Meramera::updateExtinguishMtx() {
    turnFireDirectionToSpin(10.0f);

    TVec3f trans;
    mExtinguishMtx.getTrans(trans);

    TVec3f pullVelocity;
    MR::calcPlayerSpinPullVelocity(&pullVelocity, trans);

    trans += pullVelocity * 0.9f;

    TVec3f vec48;
    vec48.set(*MR::getPlayerPos() - trans);
    f32 length = vec48.length();

    if (0.0f < length && length < 100.0f) {
        trans = *MR::getPlayerPos() + vec48 * (100.0f / length);
    }

    if (MR::isNearZero(pullVelocity)) {
        mExtinguishMtx.identity();
        mExtinguishMtx.setTrans(trans);
    } else {
        MR::makeMtxUpNoSupportPos(&mExtinguishMtx, pullVelocity, trans);
    }
}

bool Meramera::checkDirectDivingPoint() {
    mCanDive = isHitRecover();

    if (mCanDive) {
        mJumpTarget.set(mPosition);
        mJumpUp.set(-mGravity);

        mCanDive = true;
    }

    return mCanDive;
}

bool Meramera::checkDivingPoint(f32 f1, f32 accel) {
    mCanDive = false;

    if (checkDirectDivingPoint()) {
        return true;
    }

    TVec3f vec3C(mPosition);
    TVec3f vec48(mFront * f1);
    MR::rotateVecDegree(&vec48, mGravity, accel);
    vec3C += vec48;
    vec3C -= mGravity * 100.0f;

    TVec3f vec2(mGravity * 600.0f + vec48);
    mCanDive = findDivingPoint(vec3C, vec2);

    return mCanDive;
}

bool Meramera::checkDivingPointMore(f32 f1) {
    TVec3f vec30(mJumpTarget);
    MR::vecScaleAdd(&vec30, &mJumpDir, f1);
    MR::vecScaleAdd(&vec30, &mGravity, -100.0f);

    TVec3f vec2(0.0f);
    MR::vecScaleAdd(&vec30, &mJumpDir, f1);
    MR::vecScaleAdd(&vec30, &mGravity, 600.0f);

    return findDivingPoint(vec30, vec2);
}

bool Meramera::isHitRecover() const {
    switch (mElementType) {
    case ElementType_Fire:
        if (MR::isBindedGroundDamageFire(this)) {
            return true;
        }
        break;

    case ElementType_Ice:
        if (MR::isBindedGroundWater(this) || MR::isInWater(mPosition)) {
            return true;
        }
        break;
    }

    return false;
}

bool Meramera::findDivingPoint(TVec3f vec, const TVec3f& rVec) {
    // FIXME: there is some inline here
    // https://decomp.me/scratch/uajQ5
    Triangle triangle = Triangle();
    TVec3f vec84;

    bool myBool = false;

    switch (mElementType) {
    case ElementType_Fire:
        if (!MR::isExistMapCollision(mPosition, vec - mPosition) && MR::getFirstPolyOnLineToMap(&vec84, &triangle, vec, rVec) &&
            MR::isGroundCodeDamageFire(&triangle)) {
            myBool = true;
        }
        break;

    case ElementType_Ice:
        return false;
    }

    if(myBool) {
        mJumpTarget.set(vec84);
        mJumpUp.set(-mGravity);
        mJumpDir.killElement2(vec84 - mPosition, mJumpUp);
        MR::normalizeOrZero(&mJumpDir);
    }

    return myBool;
}

void Meramera::resetAppear() {
    MR::setPosition(this, mHomePosition);
    MR::zeroVelocity(this);
    MR::calcGravity(this);

    switch (mAppearStatus) {
    case Status_Runaway:
        setNerve(GET_NERVE(Meramera, MerameraNrvRunaway));
        break;

    case Status_Float:
        MR::hideModel(this);
        MR::offBind(this);

        MR::makeMtxUpNoSupportPos(&mFallEffectMtx, -mGravity, mPosition);

        mJumpUp.set(-mGravity);
        mJumpTarget.set(mPosition);

        setNerve(GET_NERVE(Meramera, MerameraNrvFloat));
        break;

    default:
        setNerve(GET_NERVE(Meramera, MerameraNrvWait));
        break;
    }

    mAppearTimer = 40;
    MR::resetPosition(this);
}

void Meramera::turnFireDirectionToSpin(f32 f1) {
    TVec3f jointPos;
    MR::copyJointPos(this, "Sin", &jointPos);

    TVec3f pullVelocity;
    MR::calcPlayerSpinPullVelocity(&pullVelocity, jointPos);

    if (MR::isNearZero(pullVelocity)) {
        return;
    }

    MR::normalize(pullVelocity, &pullVelocity);
    MR::turnVecToVecDegree(&mUp, mUp, pullVelocity, f1, mFront);
}

void Meramera::startWaitLevelSound() {
    if (mElementType == ElementType_Ice) {
        MR::startActionSound(this, "EmLvIcemeraWait");
        return;
    }

    MR::startActionSound(this, "EmLvMerameraWait");
}

void Meramera::startChaseLevelSound() {
    if (mElementType == ElementType_Ice) {
        MR::startActionSound(this, "EmLvIcemeraChase");
        return;
    }

    MR::startActionSound(this, "EmLvMerameraChase");
}

void Meramera::endRunaway() {
    MR::invalidateHitSensor(this, "break");
}

void Meramera::endStartDiving() {
    MR::invalidateHitSensor(this, "break");
}

void Meramera::exeBindStarPointer() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    MR::updateActorStateAndNextNerve(this, GET_NERVE(Meramera, MerameraNrvWait));
}

void Meramera::endBindStarPointer() {
    mWalkerStateBindStarPointer->kill();
}

void Meramera::exeBindStarPointerNoFire() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Escape");
    }
    MR::updateActorStateAndNextNerve(this, GET_NERVE(Meramera, MerameraNrvRunaway));
}

void Meramera::endBindStarPointerNoFire() {
    mWalkerStateBindStarPointer->kill();
}

void Meramera::exeReadyRestart() {
    if (!MR::isGreaterStep(this, 180)) {
        return;
    }

    MR::showModel(this);
    resetAppear();
}
