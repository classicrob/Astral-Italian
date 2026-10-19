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

namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvA KoopaJrCastleStateBattle2NrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvB KoopaJrCastleStateBattle2NrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvC KoopaJrCastleStateBattle2NrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvD KoopaJrCastleStateBattle2NrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvE KoopaJrCastleStateBattle2NrvE::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvF KoopaJrCastleStateBattle2NrvF::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle2::KoopaJrCastleStateBattle2(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x8dU\x8c\x82:\x91\xe6\x82Q\x92i\x8aK]", pHost) {}

using namespace KoopaJrCastleFunction;
using namespace NrvKoopaJrCastleStateBattle2;

void KoopaJrCastleStateBattle2::init() {
    initNerve(&KoopaJrCastleStateBattle2NrvA::sInstance, 0);
    kill();
}

void KoopaJrCastleStateBattle2::appear() {
    mIsDead = false;
    setNerve(&KoopaJrCastleStateBattle2NrvC::sInstance);
}

void KoopaJrCastleStateBattle2::control() {
    appearFireBar();
    killRingBeam();
}

void KoopaJrCastleStateBattle2::exe800D82B0() {
    if (MR::isFirstStep(this)) startCannonVolley(2, 2);
    if (isPlayerOnCastle()) {
        setNerve(&KoopaJrCastleStateBattle2NrvE::sInstance);
        return;
    }
    if (isCannonDead()) setNerve(&KoopaJrCastleStateBattle2NrvB::sInstance);
}

void KoopaJrCastleStateBattle2::exe800D8330() {
    if (MR::isFirstStep(this)) startCannonBallVolley();
    if (MR::isGreaterEqualStep(this, 120)) {
        setNerve(&KoopaJrCastleStateBattle2NrvC::sInstance);
        return;
    }
    if (isPlayerOnCastle()) setNerve(&KoopaJrCastleStateBattle2NrvE::sInstance);
}

void KoopaJrCastleStateBattle2::exe800D83B0() {
    if (MR::isFirstStep(this)) {
        appearWindFromZeroPower();
        startCannonBallVolley();
    }
    if (windFn_800D4310()) appearElectric();
    if (MR::isStep(this, 120)) startCannonVolley(2, 2);
    else if (MR::isGreaterStep(this, 120) && isCannonDead()) startCannonBallVolley();
    if (isPlayerOnCastle() && isElectricActive()) {
        setNerve(&KoopaJrCastleStateBattle2NrvF::sInstance);
        return;
    }
    if (windIsDead() && isElectricDead()) setNerve(&KoopaJrCastleStateBattle2NrvD::sInstance);
}

void KoopaJrCastleStateBattle2::exe800D8490() {
    MR::isFirstStep(this);
    if (MR::isGreaterStep(this, 180)) {
        setNerve(&KoopaJrCastleStateBattle2NrvA::sInstance);
        return;
    }
    if (isPlayerOnCastle()) setNerve(&KoopaJrCastleStateBattle2NrvE::sInstance);
}

void KoopaJrCastleStateBattle2::exe800D8500() {
    if (MR::isFirstStep(this)) startHeadPanic();
    if (isHeadFn_800D4790()) setNerve(&KoopaJrCastleStateBattle2NrvA::sInstance);
}

void KoopaJrCastleStateBattle2::exe800D8550() {
    if (MR::isFirstStep(this)) startHeadSpinSign();
    if (isHeadFn_800D4790()) setNerve(&KoopaJrCastleStateBattle2NrvB::sInstance);
}

KoopaJrCastleStateBattle2::~KoopaJrCastleStateBattle2() {}
