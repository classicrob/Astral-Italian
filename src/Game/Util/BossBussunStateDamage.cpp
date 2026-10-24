#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossBussunStateDamage {
    NEW_NERVE(BossBussunStateDamageNrvDamage, BossBussunStateDamage, Damage);
};  // namespace NrvBossBussunStateDamage

BossBussunStateDamage::BossBussunStateDamage(BossBussun* pHost) : ActorStateBase< BossBussun >("ボスブッスンダメージ", pHost) {
    mPosA.set(0.0f, 0.0f, 0.0f);
    mPosB.set(0.0f, 0.0f, 0.0f);
    mPosC.set(0.0f, 0.0f, 0.0f);
    initNerve(GET_NERVE(BossBussunStateDamage, BossBussunStateDamageNrvDamage), 0);
}

void BossBussunStateDamage::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(BossBussunStateDamage, BossBussunStateDamageNrvDamage));
    mHost->resetAnimScale();
}

void BossBussunStateDamage::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Damage");
        MR::zeroVelocity(mHost);
        mHost->mSequencer->endCamera();
        MR::startMultiActorCameraTargetSelf(mHost, mHost->mCameraInfo, "Damage", 0);
        MR::stopScene(10);
        updateDamagePos();
    }
    if (MR::isStep(this, 1)) {
        MR::emitEffectHit(mHost, mHost->mPosition, nullptr);
    }
    TVec3f* hostPos = &mHost->mPosition;
    f32 rate = MR::calcNerveRate(this, 0x28);
    MR::vecBlend(hostPos, mPosA, mPosB, rate);
    if (MR::isGreaterEqualStep(this, 1) && MR::isLessStep(this, 0x28)) {
        TVec3f axis(0.0f, 1.0f, 0.0f);
        MR::turnVecToVecDegree(&mHost->mFrontDir, mHost->mFrontDir, mPosC, 180.0f, axis);
    }
    if (MR::isStep(this, 0x32)) {
        MR::endMultiActorCamera(mHost, mHost->mCameraInfo, "Damage", false, -1);
        mHost->mSequencer->startCamera();
    }
    if (MR::isGreaterStep(this, 0x78)) {
        kill();
    }
}

void BossBussunStateDamage::updateDamagePos() {
    TVec3f dirFromHome;
    MR::calcVecToTargetPosH(&dirFromHome, mHost, mHost->mHomePos, &mHost->mUpVec);
    if (MR::isNearZero(dirFromHome, 0.001f)) {
        dirFromHome.set(mHost->mFrontDir);
    }
    dirFromHome.negate();
    dirFromHome.scale(800.0f);

    TVec3f target(mHost->mHomePos);
    target.add(dirFromHome);

    TVec3f up;
    MR::calcUpVec(&up, mHost);
    TVec3f killed;
    TVec3f upOffset(up);
    upOffset.scale(400.0f);
    target.add(upOffset);

    mPosA.set(mHost->mPosition);

    TVec3f down(up);
    down.scale(-800.0f);
    MR::getFirstPolyOnLineToMap(&mPosB, nullptr, target, down);

    killed.killElement2(mPosA - mPosB, up);
    if (!MR::normalizeOrZero(&killed)) {
        mPosC.set(killed);
        return;
    }
    mPosC.set(mHost->mFrontDir);
}

BossBussunStateDamage::~BossBussunStateDamage() {
}
