#pragma once
#include "Game/Screen/LayoutActor.hpp"
class CountUpPaneRumbler;

class ScoreCounterLayout : public LayoutActor {
public:
    ScoreCounterLayout();
    virtual ~ScoreCounterLayout();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    void exeAppear();
    void exeWait();
    void exeCountUp();
    void exeDisappear();
    void addScore(s32);
    void resetScore();
    void initScore(s32, s32);
    void show();
    void timeUp();
    void disappear();
    void updateScoreText();

    s32 mScore;
    s32 mHighScore;
    s32 mTargetScore;
    bool mIsNewRecord;
    CountUpPaneRumbler* mScoreRumbler;
    CountUpPaneRumbler* mHighScoreRumbler;
};
