#include "Game/Boss/BossStinkBugBomb.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace NrvBossStinkBugBomb {
    NEW_NERVE(BossStinkBugBombNrvWait, BossStinkBugBomb, Wait);
    NEW_NERVE(BossStinkBugBombNrvExplosion, BossStinkBugBomb, Explosion);
    NEW_NERVE(BossStinkBugBombNrvBindStarPointer, BossStinkBugBomb, BindStarPointer);
};  // namespace NrvBossStinkBugBomb

BossStinkBugBomb::BossStinkBugBomb(const char* pName) : LiveActor(pName), mPoseQuat(0.0f, 0.0f, 0.0f, 1.0f), mAngularVelocity(0.0f, 0.0f, 0.0f) {
}

void BossStinkBugBomb::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "BossStinkBugBomb", false);
    MR::onCalcGravity(this);
    MR::invalidateHitSensor(this, "Bomb");
    initNerve(GET_NERVE(BossStinkBugBomb, BossStinkBugBombNrvWait), 1);
    MR::initActorState(this, new WalkerStateBindStarPointer(this, nullptr), GET_NERVE(BossStinkBugBomb, BossStinkBugBombNrvBindStarPointer), "SupportFreeze");
    MR::invalidateClipping(this);
    makeActorDead();
}

void BossStinkBugBomb::control() {
    mAngularVelocity *= 0.96f;
    MR::rotateQuatMoment(&mPoseQuat, mAngularVelocity);
}

void BossStinkBugBomb::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPoseQuat);
    MR::setBaseScale(this, mScale);
}

void BossStinkBugBomb::start(const TVec3f& startPos, const TVec3f& startVelocity) {
    mPosition.set(startPos);
    MR::showModel(this);

    MR::validateHitSensor(this, "Body");
    MR::invalidateHitSensor(this, "Bomb");

    setNerve(GET_NERVE(BossStinkBugBomb, BossStinkBugBombNrvWait));
    appear();
    mVelocity.set(startVelocity);
}

void BossStinkBugBomb::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        MR::sendMsgEnemyAttackExplosion(pReceiver, pSender);
    } else if (MR::isSensorPlayer(pReceiver)) {
        if (MR::sendMsgEnemyAttackExplosion(pReceiver, pSender)) {
            MR::emitEffect(this, "Explosion");
            kill();
        }
    }
}

bool BossStinkBugBomb::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg) || MR::isMsgSupportTicoSpin(msg)) {
        MR::emitEffect(this, "Explosion");
        kill();
        return true;
    }

    return false;
}

bool BossStinkBugBomb::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    return false;
}

void BossStinkBugBomb::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "Signal");
    }

    MR::addVelocityToGravity(this, 1.2f);
    f32 dot = mGravity.dot(mVelocity);

    if (MR::reboundVelocityFromCollision(this, 0.8f, 5.0f, 1.0f) && dot >= 9.5f) {
        f32 liner = MR::getLinerValueFromMinMax(dot, 10.5f, 30.0f, 0.3f, 1.0f);
        MR::startActionSound(this, "BmBossBugBombGround", 100.0f * liner);
        if (MR::isBindedGroundWater(this)) {
            MR::startActionSound(this, "BmBossBugBombGroundWater", 100.0f * liner);
        }
    }

    if (MR::isLessStep(this, 112)) {
        MR::startActionSound(this, "BmBossBugBombCount1");
    } else {
        MR::startActionSound(this, "BmBossBugBombCount2");
    }

    MR::attenuateVelocity(this, MR::isBinded(this) ? 0.9f : 0.98f);

    if (MR::attachSupportTicoToTarget(this)) {
        setNerve(GET_NERVE(BossStinkBugBomb, BossStinkBugBombNrvBindStarPointer));
    }

    if (MR::isGreaterStep(this, 150)) {
        setNerve(GET_NERVE(BossStinkBugBomb, BossStinkBugBombNrvExplosion));
    }
}

void BossStinkBugBomb::exeExplosion() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);

        MR::validateHitSensor(this, "Bomb");
        MR::invalidateHitSensor(this, "Body");

        MR::emitEffect(this, "Explosion");
        MR::startActionSound(this, "BmBossBugBombBang");

        if (MR::isBindedGroundWater(this)) {
            MR::startActionSound(this, "BmBossBugBombBangWater");
        }

        MR::tryRumblePadAndCameraDistanceStrong(this, 800.0f, 1200.0f, 2000.0f);
        MR::zeroVelocity(this);
    }

    MR::setSensorRadius(this, "Bomb", MR::calcNerveEaseOutValue(this, 10, 0.0f, 280.0f));

    if (MR::isGreaterStep(this, 20)) {
        kill();
    }
}

void BossStinkBugBomb::exeBindStarPointer() {
    if (MR::isFirstStep(this)) MR::zeroVelocity(this);
    MR::updateActorStateAndNextNerve(this, GET_NERVE(BossStinkBugBomb, BossStinkBugBombNrvWait));
}
