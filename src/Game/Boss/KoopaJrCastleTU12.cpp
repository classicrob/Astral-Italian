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

namespace NrvKoopaJrCastleStateBattle1 { KoopaJrCastleStateBattle1NrvA KoopaJrCastleStateBattle1NrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle1 { KoopaJrCastleStateBattle1NrvB KoopaJrCastleStateBattle1NrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle1 { KoopaJrCastleStateBattle1NrvC KoopaJrCastleStateBattle1NrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle1::KoopaJrCastleStateBattle1(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x8dU\x8c\x82:\x91\xe6\x82P\x92i\x8aK]", pHost) {}

using namespace KoopaJrCastleFunction;
using namespace NrvKoopaJrCastleStateBattle1;

void KoopaJrCastleStateBattle1::init() {
    initNerve(&KoopaJrCastleStateBattle1NrvA::sInstance, 0);
    kill();
}

void KoopaJrCastleStateBattle1::appear() {
    mIsDead = false;
    setNerve(&KoopaJrCastleStateBattle1NrvA::sInstance);
}

void KoopaJrCastleStateBattle1::control() {
    appearFireBar();
    killRingBeam();
}

void KoopaJrCastleStateBattle1::exe800D7F60() {
    if (MR::isFirstStep(this)) startCannonVolley(2, 2);
    appearFireBar();
    killRingBeam();
    if (isPlayerOnCastle()) {
        setNerve(&KoopaJrCastleStateBattle1NrvC::sInstance);
        return;
    }
    if (isCannonDead()) setNerve(&KoopaJrCastleStateBattle1NrvB::sInstance);
}

void KoopaJrCastleStateBattle1::exe800D7FE0() {
    if (MR::isFirstStep(this)) startCannonBallVolley();
    appearFireBar();
    killRingBeam();
    if (isPlayerOnCastle()) {
        setNerve(&KoopaJrCastleStateBattle1NrvC::sInstance);
        return;
    }
    if (MR::isGreaterEqualStep(this, 120)) setNerve(&KoopaJrCastleStateBattle1NrvA::sInstance);
}

void KoopaJrCastleStateBattle1::exe800D8060() {
    if (MR::isFirstStep(this)) startHeadPanic();
    appearFireBar();
    killRingBeam();
    if (isHeadFn_800D4790()) setNerve(&KoopaJrCastleStateBattle1NrvA::sInstance);
}

KoopaJrCastleStateBattle1::~KoopaJrCastleStateBattle1() {}
