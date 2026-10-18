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

namespace NrvKoopaJrCastleStateOpening { KoopaJrCastleStateOpeningNrvA KoopaJrCastleStateOpeningNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateOpening { KoopaJrCastleStateOpeningNrvB KoopaJrCastleStateOpeningNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateOpening { KoopaJrCastleStateOpeningNrvC KoopaJrCastleStateOpeningNrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateOpening::KoopaJrCastleStateOpening(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("Phase[\x83I\x81[\x83v\x83j\x83\x93\x83O]", pHost) {}

using namespace KoopaJrCastleFunction;
using namespace NrvKoopaJrCastleStateOpening;

void KoopaJrCastleStateOpening::init() {
    initNerve(&KoopaJrCastleStateOpeningNrvA::sInstance, 0);
    kill();
}

void KoopaJrCastleStateOpening::appear() {
    mIsDead = false;
    setNerve(&KoopaJrCastleStateOpeningNrvA::sInstance);
}

void KoopaJrCastleStateOpening::exe800D6D80() {
    fn_800D3970();
    fn_800D3A40();
    if (MR::isOnSwitchA(mHost)) {
        KoopaJrCastle* castle = getCastle();
        MR::invalidateClipping(castle);
        castle->mKoopaJr->makeActorAppeared();
        setNerve(&KoopaJrCastleStateOpeningNrvB::sInstance);
    }
}

void KoopaJrCastleStateOpening::exe800D6E00() {
    if (MR::isFirstStep(this)) MR::stopStageBGM(10);
    if (tryStartDemo("\x83\x4e\x83\x62\x83\x70Jr\x83\x4c\x83\x83\x83\x62\x83\x58\x83\x8b\x93\x6f\x8f\xea", "DemoOpeningKoopaJrCastle")) {
        MR::endStartPosCamera();
        MR::validateShadow(mHost->mKoopaJr, "Opening");
        MR::invalidateShadow(mHost->mKoopaJr, "Body");
        setNerve(&KoopaJrCastleStateOpeningNrvC::sInstance);
    }
}

// NONMATCHING: target initializes unused r4 before the one-argument setPlayerPosAndWait call
void KoopaJrCastleStateOpening::exe800D6E90() {
    if (MR::isFirstStep(this)) fn_800D3770("DemoOpeningKoopaJrCastle");
    if (MR::isDemoPartFirstStep("\x83\x74\x83\x40\x83\x43\x83\x41\x83\x6f\x81\x5b\x8b\x4e\x93\xae")) {
        mHost->mBody->onFlagF6();
        appearFireBar();
    }
    if (MR::isDemoLastStep()) MR::setPlayerPosAndWait("\x83\x7d\x83\x8a\x83\x49\x88\xca\x92\x75[\x83\x66\x83\x82\x8c\xe3]");
    if (tryEndDemo("\x83\x4e\x83\x62\x83\x70Jr\x83\x4c\x83\x83\x83\x62\x83\x58\x83\x8b\x93\x6f\x8f\xea", "DemoOpeningKoopaJrCastle", false)) {
        MR::overlayWithPreviousScreen(8);
        mHost->mBody->onFlagF6();
        appearFireBarAndStart();
        MR::invalidateShadow(mHost->mKoopaJr, "Opening");
        MR::validateShadow(mHost->mKoopaJr, "Body");
        kill();
    }
}

KoopaJrCastleStateOpening::~KoopaJrCastleStateOpening() {}
