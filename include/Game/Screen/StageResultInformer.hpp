#pragma once

#include "Game/Screen/LayoutActor.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class IconAButton;
class StageResultInformer;
class SimpleEffectLayout;

class DisplayInformationForResult : public NerveExecutor {
public:
    DisplayInformationForResult(StageResultInformer*);

    void exeAppear();
    void exeDisplay();
    void exeDisappear();
    void exeIdle();

    /* 0x08 */ StageResultInformer* mHost;
    /* 0x0C */ const char* mMessageId;
    /* 0x10 */ const wchar_t* mReplaceString;
};

class StageResultInformer : public LayoutActor {
public:
    /// @brief Creates a new `StageResultInformer`.
    StageResultInformer();
    virtual ~StageResultInformer();

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();

    void exeAppearGetCometMedal();
    void exeAppearBestTime();
    void exeComplete();
    bool isCompleted() const;
    void exeDisplayGetCoin();
    void exeWaitBeforeDisappearGetCoin();
    bool shouldShowCoinOneUp();
    void exeHideClearTime();
    void exeSendCompleteMail();
    void exeMapStart();
    void exeMapEnd();
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
    bool tryShowAndKeyWaitInformationWindow(const char*, const wchar_t*);
    const Nerve* getNextNerve() const;
    void endResultPane();
    void exeDisappearGetCometMedal();
    void exeAppearGetCoin();
    void exeDisappearGetCoin();
    void exeDisappearBestTime();
    void setCounterPaneShowHide(bool, bool, bool);

private:
    /* 0x2C */ DisplayInformationForResult* mInformationDisplayer;
    /* 0x30 */ IconAButton* mIconAButton;
    /* 0x34 */ s32 mClearedStarPieceNum;
    /* 0x38 */ bool _30;
    /* 0x3C */ SimpleEffectLayout* mOneUpLayout;
    /* 0x40 */ s32 mCoinOneUps;
    /* 0x44 */ bool mResultPaneVisible;
};

namespace NrvStageResultInformer {
    class StageResultInformerNrvDisplayGetPowerStar : public Nerve {
    public: StageResultInformerNrvDisplayGetPowerStar();
        static StageResultInformerNrvDisplayGetPowerStar sInstance;
        virtual void execute(Spine*) const;
    };
    class StageResultInformerNrvDisappearGetPowerStar : public Nerve {
    public: StageResultInformerNrvDisappearGetPowerStar();
        static StageResultInformerNrvDisappearGetPowerStar sInstance;
        virtual void execute(Spine*) const;
    };
    class StageResultInformerNrvDisplayGetCometMedal : public Nerve {
    public: StageResultInformerNrvDisplayGetCometMedal();
        static StageResultInformerNrvDisplayGetCometMedal sInstance;
        virtual void execute(Spine*) const;
    };
    class StageResultInformerNrvDisappearGetCometMedal : public Nerve {
    public: StageResultInformerNrvDisappearGetCometMedal();
        static StageResultInformerNrvDisappearGetCometMedal sInstance;
        virtual void execute(Spine*) const;
    };
    class StageResultInformerNrvDisappearBestTime : public Nerve {
    public: StageResultInformerNrvDisappearBestTime();
        static StageResultInformerNrvDisappearBestTime sInstance;
        virtual void execute(Spine*) const;
    };
}

namespace NrvDisplayInformationForResult {
    class DisplayInformationForResultNrvIdle : public Nerve {
    public: DisplayInformationForResultNrvIdle();
        static DisplayInformationForResultNrvIdle sInstance;
        virtual void execute(Spine*) const;
    };
}

namespace NrvStageResultInformer {
    class StageResultInformerNrvAppearGetPowerStar : public Nerve { public: StageResultInformerNrvAppearGetPowerStar(); static StageResultInformerNrvAppearGetPowerStar sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvAppearGetCometMedal : public Nerve { public: StageResultInformerNrvAppearGetCometMedal(); static StageResultInformerNrvAppearGetCometMedal sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvComplete : public Nerve { public: StageResultInformerNrvComplete(); static StageResultInformerNrvComplete sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvGhostAppear : public Nerve { public: StageResultInformerNrvGhostAppear(); static StageResultInformerNrvGhostAppear sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvAppearGetStarPiece : public Nerve { public: StageResultInformerNrvAppearGetStarPiece(); static StageResultInformerNrvAppearGetStarPiece sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvDisplayGetStarPiece : public Nerve { public: StageResultInformerNrvDisplayGetStarPiece(); static StageResultInformerNrvDisplayGetStarPiece sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvWaitBeforeDisappearGetStarPiece : public Nerve { public: StageResultInformerNrvWaitBeforeDisappearGetStarPiece(); static StageResultInformerNrvWaitBeforeDisappearGetStarPiece sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvDisappearGetStarPiece : public Nerve { public: StageResultInformerNrvDisappearGetStarPiece(); static StageResultInformerNrvDisappearGetStarPiece sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvAppearGetCoin : public Nerve { public: StageResultInformerNrvAppearGetCoin(); static StageResultInformerNrvAppearGetCoin sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvDisplayGetCoin : public Nerve { public: StageResultInformerNrvDisplayGetCoin(); static StageResultInformerNrvDisplayGetCoin sInstance; virtual void execute(Spine*) const; virtual void executeOnEnd(Spine*) const; };
    class StageResultInformerNrvDisappearGetCoin : public Nerve { public: StageResultInformerNrvDisappearGetCoin(); static StageResultInformerNrvDisappearGetCoin sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvAppearBestTime : public Nerve { public: StageResultInformerNrvAppearBestTime(); static StageResultInformerNrvAppearBestTime sInstance; virtual void execute(Spine*) const; };
}

namespace NrvDisplayInformationForResult {
    class DisplayInformationForResultNrvAppear : public Nerve { public: DisplayInformationForResultNrvAppear(); static DisplayInformationForResultNrvAppear sInstance; virtual void execute(Spine*) const; };
}

namespace NrvStageResultInformer {
    class StageResultInformerNrvHideClearTime : public Nerve { public: StageResultInformerNrvHideClearTime(); static StageResultInformerNrvHideClearTime sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvSendCompleteMail : public Nerve { public: StageResultInformerNrvSendCompleteMail(); static StageResultInformerNrvSendCompleteMail sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvMapStart : public Nerve { public: StageResultInformerNrvMapStart(); static StageResultInformerNrvMapStart sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvRouteOpen : public Nerve { public: StageResultInformerNrvRouteOpen(); static StageResultInformerNrvRouteOpen sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvCometAppear : public Nerve { public: StageResultInformerNrvCometAppear(); static StageResultInformerNrvCometAppear sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvMapEnd : public Nerve { public: StageResultInformerNrvMapEnd(); static StageResultInformerNrvMapEnd sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvWaitBeforeCountUpPowerStar : public Nerve { public: StageResultInformerNrvWaitBeforeCountUpPowerStar(); static StageResultInformerNrvWaitBeforeCountUpPowerStar sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvWaitBeforeCountUpCometMedal : public Nerve { public: StageResultInformerNrvWaitBeforeCountUpCometMedal(); static StageResultInformerNrvWaitBeforeCountUpCometMedal sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvWaitBeforeDisplayBestTime : public Nerve { public: StageResultInformerNrvWaitBeforeDisplayBestTime(); static StageResultInformerNrvWaitBeforeDisplayBestTime sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvWaitBeforeDisappearGetCoin : public Nerve { public: StageResultInformerNrvWaitBeforeDisappearGetCoin(); static StageResultInformerNrvWaitBeforeDisappearGetCoin sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvCoinCountEnd : public Nerve { public: StageResultInformerNrvCoinCountEnd(); static StageResultInformerNrvCoinCountEnd sInstance; virtual void execute(Spine*) const; };
    class StageResultInformerNrvCoinCountWait : public Nerve { public: StageResultInformerNrvCoinCountWait(); static StageResultInformerNrvCoinCountWait sInstance; virtual void execute(Spine*) const; };
}

namespace NrvStageResultInformer { class StageResultInformerNrvCountUpPowerStar : public Nerve { public: StageResultInformerNrvCountUpPowerStar(); static StageResultInformerNrvCountUpPowerStar sInstance; virtual void execute(Spine*) const; }; }

namespace NrvStageResultInformer { class StageResultInformerNrvCountUpGetCometMedal : public Nerve { public: StageResultInformerNrvCountUpGetCometMedal(); static StageResultInformerNrvCountUpGetCometMedal sInstance; virtual void execute(Spine*) const; }; }

namespace NrvStageResultInformer { class StageResultInformerNrvDisplayBestTime : public Nerve { public: StageResultInformerNrvDisplayBestTime(); static StageResultInformerNrvDisplayBestTime sInstance; virtual void execute(Spine*) const; }; }

extern const char cMessageGhostAppear[];
