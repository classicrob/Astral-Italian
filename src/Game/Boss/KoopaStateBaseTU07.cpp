#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    const char* sOpeningDemoName = "クッパオープニングデモ";
};  // namespace

inline void KoopaStateOpeningDemo::exeDemo() {
    if (MR::tryEndDemoAndKillEffects(mHost, sOpeningDemoName)) {
        kill();
    }
}

namespace NrvKoopaStateOpeningDemo {
    NEW_NERVE(KoopaStateOpeningDemoNrvWait, KoopaStateOpeningDemo, Wait);
    NEW_NERVE(KoopaStateOpeningDemoNrvDemo, KoopaStateOpeningDemo, Demo);
};  // namespace NrvKoopaStateOpeningDemo

KoopaStateOpeningDemo::KoopaStateOpeningDemo(KoopaBattle* pHost, const JMapInfoIter& rIter, const char* pCameraName, const char* pArcName,
                                             bool hasPeach)
    : ActorStateBase< KoopaBattle >(sOpeningDemoName, pHost), mTicoBaby(nullptr), mPeach(nullptr), mPeachCage(nullptr) {
    initNerve(&NrvKoopaStateOpeningDemo::KoopaStateOpeningDemoNrvWait::sInstance, 0);
    MR::initAnimCamera(mHost, mHost->mCameraInfo, pCameraName);
    MR::initDemoSheetTalkAnim(mHost, rIter, sOpeningDemoName, pCameraName, nullptr, TVec3f(0.0f, 0.0f, 0.0f), MR::getJointMtx(mHost, "Face"));
    if (hasPeach) {
        mPeach = reinterpret_cast< LiveActor* >(MR::createDemoObjWithAnimArc("デモ用ピーチ", "Peach", "PeachAnim", "Demo", mHost->getBaseMtx()));
        mPeach->makeActorDead();
        MR::initTalkAnimDemoCast(mPeach, rIter, sOpeningDemoName, pCameraName);
        mPeachCage = reinterpret_cast< LiveActor* >(MR::createDemoObj("デモ用ピーチ泡籠", "PeachBubbleCage", "Demo", pHost->getBaseMtx()));
        mPeachCage->makeActorDead();
        MR::initTalkAnimDemoCast(mPeachCage, rIter, sOpeningDemoName, pCameraName);
    }
    mTicoBaby = reinterpret_cast< LiveActor* >(MR::createDemoObjWithAnimArc("デモ用チコベビー", "TicoBaby", pArcName, pHost->getBaseMtx()));
    mTicoBaby->makeActorDead();
    MR::initTalkAnimDemoCast(mTicoBaby, rIter, sOpeningDemoName, pCameraName);
}

void KoopaStateOpeningDemo::appear() {
    mIsDead = false;
    MR::appearKoopaDemoChamber();
    MR::killKoopaBattlePlanet();
    MR::setKoopaBattleNoA();
    setNerve(&NrvKoopaStateOpeningDemo::KoopaStateOpeningDemoNrvWait::sInstance);
}

void KoopaStateOpeningDemo::kill() {
    mIsDead = true;
    MR::killKoopaDemoChamber();
    mTicoBaby->makeActorDead();
    if (mPeach != nullptr) {
        mPeach->makeActorDead();
        mPeachCage->makeActorDead();
    }
}

void KoopaStateOpeningDemo::exeWait() {
    if (MR::tryStartTimeKeepDemoMarioPuppetable(mHost, sOpeningDemoName, nullptr)) {
        MR::resetKoopaBattleDemo(mHost);
        MR::resetKoopaBattleToDemoChamber(mHost);
        mTicoBaby->makeActorAppeared();
        if (mPeach != nullptr) {
            mPeach->makeActorAppeared();
            mPeachCage->makeActorAppeared();
        }
        setNerve(&NrvKoopaStateOpeningDemo::KoopaStateOpeningDemoNrvDemo::sInstance);
    }
}

KoopaStateOpeningDemo::~KoopaStateOpeningDemo() {
}
