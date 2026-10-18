#pragma auto_inline off
#include "Game/Boss/KoopaStateBase.hpp"
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStateMeteor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvKoopaStateMeteor {
    NEW_NERVE(KoopaStateMeteorNrvStart, KoopaStateMeteor, Start);
    NEW_NERVE(KoopaStateMeteorNrvLoop, KoopaStateMeteor, Loop);
    NEW_NERVE(KoopaStateMeteorNrvEnd, KoopaStateMeteor, End);
};  // namespace NrvKoopaStateMeteor

// NONMATCHING: ActorStateBase constructor remains out of line; target expands its initialization.
KoopaStateBase::KoopaStateBase(const char* pName, KoopaBattle* pHost) : ActorStateBase< KoopaBattle >(pName, pHost) {
}

// NONMATCHING: ActorStateBase constructor remains out of line; target expands its initialization.
KoopaStateMeteor::KoopaStateMeteor(KoopaBattle* pHost)
    : ActorStateBase< KoopaBattle >("クッパ隕石攻撃", pHost), mMode(1), mCastInterval(60), mEndStep(60), mEndFrameMax(0) {
    mEndFrameMax = MR::getBckFrameMax(pHost, "MeteorEnd");
    initNerve(&NrvKoopaStateMeteor::KoopaStateMeteorNrvStart::sInstance, 0);
}

void KoopaStateMeteor::setMode(s32 mode) {
    mMode = mode;
    switch (mode) {
    case 1:
        mCastInterval = 60;
        mEndStep = 60;
        break;
    case 2:
        mCastInterval = 80;
        mEndStep = 60;
        break;
    case 3:
        mCastInterval = 60;
        mEndStep = 60;
        break;
    case 4:
        mCastInterval = 60;
        mEndStep = 30;
        break;
    }
}

void KoopaStateMeteor::appear() {
    mIsDead = false;
    setNerve(&NrvKoopaStateMeteor::KoopaStateMeteorNrvStart::sInstance);
}

void KoopaStateMeteor::exeStart() {
    if (MR::isFirstStep(this)) {
        if (MR::sendMsgKoopaC8() > 4) {
            kill();
            return;
        }
        MR::startAction(mHost, "MeteorStart");
    }
    MR::startActionSound(mHost, "MeteoMagic", -1, -1, -1);
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvKoopaStateMeteor::KoopaStateMeteorNrvLoop::sInstance);
    }
}

void KoopaStateMeteor::exeLoop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "MeteorLoop");
    }
    if (MR::isIntervalStep(this, mCastInterval)) {
        MR::trySendMsgKoopaCastAttackRockStart();
    }
    MR::startActionSound(mHost, "MeteoMagic", -1, -1, -1);
    s32 step = mEndStep + 150 - mEndFrameMax;
    if (step < 0) {
        step = 0;
    } else if (step > 150) {
        step = 150;
    }
    if (MR::isStep(this, step)) {
        MR::startAction(mHost, "MeteorEnd");
    }
    if (MR::isGreaterEqualStep(this, 150)) {
        MR::sendMsgKoopaCastAttackRockEnd();
        setNerve(&NrvKoopaStateMeteor::KoopaStateMeteorNrvEnd::sInstance);
    }
}

void KoopaStateMeteor::exeEnd() {
    MR::startActionSound(mHost, "MeteoMagic", -1, -1, -1);
    if (MR::isGreaterEqualStep(this, mEndStep)) {
        kill();
    }
}

KoopaStateMeteor::~KoopaStateMeteor() {
}
