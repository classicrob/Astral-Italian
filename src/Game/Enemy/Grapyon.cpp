#include "Game/Enemy/Grapyon.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/MapObj/ElectricRailHolder.hpp"
#include "Game/Util.hpp"
#include <JSystem/JGeometry/TUtil.hpp>

// Single Newton step on frsqrte (stays in f32, no frsp).
extern const f32 cGrapyonFallAccel;

static inline f32 sqrtNewton(__REGISTER f32 val) {
    if (val > 0.0f) {
        __REGISTER f32 e;
        asm { frsqrte e, val }
        f32 x = e * val;
        return -((x * e) - 3.0f) * x * 0.5f;
    }
    return val;
}

namespace NrvGrapyon {
    NEW_NERVE(GrapyonNrvWait, Grapyon, Wait);
    NEW_NERVE(GrapyonNrvJump, Grapyon, Jump);
    NEW_NERVE(GrapyonNrvFall, Grapyon, Fall);
    NEW_NERVE(GrapyonNrvLand, Grapyon, Land);
    NEW_NERVE(GrapyonNrvGuard, Grapyon, Guard);
    NEW_NERVE(GrapyonNrvAttackHit, Grapyon, AttackHit);
    NEW_NERVE(GrapyonNrvDown, Grapyon, Down);
    NEW_NERVE(GrapyonNrvBlowDown, Grapyon, BlowDown);
};  // namespace NrvGrapyon

// Inlined into their nerves (defined before the NEW_NERVE).
void Grapyon::exeDead() {
    MR::emitItemHit(this);
    MR::tryHitReactionDead(this);
    kill();
}

namespace NrvGrapyon {
    NEW_NERVE(GrapyonNrvDead, Grapyon, Dead);
};  // namespace NrvGrapyon

void Grapyon::exeSupportFreeze() {
    if (MR::updateActorStateAndNextNerve(this, &NrvGrapyon::GrapyonNrvWait::sInstance)) {
        trySwitchFall();
    }
}

namespace NrvGrapyon {
    NEW_NERVE(GrapyonNrvSupportFreeze, Grapyon, SupportFreeze);
};  // namespace NrvGrapyon

#define NRV(n) (&NrvGrapyon::GrapyonNrv##n::sInstance)

Grapyon::Grapyon(const char* pName)
    : LiveActor(pName), mHead(nullptr), mAnimScaleCtrl(nullptr), mBindStar(nullptr), mScale(1.0f), mDirection(1.0f), mDirectionInit(1.0f),
      mSwitchState(false) {
    mMtx.identity();
}

void Grapyon::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "GrapyonBody", false);
    mHead = new PartsModel(this, "グラピョン頭", "GrapyonHead", mMtx.toMtxPtr(), 0x13, false);
    mHead->initWithoutIter();
    mHomePos = mPosition;
    MR::calcFrontVec(&mFront, this);
    MR::calcUpVec(&mUp, this);
    bool flag = false;
    MR::getJMapInfoArg0NoInit(rIter, &flag);
    mDirectionInit = flag ? -1.0f : 1.0f;
    initNerve(NRV(Wait), 1);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mBindStar = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mBindStar, reinterpret_cast< const Nerve* >(NRV(SupportFreeze)), "SupportFreeze");
    makeActorAppeared();
}

void Grapyon::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void Grapyon::makeActorAppeared() {
    mDirection = mDirectionInit;
    mScale = mDirection;
    updateHeadBtp();
    setNerve(NRV(Wait));
    LiveActor::makeActorAppeared();
}

void Grapyon::control() {
    mAnimScaleCtrl->updateNerve();
    if (!isNerve(NRV(SupportFreeze))) {
        if (MR::isValidSwitchA(this)) {
            mSwitchState = MR::isOnSwitchA(this);
        } else {
            mSwitchState = false;
        }
    }
    MR::fn_800145C0(this);
    if (isNormalNerve() && !isNerve(NRV(SupportFreeze))) {
        if (mBindStar->tryStartPointBind()) {
            setNerve(NRV(SupportFreeze));
            return;
        }
        if (ElectricRailFunction::isTouchRail(getSensor("Body"), nullptr, nullptr)) {
            tryElectric();
        }
    }
}

void Grapyon::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::tryStartActionNotPlaying(mHead, "Wait");
        MR::validateClipping(this);
        MR::zeroVelocity(this);
    }
    calcMove();
    if (!trySwitchFall()) {
        mUp = -mGravity;
        if (MR::isGreaterStep(this, 30)) {
            if (!MR::isNearPlayerHorizontal(this, 1000.0f)) {
                setNerve(NRV(Wait));
            } else {
                setNerve(NRV(Jump));
            }
        }
    }
}

void Grapyon::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Jump");
        MR::startAction(mHead, "Jump");
    }
    if (MR::isStep(this, 3)) {
        f32 speed = 200.0f;
        TVec3f side(1.0f, 0.0f, 0.0f);
        MR::calcSideVec(&side, this);
        if (MR::isNearPlayerHorizontal(this, 1000.0f)) {
            if (!MR::isPlayerLeftSide(this)) {
                speed *= -1.0f;
            }
        } else {
            speed *= MR::getSignHalfProbability();
            TVec3f toHome(0.0f);
            MR::calcVecToTargetHorizon(&toHome, this, &mHomePos, nullptr);
            if (__fabsf(side.dot(toHome) - speed) > 1000.0f) {
                speed *= -1.0f;
            }
        }
        s32 frames = 33;
        speed /= frames;
        MR::addVelocity(this, side * speed);
    }
    calcMove();
    if (trySwitchFall()) {
        MR::killVelocityHorizon(this);
        return;
    }
    mUp = -mGravity;
    if (MR::isActionEnd(this)) {
        MR::zeroVelocity(this);
        setNerve(NRV(Wait));
    }
}

// NONMATCHING: target keeps 2.0f * 1.2f unfolded (two literal loads) and different float register allocation around the sqrt step
#pragma push
#pragma opt_common_subs off
void Grapyon::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Fall");
        MR::startAction(mHead, "Fall");
        mDirection = -1.0f * mScale;
        updateHeadBtp();
    }
    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Fall");
        MR::startAction(mHead, "Fall");
    }
    calcMove();
    if (MR::isGreaterEqualStep(this, 10)) {
        if (MR::isShadowProjected(this, nullptr)) {
            f32 len = MR::getShadowProjectionLength(this, nullptr);
            f32 g = MR::calcGravitySpeed(this);
            f32 d = g * g + 2.0f * cGrapyonFallAccel * len;
            if (d > 0.0f) {
                f32 s = sqrtNewton(d);
                if (-g + s / cGrapyonFallAccel <= 20.0f) {
                    setNerve(NRV(Land));
                    return;
                }
            }
        }
        if (MR::isBindedGround(this)) {
            setNerve(NRV(Land));
            return;
        }
    }
    if (trySwitchFall()) {
        return;
    }
}


#pragma pop

// NONMATCHING: target hoists the inlined TVec3 add operands into r30/r31 (stack slot / register allocation only)
void Grapyon::exeLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Land");
        MR::startAction(mHead, "Land");
        MR::setBckFrame(this, 1.0f);
        TVec3f offset = mUp * MR::getBinderRadius(this);
        TVec3f offset2 = offset * 2.0f;
        TVec3f newPos(mPosition);
        newPos.add(offset2);
        MR::setPosition(this, newPos);
        mScale = mDirection;
        MR::invalidateClipping(this);
    }
    calcMove();
    if (!trySwitchFall()) {
        mUp = -mGravity;
        if (MR::isActionEnd(this)) {
            MR::zeroVelocity(this);
            setNerve(NRV(Wait));
        }
    }
}

void Grapyon::exeGuard() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Guard");
        MR::startAction(mHead, "Guard");
        MR::validateClipping(this);
    }
    calcMove();
    if (trySwitchFall()) {
        MR::zeroVelocity(this);
        return;
    }
    mUp = -mGravity;
    if (MR::isActionEnd(this)) {
        MR::zeroVelocity(this);
        setNerve(NRV(Wait));
    }
}

void Grapyon::exeAttackHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackHit");
        MR::startAction(mHead, "AttackHit");
        MR::validateClipping(this);
        MR::zeroVelocity(this);
    }
    calcMove();
    if (trySwitchFall()) {
        MR::zeroVelocity(this);
        return;
    }
    mUp = -mGravity;
    if (MR::isActionEnd(this)) {
        setNerve(NRV(Wait));
    }
}

void Grapyon::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
        MR::startAction(mHead, "Down");
        MR::invalidateClipping(this);
    }
    mUp = -mGravity;
    if (MR::isActionEnd(this)) {
        MR::emitItemStomp(this);
        MR::tryHitReactionDead(this);
        kill();
    }
}

void Grapyon::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
        MR::startAction(mHead, "BlowDown");
        MR::invalidateClipping(this);
    }
    mUp = -mGravity;
    MR::attenuateVelocity(this, 0.99f);
    if (!MR::isOnGround(this)) {
        MR::addVelocityToGravity(this, 1.2f);
    }
    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.2f, 0.7f);
    }
    if ((MR::isBindedGround(this) && MR::isGreaterStep(this, 15)) || MR::isActionEnd(this)) {
        MR::emitItemHit(this);
        MR::tryHitReactionDead(this);
        kill();
    }
}

void Grapyon::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mAnimScaleCtrl);
    MR::fn_8000F9A0(this, &mUp, &mFront);
    MR::calcFrontVec(&mFront, this);
    MtxPtr pHeadMtx = MR::getJointMtx(this, "Head");
    TVec3f headPos(pHeadMtx[0][3], pHeadMtx[1][3], pHeadMtx[2][3]);
    TVec3f up(mUp);
    if (isNerve(NRV(BlowDown))) {
        MR::extractMtxYDir(pHeadMtx, &up);
    }
    mMtx.identity();
    TVec3f scaledUp = up * mScale;
    MR::makeMtxUpFrontPos(&mMtx, scaledUp, mFront, headPos);
}

// NONMATCHING: target keeps the compare + branch of empty `if (sendMsg...) {}` bodies, MWCC drops them from our output
void Grapyon::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayerOrRide(pReceiver)) {
        if (!isNerve(NRV(Down))) {
            if (!MR::sendMsgPush(pReceiver, pSender)) {
            }
        }
    } else {
        if (isNerve(NRV(SupportFreeze))) {
            if (MR::isEqualSensor(pSender, this, "SupportFreezeAttack")) {
                if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                }
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (MR::isSensorEnemyAttack(pSender)) {
            if (isNormalNerve()) {
                if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    tryAttackHit();
                    return;
                }
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
            if (!isNerve(NRV(Down))) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    }
}

bool Grapyon::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    if (MR::isMsgFireBallAttack(msg)) {
        return tryBlowDown(pSender, pReceiver);
    }
    if (MR::isMsgDrillAttack(msg)) {
        return tryBlowDown(pSender, pReceiver);
    }
    if (MR::isMsgJetTurtleAttack(msg)) {
        return tryBlowDown(pSender, pReceiver);
    }
    if (MR::isMsgInvincibleAttack(msg)) {
        return tryBlowDown(pSender, pReceiver);
    }
    if (MR::isSensorEnemy(pReceiver)) {
        if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerTrample(msg)) {
            return tryDown();
        }
        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
            return tryGuard(pSender, pReceiver);
        }
    }
    return false;
}

bool Grapyon::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver)) {
        if (MR::isMsgEnemyAttackElectric(msg)) {
            return tryElectric();
        }
        return false;
    }
    return false;
}

bool Grapyon::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) && !MR::isSensorPlayerOrRide(pSender)) {
        MR::addVelocityFromPush(this, 3.0f, pSender, pReceiver);
    }
    return true;
}

void Grapyon::calcMove() {
    if (!MR::isOnGround(this)) {
        MR::addVelocityToGravity(this, 1.2f);
    }
    MR::attenuateVelocity(this, 0.99f);
    f32 rate = isJumpStartNerve() ? 0.99f : 0.85f;
    MR::attenuateVelocityExceptDirection(this, mGravity, rate);
    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.2f, 0.7f);
    }
}

bool Grapyon::isNormalNerve() const {
    bool result = false;
    if (isNerve(NRV(Wait)) || isNerve(NRV(Jump)) || isNerve(NRV(Fall)) || isNerve(NRV(Land)) || isNerve(NRV(Guard)) ||
        isNerve(NRV(SupportFreeze))) {
        result = true;
    }
    return result;
}

bool Grapyon::isSwitchChanged() const {
    if (MR::isValidSwitchA(this)) {
        return mSwitchState != MR::isOnSwitchA(this);
    }
    return false;
}

bool Grapyon::isJumpStartNerve() const {
    if (isNerve(NRV(Jump)) && MR::isLessStep(this, 36)) {
        return true;
    }
    return false;
}

bool Grapyon::trySwitchFall() {
    if (isSwitchChanged()) {
        setNerve(NRV(Fall));
        return true;
    }
    return false;
}

void Grapyon::updateHeadBtp() {
    bool isChance = mDirection < 0.0f;
    if (isChance) {
        MR::startBtp(mHead, "Chance");
    } else {
        MR::startBtp(mHead, "Normal");
    }
}

bool Grapyon::tryGuard(HitSensor* pSender, HitSensor* pReceiver) {
    bool ok = false;
    if (isNerve(NRV(Wait)) || isNerve(NRV(Jump))) {
        ok = true;
    }
    if (ok) {
        MR::zeroVelocity(this);
        MR::addVelocityFromPushHorizon(this, 30.0f, pSender, pReceiver);
        setNerve(NRV(Guard));
        return true;
    }
    return false;
}

bool Grapyon::tryAttackHit() {
    bool ok = false;
    if (isNerve(NRV(Wait)) || isNerve(NRV(Jump)) || isNerve(NRV(Land))) {
        ok = true;
    }
    if (ok) {
        MR::killVelocityHorizon(this);
        setNerve(NRV(AttackHit));
        return true;
    }
    return isNerve(NRV(Fall));
}

bool Grapyon::tryDown() {
    if (isNormalNerve()) {
        bool isChance = mDirection < 0.0f;
        if (isChance) {
            MR::zeroVelocity(this);
            setNerve(NRV(Down));
            return true;
        }
    }
    return false;
}

bool Grapyon::tryElectric() {
    if (isNormalNerve()) {
        MR::zeroVelocity(this);
        setNerve(NRV(Dead));
        return true;
    }
    return false;
}

bool Grapyon::tryBlowDown(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNormalNerve()) {
        MR::zeroVelocity(this);
        MR::setVelocityBlowAttack(this, pSender, pReceiver, 19.0f, 26.0f, 4);
        setNerve(NRV(BlowDown));
        return true;
    }
    return false;
}

Grapyon::~Grapyon() {
}

const f32 cGrapyonFallAccel = 1.2f;
