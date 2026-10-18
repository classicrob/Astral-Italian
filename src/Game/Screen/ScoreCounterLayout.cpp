#include "Game/Screen/ScoreCounterLayout.hpp"
#include "Game/Screen/CountUpPaneRumbler.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvScoreCounterLayout {
    NEW_NERVE(Appear, ScoreCounterLayout, Appear);
    NEW_NERVE(Wait, ScoreCounterLayout, Wait);
    NEW_NERVE(CountUp, ScoreCounterLayout, CountUp);
    class TimeUp : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            ScoreCounterLayout* actor = reinterpret_cast<ScoreCounterLayout*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) MR::startAnim(actor, "TimeUp", 0);
        }
        static TimeUp sInstance;
    };
    TimeUp TimeUp::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(Disappear, ScoreCounterLayout, Disappear);
}

ScoreCounterLayout::ScoreCounterLayout()
    : LayoutActor("スコアカウンターレイアウト", true), mScore(0), mHighScore(0), mTargetScore(0),
      mIsNewRecord(false), mScoreRumbler(nullptr), mHighScoreRumbler(nullptr) {}
ScoreCounterLayout::~ScoreCounterLayout() {}

void ScoreCounterLayout::init(const JMapInfoIter&) {
    MR::connectToSceneLayout(this);
    initLayoutManager("ScoreCounter", 2);
    mScoreRumbler = new CountUpPaneRumbler(this, "ScoreCount", 4.0f);
    mHighScoreRumbler = new CountUpPaneRumbler(this, "HiScoreCount", 4.0f);
    initNerve(&NrvScoreCounterLayout::Appear::sInstance);
    MR::registerDemoSimpleCastAll(this);
    kill();
}
void ScoreCounterLayout::appear() {
    mScoreRumbler->reset();
    mHighScoreRumbler->reset();
    MR::startAnimAndSetFrameAndStop(this, "Appear", 0.0f, 0);
    MR::startAnimAndSetFrameAndStop(this, "Record", MR::getAnimFrameMax(this, "Record") - 1, 1);
    setNerve(&NrvScoreCounterLayout::Appear::sInstance);
    LayoutActor::appear();
}
void ScoreCounterLayout::exeAppear() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "Appear", 0);
    if (MR::isAnimStopped(this, 0)) setNerve(&NrvScoreCounterLayout::Wait::sInstance);
}
void ScoreCounterLayout::exeWait() {
    if (MR::isFirstStep(this)) {
        if (mIsNewRecord) { MR::startAnim(this, "Record", 1); return; }
        MR::startAnim(this, "Wait", 0);
    }
}
void ScoreCounterLayout::exeCountUp() {
    if (MR::isFirstStep(this)) {
        mScoreRumbler->start();
        MR::startPaneAnim(this, "ScoreCount", "Flash", 0);
        if (mHighScore <= mScore) {
            mHighScoreRumbler->start();
            MR::startPaneAnim(this, "HiScoreCount", "Flash", 0);
        }
    }
    if (MR::isPaneAnimStopped(this, "ScoreCount", 0)) setNerve(&NrvScoreCounterLayout::Wait::sInstance);
}
void ScoreCounterLayout::exeDisappear() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "End", 0);
    if (MR::isAnimStopped(this, 0)) kill();
}
void ScoreCounterLayout::addScore(s32 amount) {
    mScore += amount;
    if (mHighScore < mScore) { mHighScore = mScore; mIsNewRecord = true; }
    updateScoreText();
    setNerve(&NrvScoreCounterLayout::CountUp::sInstance);
}
void ScoreCounterLayout::resetScore() {
    mScore = 0;
    mIsNewRecord = false;
    updateScoreText();
}
// NONMATCHING: equivalent positive-score selection uses one branch instead of two.
void ScoreCounterLayout::initScore(s32 bestScore, s32 targetScore) {
    mScore = 0;
    mTargetScore = targetScore;
    if (bestScore > 0) {
    } else {
        bestScore = targetScore;
    }
    mHighScore = bestScore;
    updateScoreText();
}
void ScoreCounterLayout::show() {
    setNerve(&NrvScoreCounterLayout::Appear::sInstance);
    appear();
}
void ScoreCounterLayout::timeUp() {
    if (!isNerve(&NrvScoreCounterLayout::TimeUp::sInstance)) setNerve(&NrvScoreCounterLayout::TimeUp::sInstance);
}
void ScoreCounterLayout::disappear() {
    if (!isNerve(&NrvScoreCounterLayout::Disappear::sInstance)) setNerve(&NrvScoreCounterLayout::Disappear::sInstance);
}
void ScoreCounterLayout::control() {
    mScoreRumbler->update();
    mHighScoreRumbler->update();
}
void ScoreCounterLayout::updateScoreText() {
    MR::setTextBoxNumberRecursive(this, "ScoreCount", mScore);
    MR::setTextBoxNumberRecursive(this, "HiScoreCount", mHighScore);
}
