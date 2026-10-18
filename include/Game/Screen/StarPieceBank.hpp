#pragma once
#include "Game/Screen/LayoutActor.hpp"
class ButtonPaneController;
class SimpleTalkBalloon;
class StarPieceBank : public LayoutActor {
public:
    StarPieceBank(const char*);
    virtual ~StarPieceBank();
    virtual void appear();
    virtual void control();
    void exeAppear();
    void exeSelect();
    void exeTrade();
    void exeDecided();
    void exeDisappear();
    void updateCounters();
    void updatePointer();
private:
    SimpleTalkBalloon* mExplanation;
    ButtonPaneController* mWithdrawButton;
    ButtonPaneController* mDepositButton;
    ButtonPaneController* mConfirmButton;
    ButtonPaneController* mCancelButton;
    s32 mBankedStarPieces;
    s32 mStockedStarPieces;
};
