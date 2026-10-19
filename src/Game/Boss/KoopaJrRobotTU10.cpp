#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvKoopaJrRobotStateB {
    NEW_NERVE(KoopaJrRobotStateBNrvStart, KoopaJrRobotStateB, Start);
    NEW_NERVE(KoopaJrRobotStateBNrvWait, KoopaJrRobotStateB, Wait);
    NEW_NERVE(KoopaJrRobotStateBNrvAttack, KoopaJrRobotStateB, Attack);
    NEW_NERVE(KoopaJrRobotStateBNrvSearch, KoopaJrRobotStateB, Search);
    NEW_NERVE(KoopaJrRobotStateBNrvTurnL, KoopaJrRobotStateB, Turn);
    NEW_NERVE(KoopaJrRobotStateBNrvTurnR, KoopaJrRobotStateB, Turn);
    NEW_NERVE(KoopaJrRobotStateBNrvTurnLEnd, KoopaJrRobotStateB, TurnEnd);
    NEW_NERVE(KoopaJrRobotStateBNrvTurnREnd, KoopaJrRobotStateB, TurnEnd);
    NEW_NERVE(KoopaJrRobotStateBNrvDamage1, KoopaJrRobotStateB, Damage);
    NEW_NERVE(KoopaJrRobotStateBNrvDamage2, KoopaJrRobotStateB, Damage);
};  // namespace NrvKoopaJrRobotStateB

KoopaJrRobotStateB::KoopaJrRobotStateB(KoopaJrRobot* pHost) : KoopaJrRobotStateBase("State:\x83t\x83""F\x81[\x83Y\x82Q", pHost) {
    initNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvStart::sInstance, 3);
    MR::initActorState(this, new KoopaJrRobotStateAttack(mHost, 1), &NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvAttack::sInstance, nullptr);
    MR::initActorState(this, new KoopaJrRobotStateDamage(mHost, 0), &NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvDamage1::sInstance, nullptr);
    MR::initActorState(this, new KoopaJrRobotStateDamage(mHost, 0), &NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvDamage2::sInstance, nullptr);
}

bool KoopaJrRobotStateB::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && mHost->isKillerShotEnd(pSender) && mHost->isWeakPointSensor(pReceiver)) {
        s32 hit = 4;
        for (s32 i = 0; i < 4; i++) {
            if (mHost->mParts[i]->receiveMsgExplosion(msg, pSender, pReceiver)) {
                hit = i;
            }
        }
        if (hit == 2) {
            setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvDamage2::sInstance);
        } else {
            setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvDamage1::sInstance);
        }
        return true;
    }
    return false;
}

const char* KoopaJrRobotStateB::getWaitActionName() {
    return "AngryWait";
}

bool KoopaJrRobotStateB::checkTimeout() {
    if (MR::isGreaterStep(this, 320)) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvAttack::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrRobotStateB::tryTurnToSearch() {
    if (!isPlayerNearFront()) {
        return false;
    }
    setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvSearch::sInstance);
    return true;
}

bool KoopaJrRobotStateB::tryTurnStart() {
    if (!isPlayerNearFront()) {
        return false;
    }
    s32 r = calcTurnDir();
    if (r == 1) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvTurnL::sInstance);
        return true;
    }
    if (r == 2) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvTurnR::sInstance);
        return true;
    }
    return false;
}

void KoopaJrRobotStateB::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Angry");
        MR::startAction(mHost->mBody, "Wait");
        mHost->mSeq->fn_800DC9F0();
    }
    if (MR::isActionEnd(mHost) && !tryTurnStart()) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvWait::sInstance);
    }
}

void KoopaJrRobotStateB::exeWait() {
    if (MR::isFirstStep(this)) {
        if (isPlayerNearFront()) {
            setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvSearch::sInstance);
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
        if (tryTurnToSearch()) {
            return;
        }
    }
}

void KoopaJrRobotStateB::exeDamage() {
    if (MR::updateActorState(this)) {
        if (mHost->isAllBodyPartsBroken()) {
            kill();
            return;
        }
        if (!(isNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvDamage2::sInstance) && tryTurnStart())) {
            setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvWait::sInstance);
        }
    }
}

void KoopaJrRobotStateB::exeSearch() {
    s32 r = updateSearch();
    if (r == 1) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvTurnL::sInstance);
    } else if (r == 2) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvTurnR::sInstance);
    }
}

void KoopaJrRobotStateB::exeTurn() {
    bool isLeft = isNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvTurnL::sInstance);
    s32 dir = 2;
    if (isLeft) {
        dir = 1;
    }
    s32 r = updateTurn(dir);
    if (r == 1) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvTurnLEnd::sInstance);
    } else if (r == 2) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvTurnREnd::sInstance);
    }
}

void KoopaJrRobotStateB::exeTurnEnd() {
    bool isLeft = isNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvTurnLEnd::sInstance);
    s32 dir = 2;
    if (isLeft) {
        dir = 1;
    }
    if (updateTurnEnd(dir)) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvWait::sInstance);
    }
}

void KoopaJrRobotStateB::exeAttack() {
    if (MR::updateActorState(this) && !tryTurnToSearch()) {
        setNerve(&NrvKoopaJrRobotStateB::KoopaJrRobotStateBNrvWait::sInstance);
    }
}

KoopaJrRobotStateB::~KoopaJrRobotStateB() {}
