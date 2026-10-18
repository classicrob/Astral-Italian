#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class ScoreCounterLayout;
class BombTimerLayout;
class ScoreAttackMan;
class ScoreAttackCounter : public LiveActor {
public:
    ScoreAttackCounter(const char*);
    virtual ~ScoreAttackCounter();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    void start();
    void saveBestScore();
    void scoreTrample(const LiveActor*);
    void scoreSpin(const LiveActor*);
    void scoreCombo(const LiveActor*, u32);
    bool isClear() const;
    bool isNewRecord() const;
    void addScore(s32, const LiveActor*);
    void exeCounting();
    bool isReadyToFinish() const;
    ScoreCounterLayout* mScoreLayout;
    BombTimerLayout* mTimer;
    u32 mTrampleCount;
    u32 mSpinCount;
    s32 mTimeLimit;
};
class ScoreAttackAccessor : public NameObj {
public:
    ScoreAttackAccessor();
    virtual ~ScoreAttackAccessor();
    ScoreAttackCounter* mCounter;
    ScoreAttackMan* mMan;
};
namespace ScoreAttackFunction {
    void scoreByAttack(const LiveActor*, u32, const HitSensor*);
    void scoreTrample(const LiveActor*);
    void score100Points(const LiveActor*);
    void score100PointsMultiply(const LiveActor*, s32);
    void score10PointsMultiply(s32);
    void score1000Points(const LiveActor*);
    void score300Points(const LiveActor*);
    void scoreCombo(const LiveActor*, u32);
}
namespace MR {
    void startScoreAttackResult() NO_INLINE;
    bool hasScoreAttackDeclaredPowerStar() NO_INLINE;
    void hideScoreAttackLayouts();
    void showScoreAttackLayouts();
}
