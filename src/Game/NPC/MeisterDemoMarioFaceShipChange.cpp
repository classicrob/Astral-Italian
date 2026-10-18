#include "Game/NPC/MeisterDemoMarioFaceShipChange.hpp"
#include "Game/NPC/Meister.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvMeisterDemoMarioFaceShipChange {
    NEW_NERVE(MeisterDemoMarioFaceShipChangeNrvWait, MeisterDemoMarioFaceShipChange, Wait);
}
namespace { TVec3f sStarOffset(-230.0f, 210.0f, -50.0f); }
MeisterDemoMarioFaceShipChange::MeisterDemoMarioFaceShipChange(Meister* host, const JMapInfoIter& iter) : NerveExecutor("マイスターデモ[マリオ顔惑星変化]"), mHost(host), mStar(nullptr), mUnused40(0.0f) {
    DemoFunction::registerDemoTalkMessageCtrlDirect(host, host->mMsgCtrl, "マリオ顔惑星変化");
    MR::registerDemoActionFunctorDirect(mHost, MR::Functor_Inline(this, &MeisterDemoMarioFaceShipChange::start), "マリオ顔惑星変化", nullptr);
    mStarMtx.identity();
    mStar = MR::createDemoObj("スターモデル", "PowerStar", "DemoWithShadow", mStarMtx);
    mStar->makeActorDead();
    if (GameSequenceFunction::hasStageResultSequence()) PowerStar::setupColorAtResultSequence(mStar, false);
    MR::tryRegisterDemoCast(mStar, "マリオ顔惑星変化", iter);
    initNerve(&NrvMeisterDemoMarioFaceShipChange::MeisterDemoMarioFaceShipChangeNrvWait::sInstance, 0);
}
void MeisterDemoMarioFaceShipChange::exeWait() {
    mStarMtx.set(mHost->getBaseMtx());
    if (MR::isDemoLastStep()) mHost->endDemoState();
}
void MeisterDemoMarioFaceShipChange::start() {
    mStarMtx.set(mHost->getBaseMtx());
    MR::emitEffect(mStar, "Light");
    mHost->startDemoState(this);
}
MeisterDemoMarioFaceShipChange::~MeisterDemoMarioFaceShipChange() {}
