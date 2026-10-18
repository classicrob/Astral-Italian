#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStateCommon.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaStateLastDamage {
    NEW_NERVE(KoopaStateLastDamageNrvDamage, KoopaStateLastDamage, Damage);
    NEW_NERVE(KoopaStateLastDamageNrvRollEnd, KoopaStateLastDamage, RollEnd);
};  // namespace NrvKoopaStateLastDamage

KoopaStateLastDamage::KoopaStateLastDamage(KoopaBattle* pHost) : ActorStateBase< KoopaBattle >("クッパラストダメージ", pHost) {
    mQuatA.x = 0.0f;
    mQuatA.y = 0.0f;
    mQuatA.z = 0.0f;
    mQuatA.w = 1.0f;
    mStartPos.x = 0.0f;
    mStartPos.y = 0.0f;
    mStartPos.z = 0.0f;
    mTargetPos.x = 0.0f;
    mTargetPos.y = 0.0f;
    mTargetPos.z = 0.0f;
    mStep = 0;
    mDamageType = 0;
    mDuration = 120;
    mIsLongDamage = false;
    mIsFinal = false;
    initNerve(&NrvKoopaStateLastDamage::KoopaStateLastDamageNrvDamage::sInstance, 0);
}

// NONMATCHING: stack slots now match; the target keeps &pos in r31 across the sub/set (one extra mr), ours re-materializes the address
void KoopaStateLastDamage::appear() {
    TVec3f front;
    TVec3f localPos;
    mIsDead = false;
    MR::endKoopaBattleFaceCtrl(mHost, 0);
    setNerve(&NrvKoopaStateLastDamage::KoopaStateLastDamageNrvDamage::sInstance);
    mStep = 0;
    mStartPos.set< f32 >(mHost->mPosition);
    localPos.x = 0.0f;
    localPos.y = 0.0f;
    localPos.z = -20000.0f;
    MR::calcKoopaBattleWorldPos(&mTargetPos, localPos);
    if (mIsFinal) {
        mHost->mQuatB.getZDir(front);
        TVec3f pos = mStartPos - front * 3000.0f;
        mTargetPos.set< f32 >(pos);
    }
}

void KoopaStateLastDamage::moveToTarget() {
    MR::vecBlend(&mHost->mPosition, mStartPos, mTargetPos, MR::getEaseInOutValue(mStep, 0.0f, mDuration, 0.0f, 1.0f));
    mStep++;
}

void KoopaStateLastDamage::setDamageType(HitSensor* pSensor) {
    mDamageType = 0;
    if (MR::isKoopaHeadOrBodySensor(mHost, pSensor)) {
        mDamageType = 0;
    } else if (MR::isKoopaBody2OrLegSensor(mHost, pSensor)) {
        mDamageType = 1;
    } else if (MR::isKoopaArmLSensor(mHost, pSensor)) {
        mDamageType = 2;
    } else if (MR::isKoopaArmRSensor(mHost, pSensor)) {
        mDamageType = 3;
    }
}

void KoopaStateLastDamage::exeDamage() {
    if (MR::isFirstStep(this)) {
        mHost->mQuatA.set< f32 >(mHost->mQuatB);
        mQuatA.set< f32 >(mHost->mQuatA);
        MR::stopScene(15);
        MR::startAction(mHost, "LastDamage");
        MR::sendMsgKoopaBB();
        if (!mIsLongDamage) {
            mDuration = 120;
        } else {
            mDuration = MR::getBckFrameMax(mHost) + 90.0f;
        }
        if (mIsFinal) {
            MR::startActionSound(mHost, "PgLastDown", -1, -1, -1);
        } else {
            MR::startActionSound(mHost, "PgLastDamage", -1, -1, -1);
        }
    }
    if (mIsFinal) {
        if (MR::isGreaterStep(this, 30)) {
            kill();
        }
    } else {
        MR::startSoundInCameraDistance(mHost, "LastDamageRoll", -1, -1, 31000.0f);
        if (MR::isActionEnd(mHost)) {
            MR::invalidateKoopaBattleCollision(mHost);
            mHost->getDefenseForm()->appear();
            setNerve(&NrvKoopaStateLastDamage::KoopaStateLastDamageNrvRollEnd::sInstance);
        }
    }
}

void KoopaStateLastDamage::exeRollEnd() {
    if (MR::isFirstStep(this)) {
        MR::hideModelAndOnCalcAnim(mHost);
    }
    MR::rotateQuatXAxisDegree(&mHost->mQuatA, mQuatA, MR::calcNerveEaseOutValue(this, 90, 0.0f, -1440.0f));
    MR::startSoundInCameraDistance(mHost, "LastDamageRollEnd", -1, -1, 31000.0f);
    if (MR::isGreaterEqualStep(this, 89)) {
        kill();
    }
}

KoopaStateLastDamage::~KoopaStateLastDamage() {
}
