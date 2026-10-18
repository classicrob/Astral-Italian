#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvKoopaJrRobotStateC {
    NEW_NERVE(KoopaJrRobotStateCNrvStart, KoopaJrRobotStateC, Start);
    NEW_NERVE(KoopaJrRobotStateCNrvWait, KoopaJrRobotStateC, Wait);
    NEW_NERVE(KoopaJrRobotStateCNrvAttack, KoopaJrRobotStateC, Attack);
    NEW_NERVE(KoopaJrRobotStateCNrvDamage, KoopaJrRobotStateC, Damage);
    NEW_NERVE(KoopaJrRobotStateCNrvSearch, KoopaJrRobotStateC, Search);
    NEW_NERVE(KoopaJrRobotStateCNrvTurnL, KoopaJrRobotStateC, Turn);
    NEW_NERVE(KoopaJrRobotStateCNrvTurnR, KoopaJrRobotStateC, Turn);
    NEW_NERVE(KoopaJrRobotStateCNrvTurnLEnd, KoopaJrRobotStateC, TurnEnd);
    NEW_NERVE(KoopaJrRobotStateCNrvTurnREnd, KoopaJrRobotStateC, TurnEnd);
};  // namespace NrvKoopaJrRobotStateC

KoopaJrRobotStateC::KoopaJrRobotStateC(KoopaJrRobot* pHost) : KoopaJrRobotPhase("State:\x83t\x83""F\x81[\x83Y\x82R", pHost), mFlag(false) {
    initNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvStart::sInstance, 2);
    MR::initActorState(this, new KoopaJrRobotStateAttack(mHost, 2), &NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvAttack::sInstance, nullptr);
    MR::initActorState(this, new KoopaJrRobotStateDamage(mHost, 1), &NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvDamage::sInstance, nullptr);
}

bool KoopaJrRobotStateC::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && mHost->fn_800DB9B0(pSender) && mHost->fn_800DBA70(pReceiver)) {
        if (mHost->mParts[3]->receiveMsgExplosion(msg, pSender, pReceiver)) {
            setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvDamage::sInstance);
            return true;
        }
    }
    return false;
}

void KoopaJrRobotStateC::appear() {
    mIsDead = false;
    mHost->mFollowers[4]->kill();
}

const char* KoopaJrRobotStateC::getWaitActionName() {
    return mFlag != 0 ? "AngryLastWait" : "AngryWait";
}

bool KoopaJrRobotStateC::checkTimeout() {
    if (MR::isGreaterStep(this, 320)) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvAttack::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrRobotStateC::tryTurnToSearch() {
    if (!isPlayerNearFront()) {
        return false;
    }
    setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvSearch::sInstance);
    return true;
}

bool KoopaJrRobotStateC::tryTurnStart() {
    if (!isPlayerNearFront()) {
        return false;
    }
    s32 r = calcTurnDir();
    if (r == 1) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvTurnL::sInstance);
        return true;
    }
    if (r == 2) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvTurnR::sInstance);
        return true;
    }
    return false;
}

void KoopaJrRobotStateC::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AngryMore");
        MR::startAction(mHost->mBody, "WaitOpen");
        MR::startAction(mHost->mHeadModel, "WaitAngry");
        mHost->mSeq->fn_800DC9F0();
    }
    if (MR::isActionEnd(mHost) && !tryTurnStart()) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvWait::sInstance);
    }
}

void KoopaJrRobotStateC::exeWait() {
    if (MR::isFirstStep(this)) {
        if (isPlayerNearFront()) {
            setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvSearch::sInstance);
            return;
        }
        KoopaJrRobot* pHost = mHost;
        MR::startAction(pHost, getWaitActionName());
        mHost->mSeq->fn_800DC9E0();
    }
    if (!KoopaJrRobotFunction::isPlayerInRushAir()) {
        turnToPlayerSlow();
    }
    if (!checkTimeout()) {
        tryTurnToSearch();
    }
}

void KoopaJrRobotStateC::exeDamage() {
    if (MR::updateActorState(this)) {
        if (mHost->mParts[3]->fn_800E25F0()) {
            kill();
            return;
        }
        mFlag = true;
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvWait::sInstance);
    }
}

void KoopaJrRobotStateC::exeSearch() {
    s32 r = updateSearch();
    if (r == 1) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvTurnL::sInstance);
    } else if (r == 2) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvTurnR::sInstance);
    }
}

void KoopaJrRobotStateC::exeTurn() {
    s32 dir;
    if (isNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvTurnL::sInstance)) {
        dir = 1;
    } else {
        dir = 2;
    }
    s32 r = updateTurn(dir);
    if (r == 1) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvTurnLEnd::sInstance);
    } else if (r == 2) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvTurnREnd::sInstance);
    }
}

void KoopaJrRobotStateC::exeTurnEnd() {
    s32 dir;
    if (isNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvTurnLEnd::sInstance)) {
        dir = 1;
    } else {
        dir = 2;
    }
    if (updateTurnEnd(dir)) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvWait::sInstance);
    }
}

void KoopaJrRobotStateC::exeAttack() {
    if (MR::updateActorState(this) && !tryTurnToSearch()) {
        setNerve(&NrvKoopaJrRobotStateC::KoopaJrRobotStateCNrvWait::sInstance);
    }
}

KoopaJrRobotStateC::~KoopaJrRobotStateC() {}
