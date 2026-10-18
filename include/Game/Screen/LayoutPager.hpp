#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
class LayoutActor;
class BackButton;
class ButtonPaneController;
class LayoutPager : public ActorStateBase<LayoutActor> {
public:
    LayoutPager(LayoutActor*, const char*, bool, bool, bool, bool);
    virtual ~LayoutPager();
    virtual void appear();
    virtual void kill();
    void updateButtons();
    bool isPreviousSelected() const;
    bool isNextSelected() const;
    void setPageAvailability(bool, bool, bool, bool);
    void refreshButtons() NO_INLINE;
    bool isBackSelected() const NO_INLINE;
    bool trySelectPrevious() NO_INLINE;
    bool trySelectNext() NO_INLINE;
    void forceButtonsToWait() NO_INLINE;
    void exePrevious();
    void exePreviousInvalid();
    void exeNext();
    void exeNextInvalid();
    void exeWait();
    void exeDecide();
private:
    const char* mPaneName;
    ButtonPaneController* mNextButton;
    ButtonPaneController* mPreviousButton;
    BackButton* mBackButton;
    bool mCanShowTwoPrevious;
    bool mCanShowPrevious;
    bool mCanShowNext;
    bool mCanShowTwoNext;
};
