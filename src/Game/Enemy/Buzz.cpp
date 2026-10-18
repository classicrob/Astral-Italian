#include "Game/Util.hpp"
#include "Game/Enemy/Buzz.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

namespace NrvBuzz {
    NEW_NERVE(BuzzNrvWait, Buzz, Wait);
    NEW_NERVE(BuzzNrvEaten, Buzz, Eaten);
    NEW_NERVE(BuzzNrvFreeze, Buzz, Freeze);
    NEW_NERVE(BuzzNrvStarPieceHit, Buzz, StarPieceHit);
    NEW_NERVE(BuzzNrvAttackSuccess, Buzz, AttackSuccess);
    NEW_NERVE(BuzzNrvBlowDown, Buzz, BlowDown);
    NEW_NERVE(BuzzNrvStepDown, Buzz, StepDown);
};  // namespace NrvBuzz

Buzz::Buzz(const char* pName) : LiveActor(pName) {
    mRailPos.x = 0.0f;
    mRailPos.y = 0.0f;
    mRailPos.z = 0.0f;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mSpeed = 3.0f;
    mAnimScaleController = nullptr;
    mBindState = nullptr;
    mYoshiLockOnTarget = nullptr;
}

void Buzz::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::calcFrontVec(&mFront, this);
    if (MR::isExistRail(this)) {
        MR::moveCoordAndTransToNearestRailPos(this);
        MR::initAndSetRailClipping(&mRailPos, this, 100.0f, 500.0f);
    }
    MR::getJMapInfoArg0NoInit(rIter, &mSpeed);
    initNerve(&NrvBuzz::BuzzNrvWait::sInstance, 1);
    mAnimScaleController = new AnimScaleController(nullptr);
    mBindState = new WalkerStateBindStarPointer(this, mAnimScaleController);
    MR::initActorState(this, mBindState, &NrvBuzz::BuzzNrvFreeze::sInstance, "SupportFreeze");
    mYoshiLockOnTarget = MR::createYoshiLockOnTarget(this, (MtxPtr)nullptr, TVec3f(0.0f));
    makeActorAppeared();
}

void Buzz::control() {
    mAnimScaleController->updateNerve();
    if (fn_80176410()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOnTarget);
    }
    if (fn_80176280() && mBindState->tryStartPointBind()) {
        setNerve(&NrvBuzz::BuzzNrvFreeze::sInstance);
    }
}

void Buzz::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    MR::updateBaseScale(this, mAnimScaleController);
}

void Buzz::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && fn_80176300() && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
        setNerve(&NrvBuzz::BuzzNrvAttackSuccess::sInstance);
        return;
    }
    if (fn_80176310()) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Buzz::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg) && fn_80176390()) {
        setNerve(&NrvBuzz::BuzzNrvStarPieceHit::sInstance);
        return true;
    }
    if (MR::isMsgInvincibleAttack(msg) || MR::isMsgYoshiProjectile(msg)) {
        MR::calcSensorDirectionNormalize(&mFront, pReceiver, pSender);
        MR::setVelocitySeparateHV(this, pSender, pReceiver, 30.0f, 30.0f);
        setNerve(&NrvBuzz::BuzzNrvBlowDown::sInstance);
        return true;
    }
    return false;
}

bool Buzz::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (fn_80176410()) {
            setNerve(&NrvBuzz::BuzzNrvEaten::sInstance);
            MR::zeroVelocity(this);
            return true;
        }
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        MR::setScale(this, 1.0f);
        MR::setRotation(this, 0.0f);
        kill();
        return true;
    }
    if (MR::isMsgYoshiCancelEaten(msg)) {
        MR::setScale(this, 1.0f);
        MR::setRotation(this, 0.0f);
        kill();
        return true;
    }
    return false;
}

bool Buzz::fn_80176280() const {
    if (isNerve(&NrvBuzz::BuzzNrvWait::sInstance) || isNerve(&NrvBuzz::BuzzNrvAttackSuccess::sInstance)) {
        return true;
    }
    if (isNerve(&NrvBuzz::BuzzNrvStarPieceHit::sInstance)) {
        return MR::isGreaterStep(this, 30);
    }
    return false;
}

#pragma dont_inline on
bool Buzz::fn_80176300() const {
    return isNerve(&NrvBuzz::BuzzNrvWait::sInstance);
}
#pragma dont_inline off

bool Buzz::fn_80176310() const {
    if (isNerve(&NrvBuzz::BuzzNrvWait::sInstance) || isNerve(&NrvBuzz::BuzzNrvAttackSuccess::sInstance) || isNerve(&NrvBuzz::BuzzNrvStarPieceHit::sInstance) || isNerve(&NrvBuzz::BuzzNrvFreeze::sInstance)) {
        return true;
    }
    return false;
}

bool Buzz::fn_80176390() const {
    if (isNerve(&NrvBuzz::BuzzNrvWait::sInstance) || isNerve(&NrvBuzz::BuzzNrvAttackSuccess::sInstance) || isNerve(&NrvBuzz::BuzzNrvStarPieceHit::sInstance) || isNerve(&NrvBuzz::BuzzNrvFreeze::sInstance)) {
        return true;
    }
    return false;
}

bool Buzz::fn_80176410() const {
    if (isNerve(&NrvBuzz::BuzzNrvWait::sInstance) || isNerve(&NrvBuzz::BuzzNrvAttackSuccess::sInstance) || isNerve(&NrvBuzz::BuzzNrvStarPieceHit::sInstance) || isNerve(&NrvBuzz::BuzzNrvFreeze::sInstance)) {
        return true;
    }
    return false;
}

void Buzz::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    TVec3f sp8;
    MR::fn_80014360(&sp8, this, 30.0f);
    MR::turnDirectionDegree(this, &mFront, sp8, 2.0f);
    if (MR::isExistRail(this)) {
        MR::fn_80052450(this, mSpeed, 0.2f * mSpeed, 100.0f);
    }
}

void Buzz::exeEaten() {}

void Buzz::exeFreeze() {
    MR::updateActorStateAndNextNerve(this, &NrvBuzz::BuzzNrvWait::sInstance);
}

void Buzz::exeStarPieceHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarPieceHit");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvBuzz::BuzzNrvWait::sInstance);
    }
}

void Buzz::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccess");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvBuzz::BuzzNrvWait::sInstance);
    }
}

void Buzz::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::addVelocityToGravityOrGround(this, 1.2f);
    MR::attenuateVelocity(this, 0.99f);
    if (MR::isGreaterStep(this, 20)) {
        MR::emitItemHit(this);
        kill();
    }
}

void Buzz::exeStepDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StepDown");
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}
