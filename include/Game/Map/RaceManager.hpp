#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class PlayerRacer;
class RaceManager;
class StartCountdownLayout;
class RaceLayout;

enum RaceID {
    /* 0x00 */ RaceID_Penguin,
    /* 0x01 */ RaceID_TeresaPhantom,
    /* 0x02 */ RaceID_TeresaDeathPromenade,
    /* 0x03 */ RaceID_SurfingTrial,
    /* 0x04 */ RaceID_SurfingChallenge,
};

class AbstractRacer {
public:
    /// @brief Creates a new `AbstractRacer`.
    AbstractRacer() {
        initRacer();
    }

    virtual void initRacer() {
        mRailCoord = 0.0f;
        mUnusedRaceValue = 0.0f;
        mCurrPosition.zero();
        mPrevPosition.zero();
    }
    virtual void prepRacer(const RaceManager* pRaceManager) = 0;
    virtual void startRacer() = 0;
    virtual bool updateRacer(const RaceManager* pRaceManager) = 0;
    virtual bool goalRacer() = 0;
    virtual void loseRacer() = 0;
    virtual void resetRacer(const RaceManager* pRaceManager) = 0;
    virtual void exitRacer() = 0;
    virtual void requestMovementOnRace() = 0;

    static bool compRacer(const AbstractRacer* pRacerA, const AbstractRacer* pRacerB) {
        return pRacerA->mRailCoord > pRacerB->mRailCoord;
    }

    /* 0x04 */ f32 mRailCoord;
    /* 0x08 */ f32 mUnusedRaceValue;
    /* 0x0C */ TVec3f mCurrPosition;
    /* 0x18 */ TVec3f mPrevPosition;
};

class AbstractAudience {
public:
    /// @brief Creates a new `AbstractAudience`.
    AbstractAudience() {
    }

    virtual void prepAudience() = 0;
    virtual void startAudience() = 0;
    virtual void resetAudience() = 0;
    virtual void exitAudience() = 0;
};

class AbstractRaceJudge {
public:
    virtual bool isJudgeReady() const = 0;
};

class RaceManager : public LiveActor {
public:
    /// @brief Creates a new `RaceManager`.
    RaceManager();

    /// @brief Intializes the `RaceManager` while being placed into a scene.
    /// @param rIter The reference to an iterator over a `JMapInfo`.
    virtual void init(const JMapInfoIter& rIter);

    virtual void movement();

    void exeWait();
    void exePrep();
    void exeWipeOut();
    void exeWipeWait();
    void exeWipeIn();
    void exeIntro();
    void exeCount();
    void exeRace();
    void exeDemo();
    void exeGoal();
    void exeRank();
    void updateRank() NO_INLINE;
    void prepareStart() NO_INLINE;
    void prepRace();
    void startRace();
    void goalRace();
    void resetRace();
    void prepBind();
    void entry(AbstractRacer*);
    void entry(PlayerRacer*);
    void entry(AbstractAudience*);
    bool isGoal(const AbstractRacer*) const;
    u32 getRank(const AbstractRacer*) const;
    void renewTime();

    /* 0x90 */ s32 mCurrentRace;
    /* 0x94 */ StartCountdownLayout* mStartCountdown;
    /* 0x98 */ RaceLayout* mLayout;
    /* 0x9C */ AbstractRacer* mRacer[10];
    /* 0xC4 */ AbstractAudience* mAudience[10];
    /* 0xEC */ u32 mRacerNum;
    /* 0xF0 */ u32 mAudienceNum;
    /* 0xF4 */ u32 mRank;
    /* 0xF8 */ u32 mBestTime;
    /* 0xFC */ u32 mTime;
    /* 0x100 */ PlayerRacer* mPlayerRacer;
    /* 0x104 */ LiveActor* mPlayerRide;
    /* 0x108 */ AbstractRaceJudge* mJudge;

};

namespace RaceManagerFunction {
    void entryRacerOthers(AbstractRacer*);
    void entryAudience(AbstractAudience*);
    void entryRacerPlayer(PlayerRacer*);
    void entryPlayerRide(LiveActor*);
    void startRaceWithWipe();
    void startRaceImmediately();
    u32 getRaceRank();
    u32 getRaceTime();
    const char* getRaceName(int);
    s32 getRaceId(const char*);
};  // namespace RaceManagerFunction

namespace RaceManagerFunction {
    void updateRank();
    void killRace();
    void entryJudge(AbstractRaceJudge*);
}
