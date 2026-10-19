#pragma once

#include "Game/Screen/LayoutActor.hpp"

class GalaxyNamePlatePrioritied;

class GalaxyNamePlate : public LayoutActor {
public:
    GalaxyNamePlate(const char*, bool);

    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();

    void showUnknown(bool);
    void showNew(bool);
    void showOpen(bool);
    void show(const wchar_t*, bool);
    void setPos3D(const TVec3f&);
    void setShowBalloonNozzle(bool);
    void show(const wchar_t*, s32, bool, bool);

    void exeAppearReady();
    void exeAppear();
    void exeWait();

    /* 0x20 */ const char* mGalaxyName;
    /* 0x24 */ bool mIsRequested;
    /* 0x25 */ bool mStartAppear;
    /* 0x26 */ bool mShowBalloonNozzle;
    /* 0x28 */ GalaxyNamePlatePrioritied* mDrawerEntry;
    /* 0x2C */ s32 mNameAnimFrame;
    /* 0x30 */ bool mPlayNewSe;
};
