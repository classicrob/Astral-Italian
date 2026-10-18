#include "Game/NPC/CometEventKeeper.hpp"
#include "Game/NPC/CometEventExecutorTimeLimit.hpp"
#include "Game/NPC/CometEventExecutorTimeLimitDestroy.hpp"
#include "Game/Screen/GalaxyCometScreenFilter.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

CometEventKeeper::CometEventKeeper() : mExecutor(nullptr), mScreenFilter(nullptr), mCometName(nullptr) {}

// NONMATCHING: the green-star flag uses r30 instead of r31.
void CometEventKeeper::init() {
    initCometStatus();
    s32 seconds = getTimeLimitFromScenarioData();
    if (seconds) {
        if (isStartEvent("Exterminate")) {
            CometEventExecutorTimeLimitDestroy* executor = new CometEventExecutorTimeLimitDestroy;
            executor->initWithoutIter();
            mExecutor = executor;
        } else {
            CometEventExecutorTimeLimit* executor = new CometEventExecutorTimeLimit;
            executor->initWithoutIter();
            mExecutor = executor;
        }
        mExecutor->init(seconds);
        mExecutor->stop();
    }
    s32 green = 0;
    if (MR::isProductGameScene() && MR::isPowerStarGreenInCurrentStage()) green = 1;
    if (mCometName || green) {
        mScreenFilter = new GalaxyCometScreenFilter;
        mScreenFilter->initWithoutIter();
        mScreenFilter->setCometType(green != 0);
    }
}

// NONMATCHING: the compiler folds/reorders the null branch around the tail call.
bool CometEventKeeper::isStartEvent(const char* name) const {
    if (!mCometName) return false;
    else return MR::isEqualString(mCometName, name);
}
void CometEventKeeper::startCometEventIfExist() { if (mExecutor) mExecutor->start(); }
void CometEventKeeper::stopCometEvent() { if (mExecutor) mExecutor->stop(); }
void CometEventKeeper::endCometEvent() {
    if (mExecutor) { stopCometEvent(); MR::killDestroyCounter(); }
}
void CometEventKeeper::pauseCometTimer() { if (mExecutor) mExecutor->pauseCometTimer(); }
void CometEventKeeper::resumeCometTimer() { if (mExecutor) mExecutor->resumeCometTimer(); }
void CometEventKeeper::showCometTimer() { if (mExecutor) mExecutor->showCometTimer(); }
void CometEventKeeper::hideCometTimer() { if (mExecutor) mExecutor->hideCometTimer(); }
void CometEventKeeper::addCometTimer(s32 frames) { if (mExecutor) mExecutor->addCometTimer(frames); }
void CometEventKeeper::decCometTimer(s32 frames) { if (mExecutor) mExecutor->decCometTimer(frames); }
s32 CometEventKeeper::getTimeLimitFromScenarioData() {
    if (!mCometName) return 0;
    if (GameSequenceFunction::isPadModeDreamer()) return 0;
    GalaxyStatusAccessor status = MR::makeCurrentGalaxyStatusAccessor();
    return status.getCometLimitTimer(MR::getCurrentScenarioNo());
}
void CometEventKeeper::initCometStatus() {
    GalaxyStatusAccessor status = MR::makeCurrentGalaxyStatusAccessor();
    mCometName = status.getCometName(MR::getCurrentScenarioNo());
}
