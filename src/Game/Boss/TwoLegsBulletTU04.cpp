#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvTwoLegsStateShot {
    NEW_NERVE(TwoLegsStateShotNrvStandUp, TwoLegsStateShot, StandUp);
    NEW_NERVE(TwoLegsStateShotNrvAttack, TwoLegsStateShot, Attack);
    NEW_NERVE(TwoLegsStateShotNrvReverseSign, TwoLegsStateShot, ReverseSign);
    NEW_NERVE(TwoLegsStateShotNrvSitDown, TwoLegsStateShot, SitDown);
};  // namespace NrvTwoLegsStateShot

TwoLegsStateShot::TwoLegsStateShot(TwoLegs* pHost, const JMapInfoIter& rIter)
    : ActorStateBase< TwoLegs >("State[\x92""e\x82\xf0\x94\xad\x8e\xcb]", pHost), mBullets(nullptr), mBulletNum(0), mShotCount(0) {
    mBulletNum = MR::getChildObjNum(rIter);
    mBullets = new TwoLegsBullet*[mBulletNum];
    for (s32 i = 0; i < mBulletNum; i++) {
        mBullets[i] = new TwoLegsBullet(mHost, "\x93\xf1\x8br\x83{\x83X\x82\xcc\x92""e");
        MR::initChildObj(mBullets[i], rIter, i);
    }
}

TwoLegsStateShot::~TwoLegsStateShot() {}

void TwoLegsStateShot::init() {
    initNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvStandUp::sInstance, 0);
}

void TwoLegsStateShot::appear() {
    mIsDead = false;
    mHost->jointTrackOnReturn();
    mShotCount = 0;
    if (mHost->isPhase0()) {
        setNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvStandUp::sInstance);
        return;
    }
    setNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvAttack::sInstance);
}

bool TwoLegsStateShot::isAllBulletDead() {
    if (mHost->isLastPhase()) {
        return countAliveBullets() < mBulletNum;
    }
    return countAliveBullets() == 0;
}

void TwoLegsStateShot::killBullets() {
    for (s32 i = 0; i < mBulletNum; i++) {
        if (!MR::isDead(mBullets[i])) {
            mBullets[i]->kill();
        }
    }
}

s32 TwoLegsStateShot::countAliveBullets() {
    s32 count = 0;
    for (s32 i = 0; i < mBulletNum; i++) {
        if (!MR::isDead(mBullets[i])) {
            count++;
        }
    }
    return count;
}

void TwoLegsStateShot::shootBullet() {
    TwoLegsBullet* pNew = nullptr;
    TwoLegsBullet* pAlive = nullptr;
    for (s32 i = 0; i < mBulletNum; i++) {
        TwoLegsBullet* pBullet = mBullets[i];
        if (pNew == nullptr && MR::isDead(pBullet)) {
            MR::setPosition(pBullet, mHost->mPosition);
            pBullet->appear();
            MR::tryRumblePadStrong(this, 0);
            MR::shakeCameraNormal();
            pNew = pBullet;
        } else if (!MR::isDead(pBullet)) {
            pAlive = pBullet;
        }
    }
    if (pAlive != nullptr) {
        if (MR::isRailGoingToEnd(pNew) != MR::isRailGoingToEnd(pAlive)) {
            pNew->reverseMove();
        }
    }
}

bool TwoLegsStateShot::tryAttackStrong(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvReverseSign::sInstance)) {
        return TwoLegsStateWalkDamage::tryStrongStamp(this, mHost, pSender, pReceiver);
    }
    return false;
}

bool TwoLegsStateShot::tryAttack(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvReverseSign::sInstance)) {
        return TwoLegsStateWalkDamage::tryStamp(this, mHost, pSender, pReceiver);
    }
    return false;
}

void TwoLegsStateShot::exeStandUp() {
    if (MR::isFirstStep(this)) {
        mHost->startActionUp();
    }
    if (MR::isActionEnd(mHost) && !mHost->tryKillStateIfPlayerBetweenFeet(this)) {
        setNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvAttack::sInstance);
    }
}

void TwoLegsStateShot::exeAttack() {
    if (MR::isFirstStep(this)) {
        mShotCount++;
        MR::startAction(mHost, "Attack");
    }
    if (MR::isStep(this, 30)) {
        shootBullet();
    }
    if (MR::isActionEnd(mHost) && !mHost->tryKillStateIfPlayerBetweenFeet(this)) {
        if (mHost->isPhase0()) {
            setNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvReverseSign::sInstance);
            return;
        }
        if (mHost->isLastPhase()) {
            if (countAliveBullets() < mBulletNum && mShotCount < 4) {
                setNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvAttack::sInstance);
                return;
            }
        } else if (countAliveBullets() == 0 && mShotCount < 4) {
            setNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvAttack::sInstance);
            return;
        }
        kill();
    }
}

void TwoLegsStateShot::exeReverseSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "ReverseSign");
    }
    if (MR::isActionEnd(mHost) && !mHost->tryKillStateIfPlayerBetweenFeet(this)) {
        setNerve(&NrvTwoLegsStateShot::TwoLegsStateShotNrvSitDown::sInstance);
    }
}

void TwoLegsStateShot::exeSitDown() {
    if (MR::isFirstStep(this)) {
        mHost->startActionDown();
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}
