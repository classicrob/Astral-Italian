#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class MameMuimuiGroup;
class MameMuimuiPlacementSet;
class BombTimerLayout;
class StartCountdownLayout;
class TimeUpLayout;
class ScoreCounterLayout;
class MameMuimuiWave;
namespace MR { class FunctorBase; }

class MameMuimuiScoreListener {
public:
    virtual void onScore(s32,const LiveActor*) = 0;
    virtual void onComboBreak() = 0;
    virtual void onScoreEnd();
};
class MameMuimuiScorer : public LiveActor, public MameMuimuiScoreListener {
public:
    MameMuimuiScorer(const char*);
    virtual ~MameMuimuiScorer();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void onScore(s32,const LiveActor*);
    virtual void onComboBreak();
    void registerEndCallback(const MR::FunctorBase&);
    void start();
    bool isPlaying() const;
    bool isClear() const;
    bool isNewRecord() const;
    s32 getScore() const;
    s32 getBestScore() const;
    s32 getTargetScore() const;
    MameMuimuiGroup* mGroups[3];
    MameMuimuiPlacementSet* mPlacements;
    BombTimerLayout* mTimerLayout;
    StartCountdownLayout* mCountdownLayout;
    TimeUpLayout* mTimeUpLayout;
    ScoreCounterLayout* mScoreLayout;
    MR::FunctorBase* mEndCallback;
    MameMuimuiWave* mWaves;
    TPos3f mInitialMtx;
    s32 mWaveCount;
    s32 mWaveIndex;
    s32 mTargetScore;
    s32 mScoreMultiplier;
    s32 mLevel;
    s32 mWavePattern;
    bool mIsClear;
};
