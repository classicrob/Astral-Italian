#include "Game/Util.hpp"
#include "Game/Enemy/GliderShooterBullet.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Enemy/GliderShooterBulletHolder.hpp"

namespace NrvGliderShooterBullet {
    NEW_NERVE(GliderShooterBulletNrvFly, GliderShooterBullet, Fly);
    NEW_NERVE(GliderShooterBulletNrvFreeze, GliderShooterBullet, Freeze);
};  // namespace NrvGliderShooterBullet

GliderShooterBullet::GliderShooterBullet() : LiveActor("グライダー狙撃手の弾") {
    mUnk90.x = 0.0f;
    mUnk90.y = 0.0f;
    mUnk90.z = 1.0f;
    mUnk9C.x = 0.0f;
    mUnk9C.y = 0.0f;
    mUnk9C.z = 0.0f;
    mAnimScaleController = nullptr;
    mBindState = nullptr;
}

void GliderShooterBullet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "GliderShooterBullet", false);
    MR::addEffectHitNormal(this, "Hit");
    initNerve(&NrvGliderShooterBullet::GliderShooterBulletNrvFly::sInstance, 1);
    mAnimScaleController = new AnimScaleController(nullptr);
    mBindState = new WalkerStateBindStarPointer(this, mAnimScaleController);
    MR::initActorState(this, mBindState, &NrvGliderShooterBullet::GliderShooterBulletNrvFreeze::sInstance, "SupportFreeze");
    makeActorDead();
}

void GliderShooterBullet::appear() {
    mUnk9C.set(mVelocity);
    mUnk90.set(mVelocity);
    MR::normalize(&mUnk90);
    LiveActor::appear();
    MR::emitEffect(this, "Blur");
    MR::emitEffect(this, "Glow");
    setNerve(&NrvGliderShooterBullet::GliderShooterBulletNrvFly::sInstance);
}

void GliderShooterBullet::kill() {
    MR::deleteEffect(this, "Blur");
    MR::deleteEffect(this, "Glow");
    LiveActor::kill();
}

void GliderShooterBullet::exeFly() {
    if (MR::isGreaterStep(this, 60) && MR::isBinded(this)) {
        MR::emitEffectHit(this, mPosition, "Hit");
        kill();
        return;
    }
    if (MR::isGreaterStep(this, 1200)) {
        kill();
    }
}

void GliderShooterBullet::exeFreeze() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffect(this, "Blur");
    }
    if (MR::updateActorState(this)) {
        MR::emitEffect(this, "Blur");
        mVelocity.set(mUnk9C);
        setNerve(&NrvGliderShooterBullet::GliderShooterBulletNrvFly::sInstance);
    }
}

void GliderShooterBullet::control() {
    mAnimScaleController->updateNerve();
    if (isNerve(&NrvGliderShooterBullet::GliderShooterBulletNrvFly::sInstance) && mBindState->tryStartPointBind()) {
        setNerve(&NrvGliderShooterBullet::GliderShooterBulletNrvFreeze::sInstance);
    }
}

void GliderShooterBullet::startClipped() {
    LiveActor::startClipped();
    makeActorDead();
}

void GliderShooterBullet::calcAndSetBaseMtx() {
    MR::fn_8000F880(this, &mUnk90);
}

void GliderShooterBullet::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
        MR::emitEffectHit(this, mPosition, "Hit");
        kill();
    }
}

bool GliderShooterBullet::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgEnemyAttack(msg) && isNerve(&NrvGliderShooterBullet::GliderShooterBulletNrvFreeze::sInstance)) {
        MR::emitEffectHit(this, mPosition, "Hit");
        kill();
        return true;
    }
    return false;
}

GliderShooterBulletHolder::GliderShooterBulletHolder() : LiveActorGroup("グライダー狙撃手の弾の保持", 0x40) {
}

void GliderShooterBulletHolder::init(const JMapInfoIter& rIter) {
    for (s32 i = 0; i < 0x40; i++) {
        GliderShooterBullet* pBullet = new GliderShooterBullet();
        pBullet->initWithoutIter();
        registerActor(pBullet);
    }
}
