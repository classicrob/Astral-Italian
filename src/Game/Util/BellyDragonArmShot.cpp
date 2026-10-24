#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <JSystem/JMath/JMath.hpp>

namespace {
    const f32 cDownA[] = {2.5f, 0.0f, 0.95f, 5.0f};
    const f32 cDownB[] = {2.85f, 0.0f, 0.95f, 6.8f};
    const f32 cUndergroundA[] = {3.0f, 0.0f, 0.9f, 3.0f};
    const f32 cUndergroundB[] = {3.0f, 0.0f, 0.9f, 3.0f};
    const f32 cFlyOutA[] = {3.0f, 0.0f, 0.95f, 0.0f};
    const f32 cFlyOutB[] = {3.5f, 0.0f, 0.95f, 0.0f};
    const f32 cEndA[] = {3.0f, 0.0f, 0.95f, 1.5f};
    const f32 cEndB[] = {3.5f, 0.0f, 0.95f, 2.0f};
    s32 cDiveLimit = 180;
    const s32 cDiveTimeA = 60, cDiveTimeB = 45;
    const f32 cTurnRateA = 2.0f, cTurnRateB = 1.5f;
    s32 cAimStart = 30;
    const s32 cAimTimeA = 60, cAimTimeB = 70;
    s32 cEffectLead = 10;
    const s32 cWaitTimeA = 60, cWaitTimeB = 45, cFlyOutTimeA = 20, cFlyOutTimeB = 20, cEndTimeA = 15, cEndTimeB = 15;
    const f32 cGravityA = 0.0f, cGravityB = 0.0f;
}
namespace NrvBellyDragonArmShot {
    NEW_NERVE(Down, BellyDragonArmShot, Down);
    NEW_NERVE(Underground, BellyDragonArmShot, Underground);
    NEW_NERVE(Wait, BellyDragonArmShot, Wait);
    NEW_NERVE(FlyOut, BellyDragonArmShot, FlyOut);
    NEW_NERVE(End, BellyDragonArmShot, End);
}
BellyDragonArmShot::BellyDragonArmShot(BellyDragon* pHost, u8 first)
    : ActorStateBase<BellyDragon>("State[ダイブ攻撃]", pHost) {
    mTargetPos.set(0.0f, 0.0f, 0.0f);
    mDiveDir.set(0.0f, 0.0f, 0.0f);
    mIsLv2 = false;
    mDownParam = cDownA;
    mAimTime = &cAimTimeA;
    mGravity = &cGravityA;
    mDiveTime = &cDiveTimeA;
    mTurnRate = &cTurnRateA;
    mUndergroundParam = cUndergroundA;
    mWaitTime = &cWaitTimeA;
    mFlyOutParam = cFlyOutA;
    mFlyOutTime = &cFlyOutTimeA;
    mEndTime = &cEndTimeA;
    mEndParam = cEndA;
    if (first) {
        mIsLv2 = false;
    } else {
        mIsLv2 = true;
        mDownParam = cDownB;
        mAimTime = &cAimTimeB;
        mGravity = &cGravityB;
        mDiveTime = &cDiveTimeB;
        mTurnRate = &cTurnRateB;
        mUndergroundParam = cUndergroundB;
        mWaitTime = &cWaitTimeB;
        mFlyOutParam = cFlyOutB;
        mFlyOutTime = &cFlyOutTimeB;
        mEndTime = &cEndTimeB;
        mEndParam = cEndB;
    }
}
void BellyDragonArmShot::init() {
    initNerve(&NrvBellyDragonArmShot::Down::sInstance, 0);
    kill();
}
void BellyDragonArmShot::appear() {
    mIsDead = false;
    setNerve(&NrvBellyDragonArmShot::Down::sInstance);
}
void BellyDragonArmShot::kill() {
    MR::tryDeleteEffect(mHost, "AttackDiveBlur");
    mIsDead = true;
    MR::onBind(mHost);
}
bool BellyDragonArmShot::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvBellyDragonArmShot::Down::sInstance) || isNerve(&NrvBellyDragonArmShot::Underground::sInstance)
        || isNerve(&NrvBellyDragonArmShot::FlyOut::sInstance)) {
        if (BellyDragonUtil::isLv2(mHost))
            MR::sendMsgEnemyAttackFire(pReceiver, pSender);
        else
            MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
        return true;
    }
    return false;
}
// NONMATCHING: FP operand commutation only (grouping checked); register/stack/branch lowering.
void BellyDragonArmShot::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AttackDiveDown");
        MR::emitEffect(mHost, "DiveBlowL");
        MR::emitEffect(mHost, "DiveBlowR");
    }
    if (MR::isLessEqualStep(this, *mAimTime)) {
        if (MR::isOnGroundPlayer())
            mTargetPos.set(*MR::getPlayerPos());
        else
            MR::calcMapHitAlongDirection(*MR::getPlayerPos(), &mTargetPos, *MR::getPlayerGravity(), NULL, NULL, NULL, NULL, 1000.0f);
    }
    if (MR::isGreaterEqualStep(this, cAimStart)) {
        BellyDragon* host = mHost;
        TVec3f dir(mTargetPos);
        dir -= host->mPosition;
        if (!MR::isNearZero(dir, 0.001f)) {
            MR::normalize(&dir);
            MR::turnVecToVecDegree(&mHost->mFront, mHost->mFront, dir, mDownParam[3], TVec3f(0, 1, 0));
        }
    }
    if (MR::isLessStep(this, *mDiveTime)) {
        MR::addVelocityToGravity(mHost, -*mGravity);
        f32 rate = MR::calcNerveEaseInRate(this, *mDiveTime);
        f32 turnRate = MR::sinDegree(180.0f * rate) * (1.0f - rate);
        MR::turnVecToVecDegree(&mHost->mFront, mHost->mFront, -mHost->mGravity, *mTurnRate * turnRate, TVec3f(0, 1, 0));
    }
    if (MR::isStep(this, *mDiveTime - cEffectLead))
        MR::emitEffect(mHost, "AttackDiveBlur");
    BellyDragonUtil::pushBindedByFront(mHost, mDownParam[0], mDownParam[2]);
    if (!MR::isNoBind(mHost) && MR::isBinded(mHost)) {
        MR::deleteEffect(mHost, "AttackDiveBlur");
        BellyDragon* host = mHost;
        HitSensor* body = BellyDragonUtil::getBodySensor(host);
        MR::sendMsgEnemyAttackToBindedSensor(host, body);
        MR::tryHitReaction(mHost, "ダイブ攻撃（接地）");
        MR::offBind(mHost);
        setNerve(&NrvBellyDragonArmShot::Underground::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, cDiveLimit))
        kill();
}
// NONMATCHING: literal symbol names.
void BellyDragonArmShot::exeUnderground() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AttackDiveUnderground");
        mDiveDir.set(mHost->mVelocity);
        MR::vecKillElement(mDiveDir, mHost->mGravity, &mDiveDir);
        MR::normalizeOrZero(&mDiveDir);
        BellyDragonUtil::startHeadAttackHill(mHost);
    }
    f32 rate = MR::calcNerveRate(this, 20, 30);
    TVec3f dir = mDiveDir * rate + mHost->mGravity * (1.0f - rate);
    if (!MR::isNearZero(dir, 0.001f)) {
        MR::normalize(&dir);
        BellyDragon* host = mHost;
        MR::turnVecToVecDegree(&host->mFront, host->mFront, dir, mUndergroundParam[3], TVec3f(0, 1, 0));
    }
    BellyDragonUtil::pushBindedByFront(mHost, mUndergroundParam[0], mUndergroundParam[2]);
    TVec3f velocity(mHost->mVelocity);
    if (MR::isGreaterStep(this, 30) && !MR::isNearZero(velocity, 0.001f)) {
        MR::normalize(&velocity);
        velocity.scale(700.0f + MR::calcVelocityLength(mHost));
        TVec3f pos = mHost->mPosition + velocity;
        velocity.scale(-1.0f);
        if (MR::isExistMapCollision(pos, velocity)) {
            MR::tryHitReaction(mHost, "ダイブ攻撃（惑星内側の衝突）");
            setNerve(&NrvBellyDragonArmShot::Wait::sInstance);
            return;
        }
    }
    if (MR::isGreaterStep(this, 180))
        kill();
}
void BellyDragonArmShot::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(mHost);
        BellyDragonUtil::appearHeadHill(mHost);
    }
    if (MR::isGreaterStep(this, *mWaitTime))
        setNerve(&NrvBellyDragonArmShot::FlyOut::sInstance);
}
void BellyDragonArmShot::exeFlyOut() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AttackDiveFlyOut");
        BellyDragonUtil::breakHeadHill(mHost);
        BellyDragonUtil::appearFireGroundShort(mHost);
    }
    BellyDragonUtil::pushBindedByFront(mHost, mFlyOutParam[0], mFlyOutParam[2]);
    BellyDragonUtil::updatePodFireEffect(mHost);
    if (MR::isGreaterStep(this, *mFlyOutTime)) {
        BellyDragonUtil::startBodyBindB(mHost);
        if (mIsLv2) {
            s32 num = BellyDragonUtil::getAttackableWeakPointNum(mHost);
            s32 count;
            if (num == 1)
                count = 8;
            else {
                count = 3;
                if (num == 2)
                    count = 5;
            }
            BellyDragonUtil::appearPodSparks(mHost, count);
        }
        setNerve(&NrvBellyDragonArmShot::End::sInstance);
    }
}
// NONMATCHING: literal symbol names.
void BellyDragonArmShot::exeEnd() {
    if (MR::isFirstStep(this) && !BellyDragonUtil::isLv2(mHost))
        MR::startAction(mHost, "FlyWaitNormal");
    TVec3f dir(0.0f);
    MR::calcVecToPlayerH(&dir, mHost, NULL);
    BellyDragon* host = mHost;
    MR::turnVecToVecDegree(&host->mFront, host->mFront, dir, mEndParam[3], TVec3f(0, 1, 0));
    BellyDragonUtil::pushBindedByFront(mHost, mEndParam[0], mEndParam[2]);
    if (MR::isGreaterStep(this, *mEndTime))
        kill();
}
BellyDragonArmShot::~BellyDragonArmShot() {}
