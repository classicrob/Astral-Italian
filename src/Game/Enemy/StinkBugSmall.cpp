#include "Game/Enemy/StinkBugSmall.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

// SMG2 unnamed helper (ActorMovementUtil area, setPositionToMapGround): settles velocity toward a target speed.

namespace NrvStinkBugSmall {
    NEW_NERVE(StinkBugSmallNrvWait, StinkBugSmall, Wait);
    NEW_NERVE(StinkBugSmallNrvSearch, StinkBugSmall, Search);
    NEW_NERVE(StinkBugSmallNrvDashSign, StinkBugSmall, DashSign);
    NEW_NERVE(StinkBugSmallNrvDashSignEnd, StinkBugSmall, DashSignEnd);
    NEW_NERVE(StinkBugSmallNrvDash, StinkBugSmall, Dash);
    NEW_NERVE(StinkBugSmallNrvDashEnd, StinkBugSmall, DashEnd);
    NEW_NERVE(StinkBugSmallNrvBack, StinkBugSmall, Back);
    NEW_NERVE(StinkBugSmallNrvHipDropDown, StinkBugSmall, HipDropDown);
    NEW_NERVE(StinkBugSmallNrvAttack, StinkBugSmall, Attack);
    NEW_NERVE(StinkBugSmallNrvSpinReaction, StinkBugSmall, SpinReaction);
    NEW_NERVE(StinkBugSmallNrvShakeStart, StinkBugSmall, ShakeStart);
    NEW_NERVE(StinkBugSmallNrvShake, StinkBugSmall, Shake);
    NEW_NERVE(StinkBugSmallNrvPanic, StinkBugSmall, Panic);
    NEW_NERVE(StinkBugSmallNrvRecover, StinkBugSmall, Recover);
    NEW_NERVE(StinkBugSmallNrvDPDSwoon, StinkBugSmall, DPDSwoon);
    NEW_NERVE(StinkBugSmallNrvForceFall, StinkBugSmall, ForceFall);
};  // namespace NrvStinkBugSmall

StinkBugSmall::StinkBugSmall(const char* pName) : StinkBugBase(pName), mScaleController(), mBindStarPointer(), mIsSensorOnly() {
}

void StinkBugSmall::init(const JMapInfoIter& rIter) {
    StinkBugBase::init(rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mMaxTurnDegree);
    MR::getJMapInfoArg1NoInit(rIter, &mRadius);
    MR::getJMapInfoArg7NoInit(rIter, &mIsSensorOnly);
    initSound(4, "StinkBugSmall", nullptr, TVec3f(0.0f));
    initHitSensor(3);
    if (mIsSensorOnly) {
        MR::addHitSensorEnemy(this, "Body", 8, 110.0f * mScale.x, TVec3f(0.0f, 10.0f * mScale.x, 0.0f));
    } else {
        MR::addBodyMessageSensorEnemy(this);
    }

    MR::addHitSensorAtJointEnemyAttack(this, "head", "Face", 8, 100.0f * mScale.x, TVec3f(0.0f, -30.0f * mScale.x, 0.0f));
    if (!mIsSensorOnly) {
        MR::initCollisionParts(this, "Switch", getSensor("Body"), nullptr);
    }

    initBinder(200.0f * mScale.x, 200.0f * mScale.x, 0);
    MR::setBinderExceptActor(this, this);
    MR::setBinderIgnoreMovingCollision(this);
    if (!MR::isValidSwitchDead(this)) {
        MR::declareCoin(this, 1);
    }

    MR::initStarPointerTarget(this, 130.0f * mScale.x, TVec3f(0.0f, 50.0f * mScale.x, 0.0f));
    mScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initUseStageSwitchReadA(this, rIter);
    MR::startBrk(this, "Death");
    MR::setBrkFrameAndStop(this, 0.0f);
    initNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvWait), 1);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(StinkBugSmall, StinkBugSmallNrvDPDSwoon), "SupportFreeze");
    makeActorAppeared();
}

void StinkBugSmall::exeWait() {
    if (MR::isFirstStep(this) && mMaxTurnDegree == 0.0f) {
        MR::tryStartBck(this, "Search");
    }

    fixInitPos();
    if (isPlayerInTerritory(400.0f, 600.0f, 200.0f, 200.0f)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDashSign));
    } else if (mMaxTurnDegree != 0.0f) {
        if (MR::isGreaterStep(this, 1)) {
            setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvSearch));
        }
    }
}

void StinkBugSmall::exeSearch() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(this, "Search");
    }

    fixInitPos();
    if (tryTurnSearch(1.0f)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvWait));
    } else if (isPlayerInTerritory(400.0f, 600.0f, 200.0f, 200.0f)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDashSign));
    }
}

void StinkBugSmall::exeDashSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RushStart");
        MR::startActionSound(this, "EvStinkbugSFind");
    }

    MR::startActionSound(this, "EmLvStinkbugSDashSign");
    fixInitPos();
    tryTurnDashSign(3.0f);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDashSignEnd));
    }
}

void StinkBugSmall::exeDashSignEnd() {
    if (MR::isStep(this, 10)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDash));
    }
}

void StinkBugSmall::exeDash() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Rush");
        MR::validateHitSensors(this);
    }

    MR::startActionSound(this, "EmLvStinkbugSDash");
    if (!MR::isNear(this, mHomePos, mRadius) || MR::isBindedWall(this)) {
        mVelocity.zero();
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDashEnd));
        MR::startActionSound(this, "EmStinkbugSDashEnd");
        return;
    }

    setDashVelocity(20.0f);
}

void StinkBugSmall::exeDashEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RushStop");
    }

    mVelocity.zero();
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvBack));
    }
}

void StinkBugSmall::exeBack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Back");
        MR::invalidateHitSensor(this, "head");
    }

    MR::startActionSound(this, "EmLvStinkbugSBack");
    if (MR::isNear(this, mHomePos, 10.0f)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvWait));
        return;
    }

    TVec3f tvf;
    tvf.sub(mHomePos, mPosition);
    MR::normalize(&tvf);
    MR::turnVecToPlane(&tvf, tvf, mGravity);
    mVelocity.scale(5.0f, tvf);
}

void StinkBugSmall::exeHipDropDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Death");
        MR::startBrk(this, "Death");
        if (!mIsSensorOnly) {
            MR::invalidateCollisionParts(this);
        }

        MR::invalidateHitSensors(this);
        MR::invalidateClipping(this);
        MR::startActionSound(this, "EmStompedS");
        MR::startActionSound(this, "EvStinkbugSStomped");
    }

    mVelocity.zero();
    MR::setPositionToMapGround(this, 500.0f, -1.0f);
    if (MR::isActionEnd(this)) {
        kill();
    }
}

void StinkBugSmall::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }

    mVelocity.zero();
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvBack));
    }
}

void StinkBugSmall::exeSpinReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SpinAction");
        MR::startActionSound(this, "EmGuardS");
    }

    mVelocity.zero();
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvBack));
    }
}

void StinkBugSmall::exeShakeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Repel");
    }

    if (MR::isStep(this, 40)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvShake));
    }
}

void StinkBugSmall::exeShake() {
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvBack));
    }
}

void StinkBugSmall::exePanic() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Loss");
        MR::invalidateHitSensor(this, "head");
    }

    MR::startActionSound(this, "EvLvStinkbugSPanic", -1, -1, 15);
    MR::startActionSound(this, "EmLvStinkbugSPanic");
    mVelocity.zero();
    if (!MR::isOnPlayer(getSensor("Body"))) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvRecover));
        return;
    }

    if (!MR::isStep(this, 90)) {
        return;
    }

    setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvShakeStart));
}

void StinkBugSmall::exeRecover() {
    mVelocity.zero();
    MR::startActionSound(this, "EmLvStinkbugSPanic");
    if (MR::isStep(this, 60)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvBack));
    }
}

void StinkBugSmall::exeDPDSwoon() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffectAll(this);
    }

    if (MR::isEffectValid(this, "RushSmoke")) {
        MR::deleteEffect(this, "RushSmoke");
    }

    MR::updateActorStateAndNextNerve(this, GET_NERVE(StinkBugSmall, StinkBugSmallNrvBack));
}

void StinkBugSmall::endDPDSwoon() {
    mBindStarPointer->kill();
}

void StinkBugSmall::exeForceFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Loss");
        mVelocity.zero();
        MR::calcGravity(this);
        MR::onBind(this);
    }

    mVelocity.scaleAdd(2.0f, mGravity, mVelocity);
}

void StinkBugSmall::kill() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    if (!MR::isValidSwitchDead(this)) {
        MR::appearCoinPop(this, mPosition, 1);
    }

    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmExplodeS");
    LiveActor::kill();
}

void StinkBugSmall::initAfterPlacement() {
    TVec3f v8;
    TVec3f v7;
    MR::calcUpVec(&v8, this);
    v7.scaleAdd(50.0f, v8, mPosition);
    TVec3f up;
    MR::calcUpVec(&up, this);
    up.scale(-200.0f);
    HitSensor* pSensor = getSensor("Body");
    MR::getFirstPolyOnLineToMapExceptSensor(&mHomePos, nullptr, v7, up, pSensor);
    MR::resetPosition(this, mHomePos);
}

void StinkBugSmall::control() {
    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        kill();
        return;
    }

    mScaleController->updateNerve();
    if (!tryDPDSwoon() && !tryForceFall()) {
        if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvForceFall))) {
            return;
        }

        bool isMoving = isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDash)) || isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvBack));
        if (!isMoving) {
            MR::offBind(this);
            return;
        }

        MR::onBind(this);
        if (MR::isBindedGround(this)) {
            mGravity.set(-MR::getGroundNormal(this));
        } else {
            mGravity.set(mGravity);
        }

        const TVec3f* pGravity;
        const TVec3f* pVelocity = &mVelocity;
        pGravity = &mGravity;
        mVelocity.killElement(*pVelocity, *pGravity);
        mVelocity.scaleAdd(2.0f, mGravity, mVelocity);
        TVec3f a;
        a = mFront;
        MR::turnVecToPlane(&mFront, a, mGravity);
    }
}

void StinkBugSmall::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, mFront);
    MR::blendMtx(getBaseMtx(), mtx, 0.3f, mtx);
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mScaleController);
}

void StinkBugSmall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvHipDropDown))) {
        return;
    }

    if (mIsSensorOnly && pSender == getSensor("Body")) {
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }

    if (!isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvAttack)) && !isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDPDSwoon)) &&
        MR::isSensorPlayer(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        if (MR::isPlayerDamaging()) {
            MR::sendMsgPush(pReceiver, pSender);
            return;
        }

        if (isHitHorn(pSender, pReceiver, mScale.x * 20.0f) && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
            MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
            if (!isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDash))) {
                setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvAttack));
            }
        }
    }
}

bool StinkBugSmall::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvHipDropDown))) {
        return false;
    }

    if (pReceiver != getSensor("Body")) {
        return false;
    }

    if (MR::isMsgInvincibleAttack(msg)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvHipDropDown));
        return true;
    }

    if (MR::isMsgPlayerHipDropFloor(msg) || MR::isMsgPlayerHipDrop(msg)) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvHipDropDown));
        return true;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDPDSwoon))) {
        return false;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvForceFall))) {
        return false;
    }

    if (MR::isMsgStarPieceReflect(msg) && pReceiver == getSensor("Body")) {
        if (isNrvEnableStarPieceAttack()) {
            setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvSpinReaction));
        }

        return true;
    }

    if (!isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvSpinReaction)) && !isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDash)) &&
        MR::isMsgPlayerSpinAttackOrSupportTico(msg) && pReceiver == getSensor("Body")) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvSpinReaction));
        return true;
    }

    return false;
}

bool StinkBugSmall::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvHipDropDown))) {
        return false;
    }

    if (MR::isPlayerElementModeInvincible() && (MR::isMsgFloorTouch(msg) || MR::isMsgWallTouch(msg) || MR::isMsgCeilTouch(msg))) {
        setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvHipDropDown));
        return true;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDPDSwoon))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvPanic))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvShakeStart))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvForceFall))) {
        return false;
    }

    if (MR::isMsgFloorTouch(msg)) {
        if (MR::isOnPlayer(getSensor("Body"))) {
            if (StinkBugSmall::isShakeChance()) {
                MR::sendMsgEnemyAttackFlip(pSender, pReceiver);
            } else {
                setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvPanic));
            }

            return true;
        }
    }

    return false;
}

bool StinkBugSmall::tryDPDSwoon() {
    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvHipDropDown))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDPDSwoon))) {
        return false;
    }

    if (!mBindStarPointer->tryStartPointBind()) {
        return false;
    }

    setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDPDSwoon));
    return true;
}

bool StinkBugSmall::tryForceFall() {
    if (!MR::isValidSwitchA(this) || !MR::isOnSwitchA(this)) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvHipDropDown))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDPDSwoon))) {
        return false;
    }

    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvForceFall))) {
        return false;
    }

    setNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvForceFall));
    return true;
}

bool StinkBugSmall::isShakeChance() const {
    if (isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvHipDropDown))) {
        return false;
    }

    if (!isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvShake))) {
        return false;
    }

    if (MR::isPlayerHipDropFalling()) {
        return false;
    }

    return MR::isPlayerHipDropLand() == false;
}

bool StinkBugSmall::isNrvEnableStarPieceAttack() const {
    return isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvWait)) || isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvSearch)) ||
           isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDashSign)) || isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvDashSignEnd)) ||
           isNerve(GET_NERVE(StinkBugSmall, StinkBugSmallNrvBack));
}

StinkBugSmall::~StinkBugSmall() {
}
