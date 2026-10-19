#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    const char* sDownDemoName = "クッパダウンデモ";
};  // namespace

namespace NrvKoopaStateDownDemo {
    NEW_NERVE(KoopaStateDownDemoNrvWait, KoopaStateDownDemo, Wait);
    NEW_NERVE(KoopaStateDownDemoNrvDemo, KoopaStateDownDemo, Demo);
};  // namespace NrvKoopaStateDownDemo

// NONMATCHING: host-pointer load scheduling.
KoopaStateDownDemo::KoopaStateDownDemo(KoopaBattle* pHost, const JMapInfoIter& rIter, const char* pCameraName, const char* pArcName)
    : ActorStateBase< KoopaBattle >(sDownDemoName, pHost), mTicoBaby(nullptr), mPowerStarModel(nullptr) {
    initNerve(&NrvKoopaStateDownDemo::KoopaStateDownDemoNrvWait::sInstance, 0);
    MR::initAnimCamera(mHost, mHost->mCameraInfo, pCameraName);
    KoopaBattle* pDemoHost = mHost;
    MR::initDemoSheetTalkAnim(pDemoHost, rIter, sDownDemoName, pCameraName, nullptr, TVec3f(0.0f, 0.0f, 0.0f), MR::getJointMtx(pDemoHost, "Face"));
    mTicoBaby = reinterpret_cast< LiveActor* >(MR::createDemoObjWithAnimArc("デモ用チコベビー", "TicoBaby", pArcName, pHost->getBaseMtx()));
    mTicoBaby->makeActorDead();
    MR::initTalkAnimDemoCast(mTicoBaby, rIter, sDownDemoName, pCameraName);
    KoopaBattle* pStarHost = mHost;
    mPowerStarModel = MR::createPowerStarDemoModel(pStarHost, "パワースターデモモデル", pHost->getBaseMtx());
    mPowerStarModel->makeActorDead();
    MR::initTalkAnimDemoCast(mPowerStarModel, rIter, sDownDemoName, pCameraName);
}

void KoopaStateDownDemo::appear() {
    MR::resetKoopaBattleDemo(mHost);
    mIsDead = false;
    MR::appearKoopaDemoChamber();
    MR::killKoopaBattlePlanet();
    setNerve(&NrvKoopaStateDownDemo::KoopaStateDownDemoNrvWait::sInstance);
}

void KoopaStateDownDemo::kill() {
    mIsDead = true;
    MR::invalidateShadowAll(mHost);
}

void KoopaStateDownDemo::exeWait() {
    if (MR::tryStartTimeKeepDemoMarioPuppetable(mHost, sDownDemoName, nullptr)) {
        MR::validateShadowGroup(mHost, "DemoDown");
        MR::resetKoopaBattleDemo(mHost);
        MR::resetKoopaBattleToDemoChamber(mHost);
        MR::setKoopaBattleNoA();
        mPowerStarModel->makeActorAppeared();
        mTicoBaby->makeActorAppeared();
        setNerve(&NrvKoopaStateDownDemo::KoopaStateDownDemoNrvDemo::sInstance);
    }
}

void KoopaStateDownDemo::exeDemo() {
    if (MR::isDemoLastStep()) {
        mPowerStarModel->makeActorDead();
        mTicoBaby->makeActorDead();
        MR::appearPowerStarWithoutDemo(mHost);
        MR::setPlayerToKoopaDemoChamberEnd();
        kill();
    }
}

KoopaStateDownDemo::~KoopaStateDownDemo() {
}
