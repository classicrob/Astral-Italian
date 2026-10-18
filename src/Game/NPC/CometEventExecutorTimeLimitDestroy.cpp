// NONMATCHING: literal addressing remains.
#include "Game/NPC/CometEventExecutorTimeLimitDestroy.hpp"
#include "Game/Screen/BombTimerLayout.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvCometEventExecutorTimeLimitDestroy {
    NEW_NERVE(CometEventExecutorTimeLimitDestroyNrvStartEvent,CometEventExecutorTimeLimitDestroy,StartEvent);
}
CometEventExecutorTimeLimitDestroy::CometEventExecutorTimeLimitDestroy() : LiveActor("全滅コメット実行者"),mTimerLayout(nullptr),mTimeLimit(0) {}
CometEventExecutorTimeLimitDestroy::~CometEventExecutorTimeLimitDestroy() {}
void CometEventExecutorTimeLimitDestroy::init(const JMapInfoIter&) {
    MR::connectToSceneLayoutMovement(this);
    MR::invalidateClipping(this);
    mTimerLayout=new BombTimerLayout;
    mTimerLayout->initWithoutIter();
    mTimerLayout->kill();
    initSound(4,"CometEventExecutorTimeLimitDestroy",nullptr,TVec3f(0.0f));
    initNerve(&NrvCometEventExecutorTimeLimitDestroy::CometEventExecutorTimeLimitDestroyNrvStartEvent::sInstance);
    kill();
}
void CometEventExecutorTimeLimitDestroy::init(s32 seconds) { mTimeLimit=seconds; mTimerLayout->setTimeLimit(getTimeLimit()); }
void CometEventExecutorTimeLimitDestroy::start() { appear(); setNerve(&NrvCometEventExecutorTimeLimitDestroy::CometEventExecutorTimeLimitDestroyNrvStartEvent::sInstance); }
void CometEventExecutorTimeLimitDestroy::stop() { if(!MR::isDead(this)) { kill(); mTimerLayout->kill(); } }
void CometEventExecutorTimeLimitDestroy::exeStartEvent() {
    if(MR::isFirstStep(this)) {
        mTimerLayout->appear();
        mTimerLayout->setTimeLimit(getTimeLimit());
        MR::startActionSound(this,"TimerStart",-1,-1,-1);
    }
    if(MR::canStartDemo() && mTimerLayout->isReadyToTimeUp()) {
        MR::forceKillPlayerByGroundRace();
        MR::startActionSound(this,"TimeUp",-1,-1,-1);
    }
}
void CometEventExecutorTimeLimitDestroy::pauseCometTimer() { mTimerLayout->suspend(); }
void CometEventExecutorTimeLimitDestroy::resumeCometTimer() { mTimerLayout->resume(); }
void CometEventExecutorTimeLimitDestroy::showCometTimer() { MR::showLayout(mTimerLayout); }
void CometEventExecutorTimeLimitDestroy::hideCometTimer() { MR::hideLayout(mTimerLayout); }
void CometEventExecutorTimeLimitDestroy::addCometTimer(s32) {}
void CometEventExecutorTimeLimitDestroy::readdCometTimer() {}
void CometEventExecutorTimeLimitDestroy::decCometTimer(s32) {}
u32 CometEventExecutorTimeLimitDestroy::getTimeLimit() const { if(mTimeLimit) return mTimeLimit*60; return 108000; }
