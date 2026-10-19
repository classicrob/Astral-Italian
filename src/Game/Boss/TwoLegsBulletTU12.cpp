#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/MathUtil.hpp"
#include <cstdio>

namespace NrvTwoLegsStateWalkDamage {
    NEW_NERVE(TwoLegsStateWalkDamageNrvReverseSign, TwoLegsStateWalkDamage, ReverseSign);
    NEW_NERVE(TwoLegsStateWalkDamageNrvDown, TwoLegsStateWalkDamage, Down);
    NEW_NERVE(TwoLegsStateWalkDamageNrvWalk, TwoLegsStateWalkDamage, Walk);
    NEW_NERVE(TwoLegsStateWalkDamageNrvWaitDown, TwoLegsStateWalkDamage, WaitDown);
    NEW_NERVE(TwoLegsStateWalkDamageNrvUp, TwoLegsStateWalkDamage, Up);
};  // namespace NrvTwoLegsStateWalkDamage

TwoLegsStateWalkDamage::TwoLegsStateWalkDamage(TwoLegs* pHost)
    : ActorStateBase< TwoLegs >("State[\x95\xe0\x82\xab\x81""E\x83_\x83\x81\x81[\x83W\x97L\x82\xe8]", pHost), mStepCount(0), mWalkCount(0), mAttacked(false) {}

TwoLegsStateWalkDamage::~TwoLegsStateWalkDamage() {}

void TwoLegsStateWalkDamage::init() {
    initNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvReverseSign::sInstance, 0);
}

void TwoLegsStateWalkDamage::appear() {
    mIsDead = false;
    mStepCount = 0;
    mWalkCount = 0;
    mAttacked = false;
    setNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvReverseSign::sInstance);
}

bool TwoLegsStateWalkDamage::isDamageable() {
    bool ret = false;
    if (isNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvWalk::sInstance) ||
        isNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvWaitDown::sInstance)) {
        ret = true;
    }
    return ret;
}

s32 TwoLegsStateWalkDamage::getDamageSide() {
    return mWalkCount + 1 >= 2;
}

bool TwoLegsStateWalkDamage::tryAttackStrong(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvWalk::sInstance)) {
        if (TwoLegsStateWalkNoDamage::tryStrongStamp(this, mHost, pSender, pReceiver)) {
            mAttacked = true;
            return true;
        }
        return false;
    }
    if (isNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvReverseSign::sInstance)) {
        if (isStampTiming(this, mHost, pSender)) {
            return MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
        }
        return false;
    }
    return false;
}

bool TwoLegsStateWalkDamage::tryAttack(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvWalk::sInstance)) {
        return TwoLegsStateWalkNoDamage::tryStamp(this, mHost, pSender, pReceiver);
    }
    if (isNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvReverseSign::sInstance)) {
        if (isStampTiming(this, mHost, pSender)) {
            return MR::sendMsgEnemyAttack(pReceiver, pSender);
        }
        return false;
    }
    return false;
}

bool TwoLegsStateWalkDamage::tryStrongStamp(NerveExecutor* pState, TwoLegs* pHost, HitSensor* pSender, HitSensor* pReceiver) {
    if (isStampTiming(pState, pHost, pSender)) {
        return MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
    }
    return false;
}

bool TwoLegsStateWalkDamage::tryStamp(NerveExecutor* pState, TwoLegs* pHost, HitSensor* pSender, HitSensor* pReceiver) {
    if (isStampTiming(pState, pHost, pSender)) {
        return MR::sendMsgEnemyAttack(pReceiver, pSender);
    }
    return false;
}

bool TwoLegsStateWalkDamage::isStampTiming(NerveExecutor* pState, TwoLegs* pHost, HitSensor* pSender) {
    if (pState->getNerveStep() <= 0) {
        return false;
    }
    if (!MR::isEqualSensor(pSender, pHost, "StampLeft") && !MR::isEqualSensor(pSender, pHost, "StampRight")) {
        return false;
    }
    f32 frame = MR::getBckFrame(pHost);
    bool ret = false;
    s32 lo = 50;
    s32 hi = 58;
    if ((f32)lo <= frame && frame <= (f32)hi) {
        ret = true;
    }
    return ret;
}

bool TwoLegsStateWalkDamage::isWalkEnd() {
    if (mAttacked) {
        return true;
    }
    if (mHost->isPlayerBetweenFeet()) {
        return true;
    }
    return mStepCount >= 2;
}

void TwoLegsStateWalkDamage::exeReverseSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "ReverseSign");
    }
    if (MR::isActionEnd(mHost) && !mHost->tryKillStateIfPlayerBetweenFeet(this)) {
        setNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvDown::sInstance);
    }
}

void TwoLegsStateWalkDamage::exeDown() {
    if (MR::isFirstStep(this)) {
        mHost->startActionDown();
    }
    if (MR::isActionEnd(mHost) && !mHost->trySetNerveIfPlayerBetweenFeet(this, &NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvUp::sInstance)) {
        mHost->jointTrackOn();
        setNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvWalk::sInstance);
    }
}

void TwoLegsStateWalkDamage::exeWalk() {
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        mHost->startActionLR("WalkDown");
        MR::setBckRate(mHost, 2.0f);
        mStepCount++;
    }
    if (MR::isActionEnd(mHost)) {
        MR::setBckRate(mHost, 1.0f);
        if (isWalkEnd()) {
            mWalkCount++;
            if (mWalkCount < 2) {
                mStepCount = 0;
                setNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvWaitDown::sInstance);
                return;
            }
            setNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvUp::sInstance);
            return;
        }
        setNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvWalk::sInstance);
    }
}

void TwoLegsStateWalkDamage::exeWaitDown() {
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        MR::startAction(mHost, "WaitDown");
    }
    if (!mHost->trySetNerveIfPlayerBetweenFeet(this, &NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvUp::sInstance)) {
        if (MR::isStep(this, 120)) {
            if (mAttacked) {
                mAttacked = false;
                mHost->toggleSide();
            } else if (MR::isHalfProbability()) {
                mHost->mIsLeft = true;
            } else {
                mHost->mIsLeft = false;
            }
            setNerve(&NrvTwoLegsStateWalkDamage::TwoLegsStateWalkDamageNrvWalk::sInstance);
        }
    }
}

void TwoLegsStateWalkDamage::exeUp() {
    if (MR::isFirstStep(this)) {
        mHost->placeAtAllRoot();
        mHost->startActionUp();
        mHost->jointTrackOnReturn();
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

// ---- TwoLegs helpers that live in this unit ----
void TwoLegs::startActionLR(const char* pName) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s%s", pName, mIsLeft ? "L" : "R");
    MR::startAction(this, buf);
}

#pragma dont_inline on
void TwoLegs::startActionUp() {
    MR::startAction(this, "Up");
    mIsUp = true;
}

void TwoLegs::startActionDown() {
    MR::startAction(this, "Down");
    mIsUp = false;
}

#pragma dont_inline off

bool TwoLegs::isPhase0() const {
    return mPhase == 0;
}

bool TwoLegs::isLastPhase() const {
    return mPhase - 2 == 0;
}

s32 TwoLegs::getPhaseNum() {
    return getPhaseNumInner();
}

void TwoLegs::getAllRootPos(TVec3f* pOut) {
    MR::copyJointPos(this, "AllRoot", pOut);
}

void TwoLegs::calcAllRootPosOnPlane(TVec3f* pOut) {
    TVec3f pos;
    MR::copyJointPos(this, "AllRoot", &pos);
    calcPosOnPlane(pOut, &pos);
}

void TwoLegs::calcPosOnPlane(TVec3f* pOut, const TVec3f* pPos) {
    TVec3f vec;
    vec.sub(*pPos, mBodyDir->mPos);
    calcPosOnPlaneImpl(pOut, vec);
}

void TwoLegs::calcPosOnPlaneImpl(TVec3f* pOut, const TVec3f& rVec) {
    TVec3f killed;
    MR::vecKillElement(rVec, mBodyDir->mFront, &killed);
    f32 radius = mBodyDir->getRadius();
    TVec3f dir;
    dir.setLength(killed, radius);
    pOut->add(mBodyDir->mPos, dir);
}

TwoLegsBodyDir* TwoLegs::getBodyDir() const {
    return mBodyDir;
}

const TVec3f* TwoLegs::getBodyFront() const {
    return &mBodyDir->mFront;
}

void TwoLegs::addCameraTarget(LiveActor* pActor) {
    mCamera->addTarget(pActor);
}

void TwoLegs::resetCamera() {
    mCamera->reset();
}

void TwoLegs::disableCamera() {
    mCamera->mActive = false;
}

void TwoLegs::toggleSide() {
    if (mIsLeft) {
        mIsLeft = false;
        return;
    }
    mIsLeft = true;
}

void TwoLegs::jointTrackOn() {
    TwoLegsJointCtrl* pCtrl = mJointCtrl;
    pCtrl->mTrack = true;
    pCtrl->mReturn = true;
}

void TwoLegs::jointTrackOnReturn() {
    TwoLegsJointCtrl* pCtrl = mJointCtrl;
    pCtrl->mTrack = false;
    pCtrl->mReturn = true;
}

void TwoLegs::jointTrackOff() {
    TwoLegsJointCtrl* pCtrl = mJointCtrl;
    pCtrl->mTrack = false;
    pCtrl->mReturn = false;
}

void TwoLegs::jointSpinOn() {
    mJointCtrl->mSpin = true;
}

void TwoLegs::jointReset() {
    TwoLegsJointCtrl* pCtrl = mJointCtrl;
    pCtrl->mSpin = false;
    pCtrl->mMtxR.identity();
    pCtrl->mMtxL.identity();
}

void TwoLegs::popCoins(const TVec3f& rPos) {
    mCoin->appear(rPos);
}

bool TwoLegs::isPlayerBetweenFeet() {
    if (MR::isPlayerInRush()) {
        return false;
    }
    TVec3f footR;
    MR::copyJointPos(this, "FootR", &footR);
    TVec3f footL;
    MR::copyJointPos(this, "FootL", &footL);
    TVec3f dirR;
    MR::calcDirectionVec(&dirR, mBodyDir->mPos, footR);
    TVec3f dirL;
    MR::calcDirectionVec(&dirL, mBodyDir->mPos, footL);
    TVec3f dirP;
    MR::calcDirectionVec(&dirP, mBodyDir->mPos, *MR::getPlayerCenterPos());
    f32 angleP = MR::diffAngleSigned(dirR, dirP, mBodyDir->mFront);
    f32 angleL = MR::diffAngleSigned(dirR, dirL, mBodyDir->mFront);
    bool ret = false;
    if (0.0f < angleP && angleP < angleL) {
        ret = true;
    }
    return ret;
}

bool TwoLegs::tryKillStateIfPlayerBetweenFeet(ActorStateBaseInterface* pState) {
    if (isPlayerBetweenFeet()) {
        pState->kill();
        return true;
    }
    return false;
}

bool TwoLegs::trySetNerveIfPlayerBetweenFeet(NerveExecutor* pState, const Nerve* pNerve) {
    if (isPlayerBetweenFeet()) {
        pState->setNerve(pNerve);
        return true;
    }
    return false;
}
