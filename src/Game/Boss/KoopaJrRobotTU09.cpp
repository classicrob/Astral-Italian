#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvKoopaJrRobotStateA {
    NEW_NERVE(KoopaJrRobotStateANrvWaitStart, KoopaJrRobotStateA, Start);
    NEW_NERVE(KoopaJrRobotStateANrvWait, KoopaJrRobotStateA, Wait);
    NEW_NERVE(KoopaJrRobotStateANrvAttack, KoopaJrRobotStateA, Attack);
    NEW_NERVE(KoopaJrRobotStateANrvSearch, KoopaJrRobotStateA, Search);
    NEW_NERVE(KoopaJrRobotStateANrvTurnL, KoopaJrRobotStateA, Turn);
    NEW_NERVE(KoopaJrRobotStateANrvTurnR, KoopaJrRobotStateA, Turn);
    NEW_NERVE(KoopaJrRobotStateANrvTurnLEnd, KoopaJrRobotStateA, TurnEnd);
    NEW_NERVE(KoopaJrRobotStateANrvTurnREnd, KoopaJrRobotStateA, TurnEnd);
    NEW_NERVE(KoopaJrRobotStateANrvDamage1, KoopaJrRobotStateA, Damage);
    NEW_NERVE(KoopaJrRobotStateANrvDamage2, KoopaJrRobotStateA, Damage);
};  // namespace NrvKoopaJrRobotStateA

KoopaJrRobotStateA::KoopaJrRobotStateA(KoopaJrRobot* pHost) : KoopaJrRobotStateBase("State:\x83t\x83""F\x81[\x83Y\x82P", pHost) {
    initNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvWait::sInstance, 3);
    MR::initActorState(this, new KoopaJrRobotStateAttack(mHost, 0), &NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvAttack::sInstance, nullptr);
    MR::initActorState(this, new KoopaJrRobotStateDamage(mHost, 0), &NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvDamage1::sInstance, nullptr);
    MR::initActorState(this, new KoopaJrRobotStateDamage(mHost, 0), &NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvDamage2::sInstance, nullptr);
}

bool KoopaJrRobotStateA::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && mHost->fn_800DB9B0(pSender) && mHost->fn_800DBA70(pReceiver)) {
        s32 hit = 4;
        for (s32 i = 0; i < 4; i++) {
            if (mHost->mParts[i]->receiveMsgExplosion(msg, pSender, pReceiver)) {
                hit = i;
            }
        }
        if (hit == 2) {
            setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvDamage2::sInstance);
        } else {
            setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvDamage1::sInstance);
        }
        return true;
    }
    return false;
}

void KoopaJrRobotStateA::appear() {
    mIsDead = false;
    MR::startAction(mHost->mBody, "Wait");
    setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvWaitStart::sInstance);
}

bool KoopaJrRobotStateA::isAnyPartBroken() {
    for (s32 i = 0; i < 4; i++) {
        if (mHost->mParts[i]->fn_800E25F0()) {
            return true;
        }
    }
    return false;
}

bool KoopaJrRobotStateA::checkTimeout() {
    if (MR::isGreaterStep(this, 210)) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvAttack::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrRobotStateA::tryTurnToSearch() {
    if (!isPlayerNearFront()) {
        return false;
    }
    setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvSearch::sInstance);
    return true;
}

bool KoopaJrRobotStateA::tryTurnStart() {
    if (!isPlayerNearFront()) {
        return false;
    }
    s32 r = calcTurnDir();
    if (r == 1) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvTurnL::sInstance);
        return true;
    }
    if (r == 2) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvTurnR::sInstance);
        return true;
    }
    return false;
}

void KoopaJrRobotStateA::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "WaitStart");
        turnToPlayerFast();
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvWait::sInstance);
    }
}

void KoopaJrRobotStateA::exeWait() {
    if (MR::isFirstStep(this)) {
        if (isPlayerNearFront()) {
            setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvSearch::sInstance);
            return;
        }
        KoopaJrRobot* pHost = mHost;
        MR::startAction(pHost, getWaitActionName());
        mHost->mSeq->fn_800DC9C0();
    }
    if (!KoopaJrRobotFunction::isPlayerInRushAir()) {
        turnToPlayerSlow();
    }
    checkTimeout();
    tryTurnToSearch();
}

void KoopaJrRobotStateA::exeDamage() {
    if (MR::updateActorState(this)) {
        if (isAnyPartBroken()) {
            kill();
            return;
        }
        if (!(isNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvDamage2::sInstance) && tryTurnStart())) {
            setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvWait::sInstance);
        }
    }
}

void KoopaJrRobotStateA::exeSearch() {
    s32 r = updateSearch();
    if (r == 1) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvTurnL::sInstance);
    } else if (r == 2) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvTurnR::sInstance);
    }
}

void KoopaJrRobotStateA::exeTurn() {
    bool isLeft = isNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvTurnL::sInstance);
    s32 dir = 2;
    if (isLeft) {
        dir = 1;
    }
    s32 r = updateTurn(dir);
    if (r == 1) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvTurnLEnd::sInstance);
    } else if (r == 2) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvTurnREnd::sInstance);
    }
}

void KoopaJrRobotStateA::exeTurnEnd() {
    bool isLeft = isNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvTurnLEnd::sInstance);
    s32 dir = 2;
    if (isLeft) {
        dir = 1;
    }
    if (updateTurnEnd(dir)) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvWait::sInstance);
    }
}

void KoopaJrRobotStateA::exeAttack() {
    if (MR::updateActorState(this) && !tryTurnToSearch()) {
        setNerve(&NrvKoopaJrRobotStateA::KoopaJrRobotStateANrvWait::sInstance);
    }
}

KoopaJrRobotStateA::~KoopaJrRobotStateA() {}
