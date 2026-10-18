#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/LiveActor/ActorPoseTQ.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Enemy/DanceFlower.hpp"
void fn_8017A5B0(LiveActor* pActor, HitSensor* pSender, HitSensor* pReceiver);
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/LiveActor/ActorParam.hpp"

namespace NrvDanceFlower {
    NEW_NERVE(DanceFlowerNrvWait, DanceFlower, Wait);
    NEW_NERVE(DanceFlowerNrvAttackSign, DanceFlower, AttackSign);
    NEW_NERVE(DanceFlowerNrvAttack, DanceFlower, Attack);
    NEW_NERVE(DanceFlowerNrvAttackEnd, DanceFlower, AttackEnd);
    NEW_NERVE(DanceFlowerNrvSwoonStart, DanceFlower, SwoonStart);
    NEW_NERVE(DanceFlowerNrvSwoon, DanceFlower, Swoon);
    NEW_NERVE(DanceFlowerNrvSwoonEnd, DanceFlower, SwoonEnd);
    NEW_NERVE(DanceFlowerNrvPressDown, DanceFlower, PressDown);
    NEW_NERVE(DanceFlowerNrvBlowDown, DanceFlower, BlowDown);
    NEW_NERVE(DanceFlowerNrvTongue, DanceFlower, Tongue);
    NEW_NERVE(DanceFlowerNrvFreeze, DanceFlower, Freeze);
};  // namespace NrvDanceFlower

#define GET_DANCEFLOWER_NERVE(name) (&NrvDanceFlower::name::sInstance)

// Helpers of this unit (unnamed in SMG2 symbols).


// Unit helpers (unnamed in SMG2 symbols). Defined below.


// Nerve-set checks shared by the message handlers (fn_8017A720 ... fn_8017AC90).
static bool isInMainNerves(const LiveActor* pActor) {
    return pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait)) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart)) ||
           pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd)) ||
           pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign)) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack)) ||
           pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd)) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze));
}

static bool isInSwoonOrAttackNerves(const LiveActor* pActor) {
    return pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvWait)) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart)) ||
           pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign)) ||
           pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd)) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze)) ||
           pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd));
}

static bool isInSwoonNerves(const LiveActor* pActor) {
    return pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart)) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoon)) ||
           pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonEnd));
}

static bool isInAttackNerves(const LiveActor* pActor) {
    return pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack)) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd));
}

static bool isInMoveOrAttackNerves(const LiveActor* pActor) {
    return pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackSign)) || isInAttackNerves(pActor);
}

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
    MR::initActor(this, rIter, nullptr, false);
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

    TVec3f zero(0.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, "Eye", zero);
    makeActorAppeared();
}

void DanceFlower::initAfterPlacement() {
    if (MR::isExistRail(this)) {
        this->fn_8017A660();
    }
}

void DanceFlower::kill() {
    LiveActor::kill();
    MR::tryHitReactionDead(this);
}

void DanceFlower::control() {
    mAnimScaleCtrl->updateNerve();
    MR::fn_80010600(mPose, this, 0.05f, 0.5f);
    if (this->fn_8017AB10()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    if (this->fn_8017ABE0() && mStarPointBind->tryStartPointBind()) {
        mIsSwoonStep = this->fn_8017A7F0();
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze));
    }
}

void DanceFlower::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPose);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

// Second half of attackSensor (SMG2 block_8): blow attack on enemies and push on the body sensor.
static void dfAttackBlock8(DanceFlower* pActor, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) && pActor->fn_8017A920() && MR::isEqualSensor(pSender, pActor, "Attack") &&
        MR::sendMsgToEnemyAttackBlow(pReceiver, pSender)) {
        MR::tryHitReactionBetweenSensors(pActor, nullptr, pSender, pReceiver);
    }
    if (MR::isEqualSensor(pSender, pActor, "Body")) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

// NONMATCHING: the SMG2 attack-sensor flow is approximated from the draft.
void DanceFlower::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Attack")) {
        TVec3f up;
        MR::calcUpVec(&up, this);
        TVec3f dir;
        MR::calcSensorDirection(&dir, pSender, pReceiver);
        f32 dot = dir.dot(up);
        f32 radius = MR::getSensorRadius(pReceiver);
        if (dot < -radius) {
            return;
        }
    }

    if (MR::isSensorPlayer(pReceiver) && this->fn_8017A920()) {
        if (MR::isEqualSensor(pSender, this, "Body")) {
            if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                return;
            }
            dfAttackBlock8(this, pSender, pReceiver);
        } else if (!MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
            dfAttackBlock8(this, pSender, pReceiver);
        }
    } else {
        dfAttackBlock8(this, pSender, pReceiver);
    }
}

bool DanceFlower::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgToEnemyAttackBlow(msg) && this->fn_8017AA50()) {
        fn_8017A5B0(this, pSender, pReceiver);
        return true;
    }
    return false;
}

bool DanceFlower::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg) && this->fn_8017A850()) {
        MR::invalidateClipping(this);
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart));
        return true;
    }

    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
            return false;
        }
        if (MR::isMsgJetTurtleAttack(msg)) {
            if (this->fn_8017A980()) {
                fn_8017A5B0(this, pSender, pReceiver);
            }
            return false;
        }
        bool hitProjectiles = MR::isMsgPlayerHitProjectiles(msg);
        if (hitProjectiles && this->fn_8017A980()) {
            fn_8017A5B0(this, pSender, pReceiver);
            return true;
        }
        if (MR::isMsgPlayerHitAll(msg) && this->fn_8017AA50()) {
            fn_8017A5B0(this, pSender, pReceiver);
            return true;
        }
    }
    return false;
}

bool DanceFlower::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (this->fn_8017AB10()) {
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
    if (MR::isMsgPlayerKick(msg) && this->fn_8017AC90()) {
        fn_8017A5B0(this, pSender, pReceiver);
        return true;
    }
    return false;
}

// Starts the press-down action and enters PressDown (fn_8017A520).
bool DanceFlower::fn_8017A520() {
    if (this->fn_8017A720()) {
        if (this->fn_8017A7F0()) {
            MR::startAction(this, "PressDownOpen");
        } else {
            MR::startAction(this, "PressDownClose");
        }
        MR::invalidateHitSensors(this);
        this->setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvPressDown));
        return true;
    }
    return false;
}

// Blows the flower away from the hitting sensor (fn_8017A5B0).
void fn_8017A5B0(LiveActor* pActor, HitSensor* pSender, HitSensor* pReceiver) {
    TVec3f dir;
    MR::calcVecToTargetPosH(&dir, pActor, *MR::getSensorPos(pSender), nullptr);
    DanceFlower* pFlower = static_cast<DanceFlower*>(pActor);
    TVec3f* pPoseVec = &pFlower->mPose->mFront;
    *pPoseVec = dir;
    // NONMATCHING: target also passes pPoseVec in r7; the fn_80012B90 thunk decl has no 7th arg.
    MR::fn_80012B90(pActor, 0.0f, 0.0f, pSender, pReceiver, 4);
    MR::fn_80034760(pActor, pSender, pReceiver);
    MR::invalidateHitSensors(pActor);
    pActor->setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvBlowDown));
}

// Snaps the flower to the nearest floor below its rail position (fn_8017A660).
void DanceFlower::fn_8017A660() {
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f pos(this->mPosition);
    TVec3f offs(up);
    offs.scale(-250.0f);
    TVec3f hit;
    if (MR::getFirstPolyOnLineToMap(&hit, nullptr, pos, offs)) {
        MR::setPosition(this, hit);
    }
}

// True in the main attack/swoon/wait nerves (fn_8017A720).
bool DanceFlower::fn_8017A720() const {
    return isInMainNerves(this);
}

// Returns the swoon-step flag, or true for the freeze nerve's own flag (fn_8017A7F0).
s8 DanceFlower::fn_8017A7F0() const {
    if (this->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack))) {
        return true;
    }
    if (this->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze))) {
        return this->mIsSwoonStep;
    }
    return false;
}

bool DanceFlower::fn_8017A850() const {
    return isInMainNerves(this);
}

bool DanceFlower::fn_8017A920() const {
    return isInAttackNerves(this);
}

bool DanceFlower::fn_8017A980() const {
    return isInMainNerves(this);
}

bool DanceFlower::fn_8017AA50() const {
    return isInSwoonOrAttackNerves(this);
}

bool DanceFlower::fn_8017AB10() const {
    return isInMainNerves(this);
}

bool DanceFlower::fn_8017ABE0() const {
    return isInSwoonOrAttackNerves(this);
}

bool DanceFlower::fn_8017AC90() const {
    return isInSwoonNerves(this) || this->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd));
}

void DanceFlower::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
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

void DanceFlower::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }
    if (MR::isExistRail(this)) {
        bool moved = false;
        TVec3f dir(mPose->mFront);
        MR::rotateDirectionGravityDegree(this, &dir, 0.0f);
        if (MR::moveCoordAndFollowTransAndCheckPassPointNo(this, 10.0f) >= 0) {
            moved = true;
        }
        if (MR::isRailReachedGoal(this)) {
            MR::reverseRailDirection(this);
            moved = true;
        }
        this->fn_8017A660();
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

// Yoshi tongue placeholder: the SMG2 body has no action of its own here.
void DanceFlower::exeTongue() {
}

// Freeze (star pointer) state: first step only lets the actor reset its state.
void DanceFlower::exeFreeze() {
    if (MR::isFirstStep(this)) {
        MR::fn_8000A320(this);
    }
}

