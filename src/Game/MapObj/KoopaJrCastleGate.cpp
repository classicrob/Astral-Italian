#include "Game/MapObj/KoopaJrCastleGate.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <revolution/types.h>

namespace NrvKoopaJrCastleGate {
    NEW_NERVE(KoopaJrCastleGateNrvWait, KoopaJrCastleGate, Wait);
    NEW_NERVE(KoopaJrCastleGateNrvMove, KoopaJrCastleGate, Move);
    NEW_NERVE(KoopaJrCastleGateNrvMoveFast, KoopaJrCastleGate, MoveFast);
    NEW_NERVE(KoopaJrCastleGateNrvEnd, KoopaJrCastleGate, Wait);
};  // namespace NrvKoopaJrCastleGate

KoopaJrCastleGate::KoopaJrCastleGate(const char* pName) : LiveActor(pName) {}

void KoopaJrCastleGate::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(GET_NERVE(KoopaJrCastleGate, KoopaJrCastleGateNrvWait), 0);
    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::tryRegisterDemoActionNerve(this, GET_NERVE(KoopaJrCastleGate, KoopaJrCastleGateNrvMove), nullptr);
    }
    makeActorAppeared();
}

void KoopaJrCastleGate::exeWait() {
    if (!isNerve(GET_NERVE(KoopaJrCastleGate, KoopaJrCastleGateNrvEnd))) {
        if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this)) {
            MR::invalidateClipping(this);
            setNerve(GET_NERVE(KoopaJrCastleGate, KoopaJrCastleGateNrvMoveFast));
        }
    }
}

void KoopaJrCastleGate::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Move");
    }
    if (MR::isActionEnd(this)) {
        MR::invalidateCollisionParts(this);
        setNerve(GET_NERVE(KoopaJrCastleGate, KoopaJrCastleGateNrvEnd));
    }
}

void KoopaJrCastleGate::exeMoveFast() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "MoveFast");
        if (MR::isValidSwitchB(this)) {
            MR::onSwitchB(this);
        }
    }
    if (MR::isActionEnd(this)) {
        MR::validateClipping(this);
        MR::invalidateCollisionParts(this);
        setNerve(GET_NERVE(KoopaJrCastleGate, KoopaJrCastleGateNrvEnd));
    }
}

KoopaJrCastleGate::~KoopaJrCastleGate() {}
