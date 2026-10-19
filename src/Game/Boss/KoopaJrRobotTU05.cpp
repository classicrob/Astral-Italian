#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"


namespace NrvKoopaJrRobotStateDamage {
    class KoopaJrRobotStateDamageNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KoopaJrRobotStateDamageNrvWait sInstance;
    };
    KoopaJrRobotStateDamageNrvWait KoopaJrRobotStateDamageNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(KoopaJrRobotStateDamageNrvDamageLong, KoopaJrRobotStateDamage, DamageLong);
    NEW_NERVE(KoopaJrRobotStateDamageNrvDamageShort, KoopaJrRobotStateDamage, DamageShort);
    NEW_NERVE(KoopaJrRobotStateDamageNrvDamageHead, KoopaJrRobotStateDamage, DamageHead);
    NEW_NERVE(KoopaJrRobotStateDamageNrvDamageLast, KoopaJrRobotStateDamage, DamageLast);
};  // namespace NrvKoopaJrRobotStateDamage

KoopaJrRobotStateDamage::KoopaJrRobotStateDamage(KoopaJrRobot* pHost, s32 mode)
    : ActorStateBase< KoopaJrRobot >("State:\x83_\x83\x81\x81[\x83W", pHost), mMode(mode) {
    initNerve(&NrvKoopaJrRobotStateDamage::KoopaJrRobotStateDamageNrvWait::sInstance, 0);
}

void KoopaJrRobotStateDamage::appear() {
    mIsDead = false;
    if (mMode == 1) {
        if (mHost->mParts[3]->isBroken()) {
            setNerve(&NrvKoopaJrRobotStateDamage::KoopaJrRobotStateDamageNrvDamageLast::sInstance);
            return;
        }
        setNerve(&NrvKoopaJrRobotStateDamage::KoopaJrRobotStateDamageNrvDamageHead::sInstance);
        return;
    }
    if (mHost->isAllBodyPartsBroken()) {
        setNerve(&NrvKoopaJrRobotStateDamage::KoopaJrRobotStateDamageNrvDamageShort::sInstance);
        return;
    }
    setNerve(&NrvKoopaJrRobotStateDamage::KoopaJrRobotStateDamageNrvDamageLong::sInstance);
}

void KoopaJrRobotStateDamage::exeDamageLong() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "DamageLong");
        mHost->mSeq->stopLauncherSequence();
        mHost->attackShootableKillers();
        return;
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

void KoopaJrRobotStateDamage::exeDamageShort() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "DamageShort");
        return;
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

void KoopaJrRobotStateDamage::exeDamageHead() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "DamageHead");
        MR::startActionSound(mHost, "DamageLastCrack", -1, -1, -1);
        MR::startAction(mHost->mBody, "Damage1");
        mHost->mBody->mCrack->appear();
        MR::tryHitReactionBreak(mHost->mBody);
        return;
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

void KoopaJrRobotStateDamage::exeDamageLast() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "DamageLast");
        MR::startActionSound(mHost, "DamageLastBreak", -1, -1, -1);
        MR::startAction(mHost->mBody, "Damage2");
        MR::tryHitReactionBreak(mHost->mBody);
        mHost->mBody->mCrack->kill();
        return;
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

KoopaJrRobotStateDamage::~KoopaJrRobotStateDamage() {}
