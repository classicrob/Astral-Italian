#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossBussunStateSleep {
    NEW_NERVE(BossBussunStateSleepNrvSleep, BossBussunStateSleep, Sleep);
    NEW_NERVE(BossBussunStateSleepNrvSleepReaction, BossBussunStateSleep, SleepReaction);
};  // namespace NrvBossBussunStateSleep

BossBussunStateSleep::BossBussunStateSleep(BossBussun* pHost) : ActorStateBase< BossBussun >("ボスブッスン眠り状態", pHost) {
    initNerve(GET_NERVE(BossBussunStateSleep, BossBussunStateSleepNrvSleep), 0);
}

void BossBussunStateSleep::appear() {
    mIsDead = false;
    mHost->resetBodyOffset();
    setNerve(GET_NERVE(BossBussunStateSleep, BossBussunStateSleepNrvSleep));
}

bool BossBussunStateSleep::tryWake() {
    if (!isNerve(GET_NERVE(BossBussunStateSleep, BossBussunStateSleepNrvSleepReaction))) {
        setNerve(GET_NERVE(BossBussunStateSleep, BossBussunStateSleepNrvSleepReaction));
        return true;
    }
    return false;
}

bool BossBussunStateSleep::receiveOtherMsg(u32 msg) {
    if (MR::isMsgPlayerHitAll(msg)) {
        if (!isNerve(GET_NERVE(BossBussunStateSleep, BossBussunStateSleepNrvSleepReaction))) {
            setNerve(GET_NERVE(BossBussunStateSleep, BossBussunStateSleepNrvSleepReaction));
            return true;
        }
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        if (!isNerve(GET_NERVE(BossBussunStateSleep, BossBussunStateSleepNrvSleepReaction)) || MR::isGreaterStep(this, 0x1E)) {
            setNerve(GET_NERVE(BossBussunStateSleep, BossBussunStateSleepNrvSleepReaction));
            return true;
        }
    }
    return false;
}

void BossBussunStateSleep::exeSleep() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(mHost, "Sleep");
    }
}

void BossBussunStateSleep::exeSleepReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "SleepReaction");
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(GET_NERVE(BossBussunStateSleep, BossBussunStateSleepNrvSleep));
    }
}

BossBussunStateSleep::~BossBussunStateSleep() {
}
