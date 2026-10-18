#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util.hpp"

namespace {
    const char* sEndDemoName = "クッパ最終戦終了デモ";
    const char* sEndDemoCameraName = "DemoKoopaLastBattleEnd";
};  // namespace

namespace NrvKoopaStateEndDemo {
    NEW_NERVE(KoopaStateEndDemoNrvWait, KoopaStateEndDemo, Wait);
    NEW_NERVE(KoopaStateEndDemoNrvDemo, KoopaStateEndDemo, Demo);
};  // namespace NrvKoopaStateEndDemo

KoopaStateEndDemo::KoopaStateEndDemo(KoopaBattle* pHost)
    : ActorStateBase< KoopaBattle >("クッパ最終戦終了デモ", pHost), mPeach(nullptr), mPowerStarModel(nullptr), mCastActor(nullptr), mTicoBaby(nullptr) {
}

void KoopaStateEndDemo::setCastActor(LiveActor* pActor) {
    mCastActor = pActor;
}

void KoopaStateEndDemo::initDemo(const JMapInfoIter& rIter) {
    initNerve(&NrvKoopaStateEndDemo::KoopaStateEndDemoNrvWait::sInstance, 0);
    MR::initAnimCamera(mHost, mHost->mCameraInfo, sEndDemoCameraName);
    MR::initDemoSheetTalkAnim(mHost, rIter, sEndDemoName, sEndDemoCameraName, nullptr, TVec3f(0.0f, 0.0f, 0.0f), MR::getJointMtx(mHost, "Face"));
    mPeach = reinterpret_cast< LiveActor* >(MR::createDemoObjWithAnimArc("デモ用ピーチ", "Peach", "PeachAnim", "Demo", mHost->getBaseMtx()));
    mPeach->makeActorDead();
    MR::initTalkAnimDemoCast(mPeach, rIter, sEndDemoName, sEndDemoCameraName);
    mPowerStarModel = MR::createPowerStarDemoModel(mHost, "パワースターデモモデル", mHost->getBaseMtx());
    mPowerStarModel->makeActorDead();
    MR::initTalkAnimDemoCast(mPowerStarModel, rIter, sEndDemoName, sEndDemoCameraName);
    mTicoBaby = reinterpret_cast< LiveActor* >(
        MR::createDemoObjWithAnimArc("デモ用チコベビー", "TicoBaby", "TicoBabyAnimVsKoopaLv3Galaxy", mHost->getBaseMtx()));
    mTicoBaby->makeActorDead();
    MR::initTalkAnimDemoCast(mTicoBaby, rIter, sEndDemoName, sEndDemoCameraName);
    MR::registerDemoActionFunctorDirect(
        mHost, MR::Functor(this, &KoopaStateEndDemo::lightOnPeach), sEndDemoName,
        "ライトON[ピーチ]");
    MR::registerDemoActionFunctorDirect(
        mHost, MR::Functor(this, &KoopaStateEndDemo::lightOnLastStar), sEndDemoName,
        "ライトON[ラストスター]");
    MR::registerDemoActionFunctorDirect(
        mHost, MR::Functor(this, &KoopaStateEndDemo::lightOnLastStar), sEndDemoName,
        "ライトON[ラストスター2]");
    if (mCastActor != nullptr) {
        MR::tryRegisterDemoCast(mCastActor, sEndDemoName, rIter);
    }
}

void KoopaStateEndDemo::lightOnPeach() {
    MR::setKoopaBattleNoB();
}

void KoopaStateEndDemo::lightOnLastStar() {
    MR::setKoopaBattleNoC();
}

void KoopaStateEndDemo::appear() {
    MR::validateKoopaBattleCollision(mHost);
    mHost->getDefenseForm()->kill();
    MR::resetKoopaBattleDemo(mHost);
    MR::appearKoopaFinalStarPlanet();
    MR::killKoopaBattlePlanet();
    mIsDead = false;
    setNerve(&NrvKoopaStateEndDemo::KoopaStateEndDemoNrvWait::sInstance);
}

void KoopaStateEndDemo::kill() {
    mPeach->makeActorDead();
    mPowerStarModel->makeActorDead();
    mTicoBaby->makeActorDead();
    mIsDead = true;
}

void KoopaStateEndDemo::exeWait() {
    if (MR::tryStartTimeKeepDemoMarioPuppetable(mHost, sEndDemoName, nullptr)) {
        MR::resetKoopaBattleDemo(mHost);
        MR::sendMsgKoopaC6();
        MR::resetKoopaBattleToFinalStar(mHost);
        mPeach->makeActorAppeared();
        mPowerStarModel->makeActorAppeared();
        mTicoBaby->makeActorAppeared();
        setNerve(&NrvKoopaStateEndDemo::KoopaStateEndDemoNrvDemo::sInstance);
    }
}

void KoopaStateEndDemo::exeDemo() {
    if (MR::isDemoLastStep()) {
        MR::setPlayerPosAndWait("マリオ最終デモ終了後位置");
        MR::appearPowerStarWithoutDemo(mHost);
        kill();
    }
}

KoopaStateEndDemo::~KoopaStateEndDemo() {
}
