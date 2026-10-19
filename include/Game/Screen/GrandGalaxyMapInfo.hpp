#pragma once

#include "Game/Screen/LayoutActor.hpp"

class GalaxyStatusIcon;
class GrandGalaxyMapGalaxy;
class JMapInfoIter;
class Nerve;

/// @brief Layout actor that drives the status icons of the grand galaxy map (the "MapGalaxyReport" layout).
class GrandGalaxyMapInfo : public LayoutActor {
public:
    struct Entry {
        /* 0x00 */ GalaxyStatusIcon* icon;
        /* 0x04 */ GrandGalaxyMapGalaxy* actor;
        /* 0x08 */ u32 _8;
        /* 0x0C */ s32 index;
    };

    GrandGalaxyMapInfo();

    virtual ~GrandGalaxyMapInfo();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();

    void startAppear();
    void startNextAppear();
    void startPreviousAppear();
    void requestEnd();
    void requestPreviousEnd();
    void requestNextEnd();

    void setIconState(s32, const char*);
    void setIconDefaultPanes(s32);
    void setIconNewPanes(s32);
    void setIconCometPanes(s32, bool, s32, bool, bool, bool);
    void setIconEventPane(s32, const char*);
    void setIconActor(s32, GrandGalaxyMapGalaxy*);
    void setIconIndex(s32, s32);
    void sortEntries();
    void updatePointing();
    bool isPointingBoxBase();
    const char* getSelectedIconState();

    void exeWait();
    void exeHide();
    void exeAppear();
    void exeNextAppear();
    void exePreviousAppear();
    void exeEnd();
    void exePreviousEnd();
    void exeNextEnd();

    /* 0x2C */ Entry** mEntries;
    /* 0x30 */ GalaxyStatusIcon* mSelectedIcon;
};
