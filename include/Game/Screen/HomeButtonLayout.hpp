#pragma once

#include "Game/Screen/LayoutActor.hpp"
#include <revolution/hbm.h>

class HomeButtonMenuContext {
public:
    /// @brief Creates a new `HomeButtonMenuContext`.
    HomeButtonMenuContext();

    void initControllerData();
    void initHomeButtonInfo();

    /* 0x00 */ HBMDataInfo* mHomeButtonInfo;
    /* 0x04 */ HBMControllerData* mControllerData;
};

class HomeButtonLayout : public LayoutActor {
public:
    /// @brief Creates a new `HomeButtonLayout`.
    HomeButtonLayout();

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();
    virtual void draw() const;

    void forceToDeactive();
    bool isActive() const;
    bool isActiveOrPreparing() const;
    void updateController();
    bool tryCorePadTriggerHome();

    void exeDeactive();
    void exeActive();
    void exePrepare();

    /* 0x20 */ HomeButtonMenuContext* mMenuContext;
    bool mForcedDeactivation;
};
