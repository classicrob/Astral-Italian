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
extern "C" void fn_80010600(ActorPoseTQ*, LiveActor*, f32, f32);
extern "C" void fn_8000A320(LiveActor*);
extern "C" void fn_8022BA10(LiveActor*);
extern "C" void fn_8022BAA0(LiveActor*);
extern "C" void fn_80012B90(LiveActor*, HitSensor*, HitSensor*, s32, void*, f32, f32);
extern "C" void fn_80034760(LiveActor*, HitSensor*, HitSensor*);


// Unit helpers (unnamed in SMG2 symbols). Defined below.
extern "C" {
bool fn_8017A520(DanceFlower* pActor);
void fn_8017A5B0(LiveActor* pActor, HitSensor* pSender, HitSensor* pReceiver);
void fn_8017A660(DanceFlower* pActor);
bool fn_8017A720(const DanceFlower* pActor);
s8 fn_8017A7F0(const DanceFlower* pActor);
bool fn_8017A850(const DanceFlower* pActor);
bool fn_8017A920(const DanceFlower* pActor);
bool fn_8017A980(const DanceFlower* pActor);
bool fn_8017AA50(const DanceFlower* pActor);
bool fn_8017AB10(const DanceFlower* pActor);
bool fn_8017ABE0(const DanceFlower* pActor);
bool fn_8017AC90(const DanceFlower* pActor);
}

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, const char*, const TVec3f&);
}
namespace YoshiFunction {
    void tryYoshiEat(YoshiLockOnTarget*);
}

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
        fn_8017A660(this);
    }
}

void DanceFlower::kill() {
    LiveActor::kill();
    MR::hideModel(this);
}

void DanceFlower::control() {
    mAnimScaleCtrl->updateNerve();
    fn_80010600(mPose, this, 0.05f, 0.5f);
    if (fn_8017AB10(this)) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    if (fn_8017ABE0(this) && mStarPointBind->tryStartPointBind()) {
        mIsSwoonStep = fn_8017A7F0(this);
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze));
    }
}

void DanceFlower::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPose);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

// Second half of attackSensor (SMG2 block_8): blow attack on enemies and push on the body sensor.
static void dfAttackBlock8(DanceFlower* pActor, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) && fn_8017A920(pActor) && MR::isEqualSensor(pSender, pActor, "Attack") &&
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

    if (MR::isSensorPlayer(pReceiver) && fn_8017A920(this)) {
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
    if (MR::isMsgToEnemyAttackBlow(msg) && fn_8017AA50(this)) {
        fn_8017A5B0(this, pSender, pReceiver);
        return true;
    }
    return false;
}

bool DanceFlower::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg) && fn_8017A850(this)) {
        MR::invalidateClipping(this);
        setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvSwoonStart));
        return true;
    }

    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
            return false;
        }
        if (MR::isMsgJetTurtleAttack(msg)) {
            if (fn_8017A980(this)) {
                fn_8017A5B0(this, pSender, pReceiver);
            }
            return false;
        }
        bool hitProjectiles = MR::isMsgPlayerHitProjectiles(msg);
        if (hitProjectiles && fn_8017A980(this)) {
            fn_8017A5B0(this, pSender, pReceiver);
            return true;
        }
        if (MR::isMsgPlayerHitAll(msg) && fn_8017AA50(this)) {
            fn_8017A5B0(this, pSender, pReceiver);
            return true;
        }
    }
    return false;
}

bool DanceFlower::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (fn_8017AB10(this)) {
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
    if (MR::isMsgPlayerKick(msg) && fn_8017AC90(this)) {
        fn_8017A5B0(this, pSender, pReceiver);
        return true;
    }
    return false;
}

// Starts the press-down action and enters PressDown (fn_8017A520).
extern "C" bool fn_8017A520(DanceFlower* pActor) {
    if (fn_8017A720(pActor)) {
        if (fn_8017A7F0(pActor)) {
            MR::startAction(pActor, "PressDownOpen");
        } else {
            MR::startAction(pActor, "PressDownClose");
        }
        MR::invalidateHitSensors(pActor);
        pActor->setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvPressDown));
        return true;
    }
    return false;
}

// Blows the flower away from the hitting sensor (fn_8017A5B0).
extern "C" void fn_8017A5B0(LiveActor* pActor, HitSensor* pSender, HitSensor* pReceiver) {
    TVec3f dir;
    MR::calcVecToTargetPosH(&dir, pActor, *MR::getSensorPos(pSender), nullptr);
    DanceFlower* pFlower = static_cast<DanceFlower*>(pActor);
    TVec3f* pPoseVec = &pFlower->mPose->mFront;
    *pPoseVec = dir;
    fn_80012B90(pActor, pSender, pReceiver, 4, pPoseVec, 0.0f, 0.0f);
    fn_80034760(pActor, pSender, pReceiver);
    MR::invalidateHitSensors(pActor);
    pActor->setNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvBlowDown));
}

// Snaps the flower to the nearest floor below its rail position (fn_8017A660).
extern "C" void fn_8017A660(DanceFlower* pActor) {
    TVec3f up;
    MR::calcUpVec(&up, pActor);
    TVec3f pos(pActor->mPosition);
    TVec3f offs(up);
    offs.scale(-250.0f);
    TVec3f hit;
    if (MR::getFirstPolyOnLineToMap(&hit, nullptr, pos, offs)) {
        MR::setPosition(pActor, hit);
    }
}

// True in the main attack/swoon/wait nerves (fn_8017A720).
extern "C" bool fn_8017A720(const DanceFlower* pActor) {
    return isInMainNerves(pActor);
}

// Returns the swoon-step flag, or true for the freeze nerve's own flag (fn_8017A7F0).
extern "C" s8 fn_8017A7F0(const DanceFlower* pActor) {
    if (pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttack))) {
        return true;
    }
    if (pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvFreeze))) {
        return pActor->mIsSwoonStep;
    }
    return false;
}

extern "C" bool fn_8017A850(const DanceFlower* pActor) {
    return isInMainNerves(pActor);
}

extern "C" bool fn_8017A920(const DanceFlower* pActor) {
    return isInAttackNerves(pActor);
}

extern "C" bool fn_8017A980(const DanceFlower* pActor) {
    return isInMainNerves(pActor);
}

extern "C" bool fn_8017AA50(const DanceFlower* pActor) {
    return isInSwoonOrAttackNerves(pActor);
}

extern "C" bool fn_8017AB10(const DanceFlower* pActor) {
    return isInMainNerves(pActor);
}

extern "C" bool fn_8017ABE0(const DanceFlower* pActor) {
    return isInSwoonOrAttackNerves(pActor);
}

extern "C" bool fn_8017AC90(const DanceFlower* pActor) {
    return isInSwoonNerves(pActor) || pActor->isNerve(GET_DANCEFLOWER_NERVE(DanceFlowerNrvAttackEnd));
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
        fn_8017A660(this);
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
        fn_8022BAA0(this);
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
        fn_8022BA10(this);
        kill();
    }
}

// Yoshi tongue placeholder: the SMG2 body has no action of its own here.
void DanceFlower::exeTongue() {
}

// Freeze (star pointer) state: first step only lets the actor reset its state.
void DanceFlower::exeFreeze() {
    if (MR::isFirstStep(this)) {
        fn_8000A320(this);
    }
}

