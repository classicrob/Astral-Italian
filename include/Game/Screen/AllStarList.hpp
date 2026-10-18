#pragma once

#include "Game/Screen/LayoutActor.hpp"

class ButtonPaneController;
class BackButton;

class AllStarList : public LayoutActor {
public:
    AllStarList(const char*, bool);
    virtual ~AllStarList();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();

    void requestMovementOn();
    void exeAppear();
    void exeWait();
    void exeSelected();
    void exePreviousPage();
    void exePreviousPageOver();
    void exeNextPage();
    void exeNextPageOver();
    void exeDisappear();
    void updatePageButtons(s32) NO_INLINE;
    void updateContents(s32) NO_INLINE;
    void startWaiting() NO_INLINE;

    ButtonPaneController* mRightButton;
    ButtonPaneController* mLeftButton;
    BackButton* mBackButton;
    s32 mWorldId;
    s32 mResult;
    bool mOnPause;
};
