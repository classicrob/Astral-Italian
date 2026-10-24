#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvTwoLegsStateWalkNoDamage {
    NEW_NERVE(TwoLegsStateWalkNoDamageNrvWalk, TwoLegsStateWalkNoDamage, Walk);
};  // namespace NrvTwoLegsStateWalkNoDamage

TwoLegsStateWalkNoDamage::TwoLegsStateWalkNoDamage(TwoLegs* pHost)
    : ActorStateBase< TwoLegs >("State[\x95\xe0\x82\xab\x81""E\x83_\x83\x81\x81[\x83W\x96\xb3\x82\xb5]", pHost), mStepCount(0), mAttacked(false) {}

TwoLegsStateWalkNoDamage::~TwoLegsStateWalkNoDamage() {}

void TwoLegsStateWalkNoDamage::init() {
    initNerve(&NrvTwoLegsStateWalkNoDamage::TwoLegsStateWalkNoDamageNrvWalk::sInstance, 0);
}

void TwoLegsStateWalkNoDamage::appear() {
    mIsDead = false;
    mStepCount = 0;
    mAttacked = false;
    mHost->jointTrackOn();
    setNerve(&NrvTwoLegsStateWalkNoDamage::TwoLegsStateWalkNoDamageNrvWalk::sInstance);
}

bool TwoLegsStateWalkNoDamage::tryAttackStrong(HitSensor* pSender, HitSensor* pReceiver) {
    bool ret;
    if (isStampTiming(this, mHost, pSender)) {
        ret = MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
    } else {
        ret = false;
    }
    if (ret) {
        mAttacked = true;
        return true;
    }
    return false;
}

bool TwoLegsStateWalkNoDamage::tryStrongStamp(NerveExecutor* pState, TwoLegs* pHost, HitSensor* pSender, HitSensor* pReceiver) {
    if (isStampTiming(pState, pHost, pSender)) {
        return MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
    }
    return false;
}

bool TwoLegsStateWalkNoDamage::tryStamp(NerveExecutor* pState, TwoLegs* pHost, HitSensor* pSender, HitSensor* pReceiver) {
    if (isStampTiming(pState, pHost, pSender)) {
        return MR::sendMsgEnemyAttack(pReceiver, pSender);
    }
    return false;
}

// NONMATCHING: left/right boolean and result use exchanged nonvolatile registers.
bool TwoLegsStateWalkNoDamage::isStampTiming(NerveExecutor* pState, TwoLegs* pHost, HitSensor* pSender) {
    if (pState->getNerveStep() <= 0) {
        return false;
    }
    f32 frame = MR::getBckFrame(pHost);
    bool left = pHost->mIsLeft;
    bool ret;
    if (60.0f <= frame && frame <= 76.0f) {
        ret = false;
        if ((left && MR::isEqualSensor(pSender, pHost, "StampLeft")) || (!left && MR::isEqualSensor(pSender, pHost, "StampRight"))) {
            ret = true;
        }
        return ret;
    }
    if (150.0f <= frame && frame <= 166.0f) {
        ret = false;
        if ((left && MR::isEqualSensor(pSender, pHost, "StampRight")) || (!left && MR::isEqualSensor(pSender, pHost, "StampLeft"))) {
            ret = true;
        }
        return ret;
    }
    return false;
}

// NONMATCHING: attacked-flag load is scheduled after the nonvolatile register save.
bool TwoLegsStateWalkNoDamage::isWalkEnd() {
    if (mAttacked) {
        return true;
    }
    if (mHost->isPlayerBetweenFeet()) {
        return true;
    }
    if ((f32)mStepCount < 2.0f) {
        return false;
    }
    if ((f32)mStepCount < 3.0f && MR::isHalfProbability()) {
        return false;
    }
    return true;
}

void TwoLegsStateWalkNoDamage::exeWalk() {
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        mHost->startActionLR("WalkDown");
        MR::setBckRate(mHost, 1.3f);
        mStepCount++;
    }
    if (MR::isActionEnd(mHost)) {
        if (isWalkEnd()) {
            kill();
            return;
        }
        setNerve(&NrvTwoLegsStateWalkNoDamage::TwoLegsStateWalkNoDamageNrvWalk::sInstance);
    }
}
