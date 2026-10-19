#pragma auto_inline off
#include "Game/Boss/BossJugem.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Per step flags: whether the rail direction is reversed before the move / whether the move is the short one.
static const s32 sReverseTable[7] = {0, 1, 0, 1, 0, 1, 1};
static const s32 sShortMoveTable[3] = {1, 0, 1};

namespace NrvBossJugemStateMove {
    NEW_NERVE(BossJugemStateMoveNrvMove, BossJugemStateMove, Move);
};  // namespace NrvBossJugemStateMove

BossJugemStateMove::BossJugemStateMove(BossJugem* pHost) : ActorStateBase< BossJugem >("\203\173\203\130\203\127\203\205\203\121\203\200\210\332\223\256", pHost), mRailStart(0.0f), mDirIdx(0), mLengthIdx(0) {
}

void BossJugemStateMove::init() {
    initNerve(&NrvBossJugemStateMove::BossJugemStateMoveNrvMove::sInstance, 2);
    mDirIdx = 0;
    mLengthIdx = 0;
    mRailStart = 0.0f;
}

void BossJugemStateMove::appear() {
    mIsDead = false;
    setNerve(&NrvBossJugemStateMove::BossJugemStateMoveNrvMove::sInstance);
}

// NONMATCHING: int->float conversion/constant hoisting of the step arithmetic differs (frame 0x50 vs 0x60); faithful translation of the asm
void BossJugemStateMove::exeMove() {
    if (MR::isFirstStep(this)) {
        prepare();
    }
    s32 moveFrames = 80;
    if (sShortMoveTable[mLengthIdx] == 0) {
        moveFrames = 120;
    }
    f32 length;
    if (sShortMoveTable[mLengthIdx] == 0) {
        length = 2400.0f;
    } else {
        length = 1600.0f;
    }
    f32 target = mRailStart + length;
    if (!MR::isRailCoordNotBehind(mHost, target)) {
        target = mRailStart - length;
    }
    s32 delay = 10;
    if (getNerveStep() - delay >= 0.0f) {
        LiveActor* pHost = mHost;
        MR::setRailCoord(pHost, MR::getEaseOutValue(static_cast<f32>(getNerveStep()) - delay, mRailStart, target, moveFrames));
        MR::moveTransToCurrentRailPos(mHost);
    }
    bool reachedGoal = false;
    if (MR::isRailReachedGoal(mHost)) {
        reachedGoal = true;
    }
    s32 animFrames = moveFrames;
    if (target < 0.0f) {
        animFrames = moveFrames * (s32)MR::normalizeAbs(0.0f, target, mRailStart) + 1;
    } else if (target > MR::getRailTotalLength(mHost)) {
        animFrames = moveFrames * (s32)MR::normalize(MR::getRailTotalLength(mHost), mRailStart, target) + 1;
    }
    mHost->updateMoveAnim(getNerveStep(), animFrames + 10);
    if (MR::isGreaterStep(this, moveFrames + 10) || reachedGoal) {
        if (reachedGoal) {
            MR::reverseRailDirection(mHost);
        }
        kill();
    }
}

void BossJugemStateMove::prepare() {
    if (++mDirIdx >= sizeof(sReverseTable)) {
        mDirIdx = 0;
    }
    if (++mLengthIdx >= sizeof(sShortMoveTable)) {
        mLengthIdx = 0;
    }
    if (sReverseTable[mDirIdx] == 0) {
        MR::reverseRailDirection(mHost);
    }
    f32 length;
    if (sShortMoveTable[mLengthIdx] == 0) {
        length = 2400.0f;
    } else {
        length = 1600.0f;
    }
    if (MR::isRailReachedNearGoal(mHost, length)) {
        MR::reverseRailDirection(mHost);
    }
    mRailStart = MR::getRailCoord(mHost);
}
