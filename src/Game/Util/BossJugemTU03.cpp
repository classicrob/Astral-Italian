#pragma auto_inline off
#include "Game/Boss/BossJugem.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossJugemStateSleep {
    NEW_NERVE(BossJugemStateSleepNrvSleep, BossJugemStateSleep, Sleep);
    NEW_NERVE(BossJugemStateSleepNrvSleepWait, BossJugemStateSleep, SleepWait);
    NEW_NERVE(BossJugemStateSleepNrvAwake, BossJugemStateSleep, Awake);
};  // namespace NrvBossJugemStateSleep

BossJugemStateSleep::BossJugemStateSleep(BossJugem* pHost) : ActorStateBase< BossJugem >("\203\173\203\130\203\127\203\205\203\121\203\200\220\207\226\260", pHost) {
}

void BossJugemStateSleep::init() {
    initNerve(&NrvBossJugemStateSleep::BossJugemStateSleepNrvSleep::sInstance, 0);
}

void BossJugemStateSleep::appear() {
    mIsDead = false;
    setNerve(&NrvBossJugemStateSleep::BossJugemStateSleepNrvSleep::sInstance);
}

void BossJugemStateSleep::exeSleep() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("Sleep");
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvBossJugemStateSleep::BossJugemStateSleepNrvSleepWait::sInstance);
    }
}

void BossJugemStateSleep::exeSleepWait() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("SleepWait");
    }
    if (!mHost->isPlayerTooFar()) {
        setNerve(&NrvBossJugemStateSleep::BossJugemStateSleepNrvAwake::sInstance);
    }
}

void BossJugemStateSleep::exeAwake() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("Awake");
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}
