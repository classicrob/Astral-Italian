#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class PlayerRacer;
class RaceManager;
class StartCountdownLayout;

enum RaceID {
    /* 0x00 */ RaceID_Penguin,
    /* 0x01 */ RaceID_TeresaPhantom,
    /* 0x02 */ RaceID_TeresaDeathPromenade,
    /* 0x03 */ RaceID_SurfingTrial,
    /* 0x04 */ RaceID_SurfingChallenge,
};

// PLACEHOLDER(astra): inherited SMG1 layout API; SMG2 RaceLayout is declared in Game/Screen/RaceLayout.hpp.
class RaceManagerLayout : public LayoutActor {
public:
    /// @brief Creates a new `RaceManagerLayout`.
    /// @param pName A pointer to the null-terminated name of the object.
    RaceManagerLayout(const char* pName);

    /// @brief Intializes the `RaceManagerLayout` while being placed into a scene.
    /// @param rIter The reference to an iterator over a `JMapInfo`.
    virtual void init(const JMapInfoIter& rIter);

    void playCountAndGo();
    void playGo();
    void playRecord();  // Note: this is very likely stripped
    void playGoal();
    void playLose();
    void playTimeUp();
    void playRank(u32);
    bool isAllAnimStopped();
    void hideRecordPane();
    void hideBestRecordPane();
    void hideAllPane();
    bool isPlayCountAnim() const;
    bool isPlayCountAndGoAnim() const;
    void setTime(u32);
    void setBestTime(u32);
};

class AbstractRacer {
public:
    /// @brief Creates a new `AbstractRacer`.
    AbstractRacer() {
        initRacer();
    }

    virtual void initRacer() {
        mRailCoord = 0.0f;
        _8 = 0.0f;
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
    /* 0x08 */ f32 _8;
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
    bool startWithWipe();
    bool startImmediately();
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

    /* 0x8C */ s32 mCurrentRace;
    StartCountdownLayout* mStartCountdown;
    /* 0x90 */ RaceManagerLayout* mLayout;
    /* 0x94 */ AbstractRacer* mRacer[10];
    /* 0xBC */ AbstractAudience* mAudience[10];
    /* 0xE4 */ u32 mRacerNum;
    /* 0xE8 */ u32 mAudienceNum;
    /* 0xEC */ u32 mRank;
    /* 0xF0 */ u32 mBestTime;
    /* 0xF4 */ u32 mTime;
    /* 0xF8 */ PlayerRacer* mPlayerRacer;
    LiveActor* mPlayerRide;
    AbstractRaceJudge* mJudge;

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
    void entryJudge(AbstractRaceJudge*);
}
