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

const volatile s32 sKoopaJrCastleBattle3WaitStep = 0;

namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvA KoopaJrCastleStateBattle3NrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvB KoopaJrCastleStateBattle3NrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvC KoopaJrCastleStateBattle3NrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvD KoopaJrCastleStateBattle3NrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvE KoopaJrCastleStateBattle3NrvE::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvF KoopaJrCastleStateBattle3NrvF::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvG KoopaJrCastleStateBattle3NrvG::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvH KoopaJrCastleStateBattle3NrvH::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvI KoopaJrCastleStateBattle3NrvI::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvJ KoopaJrCastleStateBattle3NrvJ::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvK KoopaJrCastleStateBattle3NrvK::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvL KoopaJrCastleStateBattle3NrvL::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvM KoopaJrCastleStateBattle3NrvM::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvN KoopaJrCastleStateBattle3NrvN::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle3::KoopaJrCastleStateBattle3(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x8dU\x8c\x82:\x91\xe6\x82R\x92i\x8aK]", pHost) {
    _pad[0] = 0;
}

using namespace KoopaJrCastleFunction;
using namespace NrvKoopaJrCastleStateBattle3;

void KoopaJrCastleStateBattle3::init() {
    initNerve(&KoopaJrCastleStateBattle3NrvA::sInstance, 0);
    kill();
}

void KoopaJrCastleStateBattle3::appear() {
    mIsDead = false;
    resetBodyOffsetAndShadows();
    setNerve(&KoopaJrCastleStateBattle3NrvA::sInstance);
}

void KoopaJrCastleStateBattle3::control() {
    appearFireBar();
    killRingBeam();
}

void KoopaJrCastleStateBattle3::exeCannonBallVolleyBeforeMove() {
    if (MR::isFirstStep(this)) startCannonBallVolley();
    if (!tryPlayerOnCastle() && MR::isGreaterEqualStep(this, 90))
        setNerve(&KoopaJrCastleStateBattle3NrvE::sInstance);
}

void KoopaJrCastleStateBattle3::exeCannonBallVolleyBeforeWind() {
    if (MR::isFirstStep(this)) startCannonBallVolley();
    if (!tryPlayerOnCastle() && MR::isGreaterEqualStep(this, 90))
        setNerve(&KoopaJrCastleStateBattle3NrvG::sInstance);
}

void KoopaJrCastleStateBattle3::exeCannonBallVolleyBeforeChase() {
    if (MR::isFirstStep(this)) startCannonBallVolley();
    if (!tryPlayerOnCastle() && MR::isGreaterEqualStep(this, 90))
        setNerve(&KoopaJrCastleStateBattle3NrvE::sInstance);
}

void KoopaJrCastleStateBattle3::exeMoveStraight() {
    if (MR::isFirstStep(this)) bodyStartMove0();
    if (!tryPlayerOnCastle() && bodyIsNotMove()) {
        killCannon();
        setNerve(&KoopaJrCastleStateBattle3NrvA::sInstance);
    }
}

void KoopaJrCastleStateBattle3::exeMoveChase() {
    if (MR::isFirstStep(this)) {
        bodyStartMove1();
        startCannonBallVolley();
        _pad[0] = 0;
    }
    if (!tryPlayerOnCastle()) {
        if (!bodyIsMoveState() && !_pad[0]) {
            _pad[0] = 1;
            startCannonVolley(1, 1);
        }
        if (_pad[0] && isCannonDead()) startCannonVolley(1, 1);
        if (bodyIsNotMove()) {
            killCannon();
            setNerve(&KoopaJrCastleStateBattle3NrvB::sInstance);
        }
    }
}

void KoopaJrCastleStateBattle3::exeMoveAroundCenter() {
    if (MR::isFirstStep(this)) {
        bodyStartMove2();
        startCannonBallVolley();
        _pad[0] = 0;
    }
    if (!tryPlayerOnCastle()) {
        if (!bodyIsMoveState() && !_pad[0]) {
            _pad[0] = 1;
            startCannonVolley(1, 1);
        }
        if (_pad[0] && isCannonDead()) startCannonVolley(1, 1);
        if (bodyIsNotMove()) {
            killCannon();
            if (isBodyMoveFlagSet()) {
                setNerve(&KoopaJrCastleStateBattle3NrvH::sInstance);
                return;
            }
            setNerve(&KoopaJrCastleStateBattle3NrvC::sInstance);
        }
    }
}

void KoopaJrCastleStateBattle3::exeWindAndElectric() {
    if (MR::isFirstStep(this)) {
        appearWindFromZeroPower();
        startCannonBallVolley();
    }
    if (isWindBlowing()) appearElectric();
    if (MR::isStep(this, 120)) startCannonVolley(1, 1);
    else if (MR::isGreaterStep(this, 120) && isCannonDead()) startCannonVolley(1, 1);
    if (isPlayerOnCastle() && isElectricActive()) {
        setNerve(&KoopaJrCastleStateBattle3NrvN::sInstance);
        return;
    }
    if (windIsDead() && isElectricDead()) setNerve(&KoopaJrCastleStateBattle3NrvH::sInstance);
}

void KoopaJrCastleStateBattle3::exeCannonVolley() {
    if (MR::isFirstStep(this)) startCannonVolley(2, 2);
    if (isCannonDead()) setNerve(&KoopaJrCastleStateBattle3NrvL::sInstance);
}

void KoopaJrCastleStateBattle3::exeShockSign() {
    if (MR::isFirstStep(this)) startCannonShockVolley();
    if (MR::isGreaterStep(this, 60)) setNerve(&KoopaJrCastleStateBattle3NrvK::sInstance);
}

void KoopaJrCastleStateBattle3::exeShockVolley() {
    if (MR::isFirstStep(this)) startCannonShockVolleyImmediately();
    if (isCannonDead()) setNerve(&KoopaJrCastleStateBattle3NrvL::sInstance);
}

void KoopaJrCastleStateBattle3::exeSelectAttack() {
    MR::isFirstStep(this);
    if (isPlayerOnCastle()) {
        setNerve(&KoopaJrCastleStateBattle3NrvM::sInstance);
        return;
    }
    if (MR::isGreaterEqualStep(this, 90)) {
        if (isPlayerNearHead()) {
            setNerve(&KoopaJrCastleStateBattle3NrvG::sInstance);
            return;
        }
        setNerve(&KoopaJrCastleStateBattle3NrvE::sInstance);
    }
}

void KoopaJrCastleStateBattle3::exeHeadPanic() {
    if (MR::isFirstStep(this)) startHeadPanic();
    if (isHeadPanicOrSpinEnd()) setNerve(&KoopaJrCastleStateBattle3NrvA::sInstance);
}

void KoopaJrCastleStateBattle3::exeHeadSpinSign() {
    if (MR::isFirstStep(this)) startHeadSpinSign();
    if (isHeadPanicOrSpinEnd()) setNerve(&KoopaJrCastleStateBattle3NrvA::sInstance);
}

bool KoopaJrCastleStateBattle3::tryPlayerOnCastle() {
    if (isPlayerOnCastle()) {
        bodyTryStop();
        killCannon();
        setNerve(&KoopaJrCastleStateBattle3NrvM::sInstance);
        return true;
    }
    return false;
}

KoopaJrCastleStateBattle3::~KoopaJrCastleStateBattle3() {}
