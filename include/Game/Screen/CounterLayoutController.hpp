#pragma once

#include "Game/Screen/LayoutActor.hpp"

class CoinCounter;
class CometMedalCounter;
class MarioMeter;
class PlayerLeft;
class StarCounter;
class StarPieceCounter;

class CounterLayoutController : public LayoutActor {
public:
    /// @brief Creates a new `CounterLayoutController`.
    CounterLayoutController();

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    virtual void control();

    void forceAppear(bool);
    void setHPMeter(MarioMeter*);
    void requestedTicoEat(bool);
    static bool isInvalidSystemStateShowCounter();
    static bool isWaitToDisappearCounter(const LayoutActor*);
    void showAllLayout();
    void hideAllLayout();
    bool isHiddenOrWaitAllLayout() const;
    bool isPlayerMoving() const;
    void appearAllCounterWithoutStar();
    void killAllCoounter();
    bool tryEndTicoEatStarPiece();
    void exePlayerMoving();
    void exePlayerNotMoving();

    /* 0x2C */ s32 mPlayerNotMovingFrame;
    /* 0x30 */ bool mIsTicoEatRequested;
    /* 0x34 */ CoinCounter* mCoinCounter;
    /* 0x38 */ StarPieceCounter* mStarPieceCounter;
    /* 0x3C */ PlayerLeft* mPlayerLeft;
    /* 0x40 */ StarCounter* mStarCounter;
    /* 0x44 */ MarioMeter* mHPMeter;
    /* 0x48 */ CometMedalCounter* mCometMedalCounter;
};
