#pragma auto_inline off
#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/LiveActor/ActorPoseTQ.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Enemy/DanceFlower.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/LiveActor/ActorParam.hpp"

#define BATCH_NERVE(Name, Method) \
    class Name : public Nerve { public: virtual void execute(Spine* spine) const { static_cast<DanceFlower*>(spine->mExecutor)->Method(); } static Name sInstance; }; \
    Name Name::sInstance ATTRIBUTE_WEAK;

namespace NrvDanceFlower {
    BATCH_NERVE(DanceFlowerNrvWait, exeDanceFlowerWait);
    NEW_NERVE(DanceFlowerNrvAttackSign, DanceFlower, AttackSign);
    BATCH_NERVE(DanceFlowerNrvAttack, exeDanceFlowerAttack);
    NEW_NERVE(DanceFlowerNrvAttackEnd, DanceFlower, AttackEnd);
    NEW_NERVE(DanceFlowerNrvSwoonStart, DanceFlower, SwoonStart);
    NEW_NERVE(DanceFlowerNrvSwoon, DanceFlower, Swoon);
    NEW_NERVE(DanceFlowerNrvSwoonEnd, DanceFlower, SwoonEnd);
    NEW_NERVE(DanceFlowerNrvPressDown, DanceFlower, PressDown);
    NEW_NERVE(DanceFlowerNrvBlowDown, DanceFlower, BlowDown);
    class DanceFlowerNrvTongue : public Nerve { public: virtual void execute(Spine*) const {} static DanceFlowerNrvTongue sInstance; };
    DanceFlowerNrvTongue DanceFlowerNrvTongue::sInstance ATTRIBUTE_WEAK;
    class DanceFlowerNrvFreeze : public Nerve { public: virtual void execute(Spine*) const; static DanceFlowerNrvFreeze sInstance; };
    DanceFlowerNrvFreeze DanceFlowerNrvFreeze::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvDanceFlower

#define GET_DANCEFLOWER_NERVE(name) (&NrvDanceFlower::name::sInstance)

DanceFlower::DanceFlower(const char* pName) : LiveActor(pName) {
    mPose = new ActorPoseTQ();
    mRailClip.x = 0.0f;
    mRailClip.y = 0.0f;
    mRailClip.z = 0.0f;
    mAnimScaleCtrl = nullptr;
    mStarPointBind = nullptr;
    mYoshiLockOn = nullptr;
    mIsSwoonStep = false;
}

DanceFlower::~DanceFlower() {
}

void DanceFlower::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToStartPos(this);
        MR::initAndSetRailClipping(&mRailClip, this, 100.0f, 500.0f);
    }

    MR::initActorPoseTQ(mPose, this);
    MR::addToAttributeGroupSearchTurtle(this);
    initNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait), 1);

    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointBind, GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze), "SupportFreeze");

    mYoshiLockOn = MR::createYoshiLockOnTarget(this, "Eye", TVec3f(0.0f));
    makeActorAppeared();
}

void DanceFlower::initAfterPlacement() {
    if (MR::isExistRail(this)) {
        this->snapToFloorPos();
    }
}

void DanceFlower::kill() {
    LiveActor::kill();
    MR::tryHitReactionDead(this);
}

void DanceFlower::control() {
    mAnimScaleCtrl->updateNerve();
    MR::blendActorPoseFromGroundAndFront(mPose, this, 0.05f, 0.5f);
    if (this->isEatableNerve()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    if (this->isStarPointBindableNerve() && mStarPointBind->tryStartPointBind()) {
        mIsSwoonStep = this->isPressDownOpen();
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze));
    }
}

void DanceFlower::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPose);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void DanceFlower::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Attack")) {
        TVec3f up;
        TVec3f dir;
        MR::calcUpVec(&up, this);
        MR::calcSensorDirection(&dir, pSender, pReceiver);
        f32 dot = dir.dot(up);
        if (dot < -MR::getSensorRadius(pReceiver)) return;
    }
    if (MR::isSensorPlayer(pReceiver) && isAttackActiveNerve()) {
        if (MR::isEqualSensor(pSender, this, "Body")) {
            if (MR::sendMsgEnemyAttack(pReceiver, pSender)) return;
        } else if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) return;
    }
    if (MR::isSensorEnemy(pReceiver) && isAttackActiveNerve() && MR::isEqualSensor(pSender, this, "Attack") &&
    MR::sendMsgToEnemyAttackBlow(pReceiver, pSender)) {
        MR::tryHitReactionBetweenSensors(this, "攻撃ヒット", pSender, pReceiver);
    }
    if (MR::isEqualSensor(pSender, this, "Body")) MR::sendMsgPush(pReceiver, pSender);
}

bool DanceFlower::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgToEnemyAttackBlow(msg) && this->isBlowHittableNerve()) {
        startBlowDownReaction(pSender, pReceiver);
        return true;
    }
    return false;
}

bool DanceFlower::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg) && this->isSwoonStartableNerve()) {
        MR::invalidateClipping(this);
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart));
        return true;
    }

    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
            return tryStartPressDown();
        }
        if (MR::isMsgJetTurtleAttack(msg)) {
            if (this->isProjectileHittableNerve()) {
                startBlowDownReaction(pSender, pReceiver);
            }
            return false;
        }
        bool hitProjectiles = MR::isMsgPlayerHitProjectiles(msg);
        if (hitProjectiles && this->isProjectileHittableNerve()) {
            startBlowDownReaction(pSender, pReceiver);
            return true;
        }
        if (MR::isMsgPlayerHitAll(msg) && this->isBlowHittableNerve()) {
            startBlowDownReaction(pSender, pReceiver);
            return true;
        }
    }
    return false;
}

bool DanceFlower::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (this->isEatableNerve()) {
            if (isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack))) {
                setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart));
                return false;
            }
            MR::stopBck(this);
            MR::invalidateHitSensors(this);
            setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvTongue));
            return true;
        }
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        makeActorDead();
        return true;
    }
    if (MR::isMsgYoshiCancelEaten(msg)) {
        makeActorDead();
        return true;
    }
    if (MR::isMsgPlayerKick(msg) && this->isSwoonNerve()) {
        startBlowDownReaction(pSender, pReceiver);
        return true;
    }
    return false;
}

bool DanceFlower::tryStartPressDown() {
    if (this->isPressableNerve()) {
        if (this->isPressDownOpen()) {
            MR::startAction(this, "PressDownOpen");
        } else {
            MR::startAction(this, "PressDownClose");
        }
        this->setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvPressDown));
        MR::invalidateHitSensors(this);
        return true;
    }
    return false;
}

void DanceFlower::startBlowDownReaction(HitSensor* pSender, HitSensor* pReceiver) {
    TVec3f dir;
    MR::calcVecToTargetPosH(&dir, this, *MR::getSensorPos(pSender), nullptr);
    ActorPoseTQ* pose = mPose;
    pose->mFront.x = dir.x;
    pose->mFront.y = dir.y;
    pose->mFront.z = dir.z;
    MR::setVelocityBlowAttack(this, pSender, pReceiver, 30.0f, 30.0f, 4);
    MR::tryHitReactionBlowBetweenSensors(this, pSender, pReceiver);
    setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvBlowDown));
    MR::invalidateHitSensors(this);
}

// NONMATCHING: argument setup order differs for the copied probe position.
void DanceFlower::snapToFloorPos() {
    TVec3f pos = mPosition + mGravity * -250.0f;
    TVec3f hit(0.0f, 0.0f, 0.0f);
    if (MR::getFirstPolyOnLineToMap(&hit, nullptr, pos, mGravity * 500.0f)) MR::setPosition(this, hit);
}

bool DanceFlower::isPressableNerve() const {
    if (isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze))) return true;
    return false;
}

s32 DanceFlower::isPressDownOpen() const {
    if (this->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack))) {
        return true;
    }
    if (this->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze))) {
        return this->mIsSwoonStep;
    }
    return false;
}

bool DanceFlower::isSwoonStartableNerve() const {
    if (isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze))) return true;
    return false;
}

bool DanceFlower::isAttackActiveNerve() const {
    if (isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd))) return true;
    return false;
}

bool DanceFlower::isProjectileHittableNerve() const {
    if (isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze))) return true;
    return false;
}

bool DanceFlower::isBlowHittableNerve() const {
    if (isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze))) return true;
    return false;
}

bool DanceFlower::isEatableNerve() const {
    if (isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze))) return true;
    return false;
}

bool DanceFlower::isStarPointBindableNerve() const {
    if (isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd))) return true;
    return false;
}

bool DanceFlower::isSwoonNerve() const {
    if (isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) ||
    isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd))) return true;
    return false;
}

void DanceFlower::exeDanceFlowerWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Wait");
    }
    if (MR::isGreaterStep(this, 0x3C)) {
        MR::invalidateClipping(this);
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign));
    }
}

void DanceFlower::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack));
    }
}

// NONMATCHING: the rail-turn flag initialization is scheduled before the direction-pointer setup.
void DanceFlower::exeDanceFlowerAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }
    if (MR::isExistRail(this)) {
        bool moved = false;
        MR::rotateDirectionGravityDegree(this, &mPose->mFront, 0.0f);
        if (MR::moveCoordAndFollowTransAndCheckPassPointNo(this, 10.0f) >= 0) {
            moved = true;
        }
        if (MR::isRailReachedGoal(this)) {
            MR::reverseRailDirection(this);
            moved = true;
        }
        this->snapToFloorPos();
        if (moved) {
            setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd));
        }
    } else if (MR::isGreaterStep(this, 0x46)) {
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd));
    }
}

void DanceFlower::exeAttackEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackEnd");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait));
    }
}

void DanceFlower::exeSwoonStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonStart");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon));
    }
}

void DanceFlower::exeSwoon() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Swoon");
    }
    if (MR::isGreaterStep(this, 0xB4)) {
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd));
    }
}

void DanceFlower::exeSwoonEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonEnd");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign));
    }
}

void DanceFlower::exePressDown() {
    if (MR::isActionEnd(this)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void DanceFlower::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
    }
    MR::addVelocityToGravity(this, 1.2f);
    MR::attenuateVelocity(this, 0.99f);
    if (MR::isGreaterStep(this, 0x14)) {
        MR::emitItemHit(this);
        kill();
    }
}

void NrvDanceFlower::DanceFlowerNrvFreeze::execute(Spine* spine) const {
    DanceFlower* actor = static_cast<DanceFlower*>(spine->mExecutor);
    if (MR::isFirstStep(actor)) MR::stopAllAnim(actor);
    MR::updateActorStateAndNextNerve(actor, GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait));
}
