#include "Game/Enemy/BasaBasa.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/AnimStampController.hpp"
#include "Game/Enemy/SpinHitController.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <revolution/mtx.h>

namespace NrvBasaBasa {
    NEW_NERVE(BasaBasaNrvWait, BasaBasa, Wait);
    NEW_NERVE(BasaBasaNrvAirWait, BasaBasa, AirWait);
    NEW_NERVE(BasaBasaNrvChaseStart, BasaBasa, ChaseStart);
    NEW_NERVE(BasaBasaNrvChase, BasaBasa, Chase);
    NEW_NERVE(BasaBasaNrvQuickTurnStart, BasaBasa, QuickTurnStart);
    NEW_NERVE(BasaBasaNrvQuickTurn, BasaBasa, QuickTurn);
    NEW_NERVE(BasaBasaNrvPunchDown, BasaBasa, PunchDown);
    NEW_NERVE(BasaBasaNrvTrampleDown, BasaBasa, TrampleDown);
    NEW_NERVE(BasaBasaNrvAttackStart, BasaBasa, AttackStart);
    NEW_NERVE(BasaBasaNrvAttack, BasaBasa, Attack);
    NEW_NERVE(BasaBasaNrvAttackEnd, BasaBasa, AttackEnd);
    NEW_NERVE(BasaBasaNrvAttackEndRecover, BasaBasa, AttackEndRecover);
    NEW_NERVE(BasaBasaNrvHitBack, BasaBasa, HitBack);
    NEW_NERVE(BasaBasaNrvHitBackEnd, BasaBasa, HitBackEnd);
    NEW_NERVE(BasaBasaNrvComeHome, BasaBasa, ComeHome);
    NEW_NERVE(BasaBasaNrvAttachCelling, BasaBasa, AttachCelling);
    NEW_NERVE_ONEND(BasaBasaNrvDPDSwoon, BasaBasa, DPDSwoon, DPDSwoon);
    NEW_NERVE(BasaBasaNrvStun, BasaBasa, Stun);
};  // namespace NrvBasaBasa

BasaBasa::BasaBasa(const char* pName)
    : LiveActor(pName), mHangModel(nullptr), mScaleController(nullptr), mStampController(nullptr), mSpinHitController(nullptr), mFront(0.0f, 0.0f, 1.0f),
      mChaseRange(2000.0f), mAttackDistance(0.0f), mTargetPos(0), mWobbleDegree(0.0f), mHoverTarget(0.0f, 0.0f, 0.0f), mIsIceModel(false), mHomePos(0.0f, 0.0f, 0.0f), mHomeRange(-1.0f), mReturnStep(0), mHasGroundBelow(0) {
}

// NONMATCHING: pointing-offset local and constructor temporaries occupy different stack slots.
void BasaBasa::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    const char* objName;
    MR::getObjectName(&objName, rIter);
    mIsIceModel = MR::isEqualString(objName, "BasaBasaIce");
    MR::initActor(this, rIter, objName, false);
    MR::connectToSceneEnemy(this);
    initHitSensor(2);
    MR::addHitSensorAtJointEnemy(this, "Body", "JointRoot", 8, 90.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointEnemyAttack(this, "attack", "JointRoot", 8, 60.0f, TVec3f(0.0f, 0.0f, 0.0f));
    initBinder(110.0f, 0.0f, 0);
    MR::initLightCtrl(this);
    initEffectKeeper(1, nullptr, false);
    MR::addEffectHitNormal(this, nullptr);
    initSound(4, "BasaBasa", nullptr, TVec3f(0.0f));
    MR::initShadowVolumeSphere(this, 100.0f);
    MR::validateClipping(this);
    if (MR::isConnectedWithRail(rIter)) {
        initRailRider(rIter);
        mTargetPos = &MR::getRailPos(this);
    } else {
        mTargetPos = MR::getPlayerCenterPos();
    }

    MR::onCalcGravity(this);
    MR::initStarPointerTarget(this, 100.0f, TVec3f(0, 0, 0));
    mScaleController = new AnimScaleController(nullptr);
    mStampController = new AnimStampController();
    mSpinHitController = new SpinHitController(this, 20, 5, 0.0f, 20.0f, 10.0f);
    initHangModel();
    MR::getJMapInfoArg0NoInit(rIter, &mChaseRange);
    MR::getJMapInfoArg1NoInit(rIter, &mHomeRange);
    if (-1.0f == mHomeRange) {
        mHomeRange = 1000.0f + mChaseRange;
    }

    MR::calcFrontVec(&mFront, this);
    mAttackDistance = MR::getRandom(600.0f, 800.0f);
    mWobbleDegree = MR::getRandom(-25.0f, 25.0f);
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
}

void BasaBasa::preloadIceFreezeResources(const JMapInfoIter& rIter) {
    const char* name = nullptr;
    MR::getObjectName(&name, rIter);
    if (MR::isEqualString(name, "BasaBasaIce")) {
        MR::enablePlayerResourceFreezeIce();
    }
}

void BasaBasa::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        mHangModel->appear();
        mTargetPos = MR::getPlayerCenterPos();
        MR::validateClipping(this);
        MR::validateHitSensors(this);
        MR::offBind(this);
    }

    if (isNearTarget(mChaseRange)) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvChaseStart));
    }
}

void BasaBasa::exeAirWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Fly");
        if (MR::isExistRail(this)) {
            mTargetPos = &MR::getRailPos(this);
        } else {
            mTargetPos = MR::getPlayerCenterPos();
        }

        MR::validateClipping(this);
        MR::validateHitSensors(this);
    }

    tuneHeight();
    if (isNearTarget(mChaseRange)) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvChase));
    } else if (!tryComeHome()) {
        trySetNerveDPDSwoon();
    }
}

void BasaBasa::exeChaseStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FlyStart");
        MR::startActionSound(this, "EvBasabasaFind");
    }

    if (MR::isLessStep(this, 50)) {
        mVelocity.scaleAdd(15.0f, mGravity, mVelocity);
    }

    if (MR::isStep(this, 15)) {
        MR::onBind(this);
    }

    if (MR::isStep(this, 34)) {
        mHangModel->kill();
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvChase));
    }
}

void BasaBasa::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(this, "Fly");
        MR::onBind(this);
    }

    TVec3f v5;
    v5.sub(*mTargetPos, mPosition);
    v5.orthogonalize(mGravity);
    MR::normalizeOrZero(&v5);
    if (!MR::isNearZero(v5)) {
        TVec3f v4;
        v4 = v5;
        MR::rotateVecDegree(&v4, mGravity, mWobbleDegree);
        MR::turnDirectionDegree(this, &mFront, v4, 1.7f);
    }

    mVelocity.scaleAdd(2.0f, mFront, mVelocity);
    tuneHeight();
    if (!trySetNerveDPDSwoon()) {
        if (MR::isNearPlayer(mHomePos, mHomeRange) && MR::isNearPlayer(this, mAttackDistance)) {
            setNerve(GET_NERVE(BasaBasa, BasaBasaNrvAttackStart));
        } else if (!MR::isFaceToTargetHorizontalDegree(this, *mTargetPos, mFront, 60.0f)) {
            setNerve(GET_NERVE(BasaBasa, BasaBasaNrvQuickTurnStart));
        } else {
            if (tryComeHome()) {
                return;
            }
        }
    }
}

void BasaBasa::exeQuickTurnStart() {
    mVelocity.scaleAdd(2.0f, mFront, mVelocity);
    if (MR::isStep(this, 45)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvQuickTurn));
    } else {
        trySetNerveDPDSwoon();
    }
}

void BasaBasa::exeQuickTurn() {
    mVelocity.scaleAdd(2.0f, mFront, mVelocity);
    MR::turnDirectionToTargetDegree(this, &mFront, *mTargetPos, 2.5500001907348633f);
    if (MR::isFaceToTargetHorizontalDegree(this, *mTargetPos, mFront, 2.5500001907348633f)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvChase));
    } else {
        trySetNerveDPDSwoon();
    }
}

void BasaBasa::exeTrampleDown() {
    if (MR::isFirstStep(this)) {
        MR::stopBck(this);
        mVelocity.zero();
        mStampController->startAnim(mGravity);
        MR::invalidateHitSensors(this);
        MR::startActionSound(this, "EvBasabasaDamage");
        MR::startActionSound(this, "EmStompedS");
    }

    mStampController->updateNerve();

    if (mStampController->isAnimEnd()) {
        MR::emitItemStomp(this);
        kill();
    }
}

void BasaBasa::exePunchDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Blow");
        MR::invalidateClipping(this);
        MR::invalidateHitSensors(this);
        MR::startActionSound(this, "EvBasabasaDamage");
        MR::startBlowHitSound(this);
        MR::onBind(this);
    }

    if (mSpinHitController->execute(this)) {
        MR::emitItemHit(this);
    }
}

void BasaBasa::exeAttackStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackStart");
    }

    MR::turnDirectionToTargetDegree(this, &mFront, *mTargetPos, 2.5500001907348633f);
    TVec3f v3;
    v3.scaleAdd(2.0f, mFront, mGravity);
    v3.negate();
    MR::normalizeOrZero(&v3);
    if (MR::isNearZero(v3)) {
        v3.set(-mGravity);
    }

    mVelocity.scaleAdd(0.2f, v3, mVelocity);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvAttack));
    } else {
        trySetNerveDPDSwoon();
    }
}

void BasaBasa::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::startActionSound(this, "EvBasabasaAttack");
        TVec3f playerUp;
        MR::getPlayerUpVec(&playerUp);
        mHoverTarget.scaleAdd(100.0f, playerUp, *MR::getPlayerPos());
    }

    MR::startActionSound(this, "EmLvBasabasaAttack");
    TVec3f v9;
    v9.sub(mHoverTarget, mPosition);
    if (mIsIceModel) {
        TVec3f v8;
        MR::calcUpVec(&v8, this);
        TVec3f v7 = v9.killElement(v8);
        TVec3f v6;
        v6.scale(v8.dot(v9), v8);
        v9.scaleAdd(5.0f, v6, v7);
    }
    TVec3f v5;
    MR::normalize(v9, &v5);
    MR::turnDirectionToTargetDegree(this, &mFront, mHoverTarget, 2.5500001907348633f);
    mVelocity.scaleAdd(10.0f, v5, mVelocity);
    if (MR::isNear(this, mHoverTarget, 40.0f) || MR::isBindedWall(this) || MR::isStep(this, 180)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvAttackEnd));
    } else {
        trySetNerveDPDSwoon();
    }
}

void BasaBasa::exeAttackEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackEnd");
        if (MR::getShadowProjectionLength(this, nullptr) < 200.0f) {
            MR::emitEffect(this, "AttackEnd");
        }
    }

    mVelocity.scaleAdd(0.1f, mFront, mVelocity);
    if (MR::isStep(this, 50)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvAttackEndRecover));
    } else {
        trySetNerveDPDSwoon();
    }
}

void BasaBasa::exeAttackEndRecover() {
    if (MR::isFirstStep(this)) {
        mVelocity.zero();
    }

    TVec3f v3;
    v3.scaleAdd(-0.5f, mFront, mGravity);
    v3.negate();
    MR::normalize(&v3);
    if (MR::getBckFrame(this) >= 116.0f) {
        mVelocity.scaleAdd(0.3f, v3, mVelocity);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvChase));
    }

    trySetNerveDPDSwoon();
}

void BasaBasa::exeHitBack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Reaction");
        MR::onBind(this);
        MR::turnDirectionToPlayerDegree(this, &mFront, 180.0f);
        MR::startActionSound(this, "EmIcebasaSpinBlow");
    }

    if (MR::isLessEqualStep(this, 5)) {
        TVec3f v3;
        v3.add(mFront, mGravity);
        v3.negate();
        MR::normalizeOrZero(&v3);
        if (MR::isNearZero(v3)) {
            v3.set(-mGravity);
        }
        mVelocity.scaleAdd(15.0f, v3, mVelocity);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvHitBackEnd));
    } else {
        trySetNerveDPDSwoon();
    }
}

void BasaBasa::exeHitBackEnd() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(this, "Fly");
    }

    if (MR::isStep(this, 60)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvChase));
    } else {
        trySetNerveDPDSwoon();
    }
}

void BasaBasa::exeComeHome() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(this, "Fly");
        mTargetPos = &mHomePos;
    }

    MR::turnDirectionToTargetDegree(this, &mFront, *mTargetPos, 2.5500001907348633f);
    mVelocity.scaleAdd(2.0f, mFront, mVelocity);
    tuneHeight();
    if (isNearTarget(300.0f)) {
        if (mHasGroundBelow) {
            setNerve(GET_NERVE(BasaBasa, BasaBasaNrvAttachCelling));
            return;
        } else if (MR::isExistRail(this)) {
            setNerve(GET_NERVE(BasaBasa, BasaBasaNrvAirWait));
            return;
        }
    }

    if (MR::isGreaterEqualStep(this, 120) && MR::isNearPlayer(this, mChaseRange) && MR::isNearPlayer(mHomePos, mHomeRange)) {
        mTargetPos = MR::getPlayerCenterPos();
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvChase));
    } else {
        trySetNerveDPDSwoon();
    }
}

void BasaBasa::exeAttachCelling() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Return");
        MR::offBind(this);
        MR::invalidateHitSensors(this);
    }

    MR::turnDirectionDegree(this, &mFront, mHomeFront, 2.5500001907348633f);
    if (MR::isNear(this, mHomePos, 10.0f)) {
        mVelocity.zero();
        MR::resetPosition(this, mHomePos);
    } else {
        TVec3f v2;
        v2.sub(mHomePos, mPosition);
        mVelocity.scale(5.0f, v2);
    }

    if (MR::isStep(this, 42)) {
        mHangModel->appear();
    }

    if (MR::isActionEnd(this)) {
        mVelocity.zero();
        MR::validateHitSensors(this);
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvWait));
    }
}

void BasaBasa::exeDPDSwoon() {
    if (MR::isFirstStep(this)) {
        MR::stopBck(this);
        MR::emitEffect(this, "Touch");
        mScaleController->startDpdHitVibration();
        MR::startDPDHitSound();
    }

    MR::startDPDFreezeLevelSound(this);

    if (!MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvAirWait));
    }
}

void BasaBasa::endDPDSwoon() {
    MR::deleteEffect(this, "Touch");
    mScaleController->stopAndReset();
}

void BasaBasa::exeStun() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Swoon");
        MR::startActionSound(this, "EvBasabasaDamage");
    }

    mVelocity.scaleAdd(1.0f, mGravity, mVelocity);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvChase));
    }
}

void BasaBasa::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
    TVec3f v4;
    v4.scaleAdd(20.0f, mGravity, mPosition);
    TVec3f v3;
    v3.scale(-30.0f, mGravity);
    mHasGroundBelow = MR::getFirstPolyOnLineToMap(&mPosition, nullptr, v4, v3);
    if (mHasGroundBelow) {
        initNerve(GET_NERVE(BasaBasa, BasaBasaNrvWait));
    } else {
        initNerve(GET_NERVE(BasaBasa, BasaBasaNrvAirWait));
    }

    mHomePos.set(mPosition);
    MR::calcFrontVec(&mHomeFront, this);
}

void BasaBasa::kill() {
    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmExplodeS");
    mHangModel->kill();
    LiveActor::kill();
}

void BasaBasa::control() {
    if (mIsIceModel) {
        MR::startActionSound(this, "EmLvIcebasaChill", -1, -1, -1);
    }
    mScaleController->updateNerve();
    if (!tryClippingAndResetPos()) {
        controlVelocity();
        updateRailType();
    }
}

void BasaBasa::calcAndSetBaseMtx() {
    TPos3f v11;
    MR::calcMtxFromGravityAndZAxis(&v11, this, mGravity, mFront);
    TPos3f v10;
    v10 = mStampController->mScaleMtx;
    v11.concat(v10, v11);
    v11.mMtx[0][3] = mPosition.x;
    v11.mMtx[1][3] = mPosition.y;
    v11.mMtx[2][3] = mPosition.z;
    MR::setBaseTRMtx(this, v11);
    MR::updateBaseScale(this, mScaleController);
}

void BasaBasa::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    bool v6 = isNerve(GET_NERVE(BasaBasa, BasaBasaNrvTrampleDown)) || isNerve(GET_NERVE(BasaBasa, BasaBasaNrvPunchDown));

    if (v6) {
        return;
    }

    if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvHitBack))) {
        return;
    }

    if (MR::isSensorEnemyAttack(pSender) && MR::isSensorPlayerOrYoshiOrRide(pReceiver)) {
        bool v7 = false;
        if (!MR::isPlayerHipDropFalling()) {
            if (mIsIceModel) {
                v7 = MR::sendMsgEnemyAttackFreeze(pReceiver, pSender);
            } else if (!isNerve(GET_NERVE(BasaBasa, BasaBasaNrvDPDSwoon))) {
                bool v8;
                if (MR::isPlayerSwimming()) {
                    v8 = MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
                } else {
                    v8 = MR::sendMsgEnemyAttack(pReceiver, pSender);
                }

                v7 = v8;
            }
        }

        if (v7) {
            MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
            setNerve(GET_NERVE(BasaBasa, BasaBasaNrvHitBack));
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else if (MR::isSensorEnemy(pSender)) {
        if (MR::isSensorEnemy(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
            TVec3f v10;
            v10.sub(mPosition, MR::getSensorHost(pReceiver)->mPosition);
            MR::normalizeOrZero(&v10);
            if (mVelocity.dot(v10) < 0.0f) {
                const TVec3f& vel = mVelocity;
                mVelocity.scaleAdd(-v10.dot(vel), v10, vel);
            }
        }
    }
}

bool BasaBasa::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        if (isNrvEnableStun()) {
            setNerve(GET_NERVE(BasaBasa, BasaBasaNrvStun));
        }

        return true;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    bool v9 = isNerve(GET_NERVE(BasaBasa, BasaBasaNrvTrampleDown)) || isNerve(GET_NERVE(BasaBasa, BasaBasaNrvPunchDown));

    if (v9) {
        return false;
    }

    if (mIsIceModel && !MR::isPlayerElementModeIce()) {
        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
            return false;
        } else if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
            MR::sendMsgEnemyAttackFreeze(pSender, pReceiver);
            return true;
        }
    }

    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        MR::tryRumbleDefaultHit(this, 0);
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvTrampleDown));
        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        mSpinHitController->start(this, (*MR::getSensorPos(pSender)), (*MR::getSensorPos(pReceiver)));
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvPunchDown));
        return true;
    }

    return false;
}

bool BasaBasa::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvDPDSwoon))) {
        return false;
    }

    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }

    if (MR::isSensorEnemy(pSender) || MR::isSensorMapObj(pSender)) {
        TVec3f v7;
        v7.sub(mPosition, MR::getSensorHost(pSender)->mPosition);
        MR::normalizeOrZero(&v7);
        mVelocity.scaleAdd(1.5f, v7, mVelocity);
        return true;
    }

    return false;
}

bool BasaBasa::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }

    if (MR::isMsgToEnemyAttackBlow(msg)) {
        mSpinHitController->startWithoutStopScene(this, (*MR::getSensorPos(pSender)), (*MR::getSensorPos(pReceiver)));
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvPunchDown));
        return true;
    }

    if (MR::isMsgToEnemyAttackTrample(msg)) {
        mSpinHitController->startWithoutStopScene(this, (*MR::getSensorPos(pSender)), (*MR::getSensorPos(pReceiver)));
        setNerve(GET_NERVE(BasaBasa, BasaBasaNrvPunchDown));
        return true;
    }

    return false;
}

bool BasaBasa::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!mIsIceModel) {
        return false;
    }

    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }

    if (MR::isMsgSpinStormRange(msg) && MR::isPlayerElementModeNormal()) {
        if (500.0f < MR::calcDistance(pSender, pReceiver, nullptr)) {
            return false;
        } else {
            setNerve(GET_NERVE(BasaBasa, BasaBasaNrvHitBack));
            return false;
        }
    }

    return false;
}

void BasaBasa::initHangModel() {
    const char* modelName = mIsIceModel ? "BasaBasaIceHang" : "BasaBasaHang";
    mHangModel = MR::createModelObjEnemy("ぶら下がりモデル", modelName, MR::getJointMtx(this, "JointRoot"));
    mHangModel->initWithoutIter();
    mHangModel->kill();
}

bool BasaBasa::tryClippingAndResetPos() {
    if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvAttachCelling))) {
        return false;
    }

    if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvChaseStart))) {
        return false;
    }

    f32 radius = MR::getSensorRadius(this, "Body");
    if (!MR::isJudgedToClipFrustum(mPosition, radius) || MR::isNearPlayer(this, 2500.0f)) {
        mReturnStep = 0;
        return false;
    } else {
        if (mReturnStep < 180) {
            mReturnStep++;
            return false;
        } else {
            MR::resetPosition(this, mHomePos);
            mFront.set(mHomeFront);
            mVelocity.zero();
            mReturnStep = 0;
            if (mHasGroundBelow != 0) {
                setNerve(GET_NERVE(BasaBasa, BasaBasaNrvWait));
            } else {
                setNerve(GET_NERVE(BasaBasa, BasaBasaNrvAirWait));
            }
            return true;
        }
    }
}

bool BasaBasa::trySetNerveDPDSwoon() {
    if (!MR::isStarPointerPointing2PAttachTarget(this)) {
        return false;
    }

    setNerve(GET_NERVE(BasaBasa, BasaBasaNrvDPDSwoon));
    return true;
}

bool BasaBasa::tryComeHome() {
    if (MR::isNear(this, mHomePos, mHomeRange) && isNearTarget(mChaseRange)) {
        return false;
    }

    setNerve(GET_NERVE(BasaBasa, BasaBasaNrvComeHome));
    return true;
}

void BasaBasa::updateRailType() {
    if (MR::isExistRail(this) && mTargetPos == &MR::getRailPos(this)) {
        MR::moveCoord(this, 1.0f);
        if (MR::isNearPlayer(mHomePos, mChaseRange)) {
            mTargetPos = MR::getPlayerPos();
        }
    }
}

void BasaBasa::controlVelocity() {
    bool v1 = isNerve(GET_NERVE(BasaBasa, BasaBasaNrvTrampleDown)) || isNerve(GET_NERVE(BasaBasa, BasaBasaNrvPunchDown));

    if (v1) {
        return;
    }

    TVec3f v15;
    MR::calcSideVec(&v15, this);
    f32 v3 = v15.dot(mVelocity);
    TVec3f v14;
    v14.scale(v3, v15);
    v14.scale(0.5f);
    const TVec3f& vel = mVelocity;
    mVelocity.scaleAdd(-v15.dot(vel), v15, vel);
    mVelocity.add(v14);
    f32 v5 = 0.95f;
    if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvAttackEnd))) {
        v5 = 0.96f;
    } else if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvChaseStart))) {
        v5 = 0.45f;
    }

    mVelocity.scale(v5);
    f32 v6;
    if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvChaseStart))) {
        v6 = 15.0f;
    } else if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvHitBack))) {
        v6 = 15.0f;
    } else if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvQuickTurn))) {
        v6 = 8.0f;
    } else if (isNerve(GET_NERVE(BasaBasa, BasaBasaNrvAttack))) {
        v6 = 23.0f;
    } else {
        v6 = 8.0f;
    }

    if (mVelocity.length() > v6) {
        mVelocity.setLength(v6);
    } else {
        if (MR::isNearZero(mVelocity)) {
            mVelocity.zero();
        }
    }
}

void BasaBasa::tuneHeight() {
    if (MR::isShadowProjected(this, nullptr)) {
        TVec3f v5;
        if (isNearTarget(mAttackDistance)) {
            v5.sub(*mTargetPos, mPosition);
            TVec3f* gravityPtr = &mGravity;
            v5.scale(gravityPtr->dot(v5), *gravityPtr);
        } else {
            v5.scale(MR::getShadowProjectionLength(this, nullptr) - (mAttackDistance - 350.0f), mGravity);
        }

        if (v5.length() < 2.0f) {
            return;
        } else {
            TVec3f v4;
            v4.set(v5);
            MR::normalize(&v4);
            mVelocity.scaleAdd(2.0f, v4, mVelocity);
        }
    }
}

bool BasaBasa::isNearTarget(f32 a1) const {
    if (mTargetPos == MR::getPlayerCenterPos()) {
        return MR::isNearPlayer(this, a1);
    } else {
        return MR::isNear(this, *mTargetPos, a1);
    }
}

bool BasaBasa::isNrvEnableStun() const {
    bool v1 = isNerve(GET_NERVE(BasaBasa, BasaBasaNrvTrampleDown)) || isNerve(GET_NERVE(BasaBasa, BasaBasaNrvPunchDown));

    if (v1) {
        return false;
    }

    v1 = !isNerve(GET_NERVE(BasaBasa, BasaBasaNrvWait)) && !isNerve(GET_NERVE(BasaBasa, BasaBasaNrvAttachCelling));

    return v1;
}

BasaBasa::~BasaBasa() {
}
