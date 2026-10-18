#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStateCommon.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util.hpp"

namespace {
    // arena positions the shell rolls to (local to the battle pose)
    TVec3f sRollPositions[4] = {
        TVec3f(-3000.0f, 3000.0f, -8000.0f),
        TVec3f(3000.0f, -3000.0f, -8000.0f),
        TVec3f(-3000.0f, -3000.0f, -8000.0f),
        TVec3f(3000.0f, 3000.0f, -8000.0f),
    };
    s32 sRollPositionNum = 4;
};  // namespace

namespace NrvKoopaStateLastAttack {
    NEW_NERVE(KoopaStateLastAttackNrvChaseRoll, KoopaStateLastAttack, ChaseRoll);
    NEW_NERVE(KoopaStateLastAttackNrvAttackSign, KoopaStateLastAttack, AttackSign);
    NEW_NERVE(KoopaStateLastAttackNrvMoveLoop, KoopaStateLastAttack, MoveLoop);
    NEW_NERVE(KoopaStateLastAttackNrvAttack, KoopaStateLastAttack, Attack);
    NEW_NERVE(KoopaStateLastAttackNrvAttackEnd, KoopaStateLastAttack, AttackEnd);
    NEW_NERVE(KoopaStateLastAttackNrvMoveEndRoll, KoopaStateLastAttack, MoveEndRoll);
    NEW_NERVE(KoopaStateLastAttackNrvMoveEndLeft, KoopaStateLastAttack, MoveEndLeft);
};  // namespace NrvKoopaStateLastAttack

KoopaStateLastAttack::KoopaStateLastAttack(KoopaBattle* pHost)
    : ActorStateBase< KoopaBattle >("クッパ最終攻撃", pHost), mQuatA(0.0f, 0.0f, 0.0f, 1.0f), mQuatB(0.0f, 0.0f, 0.0f, 1.0f) {
    mStartPos.x = 0.0f;
    mStartPos.y = 0.0f;
    mStartPos.z = 0.0f;
    mTargetPos.x = 0.0f;
    mTargetPos.y = 0.0f;
    mTargetPos.z = 0.0f;
    mTargetIdx = 0;
    mIsChaseRoll = false;
    initNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvAttackSign::sInstance, 0);
}

void KoopaStateLastAttack::appear() {
    mIsDead = false;
    if (mIsChaseRoll) {
        setNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvChaseRoll::sInstance);
    } else {
        setNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvAttackSign::sInstance);
    }
}

void KoopaStateLastAttack::kill() {
    mIsDead = true;
}

void KoopaStateLastAttack::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvMoveLoop::sInstance) ||
        isNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvAttack::sInstance)) {
        if (!(MR::isSensorPlayer(pReceiver) && MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender))) {
            if (MR::sendMsgKoopaPunchAttack(pReceiver, pSender)) {
            }
        }
    }
}

void KoopaStateLastAttack::exeChaseRoll() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "ShellRotate");
        mStartPos.set< f32 >(mHost->mPosition);
        MR::startKoopaBattleFaceCtrl(mHost, 30);
        MR::calcKoopaBattleWorldPos(&mTargetPos, sRollPositions[mTargetIdx]);
        mQuatA.set< f32 >(mHost->mQuatA);
        mTargetIdx++;
        if (mTargetIdx >= sRollPositionNum) {
            mTargetIdx = 0;
        }
    }
    MR::vecBlend(&mHost->mPosition, mStartPos, mTargetPos, MR::calcNerveRate(this, 75));
    MR::startSoundInCameraDistance(mHost, "LastChaseRoll", -1, -1, 31000.0f);
    TVec3f toPlayer;
    MR::calcVecToPlayer(&toPlayer, mHost);
    MR::turnQuatZDirRad(&mQuatA, mQuatA, toPlayer, 0.034906585f);
    f32 angle = MR::calcNerveValue(this, 75, 0.0f, 1440.0f);
    MR::rotateQuatXAxisDegree(&mHost->mQuatA, mQuatA, angle);
    if (MR::isStep(this, 75)) {
        MR::showModel(mHost);
    }
    if (MR::isGreaterStep(this, 75)) {
        MR::validateKoopaBattleCollision(mHost);
        mHost->getDefenseForm()->kill();
        setNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvMoveEndLeft::sInstance);
    }
}

void KoopaStateLastAttack::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "LastAttackSign");
        MR::startKoopaBattleFaceCtrl(mHost, 30);
        MR::zeroVelocity(mHost);
        mStartPos.set< f32 >(mHost->mPosition);
    }
    if (MR::isLessEqualStep(this, 45)) {
        TVec3f toPlayer;
        MR::calcVecToPlayer(&toPlayer, mHost);
        MR::turnQuatZDirRad(&mHost->mQuatA, mHost->mQuatA, toPlayer, 0.034906585f);
        MR::calcKoopaNearestAttackRockPos(&mTargetPos);
    }
    if (MR::isActionEnd(mHost)) {
        TVec3f dir;
        dir.set< f32 >(mStartPos - mTargetPos);
        MR::normalizeOrZero(&dir);
        mTargetPos.add(mTargetPos, dir * 3000.0f);
        setNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvMoveLoop::sInstance);
    }
}

void KoopaStateLastAttack::exeMoveLoop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "LastAttackMoveLoop");
    }
    TVec3f dir;
    dir.set< f32 >(mTargetPos - mStartPos);
    MR::normalizeOrZero(&dir);
    MR::turnQuatZDirRad(&mHost->mQuatA, mHost->mQuatA, dir, 0.034906585f);
    MR::vecBlend(&mHost->mPosition, mStartPos, mTargetPos, MR::calcNerveValue(this, 45, 0.0f, 0.8f));
    if (MR::isGreaterEqualStep(this, 44)) {
        setNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvAttack::sInstance);
    }
}

void KoopaStateLastAttack::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "LastAttack");
    }
    MR::vecBlend(&mHost->mPosition, mStartPos, mTargetPos, MR::calcNerveEaseOutValue(this, 20, 0.8f, 1.0f));
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvAttackEnd::sInstance);
    }
}

void KoopaStateLastAttack::exeAttackEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "LastAttackEnd");
        MR::endKoopaBattleFaceCtrl(mHost, 30);
        mStartPos.set< f32 >(mHost->mPosition);
        TVec3f front(0.0f, 0.0f, 1.0f);
        mHost->mQuatA.getZDir(front);
        mTargetPos.set< f32 >(mStartPos - front * 2000.0f);
        mQuatA.set< f32 >(mHost->mQuatA);
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 60);
    MR::vecBlend(&mHost->mPosition, mStartPos, mTargetPos, rate);
    KoopaBattle* pHost = mHost;
    TQuat4f* pQuat = &pHost->mQuatA;
    pQuat->set< f32 >(mQuatA);
    pQuat->slerp(pHost->mQuatB, rate);
    if (MR::isActionEnd(mHost) && MR::isGreaterEqualStep(this, 60)) {
        MR::invalidateKoopaBattleCollision(mHost);
        mHost->getDefenseForm()->appear();
        setNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvMoveEndRoll::sInstance);
    }
}

void KoopaStateLastAttack::exeMoveEndRoll() {
    if (MR::isFirstStep(this)) {
        MR::hideModelAndOnCalcAnim(mHost);
        mStartPos.set< f32 >(mHost->mPosition);
        MR::startKoopaBattleFaceCtrl(mHost, 30);
        if (mIsChaseRoll) {
            MR::calcKoopaBattleWorldPos(&mTargetPos, sRollPositions[mTargetIdx]);
            mTargetIdx++;
            if (mTargetIdx >= sRollPositionNum) {
                mTargetIdx = 0;
            }
        } else {
            MR::calcKoopaBattleWorldPos(&mTargetPos, TVec3f(0.0f, 0.0f, -12000.0f));
        }
        mQuatA.set< f32 >(mHost->mQuatA);
        TVec3f dir;
        MR::normalizeOrZero(&dir, mStartPos - mTargetPos);
        MR::turnQuatZDirRate(&mQuatB, mHost->mQuatB, dir, 1.0f);
    }
    MR::vecBlend(&mHost->mPosition, mStartPos, mTargetPos, MR::calcNerveEaseInOutRate(this, 75));
    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    TQuat4f* pQuat = &mHost->mQuatA;
    pQuat->set< f32 >(mQuatA);
    pQuat->slerp(mQuatB, rate);
    f32 angle = MR::calcNerveEaseOutValue(this, 75, 0.0f, -1440.0f);
    MR::rotateQuatXAxisDegree(&mHost->mQuatA, mHost->mQuatA, angle);
    MR::startSoundInCameraDistance(mHost, "LastAttackMoveEndRoll", -1, -1, 31000.0f);
    if (MR::isStep(this, 75) && mIsChaseRoll) {
        MR::showModel(mHost);
    }
    if (MR::isGreaterStep(this, 75)) {
        if (mIsChaseRoll) {
            MR::validateKoopaBattleCollision(mHost);
            mHost->getDefenseForm()->kill();
            setNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvMoveEndLeft::sInstance);
        } else {
            kill();
        }
    }
}

void KoopaStateLastAttack::exeMoveEndLeft() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "LastMoveEndLeft");
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvAttackSign::sInstance);
    }
}

bool KoopaStateLastAttack::isAttacking() const {
    if (isNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvAttackSign::sInstance) ||
        isNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvMoveLoop::sInstance) ||
        isNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvAttack::sInstance) ||
        isNerve(&NrvKoopaStateLastAttack::KoopaStateLastAttackNrvAttackEnd::sInstance)) {
        return true;
    }
    return false;
}

KoopaStateLastAttack::~KoopaStateLastAttack() {
}
