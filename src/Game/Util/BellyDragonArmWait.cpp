#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    const f32 cMoveA[] = {2.0f, -0.8f, 0.9f, 1.5f};
    const f32 cWaitA[] = {1.5f, 1.3f, 0.9f, 2.0f};
    const f32 cMoveB[] = {2.5f, -1.2f, 0.9f, 1.5f};
    const f32 cWaitB[] = {2.0f, 1.6f, 0.9f, 2.5f};
    const f32 cSightA[] = {1500.0f, 45.0f, 70.0f};
    const f32 cSightB[] = {1500.0f, 45.0f, 85.0f};
    const s32 cMoveTimeA = 120, cWaitTimeA = 180;
    const s32 cMoveTimeB = 60, cWaitTimeB = 120;
}
namespace NrvBellyDragonArmWait {
    NEW_NERVE(Move, BellyDragonArmWait, Move);
    NEW_NERVE(Wait, BellyDragonArmWait, Wait);
}
BellyDragonArmWait::BellyDragonArmWait(BellyDragon* pHost, u8 first)
    : ActorStateBase<BellyDragon>("State[回遊]", pHost), mMoveCount(2), mMoveTime(&cMoveTimeA), mMoveParam(cMoveA),
      mWaitTime(&cWaitTimeA), mWaitParam(cWaitA), mSightParam(cSightA) {
    if (!first) {
        mMoveTime = &cMoveTimeB;
        mMoveParam = cMoveB;
        mWaitTime = &cWaitTimeB;
        mWaitParam = cWaitB;
        mSightParam = cSightB;
    }
}
void BellyDragonArmWait::init() {
    initNerve(&NrvBellyDragonArmWait::Move::sInstance, 0);
    mMoveCount = 2;
    kill();
}
void BellyDragonArmWait::appear() {
    mIsDead = false;
    MR::startAction(mHost, "FlyWaitNormal");
    setNerve(&NrvBellyDragonArmWait::Wait::sInstance);
}
void BellyDragonArmWait::startAttack() { mMoveCount = 2; }
void BellyDragonArmWait::startDamage() { mMoveCount = 0; }
void BellyDragonArmWait::exeMove() {
    if (MR::isFirstStep(this))
        --mMoveCount;
    BellyDragonUtil::pullBindedByFront(mHost, mMoveParam);
    if (!tryEnd() && MR::isGreaterStep(this, *mMoveTime))
        setNerve(&NrvBellyDragonArmWait::Wait::sInstance);
}
void BellyDragonArmWait::exeWait() {
    BellyDragonUtil::pullBindedByFront(mHost, mWaitParam);
    if (!tryEnd() && (MR::isGreaterStep(this, *mWaitTime) || MR::isBinded(mHost)))
        setNerve(&NrvBellyDragonArmWait::Move::sInstance);
}
bool BellyDragonArmWait::tryEnd() {
    if (mMoveCount <= 0 && MR::isInSightFanPlayer(mHost, mHost->mFront, mSightParam[0], mSightParam[1], mSightParam[2])) {
        kill();
        return true;
    }
    return false;
}
BellyDragonArmWait::~BellyDragonArmWait() {}
