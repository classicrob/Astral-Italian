#include "Game/NPC/CometEventExecutorTimeLimit.hpp"
#include "Game/Screen/TimeLimitLayout.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvCometEventExecutorTimeLimit {
    NEW_NERVE(CometEventExecutorTimeLimitStartEvent, CometEventExecutorTimeLimit, StartEvent);
}

CometEventExecutorTimeLimit::CometEventExecutorTimeLimit()
    : LiveActor("制限時間コメット実行者"), mTimeLimitLayout(nullptr), mTimeLimit(0) {}

void CometEventExecutorTimeLimit::init(const JMapInfoIter&) {
    MR::connectToSceneLayoutMovement(this);
    MR::invalidateClipping(this);
    mTimeLimitLayout = new TimeLimitLayout;
    mTimeLimitLayout->setDisplayModeOnNormal(true);
    mTimeLimitLayout->initWithoutIter();
    mTimeLimitLayout->kill();
    MR::connectToSceneLayout(mTimeLimitLayout);
    initNerve(&NrvCometEventExecutorTimeLimit::CometEventExecutorTimeLimitStartEvent::sInstance);
    kill();
}
void CometEventExecutorTimeLimit::init(s32 seconds) {
    mTimeLimit = seconds;
    mTimeLimitLayout->setTimeLimit(getTimeLimit());
}
void CometEventExecutorTimeLimit::start() {
    appear();
    setNerve(&NrvCometEventExecutorTimeLimit::CometEventExecutorTimeLimitStartEvent::sInstance);
}
void CometEventExecutorTimeLimit::stop() {
    if (!MR::isDead(this)) { kill(); mTimeLimitLayout->kill(); }
}
void CometEventExecutorTimeLimit::exeStartEvent() {
    if (MR::isFirstStep(this)) { mTimeLimitLayout->appear(); mTimeLimitLayout->setTimeLimit(getTimeLimit()); }
    if (MR::canStartDemo() && mTimeLimitLayout->isReadyToTimeUp()) MR::forceKillPlayer();
}
void CometEventExecutorTimeLimit::pauseCometTimer() { mTimeLimitLayout->pause(); }
void CometEventExecutorTimeLimit::resumeCometTimer() { mTimeLimitLayout->resume(); }
void CometEventExecutorTimeLimit::showCometTimer() { MR::showLayout(mTimeLimitLayout); }
void CometEventExecutorTimeLimit::hideCometTimer() { MR::hideLayout(mTimeLimitLayout); }
void CometEventExecutorTimeLimit::addCometTimer(s32 frames) { mTimeLimitLayout->addTimeLimit(frames); }
void CometEventExecutorTimeLimit::readdCometTimer() { mTimeLimitLayout->addTimeLimit(getTimeLimit()); }
u32 CometEventExecutorTimeLimit::getTimeLimit() const { if (mTimeLimit) return mTimeLimit * 60; return 108000; }
void CometEventExecutorTimeLimit::decCometTimer(s32) {}
CometEventExecutorTimeLimit::~CometEventExecutorTimeLimit() {}
