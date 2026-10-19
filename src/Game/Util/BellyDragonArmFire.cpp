#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    const f32 cSignA[] = {0.8f, -0.5f, 0.9f, 2.5f};
    const f32 cSignB[] = {1.0f, -0.5f, 0.9f, 3.0f};
    const f32 cDownA[] = {1.5f, 0.0f, 0.93f, 2.0f};
    const f32 cDownB[] = {1.9f, 0.0f, 0.93f, 2.0f};
    const f32 cLandA[] = {1.2f, 0.5f, 0.93f, 1.0f};
    const f32 cLandB[] = {1.5f, 0.5f, 0.93f, 1.0f};
    const f32 cUpA[] = {1.5f, -1.0f, 0.9f, 2.0f};
    const f32 cUpB[] = {1.5f, -1.0f, 0.9f, 2.5f};
    const f32 cCancel[] = {1.5f, -0.5f, 0.9f, 1.0f};
    const s32 cAttackNumA = 1, cAttackNumLv2 = 1, cAttackNumB = 2;
    const s32 cSignTimeA = 60, cSignTimeB = 45;
    const s32 cAimTimeA = 35, cAimTimeB = 40;
    const s32 cUpTimeA = 90, cUpTimeB = 60;
}
namespace NrvBellyDragonArmFire {
    NEW_NERVE(Sign, BellyDragonArmFire, Sign);
    NEW_NERVE(Down, BellyDragonArmFire, Down);
    NEW_NERVE(Land, BellyDragonArmFire, Land);
    NEW_NERVE(Up, BellyDragonArmFire, Up);
    NEW_NERVE(Cancel, BellyDragonArmFire, Cancel);
}

BellyDragonArmFire::BellyDragonArmFire(BellyDragon* pHost, u8 first)
    : ActorStateBase<BellyDragon>("State[噛み付き攻撃]", pHost), mIsLv2(false), mAttackCount(0) {
    mTargetPos.set(0.0f, 0.0f, 0.0f);
    mIsEnd = false;
    mAttackNum = &cAttackNumA;
    mSignTime = &cSignTimeA;
    mSignParam = cSignA;
    mAimTime = &cAimTimeA;
    mDownParam = cDownA;
    mLandParam = cLandA;
    mUpTime = &cUpTimeA;
    mUpParam = cUpA;
    if (BellyDragonUtil::isLv2(mHost))
        mAttackNum = &cAttackNumLv2;
    if (first) {
        mIsLv2 = false;
    } else {
        mIsLv2 = true;
        mAttackNum = &cAttackNumB;
        mSignTime = &cSignTimeB;
        mSignParam = cSignB;
        mAimTime = &cAimTimeB;
        mDownParam = cDownB;
        mLandParam = cLandB;
        mUpTime = &cUpTimeB;
        mUpParam = cUpB;
    }
}

void BellyDragonArmFire::init() {
    initNerve(&NrvBellyDragonArmFire::Sign::sInstance, 0);
    kill();
}
void BellyDragonArmFire::appear() {
    mIsDead = false;
    mAttackCount = 0;
    mIsEnd = false;
    setNerve(&NrvBellyDragonArmFire::Sign::sInstance);
}
void BellyDragonArmFire::kill() {
    mIsDead = true;
    MR::onBind(mHost);
}
bool BellyDragonArmFire::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvBellyDragonArmFire::Down::sInstance) || isNerve(&NrvBellyDragonArmFire::Land::sInstance)) {
        if (BellyDragonUtil::isLv2(mHost))
            MR::sendMsgEnemyAttackFire(pReceiver, pSender);
        else
            MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
        return true;
    }
    return false;
}
void BellyDragonArmFire::exeSign() {
    if (MR::isFirstStep(this))
        MR::startAction(mHost, "AttackJawSign");
    BellyDragonUtil::pullBindedByFront(mHost, mSignParam);
    BellyDragonUtil::updatePodFireEffect(mHost);
    if (MR::isGreaterStep(this, *mSignTime))
        setNerve(&NrvBellyDragonArmFire::Down::sInstance);
}
// NONMATCHING: vector inlining and register allocation.
void BellyDragonArmFire::exeDown() {
    if (MR::isFirstStep(this))
        MR::startAction(mHost, "AttackJawDown");
    if (MR::isLessEqualStep(this, *mAimTime)) {
        if (MR::isOnGroundPlayer())
            mTargetPos.set(*MR::getPlayerPos());
        else
            MR::calcMapHitAlongDirection(*MR::getPlayerPos(), &mTargetPos, *MR::getPlayerGravity(), NULL, NULL, NULL, NULL, 1000.0f);
    }
    TVec3f dir(mTargetPos);
    dir -= mHost->mPosition;
    if (!MR::isNearZero(dir, 0.001f)) {
        MR::normalize(&dir);
        MR::turnVecToVecDegree(&mHost->mFront, mHost->mFront, dir, mDownParam[3], TVec3f(0.0f, 1.0f, 0.0f));
    }
    BellyDragonUtil::pushBindedByFront(mHost, mDownParam[0], mDownParam[2]);
    BellyDragonUtil::updatePodFireEffect(mHost);
    if (!MR::isNoBind(mHost) && MR::isBinded(mHost)) {
        MR::offBind(mHost);
        MR::sendMsgEnemyAttackToBindedSensor(mHost, BellyDragonUtil::getBodySensor(mHost));
        MR::tryHitReaction(mHost, "噛み付き攻撃（接地）");
        BellyDragonUtil::appearJawHill(mHost);
        setNerve(&NrvBellyDragonArmFire::Land::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 180)) {
        setNerve(&NrvBellyDragonArmFire::Up::sInstance);
        return;
    }
    tryPlayerDamage();
}
void BellyDragonArmFire::exeLand() {
    if (MR::isFirstStep(this))
        MR::startAction(mHost, "AttackJaw");
    if (BellyDragonUtil::isLv2(mHost) && MR::isStep(this, 25))
        BellyDragonUtil::appearFireGround(mHost);
    BellyDragonUtil::pullBindedByFront(mHost, mLandParam);
    tryPlayerDamage();
    BellyDragonUtil::updatePodFireEffect(mHost);
    if (MR::isGreaterStep(this, 30)) {
        if (mIsEnd) {
            setNerve(&NrvBellyDragonArmFire::Cancel::sInstance);
            return;
        }
        setNerve(&NrvBellyDragonArmFire::Up::sInstance);
    }
}
void BellyDragonArmFire::exeUp() {
    if (MR::isFirstStep(this)) {
        if (mAttackCount + 1 >= *mAttackNum)
            MR::startAction(mHost, "AttackDiveSign");
        else
            MR::startAction(mHost, "AttackJawSign");
    }
    BellyDragonUtil::pullBindedByFront(mHost, mUpParam);
    tryPlayerDamage();
    if (mAttackCount <= *mAttackNum)
        BellyDragonUtil::updatePodFireEffect(mHost);
    if (MR::isGreaterStep(this, *mUpTime)) {
        MR::onBind(mHost);
        if (++mAttackCount >= *mAttackNum || mIsEnd) {
            kill();
            return;
        }
        setNerve(&NrvBellyDragonArmFire::Down::sInstance);
    }
}
void BellyDragonArmFire::exeCancel() {
    if (MR::isFirstStep(this))
        MR::startAction(mHost, "AttackJawCancel");
    MR::escapeFromPlayer(mHost, &mHost->mFront, cCancel[0], cCancel[1], cCancel[2], cCancel[3]);
    BellyDragonUtil::updatePodFireEffect(mHost);
    if (MR::isGreaterStep(this, 90)) {
        MR::onBind(mHost);
        kill();
    }
}
bool BellyDragonArmFire::tryPlayerDamage() {
    if (MR::isPlayerDamaging() && !mIsEnd) {
        mIsEnd = true;
        return true;
    }
    return false;
}
BellyDragonArmFire::~BellyDragonArmFire() {}
