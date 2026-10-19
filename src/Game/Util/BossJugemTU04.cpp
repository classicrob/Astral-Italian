#pragma auto_inline off
#include "Game/Boss/BossJugem.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossJugemStateThrowOne {
    NEW_NERVE(BossJugemStateThrowOneNrvThrowOneSign, BossJugemStateThrowOne, ThrowOneSign);
    NEW_NERVE(BossJugemStateThrowOneNrvThrowOne, BossJugemStateThrowOne, ThrowOne);
};  // namespace NrvBossJugemStateThrowOne

namespace NrvBossJugemStateThrowTwo {
    NEW_NERVE(BossJugemStateThrowTwoNrvThrowTwoSign, BossJugemStateThrowTwo, ThrowTwoSign);
    NEW_NERVE(BossJugemStateThrowTwoNrvThrowTwo, BossJugemStateThrowTwo, ThrowTwo);
};  // namespace NrvBossJugemStateThrowTwo

BossJugemStateThrowOne::BossJugemStateThrowOne(BossJugem* pHost)
    : ActorStateBase< BossJugem >("\203\173\203\130\203\127\203\205\203\121\203\200\223\212\202\260\201\151\202\120\202\302\201\152", pHost), mSpeed(41.0f), mAngle(34.0f), mIsDouble(false), mMoveStep(0), mRailStart(0.0f) {
}

void BossJugemStateThrowOne::init() {
    initNerve(&NrvBossJugemStateThrowOne::BossJugemStateThrowOneNrvThrowOneSign::sInstance, 0);
}

void BossJugemStateThrowOne::appear() {
    mIsDead = false;
    setNerve(&NrvBossJugemStateThrowOne::BossJugemStateThrowOneNrvThrowOneSign::sInstance);
}

void BossJugemStateThrowOne::exeThrowOneSign() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("ThrowOneSign");
        if (mIsDouble) {
            prepareMove();
        }
    }
    if (MR::isStep(this, 18)) {
        mHost->appearHoldTogezoR();
    }
    if (mIsDouble) {
        mHost->turnToPlayer(0.06f);
        updateMove();
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvBossJugemStateThrowOne::BossJugemStateThrowOneNrvThrowOne::sInstance);
    }
}

void BossJugemStateThrowOne::exeThrowOne() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("ThrowOne");
    }
    if (MR::isStep(this, 17)) {
        mHost->releaseTogezoR(mAngle, mSpeed);
    }
    if (mIsDouble) {
        mHost->turnToPlayer(0.06f);
        updateMove();
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

void BossJugemStateThrowOne::prepareMove() {
    mMoveStep = 0;
    if (MR::isRailReachedNearGoal(mHost, 1600.0f)) {
        MR::reverseRailDirection(mHost);
    }
    mRailStart = MR::getRailCoord(mHost);
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void BossJugemStateThrowOne::updateMove() {
    if (!mIsDead) {
        BossJugem* pHost = mHost;
        s32 frameA = MR::getBckFrameMax(pHost, "ThrowOne");
        s32 frameB = MR::getBckFrameMax(pHost, "ThrowOneSign");
        f32 frameMax = frameB + frameA + 1;
        f32 dist = 1600.0f;
        f32 target = mRailStart + dist;
        mMoveStep++;
        if (!MR::isRailCoordNotBehind(mHost, target)) {
            target = mRailStart - dist;
        }
        LiveActor* pActor = mHost;
        MR::setRailCoord(pActor, MR::getLinerValue(mMoveStep, mRailStart, target, frameMax));
        MR::moveTransToCurrentRailPos(mHost);
    }
}

BossJugemStateThrowTwo::BossJugemStateThrowTwo(BossJugem* pHost)
    : ActorStateBase< BossJugem >("\203\173\203\130\203\127\203\205\203\121\203\200\223\212\202\260\201\151\202\121\202\302\201\152", pHost), mSpeed(41.0f), mAngle(34.0f), mIsDouble(false), mMoveStep(0), mRailStart(0.0f) {
}

void BossJugemStateThrowTwo::init() {
    initNerve(&NrvBossJugemStateThrowTwo::BossJugemStateThrowTwoNrvThrowTwoSign::sInstance, 0);
}

void BossJugemStateThrowTwo::appear() {
    mIsDead = false;
    setNerve(&NrvBossJugemStateThrowTwo::BossJugemStateThrowTwoNrvThrowTwoSign::sInstance);
}

void BossJugemStateThrowTwo::exeThrowTwoSign() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("ThrowTwoSign");
        if (mIsDouble) {
            prepareMove();
        }
    }
    if (MR::isStep(this, 18)) {
        mHost->appearHoldTogezoR();
    }
    if (MR::isStep(this, 43)) {
        mHost->appearHoldTogezoL();
    }
    if (mIsDouble) {
        mHost->turnToPlayer(0.06f);
        updateMove();
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvBossJugemStateThrowTwo::BossJugemStateThrowTwoNrvThrowTwo::sInstance);
    }
}

void BossJugemStateThrowTwo::exeThrowTwo() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("ThrowTwo");
    }
    if (MR::isStep(this, 8)) {
        mHost->releaseTogezoR(mAngle, mSpeed);
    }
    if (MR::isStep(this, 33)) {
        mHost->releaseTogezoL(mAngle, mSpeed);
    }
    if (mIsDouble) {
        mHost->turnToPlayer(0.06f);
        updateMove();
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

void BossJugemStateThrowTwo::prepareMove() {
    mMoveStep = 0;
    if (MR::isRailReachedNearGoal(mHost, 2400.0f)) {
        MR::reverseRailDirection(mHost);
    }
    mRailStart = MR::getRailCoord(mHost);
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void BossJugemStateThrowTwo::updateMove() {
    if (!mIsDead) {
        BossJugem* pHost = mHost;
        s32 frameA = MR::getBckFrameMax(pHost, "ThrowTwo");
        s32 frameB = MR::getBckFrameMax(pHost, "ThrowTwoSign");
        f32 frameMax = frameB + frameA + 1;
        f32 dist = 2400.0f;
        f32 target = mRailStart + dist;
        mMoveStep++;
        if (!MR::isRailCoordNotBehind(mHost, target)) {
            target = mRailStart - dist;
        }
        LiveActor* pActor = mHost;
        MR::setRailCoord(pActor, MR::getLinerValue(mMoveStep, mRailStart, target, frameMax));
        MR::moveTransToCurrentRailPos(mHost);
    }
}
