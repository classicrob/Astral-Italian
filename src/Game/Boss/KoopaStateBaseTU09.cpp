#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaFinalBattlePos.hpp"
#include "Game/Boss/KoopaStateCommon.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaStateFireAttack {
    NEW_NERVE(KoopaStateFireAttackNrvFireStart, KoopaStateFireAttack, FireStart);
    NEW_NERVE(KoopaStateFireAttackNrvFireLoop, KoopaStateFireAttack, FireLoop);
    NEW_NERVE(KoopaStateFireAttackNrvFireEnd, KoopaStateFireAttack, FireEnd);
};  // namespace NrvKoopaStateFireAttack

KoopaStateFireAttack::KoopaStateFireAttack(KoopaBattle* pHost)
    : ActorStateBase< KoopaBattle >("クッパ炎アタック", pHost), mFireBalls(nullptr), mFireCount(1), mIsLoopStarted(false) {
    DeriveActorGroup< KoopaFireBall >* pGroup = new DeriveActorGroup< KoopaFireBall >("火炎管理", 32);
    mFireBalls = pGroup;
    for (s32 i = 0; i < pGroup->mMaxCount; i++) {
        KoopaFireBall* pBall = new KoopaFireBall("クッパ火炎");
        pBall->initWithoutIter();
        pGroup->registerActor(pBall);
    }
    initNerve(&NrvKoopaStateFireAttack::KoopaStateFireAttackNrvFireStart::sInstance, 0);
}

void KoopaStateFireAttack::appear() {
    mIsDead = false;
    setNerve(&NrvKoopaStateFireAttack::KoopaStateFireAttackNrvFireStart::sInstance);
    MR::setKoopaBattleNo0();
}

void KoopaStateFireAttack::kill() {
    MR::zeroVelocity(mHost);
    mIsDead = true;
    MR::setKoopaBattleNo0();
    MR::setKoopaBattleVolumeScaleDownRate(mHost, 10);
}

void KoopaStateFireAttack::setFireCount(s32 count) {
    mFireCount = count;
}

void KoopaStateFireAttack::exeFireStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "FireStart");
    }
    TVec3f base;
    MR::getKoopaBattleBasePos(&base);
    TVec3f toActor = mHost->mPosition - base;
    MR::normalizeOrZero(&toActor);
    TVec3f toTarget;
    MR::calcVecToTargetPosH(&toTarget, mHost, *MR::getPlayerPos(), &toActor);
    MR::turnQuatYDirRad(&mHost->mQuatA, mHost->mQuatA, toTarget, 0.04363323f);
    TVec3f negActor = -toActor;
    MR::turnQuatZDirRad(&mHost->mQuatA, mHost->mQuatA, negActor, 0.10471976f);
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvKoopaStateFireAttack::KoopaStateFireAttackNrvFireLoop::sInstance);
    }
}

void KoopaStateFireAttack::exeFireLoop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "FireLoopStart");
        MR::setKoopaBattleVolumeScaleUpRate(mHost, 60);
        MR::setKoopaBattleNo1();
        mIsLoopStarted = false;
    }
    if (!mIsLoopStarted && MR::isActionEnd(mHost)) {
        MR::startAction(mHost, "FireLoop");
        mIsLoopStarted = true;
    }
    moveAroundArena(MR::calcNerveEaseInOutRate(this, 20));
    if (MR::isIntervalStep(this, 10)) {
        KoopaFireBall* pBall = mFireBalls->getDeadMember();
        if (pBall != nullptr) {
            pBall->shot(mFireCount);
        }
    }
    if (MR::isGreaterStep(this, 360)) {
        MR::zeroVelocity(mHost);
        setNerve(&NrvKoopaStateFireAttack::KoopaStateFireAttackNrvFireEnd::sInstance);
    }
}

void KoopaStateFireAttack::exeFireEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "FireEnd");
        MR::setKoopaBattleVolumeScaleDownRate(mHost, 60);
        MR::setKoopaBattleNo0();
    }
    moveAroundArena(1.0f - MR::calcNerveEaseInOutRate(this, 60));
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

// NONMATCHING: target calls TVec3f::operator*(f32) out of line (bl __ml__) here; ours expands it inline. Calls/constants agree.
void KoopaStateFireAttack::moveAroundArena(f32 speed) {
    TVec3f base;
    MR::getKoopaBattleBasePos(&base);
    TVec3f toActor = mHost->mPosition - base;
    MR::normalizeOrZero(&toActor);
    TVec3f toPlayerDir;
    MR::normalizeOrZero(&toPlayerDir, *MR::getPlayerPos() - base);
    MR::setPosition(mHost, base + toActor * 4000.0f);
    f32 angle = 57.29578f * MR::diffAngleAbs(toActor, toPlayerDir);
    f32 rate = MR::normalize(angle, 70.0f, 110.0f);
    TVec3f toTarget;
    MR::calcVecToTargetPosH(&toTarget, mHost, *MR::getPlayerPos(), &toActor);
    MR::turnQuatYDirRad(&mHost->mQuatA, mHost->mQuatA, toTarget, 0.01745329f * (2.5f * rate));
    MR::turnQuatZDirRad(&mHost->mQuatA, mHost->mQuatA, -toActor, 0.10471976f);
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    JMAVECScaleAdd(&toActor, &up, &up, -toActor.dot(up));
    MR::normalizeOrZero(&up);
    MR::fn_80010CB0(mHost, up, 4.5f * speed);
    MR::turnVecToPlane(&mHost->mVelocity, mHost->mVelocity, toActor);
    MR::attenuateVelocity(mHost, 0.95f);
}

KoopaStateFireAttack::~KoopaStateFireAttack() {
}
