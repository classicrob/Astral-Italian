#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Enemy/MameMuimuiListener.hpp"

class MameMuimui;

class MameMuimuiGroup : public LiveActorGroup {
public:
    MameMuimuiGroup(const char*, int);
    virtual ~MameMuimuiGroup();
    MameMuimui* fn_801C1610();  // a dead actor of the group or null
};

class BombTimerLayout;
class StartCountdownLayout;
class TimeUpLayout;
class ScoreCounterLayout;
class ResourceHolder;
namespace MR { class FunctorBase; }

// One spawn position of the MuimuiHole table (SMG2-only, unit-local helpers named by role).
struct MameMuimuiPlacement {
    MameMuimuiPlacement();  // fn_801C0600
    TVec3f mPos;
    s32 _C;
};

// Position table built from the "PositionList" CSV: 8 bytes (array + count).
class MameMuimuiPlacementSet {
public:
    MameMuimuiPlacementSet(s32 num);                       // fn_801C0580
    void setPos(s32 idx, const TVec3f& rPos);              // fn_801C0620
    void getPos(s32 idx, TVec3f* pOut) const;              // fn_801C0650 (pOut <- entry idx)
    MameMuimuiPlacement* mPlacements;
    s32 mNum;
};

// One row of the "Sequence" CSV: the spawn type (0 none, 1 green, 2 yellow, 3 spike) of each of the 9 holes.
struct MameMuimuiWave {
    s32 mTypes[9];
};

// Kept for the NPC lane's code: the scorer's secondary base is the MameMuimuiListener of MameMuimui.hpp.
typedef MameMuimuiListener MameMuimuiScoreListener;

class MameMuimuiScorer : public LiveActor, public MameMuimuiListener {
public:
    MameMuimuiScorer(const char*);
    virtual ~MameMuimuiScorer();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void fn_801C19C0(s32 points, MameMuimui* pActor);  // body: fn_801C0F80 (score a hit)
    virtual void fn_801C19B0(MameMuimui* pActor);              // body: fn_801C1050 (combo break)
    void registerEndCallback(const MR::FunctorBase&);
    void start();
    bool isPlaying() const;
    bool isClear() const;
    bool isNewRecord() const;
    s32 getScore() const;
    s32 getBestScore() const;
    s32 getTargetScore() const;

    // SMG2-only members (original names unknown; named by address)
    void fn_801C0BD0(ResourceHolder* pHolder);  // read the PositionList CSV, create the hole decorations
    void fn_801C0E30(ResourceHolder* pHolder);  // read the Sequence CSV
    void fn_801C10A0();                         // nerve: ResetPlayerStart
    void fn_801C1120();                         // nerve: countdown
    void fn_801C11B0();                         // nerve: start the timer
    void fn_801C1220();                         // nerve: time up
    void fn_801C1350();                         // nerve: playing
    void fn_801C13F0();                         // nerve: ResetPlayerEnd
    void fn_801C1470();                         // reset the game state
    void fn_801C14C0();                         // spawn the next wave
    void fn_801C1660();                         // retreat every green/spike/yellow muimui
    bool fn_801C1720();                         // no living muimui left

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
