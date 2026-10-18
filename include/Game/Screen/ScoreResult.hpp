#pragma once
#include "Game/Screen/LayoutActor.hpp"
class LiveActor;
class ScoreResult : public LayoutActor {
public:
    ScoreResult(const char*);
    virtual ~ScoreResult();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    void exeAppear();
    void appearAt(s32, const TVec3f&);
    TVec2f mStartPosition;
    s32 mScore;
};
class ScoreResultKeeper : public NameObj {
public:
    ScoreResultKeeper();
    virtual ~ScoreResultKeeper();
    ScoreResult* findUnused();
    void killAll();
    ScoreResult** mResults;
    s32 mCapacity;
};
namespace MR {
    void createScoreResultKeeper();
    void appearScoreResult(s32, const TVec3f&);
    void appearScoreResult(s32, const LiveActor*);
    void killAllScoreResults();
}
