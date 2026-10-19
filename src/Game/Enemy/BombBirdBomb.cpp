#include "Game/Enemy/BombBirdBomb.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"


namespace {
    static const f32 sGroundFric = 1.0f;
    // static const f32 sAirFric = _;
    static const f32 sMomentFric = 0.96f;
    static const f32 sGravityAccel = 1.2f;
    static const f32 sReboundRate = 0.4f;
    static const f32 sReboundSpeedMin = 5.0f;
    static const s32 sWaitTime = 150;
    static const s32 sExplosionMaxTime = 10;
    static const s32 sExplosionTime = 20;
    static const f32 sExplosionRadius = 280.0f;
    // static const f32 sBombBoundNoteOffVel = _;
    // static const f32 sBombBoundVolMinVel = _;
    // static const f32 sBombBoundVolMaxVel = _;
    // static const f32 hShiftingSandVel = _;
};  // namespace

namespace NrvBombBirdBomb {
    NEW_NERVE(HostTypeNrvHold, BombBirdBomb, Hold);
    NEW_NERVE(HostTypeNrvWait, BombBirdBomb, Wait);
    NEW_NERVE(HostTypeNrvExplosion, BombBirdBomb, Explosion);
    NEW_NERVE(HostTypeNrvBindStarPointer, BombBirdBomb, BindStarPointer);
};  // namespace NrvBombBirdBomb

BombBirdBomb::BombBirdBomb(const char* pName) : LiveActor(pName), mBindStarPtr(), mPoseQuat(0.0f, 0.0f, 0.0f, 1.0f), mMoment(0.0f, 0.0f, 0.0f), mSpinCooldown(0) {
}

void BombBirdBomb::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "BombBirdBomb", false);
    MR::invalidateHitSensor(this, "bomb");
    MR::onCalcGravity(this);
    initNerve(GET_NERVE(BombBirdBomb, HostTypeNrvHold), 1);
    mBindStarPtr = new WalkerStateBindStarPointer(this, nullptr);
    MR::initActorState(this, mBindStarPtr, GET_NERVE(BombBirdBomb, HostTypeNrvBindStarPointer), "SupportFreeze");
    MR::invalidateClipping(this);
    makeActorDead();
}

void BombBirdBomb::kill() {
    LiveActor::kill();
    MR::deleteEffect(this, "PointerTouch");
    MR::deleteEffect(this, "LocusSmoke");
}

void BombBirdBomb::control() {
    mMoment.mult(::sMomentFric);
    MR::rotateQuatMoment(&mPoseQuat, mMoment);
}

void BombBirdBomb::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPoseQuat);
    MR::setBaseScale(this, mScale);
}

void BombBirdBomb::hold(const TVec3f& rPos) {
    MR::resetPosition(this, rPos);
    appear();
    MR::showModel(this);
    setNerve(GET_NERVE(BombBirdBomb, HostTypeNrvHold));
}

void BombBirdBomb::start(const TVec3f& rTrans, const TVec3f& rVelocity) {
    MR::setPosition(this, rTrans);
    MR::showModel(this);
    MR::validateHitSensor(this, "Body");
    MR::invalidateHitSensor(this, "bomb");
    setNerve(GET_NERVE(BombBirdBomb, HostTypeNrvWait));
    mVelocity.set(rVelocity);
    MR::emitEffect(this, "LocusSmoke");
    mExplodeCountdown = 150;
    mSpinCooldown = 0;
}

void BombBirdBomb::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(BombBirdBomb, HostTypeNrvExplosion))) {
        if (MR::isSensorEnemyAttack(pSender)) {
            MR::sendMsgEnemyAttackExplosion(pReceiver, pSender);
            return;
        }
        return;
    }
    if (MR::isSensorPlayerOrYoshi(pReceiver) || MR::isSensorEnemy(pReceiver)) {
        if (isNerve(GET_NERVE(BombBirdBomb, HostTypeNrvWait))) {
            if (MR::sendMsgEnemyAttackExplosion(pReceiver, pSender)) {
                MR::emitEffect(this, "Explosion");
                MR::startActionSound(this, "EmBombExplosion");
                kill();
                return;
            }
            MR::sendMsgPush(pReceiver, pSender);
        }
        if (isNerve(GET_NERVE(BombBirdBomb, HostTypeNrvBindStarPointer))) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool BombBirdBomb::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    return false;
}

bool BombBirdBomb::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (MR::isMsgFireBallAttack(msg)) {
        setNerve(GET_NERVE(BombBirdBomb, HostTypeNrvExplosion));
        return true;
    }

    if (MR::isMsgSupportTicoSpin(msg) && mSpinCooldown == 0 && !isNerve(GET_NERVE(BombBirdBomb, HostTypeNrvExplosion))) {
        MR::addVelocitySeparateHV(this, 18.0f, 18.0f, pSender, pReceiver, 4);
        mSpinCooldown = 30;
        setNerve(GET_NERVE(BombBirdBomb, HostTypeNrvWait));
        return true;
    }

    return false;
}

bool BombBirdBomb::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(BombBirdBomb, HostTypeNrvExplosion))) {
        return false;
    }

    if (MR::isMsgExplosionAttack(msg)) {
        setNerve(GET_NERVE(BombBirdBomb, HostTypeNrvExplosion));
        return true;
    }

    return false;
}

void BombBirdBomb::exeBindStarPointer() {
    MR::updateActorStateAndNextNerve(this, GET_NERVE(BombBirdBomb, HostTypeNrvWait));
    mSpinCooldown = mSpinCooldown - 1;
    if (mSpinCooldown < 0) {
        mSpinCooldown = 0;
    }
}

void BombBirdBomb::exeHold() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Normal");
    }

    mVelocity.zero();
}

void BombBirdBomb::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Signal");
    }

    f32 dotP = MR::calcGravitySpeed(this);
    TVec3f velocity;
    velocity.set(0.0f);

    if (MR::calcVelocityAreaOrRailMoveOnGround(&velocity, this)) {
        mVelocity.set(velocity * ::sGroundFric);
        MR::addVelocityToGravity(this, ::sGravityAccel);

        if (dotP >= 5.5f) {
            MR::startActionSound(this, "EmBombbirdbombLandSand");
        }
    } else {
        MR::addVelocityToGravity(this, ::sGravityAccel);

        if (MR::reboundVelocityFromCollision(this, ::sReboundRate, ::sReboundSpeedMin, 1.0f) && dotP >= 5.5f) {
            MR::startActionSound(this, "EmBombbirdbombLand");
        }
    }

    if (mSpinCooldown == 0 && mBindStarPtr->tryStartPointBind()) {
        setNerve(GET_NERVE(BombBirdBomb, HostTypeNrvBindStarPointer));
        return;
    }
    if (--mSpinCooldown < 0) {
        mSpinCooldown = 0;
    }
    if (--mExplodeCountdown < 0) {
        setNerve(GET_NERVE(BombBirdBomb, HostTypeNrvExplosion));
    }
}

void BombBirdBomb::exeExplosion() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        MR::validateHitSensor(this, "bomb");
        MR::invalidateHitSensor(this, "Body");
        MR::emitEffect(this, "Explosion");
        MR::startActionSound(this, "EmBombExplosion");
        MR::tryRumblePadAndCameraDistanceStrong(this, 800.0f, 1200.0f, 2000.0f);
        mVelocity.zero();
    }

    MR::setSensorRadius(this, "bomb", MR::calcNerveEaseOutValue(this, ::sExplosionMaxTime, 0.0f, ::sExplosionRadius));

    if (MR::isGreaterStep(this, ::sExplosionTime)) {
        MR::invalidateHitSensor(this, "bomb");
        kill();
    }
}
