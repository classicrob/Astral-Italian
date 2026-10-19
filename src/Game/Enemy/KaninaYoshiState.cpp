#pragma auto_inline off
#include "Game/Enemy/KaninaYoshiState.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Enemy/Kanina.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/GeometryBindUtil.hpp"

namespace NrvKaninaYoshiState {
    NEW_NERVE(KaninaYoshiStateNrvWait, KaninaYoshiState, Wait);
    NEW_NERVE(KaninaYoshiStateNrvEaten, KaninaYoshiState, Eaten);
    NEW_NERVE(KaninaYoshiStateNrvShot, KaninaYoshiState, Shot);
    NEW_NERVE_ONEND(KaninaYoshiStateNrvFall, KaninaYoshiState, Fall, Fall);
    NEW_NERVE(KaninaYoshiStateNrvTrampled, KaninaYoshiState, Trampled);
    NEW_NERVE(KaninaYoshiStateNrvTongue, KaninaYoshiState, Tongue);
}
namespace {
    TVec3f sForwardTangent(0.0f, 0.0f, 0.0f);
    TVec3f sPeakTangent(0.0f, 0.0f, 0.0f);
    TVec3f sReturnTangent(0.0f, 0.0f, 2000.0f);

    LiveActor* createClaw(Kanina* host) NO_INLINE {
        LiveActor* claw = MR::createDemoObj("カニーナのツメ", "KaninaClaw", host->getBaseMtx());
        claw->initWithoutIter();
        claw->makeActorDead();
        return claw;
    }
}

KaninaYoshiState::KaninaYoshiState(Kanina* host, bool shotMode)
    : ActorStateBase<Kanina>("ツメ状態", host), mShotMode(shotMode), mClaw(NULL), mHit(false), mLocalVelocity(TVec3f(0.0f)),
      mFront(0.0f, 0.0f, 1.0f), mStopMovement(false), mArcHeight(0.0f), mArcStep(0) {
    initNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvWait::sInstance, 0);
    mClaw = createClaw(mHost);
    mShotMtx.identity();
}

void KaninaYoshiState::appear() {
    mIsDead = false;
    MR::deleteEffectAll(mHost);
    setNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvEaten::sInstance);
}

void KaninaYoshiState::kill() {
    MR::validateClipping(mHost);
    MR::tryHitReactionDead(mClaw);
    mClaw->kill();
    mIsDead = true;
}

// NONMATCHING: equivalent initial-state tests use a different branch arrangement.
void KaninaYoshiState::control() {
    if (isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvWait::sInstance) || isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvEaten::sInstance)) return;
    YoshiFunction::tryYoshiProjectile(mHost->mYoshiLockOn);
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(mHost, mFront);
    MR::calcFrontVec(&mFront, mHost);
    if (!mStopMovement) {
        TVec3f velocity(0.0f);
        PSMTXMultVecSR(mHost->getBaseMtx(), &mLocalVelocity, &velocity);
        MR::setVelocity(mHost, velocity);
    } else {
        MR::setVelocity(mHost, TVec3f(0.0f));
    }
}

void KaninaYoshiState::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if ((isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvShot::sInstance) || isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvFall::sInstance)) && !mHit) {
        if (MR::isSensorPlayerOrRide(receiver) && !MR::isSensorGhost(receiver)) {
            if (MR::sendMsgPush(receiver, sender)) {
                MR::tryRumblePadStrong(this, 0);
                kill();
            }
        } else if (MR::isSensorEnemy(receiver) || MR::isSensorMapObj(receiver) || MR::isSensorNpc(receiver) || MR::isSensorYoshi(receiver)) {
            if (MR::sendArbitraryMsg(8, receiver, sender)) mHit = true;
        }
    }
}

bool KaninaYoshiState::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgPlayerTrample(msg) && (isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvShot::sInstance) || isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvFall::sInstance))) {
        const TVec3f* gravity = &mHost->mGravity;
        mLocalVelocity.scale(gravity->dot(mLocalVelocity), *gravity);
        setNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvTrampled::sInstance);
        return true;
    }
    return false;
}

bool KaninaYoshiState::tryStartTongue() {
    if (!isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvShot::sInstance)) return false;
    setNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvTongue::sInstance);
    MR::zeroVelocity(mHost);
    return true;
}

// NONMATCHING: equivalent termination tests use a different branch arrangement.
bool KaninaYoshiState::tryKill() {
    if (MR::isInDeath(mHost, TVec3f(0.0f, 0.0f, 0.0f))) {
        kill();
        return true;
    }
    if (mShotMode == 0) {
        bool dead = false;
        if (mHit || MR::isGreaterEqualStep(this, 300)) dead = true;
        if (dead) {
            kill();
            return true;
        }
    }
    if (mShotMode == 1 && MR::isBinded(mClaw)) {
        MR::sendMsgToBindedSensor(8, mHost->getSensor("Attack"));
        kill();
        return true;
    }
    return false;
}

bool KaninaYoshiState::receiveMsgYoshi(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgYoshiReleaseEaten(msg) && isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvEaten::sInstance)) {
        setNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvShot::sInstance);
        return true;
    }
    if (MR::isMsgYoshiStartTongue(msg)) return tryStartTongue();
    if (MR::isMsgYoshiLoopTongue(msg) && isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvTongue::sInstance)) return true;
    if (MR::isMsgYoshiStartEaten(msg) && isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvTongue::sInstance)) {
        setNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvEaten::sInstance);
        return true;
    }
    if (MR::isMsgYoshiCancelEaten(msg) && isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvEaten::sInstance)) {
        MR::validateClipping(mHost);
        mClaw->kill();
        mIsDead = true;
        return true;
    }
    return false;
}

bool KaninaYoshiState::isYoshiNerve() const {
    if (mIsDead) return false;
    if (isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvEaten::sInstance)) return true;
    if (isNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvShot::sInstance) && MR::isLessStep(this, 20)) return true;
    return false;
}

// NONMATCHING: Euler extraction and vector construction use different temporary slots.
void KaninaYoshiState::followYoshiMouth() {
    TPos3f mtx;
    MR::calcYoshiTongueTipMtx(&mtx);
    mHost->mPosition.set<f32>(mtx.mMtx[0][3], mtx.mMtx[1][3], mtx.mMtx[2][3]);
    mtx.getEuler(mHost->mRotation);
    MR::setRotation(mHost, mHost->mRotation * 57.29578f);
}

void KaninaYoshiState::exeWait() {}

void KaninaYoshiState::exeEaten() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(mHost);
        MR::stopBck(mClaw);
        MR::stopBrk(mClaw);
        MR::deleteEffectAll(mClaw);
        MR::hideModelAndOnCalcAnim(mClaw);
    }
    followYoshiMouth();
}

void KaninaYoshiState::exeShot() {
    if (mShotMode == 0) exeShotStraight();
    else exeShotArc();
}

// NONMATCHING: restored registers are scheduled in a different order.
void KaninaYoshiState::exeShotStraight() {
    if (MR::isFirstStep(this)) {
        startShot();
        MR::getPlayerThrowVec(&mHost->mVelocity);
        MR::attenuateVelocity(mHost, 40.0f);
        mHit = false;
    }
    if (MR::isBinded(mClaw) && MR::isGreaterEqualStep(this, 1)) {
        MR::sendMsgToBindedSensor(8, mHost->getSensor("Attack"));
        mHit = true;
    }
    tryKill();
}

// NONMATCHING: type/model pointer loads use different scheduling.
void KaninaYoshiState::startShot() {
    MR::invalidateClipping(mHost);
    MR::tryRumblePadStrong(mHost, 0);
    MR::hideModelAndOnCalcAnim(mHost);
    MR::deleteEffectAll(mHost);
    followYoshiMouth();
    mClaw->appear();
    MR::showModelIfHidden(mClaw);
    MR::startAction(mClaw, "Spin");
    const char* color = "Blue";
    if (mHost->mType == Kanina::KaninaType_Red) color = "Red";
    MR::startAction(mClaw, color);
    MR::startActionSound(mClaw, "Shot", -1, -1, -1);
    MR::invalidateClipping(mClaw);
}

// NONMATCHING: spline endpoint temporaries and split-rate arithmetic use different scheduling.
void KaninaYoshiState::calcShotPosition(TVec3f* out, f32 progress) {
    f32 split = 0.45f;
    TVec3f start(0.0f, 0.0f, 0.0f), tangent0(0.0f, 0.0f, 0.0f), end(0.0f, 0.0f, mArcHeight), tangent1(0.0f, 0.0f, 0.0f);
    f32 rate;
    if (progress < split) {
        rate = progress / split;
        tangent0 = sForwardTangent;
        tangent1 = sPeakTangent;
    } else {
        rate = 1.0f;
        if (split < 1.0f) rate -= (progress - split) / (1.0f - split);
        tangent0 = sReturnTangent;
        tangent1 = -sPeakTangent;
    }
    HermiteSpline curve(start, tangent0, end, tangent1);
    curve.calcPosition(out, rate);
}

// NONMATCHING: vector difference and scale clamping use different scheduling.
void KaninaYoshiState::exeShotArc() {
    if (MR::isFirstStep(this)) {
        startShot();
        mStopMovement = false;
        mArcStep = 0;
        mArcHeight = 2000.0f;
        MR::getPlayerThrowVec(&mFront);
        mShotMtx.identity();
        MR::calcMtxFromGravityAndZAxis(&mShotMtx, mHost, mFront);
        MR::setBaseTRMtx(mHost, mShotMtx);
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(mClaw)) MR::onBind(mClaw);
    s32 duration = 180;
    f32 rate0 = static_cast<f32>(mArcStep) / static_cast<f32>(duration);
    f32 rate1 = static_cast<f32>(mArcStep + 1) / static_cast<f32>(duration);
    TVec3f first(0.0f), second(0.0f);
    calcShotPosition(&first, rate0);
    calcShotPosition(&second, rate1);
    TVec3f delta(second);
    delta.sub(first);
    mLocalVelocity = delta;
    mClaw->mScale.x = MR::clamp(mClaw->mScale.x + 0.2f, 0.0f, 1.0f);
    MR::setScale(mClaw, TVec3f(mClaw->mScale.x));
    if (!tryKill()) {
        if (mArcStep > 135) {
            setNerve(&NrvKaninaYoshiState::KaninaYoshiStateNrvFall::sInstance);
            return;
        }
        ++mArcStep;
    }
}

void KaninaYoshiState::exeFall() {
    if (MR::isFirstStep(this)) MR::setBckRate(mClaw, 1.0f);
    mLocalVelocity *= 0.98f;
    mLocalVelocity.y += -0.5f;
    if (MR::isBinded(mClaw) || MR::isGreaterStep(this, 300)) {
        if (MR::isBinded(mClaw)) MR::sendMsgToBindedSensor(8, mHost->getSensor("Attack"));
        kill();
    }
}
void KaninaYoshiState::endFall() { MR::setBckRate(mClaw, 1.0f); }

void KaninaYoshiState::exeTrampled() {
    mLocalVelocity.y += -1.0f;
    if (MR::isBinded(mClaw) || MR::isGreaterStep(this, 300)) kill();
}
void KaninaYoshiState::exeTongue() {}
KaninaYoshiState::~KaninaYoshiState() {}
