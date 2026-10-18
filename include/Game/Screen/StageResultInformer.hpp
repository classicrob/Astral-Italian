#pragma once

#include "Game/Screen/LayoutActor.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class IconAButton;
class StageResultInformer;

class DisplayInformationForResult : public NerveExecutor {
public:
    DisplayInformationForResult(StageResultInformer*);

    void exeAppear();
    void exeDisplay();
    void exeDisappear();
    void exeIdle();

    /* 0x08 */ StageResultInformer* mHost;
    /* 0x0C */ const char* mMessageId;
    /* 0x10 */ const char* mBgmName;
};

class StageResultInformer : public LayoutActor {
public:
    /// @brief Creates a new `StageResultInformer`.
    StageResultInformer();
    virtual ~StageResultInformer();

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();

    void exeCountUpGetCometMedal();
    void exeDisplayGetCometMedal();
    void exeDisplayBestTime();
    void updateStarPieceCount();
    void updateCoinCount();
    void hideCounterPanes();
    void exeAppearGetPowerStar();
    void exeWaitBeforeCountUpPowerStar();
    void exeCountUpPowerStar();
    void exeDisplayGetPowerStar();
    void exeDisappearGetPowerStar();
    void exeShowGalaxyComplete();
    void exeAppearUpdateBestScore();
    void exeDisplayUpdateBestScore();
    void exeDisappearUpdateBestScore();
    void exeAppearGetStarPiece();
    void exeDisplayGetStarPiece();
    void exeWaitBeforeDisappearGetStarPiece();
    void exeDisappearGetStarPiece();
    void exeShowNewGalaxyDiscover();
    void exeShowGalaxyMap();
    void exeShowGetPictureBook();
    void exeShowTicoGalaxyAppear();
    void exeShowAstroMapForTico();
    void decideNextNerve();
    bool tryWaitSystemPadTriggerDecide(const Nerve*);
    bool tryWaitIntervalBeforeKeyWait(const Nerve*, int);
    bool tryShowAndKeyWaitInformationWindow(const char*, const Nerve*);
    void setCounterPaneShowHide(bool, bool, bool);

private:
    /* 0x2C */ DisplayInformationForResult* mInformationDisplayer;
    /* 0x30 */ IconAButton* mIconAButton;
    /* 0x34 */ s32 mClearedStarPieceNum;
    /* 0x38 */ bool _30;
    /* 0x3C */ s32 mPreviousPowerStarNum;
    /* 0x40 */ s32 mPreviousCometMedalNum;
    /* 0x44 */ bool mResultPaneVisible;
};

namespace NrvStageResultInformer {
    class StageResultInformerNrvDisplayGetPowerStar : public Nerve {
    public:
        static StageResultInformerNrvDisplayGetPowerStar sInstance;
        virtual void execute(Spine*) const;
    };
    class StageResultInformerNrvDisappearGetPowerStar : public Nerve {
    public:
        static StageResultInformerNrvDisappearGetPowerStar sInstance;
        virtual void execute(Spine*) const;
    };
    class StageResultInformerNrvDisplayGetCometMedal : public Nerve {
    public:
        static StageResultInformerNrvDisplayGetCometMedal sInstance;
        virtual void execute(Spine*) const;
    };
    class StageResultInformerNrvDisappearGetCometMedal : public Nerve {
    public:
        static StageResultInformerNrvDisappearGetCometMedal sInstance;
        virtual void execute(Spine*) const;
    };
    class StageResultInformerNrvDisappearBestTime : public Nerve {
    public:
        static StageResultInformerNrvDisappearBestTime sInstance;
        virtual void execute(Spine*) const;
    };
}

namespace NrvDisplayInformationForResult {
    class DisplayInformationForResultNrvIdle : public Nerve {
    public:
        static DisplayInformationForResultNrvIdle sInstance;
        virtual void execute(Spine*) const;
    };
}
