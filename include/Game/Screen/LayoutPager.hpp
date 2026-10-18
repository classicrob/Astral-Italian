#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
class LayoutActor;
class ButtonPaneController;
class LayoutPager : public ActorStateBaseInterface {
public:
    LayoutPager(LayoutActor*, const char*, bool, bool, bool, bool);
    virtual ~LayoutPager();
    virtual void appear();
    virtual bool update();
    bool isPreviousSelected() const;
    bool isNextSelected() const;
    void setPageAvailability(bool, bool, bool, bool);
private:
    LayoutActor* mHost;
    const char* mPaneName;
    ButtonPaneController* mPreviousButton;
    ButtonPaneController* mNextButton;
    LayoutActor* mBackButton;
    bool mCanShowTwoPrevious;
    bool mCanShowPrevious;
    bool mCanShowNext;
    bool mCanShowTwoNext;
};
