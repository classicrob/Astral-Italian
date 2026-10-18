#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaStateBattleStart {
    NEW_NERVE(KoopaStateBattleStartNrvLoop, KoopaStateBattleStart, BattleStartLoop);
    NEW_NERVE(KoopaStateBattleStartNrvEnd, KoopaStateBattleStart, BattleStartEnd);
};  // namespace NrvKoopaStateBattleStart

KoopaStateBattleStart::KoopaStateBattleStart(KoopaBattle* pHost)
    : ActorStateBase< KoopaBattle >("クッパ戦闘開始状態", pHost), mStartPos(0.0f, 0.0f, 0.0f), mEndPos(0.0f, 0.0f, 0.0f) {
    initNerve(&NrvKoopaStateBattleStart::KoopaStateBattleStartNrvLoop::sInstance, 0);
}

void KoopaStateBattleStart::initPosition() {
    mEndPos.set< f32 >(mHost->mPosition);
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    mStartPos.set< f32 >(mEndPos + up * 8000.0f);
    mHost->mPosition.set< f32 >(mStartPos);
}

void KoopaStateBattleStart::appear() {
    mIsDead = false;
    setNerve(&NrvKoopaStateBattleStart::KoopaStateBattleStartNrvLoop::sInstance);
}

void KoopaStateBattleStart::exeBattleStartLoop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "BattleStartLoop");
    }
    TVec3f* pPos = &mHost->mPosition;
    MR::vecBlend(pPos, mStartPos, mEndPos, MR::calcNerveEaseInRate(this, 120));
    if (MR::isGreaterEqualStep(this, 119)) {
        setNerve(&NrvKoopaStateBattleStart::KoopaStateBattleStartNrvEnd::sInstance);
    }
}

// NONMATCHING: register allocation only (target reuses r31 for the temp, then for &mHost->mPosition computed before operator*)
void KoopaStateBattleStart::exeBattleStartEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "BattleStartEnd");
    }
    f32 rate = MR::calcNerveRate(this, 60);
    f32 ease = 1.0f + 0.5f * (rate * (1.0f + rate * (rate - 2.0f)));
    mHost->mPosition.set< f32 >(mStartPos + (mEndPos - mStartPos) * ease);
    if (MR::isGreaterEqualStep(this, 60)) {
        kill();
    }
}

KoopaStateBattleStart::~KoopaStateBattleStart() {
}
