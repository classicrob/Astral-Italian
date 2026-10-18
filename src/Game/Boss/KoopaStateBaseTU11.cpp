#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStateCommon.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    const char* sDownDemoName = "クッパダウンデモ";
    const char* sDownCameraName = "DemoKoopaVs3Down";
    const char* sStartDemoName = "クッパ最終戦開始デモ";
    const char* sStartCameraName = "DemoKoopaLastBattleStart";
};  // namespace

namespace NrvKoopaStatePreFinalDemo {
    NEW_NERVE(KoopaStatePreFinalDemoNrvWait, KoopaStatePreFinalDemo, Wait);
    NEW_NERVE(KoopaStatePreFinalDemoNrvDemoDown, KoopaStatePreFinalDemo, DemoDown);
    NEW_NERVE(KoopaStatePreFinalDemoNrvStayLoop, KoopaStatePreFinalDemo, StayLoop);
    NEW_NERVE(KoopaStatePreFinalDemoNrvDemoStart, KoopaStatePreFinalDemo, DemoStart);
    NEW_NERVE(KoopaStatePreFinalDemoNrvDemoEnd, KoopaStatePreFinalDemo, DemoEnd);
};  // namespace NrvKoopaStatePreFinalDemo

KoopaStatePreFinalDemo::KoopaStatePreFinalDemo(KoopaBattle* pHost, const JMapInfoIter&)
    : ActorStateBase< KoopaBattle >("クッパ最終戦前シーケンス", pHost), mTicoBaby(nullptr), _14(nullptr), mPowerStarModel(nullptr), mCastActor(nullptr) {
}

void KoopaStatePreFinalDemo::setCastActor(LiveActor* pActor) {
    mCastActor = pActor;
}

void KoopaStatePreFinalDemo::initDemo(const JMapInfoIter& rIter) {
    initNerve(&NrvKoopaStatePreFinalDemo::KoopaStatePreFinalDemoNrvWait::sInstance, 0);
    MR::initAnimCamera(mHost, mHost->mCameraInfo, sDownCameraName);
    fn_80020860(mHost, rIter, sDownDemoName, sDownCameraName);
    MR::initAnimCamera(mHost, mHost->mCameraInfo, sStartCameraName);
    MR::initDemoSheetTalkAnim(mHost, rIter, sStartDemoName, sStartCameraName, nullptr, TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    mTicoBaby = reinterpret_cast< LiveActor* >(
        MR::createDemoObjWithAnimArc("デモ用チコベビー", "TicoBaby", "TicoBabyAnimVsKoopaLv3Galaxy", mHost->getBaseMtx()));
    mTicoBaby->makeActorDead();
    MR::initTalkAnimDemoCast(mTicoBaby, rIter, sDownDemoName, sDownCameraName);
    MR::initTalkAnimDemoCast(mTicoBaby, rIter, sStartDemoName, sStartCameraName);
    mPowerStarModel = MR::createPowerStarDemoModel(mHost, "パワースターデモモデル", mHost->getBaseMtx());
    mPowerStarModel->makeActorDead();
    MR::invalidateClipping(mPowerStarModel);
    MR::initTalkAnimDemoCast(mPowerStarModel, rIter, sDownDemoName, sDownCameraName);
    MR::initTalkAnimDemoCast(mPowerStarModel, rIter, sStartDemoName, sStartCameraName);
    if (mCastActor != nullptr) {
        MR::tryRegisterDemoCast(mCastActor, sStartDemoName, rIter);
    }
    MR::registerDemoActionFunctorDirect(mHost, MR::FunctorV0M< KoopaStatePreFinalDemo*, void (KoopaStatePreFinalDemo::*)() >(
                                                   this, &KoopaStatePreFinalDemo::lightOnFinalBattle),
                                        sStartDemoName, "ライトON[最終バトル]");
}

void KoopaStatePreFinalDemo::appear() {
    mIsDead = false;
    MR::appearKoopaDemoChamber();
    MR::killKoopaBattlePlanet();
    MR::setKoopaBattleNoA();
    mCastActor->makeActorDead();
    setNerve(&NrvKoopaStatePreFinalDemo::KoopaStatePreFinalDemoNrvWait::sInstance);
}

void KoopaStatePreFinalDemo::kill() {
    mIsDead = true;
    mPowerStarModel->makeActorDead();
    mTicoBaby->makeActorDead();
    MR::killKoopaDemoChamber();
}

void KoopaStatePreFinalDemo::lightOnFinalBattle() {
    MR::setKoopaBattleNo2();
}

void KoopaStatePreFinalDemo::exeWait() {
    if (MR::tryStartTimeKeepDemoMarioPuppetable(mHost, sDownDemoName, nullptr)) {
        MR::resetKoopaBattleDemo(mHost);
        MR::resetKoopaBattleToDemoChamber(mHost);
        mTicoBaby->makeActorAppeared();
        mPowerStarModel->makeActorAppeared();
        setNerve(&NrvKoopaStatePreFinalDemo::KoopaStatePreFinalDemoNrvDemoDown::sInstance);
    }
}

void KoopaStatePreFinalDemo::exeDemoDown() {
    if (MR::tryEndDemoAndKillEffects(mHost, sDownDemoName)) {
        mTicoBaby->makeActorDead();
        MR::hideModel(mHost);
        MR::startAction(mHost, "Wait");
        MR::setPlayerToKoopaDemoChamberEnd();
        setNerve(&NrvKoopaStatePreFinalDemo::KoopaStatePreFinalDemoNrvStayLoop::sInstance);
    }
}

void KoopaStatePreFinalDemo::exeStayLoop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mPowerStarModel, "DemoKoopaStayLoop");
        MR::setKoopaBattleNoA();
    }
    TVec3f pos;
    MR::copyJointPos(mPowerStarModel, "PowerStar", &pos);
    if (MR::isNearPlayer(pos, 350.0f)) {
        setNerve(&NrvKoopaStatePreFinalDemo::KoopaStatePreFinalDemoNrvDemoStart::sInstance);
    }
}

void KoopaStatePreFinalDemo::exeDemoStart() {
    if (MR::tryStartTimeKeepDemoMarioPuppetable(mHost, sStartDemoName, nullptr)) {
        mTicoBaby->makeActorAppeared();
        MR::startBreakKoopaDemoChamber();
        MR::resetKoopaBattleDemo(mHost);
        MR::resetKoopaBattleToDemoChamber(mHost);
        MR::showModel(mHost);
        MR::stopStageBGM(0);
        setNerve(&NrvKoopaStatePreFinalDemo::KoopaStatePreFinalDemoNrvDemoEnd::sInstance);
    }
}

void KoopaStatePreFinalDemo::exeDemoEnd() {
    if (MR::isDemoActive(sStartDemoName) && MR::isDemoPartActive("終了アニメ") && MR::getDemoPartStep("終了アニメ") == 150) {
        MR::setStageBGMState(2, 30);
    }
    if (MR::tryEndDemoAndKillEffects(mHost, sStartDemoName)) {
        mTicoBaby->makeActorDead();
        kill();
    }
}

KoopaStatePreFinalDemo::~KoopaStatePreFinalDemo() {
}
