#include "Game/Screen/RaceLayout.hpp"
#include "Game/Util.hpp"

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
    bool stopped = false;
    if (MR::isPaneAnimStopped(this, "Goal", 0) && MR::isPaneAnimStopped(this, "Lose", 0)
        && MR::isPaneAnimStopped(this, "ScoreTime", 0) && MR::isPaneAnimStopped(this, "Rank", 0)) stopped = true;
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
