#include "Game/Screen/RaceLayout.hpp"
#include "Game/Util.hpp"

using namespace NrvRaceLayout;

RaceLayout::RaceLayout(const char* name) : LayoutActor(name, true) {}
void RaceLayout::init(const JMapInfoIter&) {
    MR::connectToSceneLayout(this);
    initLayoutManager("Race", 1);
    initNerve(&RaceLayoutNrvHide::sInstance);
    MR::createAndAddPaneCtrl(this, "Goal", 1);
    MR::createAndAddPaneCtrl(this, "Lose", 1);
    MR::createAndAddPaneCtrl(this, "Rank", 1);
    MR::createAndAddPaneCtrl(this, "ScoreTime", 1);
    initEffectKeeper(2, "Race", 0);
    hideAllPane();
}
void RaceLayout::appear() { LayoutActor::appear(); setNerve(&RaceLayoutNrvAppear::sInstance); }
void RaceLayout::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showPane(this, "Record");
        MR::showPane(this, "BestRecord");
        MR::startPaneAnim(this, "ScoreTime", "Appear", 0);
    }
    if (MR::isPaneAnimStopped(this, "ScoreTime", 0)) setNerve(&RaceLayoutNrvWait::sInstance);
}
void RaceLayout::exeWait() {
    if (MR::isFirstStep(this)) MR::startPaneAnim(this, "ScoreTime", "Wait", 0);
    if (MR::isPlayerDead()) MR::hidePane(this, "Goal");
}

void RaceLayout::playGoal() {
    MR::showPane(this, "Goal");
    MR::startPaneAnim(this, "Goal", "Goal", 0);
    MR::startPaneAnim(this, "ScoreTime", "Goal", 0);
    MR::emitEffect(this, "RaceGoal");
}
void RaceLayout::playGoalWithTime(u32 time) {
    MR::updateClearTimeTextBox(this, "BestTimeCounter", time);
    MR::showPane(this, "Goal");
    MR::startPaneAnim(this, "Goal", "GoalBestTime", 0);
    MR::startPaneAnim(this, "ScoreTime", "GoalBestTime", 0);
    MR::emitEffect(this, "RaceGoal");
}
void RaceLayout::playLose() {
    MR::showPane(this, "Lose");
    MR::startPaneAnim(this, "Lose", "Lose", 0);
    MR::startPaneAnim(this, "ScoreTime", "Lose", 0);
}
void RaceLayout::playRank(u32 rank) {
    MR::showPane(this, "Rank");
    MR::startPaneAnim(this, "Rank", "Rank", 0);
    MR::setPaneAnimFrameAndStop(this, "Rank", rank, 0);
}
// NONMATCHING: the pooled-string base and boolean result use swapped r30/r31 registers.
bool RaceLayout::isAllAnimStopped() {
    const bool stopped = MR::isPaneAnimStopped(this, "Goal", 0) && MR::isPaneAnimStopped(this, "Lose", 0)
        && MR::isPaneAnimStopped(this, "ScoreTime", 0) && MR::isPaneAnimStopped(this, "Rank", 0);
    return stopped;
}
void RaceLayout::hideAllPane() {
    MR::hidePane(this, "Goal");
    MR::hidePane(this, "Lose");
    MR::hidePane(this, "Record");
    MR::hidePane(this, "BestRecord");
    MR::hidePane(this, "Rank");
    setNerve(&NrvRaceLayout::RaceLayoutNrvHide::sInstance);
}
void RaceLayout::setTime(u32 time) { MR::updateClearTimeTextBox(this, "TimeCounter", time); }
void RaceLayout::setBestTime(u32 time) { MR::updateClearTimeTextBox(this, "BestTimeCounter", time); }

RaceLayout::~RaceLayout() {}

namespace NrvRaceLayout {
    RaceLayoutNrvHide::RaceLayoutNrvHide() {}
    RaceLayoutNrvHide RaceLayoutNrvHide::sInstance;
    RaceLayoutNrvAppear::RaceLayoutNrvAppear() {}
    RaceLayoutNrvAppear RaceLayoutNrvAppear::sInstance;
    RaceLayoutNrvWait::RaceLayoutNrvWait() {}
    RaceLayoutNrvWait RaceLayoutNrvWait::sInstance;
    void RaceLayoutNrvWait::execute(Spine* spine) const { static_cast<RaceLayout*>(spine->mExecutor)->exeWait(); }
    void RaceLayoutNrvAppear::execute(Spine* spine) const { static_cast<RaceLayout*>(spine->mExecutor)->exeAppear(); }
    void RaceLayoutNrvHide::execute(Spine* spine) const {  }
}
