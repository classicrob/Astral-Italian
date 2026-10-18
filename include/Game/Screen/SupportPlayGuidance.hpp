#pragma once
#include "Game/Screen/LayoutActor.hpp"
class IconAButton;
class SupportPlayGuidance : public LayoutActor {
public:
    SupportPlayGuidance();
    virtual ~SupportPlayGuidance();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    void requestAppear();
    void exeAppear();
    void exeWait();
    void exeDisappear();
    IconAButton* mIconAButton;
};
