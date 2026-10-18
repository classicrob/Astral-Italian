#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvTwoLegsStateJump {
    NEW_NERVE(TwoLegsStateJumpNrvJump, TwoLegsStateJump, Jump);
};  // namespace NrvTwoLegsStateJump

TwoLegsStateJump::TwoLegsStateJump(TwoLegs* pHost) : ActorStateBase< TwoLegs >("State[\x83W\x83\x83\x83\x93\x83v]", pHost), mJumpCount(0), mAttacked(false), mFlag(false) {}

TwoLegsStateJump::~TwoLegsStateJump() {}

void TwoLegsStateJump::init() {
    initNerve(&NrvTwoLegsStateJump::TwoLegsStateJumpNrvJump::sInstance, 0);
}

void TwoLegsStateJump::appear() {
    mIsDead = false;
    mJumpCount = 0;
    mAttacked = false;
    mFlag = false;
    mHost->jointTrackOnReturn();
    setNerve(&NrvTwoLegsStateJump::TwoLegsStateJumpNrvJump::sInstance);
}

bool TwoLegsStateJump::tryAttackStrong(HitSensor* pSender, HitSensor* pReceiver) {
    if (isStamped(pSender) && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
        mFlag = true;
        return true;
    }
    return false;
}

bool TwoLegsStateJump::tryAttack(HitSensor* pSender, HitSensor* pReceiver) {
    if (isStamped(pSender)) {
        return MR::sendMsgEnemyAttack(pReceiver, pSender);
    }
    return false;
}

u8 TwoLegsStateJump::getAttackFlag() const {
    return mAttacked;
}

bool TwoLegsStateJump::isStamped(HitSensor* pSender) {
    if (!MR::isEqualSensor(pSender, mHost, "StampLeft") && !MR::isEqualSensor(pSender, mHost, "StampRight")) {
        return false;
    }
    f32 frame = MR::getBckFrame(mHost);
    bool ret = false;
    if (60.0f <= frame && frame <= 72.0f) {
        ret = true;
    }
    return ret;
}

bool TwoLegsStateJump::isJumpEnd() {
    if (mFlag) {
        return true;
    }
    if ((f32)mJumpCount < 2.0f) {
        return false;
    }
    if ((f32)mJumpCount < 3.0f && MR::isHalfProbability()) {
        return false;
    }
    return true;
}

bool TwoLegsStateJump::tryNextJump() {
    if (mHost->isPlayerBetweenFeet()) {
        return false;
    }
    if (!isJumpEnd()) {
        setNerve(&NrvTwoLegsStateJump::TwoLegsStateJumpNrvJump::sInstance);
        return true;
    }
    if (mAttacked) {
        return false;
    }
    if (mFlag) {
        mFlag = false;
        mHost->toggleSide();
    } else {
        if (mHost->mIsLeft) {
            mHost->mIsLeft = false;
        } else {
            mHost->mIsLeft = true;
        }
    }
    mJumpCount = 0;
    mAttacked = true;
    return true;
}

void TwoLegsStateJump::exeJump() {
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        mHost->startActionLR("Jump");
        MR::setBckRate(mHost, 1.2f);
        mJumpCount++;
    }
    if (MR::isActionEnd(mHost) && !tryNextJump()) {
        kill();
    }
}
