#pragma once
#include "Game/Screen/SimpleLayout.hpp"
class GalaxyWorldMapInfo;
class WorldMapArrow : public SimpleLayout {
public:
    WorldMapArrow();
    virtual ~WorldMapArrow();
    virtual void control();
    void start(GalaxyWorldMapInfo*, s32);
    void disappear();
    void exeDelay();
    void exeDisappear();
    s32 mAppearDelay;
    GalaxyWorldMapInfo* mTarget;
    TVec2f mScreenPosition;
};
