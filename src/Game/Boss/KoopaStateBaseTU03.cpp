#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaStateDamage {
    NEW_NERVE(KoopaStateDamageNrvDamage, KoopaStateDamage, Damage);
    NEW_NERVE(KoopaStateDamageNrvDamageEnd, KoopaStateDamage, DamageEnd);
};  // namespace NrvKoopaStateDamage

KoopaStateDamage::KoopaStateDamage(KoopaBattle* pHost, s32 param)
    : ActorStateBase< KoopaBattle >("クッパダメージ", pHost), mDamageType(0), mParam(param), mIsFinal(false) {
    initNerve(&NrvKoopaStateDamage::KoopaStateDamageNrvDamage::sInstance, 0);
}

void KoopaStateDamage::appear() {
    mIsDead = false;
    MR::endKoopaBattleFaceCtrl(mHost, 0);
    setNerve(&NrvKoopaStateDamage::KoopaStateDamageNrvDamage::sInstance);
}

void KoopaStateDamage::setDamageType(HitSensor* pSensor) {
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

void KoopaStateDamage::setFinal(bool isFinal) {
    mIsFinal = isFinal;
}

void KoopaStateDamage::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::stopScene(15);
        const char* pAction;
        switch (mDamageType) {
        case 0:
            pAction = "DamageAbove";
            break;
        case 1:
            pAction = "DamageBelow";
            break;
        case 2:
            pAction = "DamageLeft";
            break;
        case 3:
            pAction = "DamageRight";
            break;
        default:
            pAction = "DamageAbove";
            break;
        }
        MR::startAction(mHost, pAction);
        MR::sendMsgKoopaBB();
    }
    if (mIsFinal) {
        if (mParam == 0) {
            MR::startActionSound(mHost, "PgDamageToPowerUp", -1, -1, -1);
        } else {
            MR::startActionSound(mHost, "PgDown", -1, -1, -1);
        }
    } else if (static_cast< u32 >(mDamageType) <= 1) {
        MR::startActionSound(mHost, "PgDamageBelowNAbove", -1, -1, -1);
    } else {
        MR::startActionSound(mHost, "PgDamageLeftNRight", -1, -1, -1);
    }
    if (mIsFinal) {
        if (MR::isGreaterStep(this, 30)) {
            kill();
        }
    } else if (MR::isActionEnd(mHost)) {
        setNerve(&NrvKoopaStateDamage::KoopaStateDamageNrvDamageEnd::sInstance);
    }
}

void KoopaStateDamage::exeDamageEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "DamageEnd");
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

KoopaStateDamage::~KoopaStateDamage() {
}
