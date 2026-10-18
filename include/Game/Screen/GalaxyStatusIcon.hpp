#pragma once

#include "Game/Screen/LayoutActor.hpp"

class JMapInfoIter;

/// @brief Status icon of a galaxy in the grand galaxy map (the "MapGalaxyState" layout).
class GalaxyStatusIcon : public LayoutActor {
public:
    GalaxyStatusIcon();

    virtual ~GalaxyStatusIcon();
    virtual void init(const JMapInfoIter&);
    virtual void control();

    void hideAllPanes();
    void setDefaultPanes();
    void setNewPanes();
    void setCometPanes(bool, bool, bool, bool, bool);
    void setEventPane(s32);
    void startCometHideAnim();
    void startCometAppear();
    bool updateCometAppear();
    bool isPointingBox();
    void showForTime();

    /* 0x2C */ s32 mState;
    /* 0x30 */ bool mIsGreen;
    /* 0x31 */ bool mIsAppearing;
    /* 0x32 */ bool mIsEnabled;
    /* 0x33 */ bool mIsPointable;
    /* 0x34 */ s32 mShowTimer;
};
