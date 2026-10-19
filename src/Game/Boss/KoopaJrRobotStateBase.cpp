#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"

// ---- phase state base ----
KoopaJrRobotStateBase::KoopaJrRobotStateBase(const char* pName, KoopaJrRobot* pHost) : ActorStateBase< KoopaJrRobot >(pName, pHost), _10(0.0f), _14(0.0f, 0.0f, 1.0f) {}

void KoopaJrRobotStateBase::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if ((!mHost->isArmSensor(pSender) || !MR::isSensorType(pReceiver, 0x2D) || !MR::sendMsgEnemyAttackExplosion(pReceiver, pSender)) &&
        MR::isSensorPlayerOrYoshiOrRide(pReceiver) && !MR::sendMsgEnemyAttackElectric(pReceiver, pSender)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool KoopaJrRobotStateBase::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    for (s32 i = 0; i < 4; i++) {
        if (mHost->mParts[i]->receiveMsgPlayerAttack(msg, pSender, pReceiver)) {
            return true;
        }
    }
    return false;
}

void KoopaJrRobotStateBase::turnToPlayerFast() {
    MR::turnDirectionToPlayerDegreeHorizon(mHost, &mHost->mZAxis, 180.0f);
}

void KoopaJrRobotStateBase::turnToPlayerSlow() {
    MR::turnDirectionToPlayerDegreeHorizon(mHost, &mHost->mZAxis, 2.0f);
}

s32 KoopaJrRobotStateBase::updateSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Search");
        mHost->mSeq->fn_800DC9D0();
    }
    if (MR::isStep(this, 110)) {
        mHost->mCollisionC->appear();
    }
    if (MR::isActionEnd(mHost)) {
        return calcTurnDir();
    }
    return 0;
}

// NONMATCHING: shared TVec3 unary-negation helper remains out of line
s32 KoopaJrRobotStateBase::calcTurnDir() {
    MR::calcVecToPlayerH(&_14, mHost, nullptr);
    TVec3f up(-mHost->mGravity);
    _10 = 57.295776f * MR::diffAngleSigned(_14, mHost->mZAxis, up);
    bool positive = 0.0f < _10;
    s32 dir = 2;
    if (positive) dir = 1;
    return dir;
}

s32 KoopaJrRobotStateBase::updateTurn(s32 dir) {
    if (dir != 1 && dir != 2) {
        return 0;
    }
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, dir == 1 ? "TurnL" : "TurnR");
        return 0;
    }
    bool turned = MR::turnDirectionToPlayerDegreeHorizon(mHost, &mHost->mZAxis, 2.0f);
    s32 result = 0;
    if (turned) result = dir;
    return result;
}

bool KoopaJrRobotStateBase::updateTurnEnd(s32 dir) {
    if (dir != 1 && dir != 2) {
        return false;
    }
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, dir == 1 ? "TurnLEnd" : "TurnREnd");
    }
    return MR::isActionEnd(mHost);
}

bool KoopaJrRobotStateBase::isPlayerNearFront() {
    return mHost->isPlayerNearBack(45.0f);
}

const char* KoopaJrRobotStateBase::getWaitActionName() {
    return "Wait";
}





