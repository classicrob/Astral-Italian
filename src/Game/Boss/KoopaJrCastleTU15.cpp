#pragma auto_inline off
#include "Game/Boss/KoopaJrCastleNerves.hpp"
#include <cmath>
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"

namespace NrvKoopaJrCastleStateBattle4 { KoopaJrCastleStateBattle4NrvA KoopaJrCastleStateBattle4NrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle4 { KoopaJrCastleStateBattle4NrvB KoopaJrCastleStateBattle4NrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle4 { KoopaJrCastleStateBattle4NrvC KoopaJrCastleStateBattle4NrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle4::KoopaJrCastleStateBattle4(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x83_\x83\x81\x81[\x83W]", pHost) {}

using namespace KoopaJrCastleFunction;
using namespace NrvKoopaJrCastleStateBattle4;

void KoopaJrCastleStateBattle4::init() {
    initNerve(&KoopaJrCastleStateBattle4NrvA::sInstance, 0);
    MR::initAnimCamera(mHost->mBody, mHost->mCameraInfo, "DemoDownKoopaJrCastle");
    kill();
}

void KoopaJrCastleStateBattle4::appear() {
    mIsDead = false;
    KoopaJrCastle* castle = getCastle();
    stopBattleHelpers();
    s32 phase = castle->mStateBattle->mPhase;
    if (phase == 3) {
        castle->mBody->setNerveDummy1();
        castle->mHead->setNerveA();
        castle->mStateBattle->mPhase--;
        setNerve(&KoopaJrCastleStateBattle4NrvA::sInstance);
        return;
    }
    if (phase == 2) {
        castle->mBody->setNerveDummy2();
        castle->mHead->setNerveB();
        castle->mStateBattle->mPhase--;
        setNerve(&KoopaJrCastleStateBattle4NrvA::sInstance);
        return;
    }
    killFireBar();
    castle->mBody->setNerveDead();
    breakCastleCapsule(castle->mHead);
    castle->mStateBattle->mPhase--;
    MR::stopStageBGM(10);
    MR::startAction(castle->mKoopaJr, "Stumble");
    setNerve(&KoopaJrCastleStateBattle4NrvB::sInstance);
}

void KoopaJrCastleStateBattle4::exe800D93F0() {
    tryPushPlayerOffCastle(mHost);
    fn_800D5040(0.4f);
    if (MR::isGreaterStep(this, 120)) kill();
}

void KoopaJrCastleStateBattle4::exe800D9450() {
    if (MR::isFirstStep(this)) startPartsDemoActions("DemoDownKoopaJrCastle");
    resetPlayerToStart();
    if (MR::isStep(this, 320)) getCastle()->mBreakModel->appear();
    if (tryEndDemo("DemoDownKoopaJrCastle", nullptr, true)) kill();
}

KoopaJrCastleStateBattle4::~KoopaJrCastleStateBattle4() {}

void NrvKoopaJrCastleStateBattle4::KoopaJrCastleStateBattle4NrvB::execute(Spine* spine) const {
    KoopaJrCastleStateBattle4* state = reinterpret_cast<KoopaJrCastleStateBattle4*>(spine->mExecutor);
    if (KoopaJrCastleFunction::tryStartDemo("DemoDownKoopaJrCastle", nullptr))
        state->setNerve(&KoopaJrCastleStateBattle4NrvC::sInstance);
}
