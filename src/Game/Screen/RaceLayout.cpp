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
