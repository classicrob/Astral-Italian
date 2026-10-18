#pragma once

#include "Game/Screen/LayoutActor.hpp"
#include "Game/Screen/SimpleLayout.hpp"

/// Layout "MapGalaxyWorld" ("ワールドマップ基本情報", 0x30 bytes).
class MapGalaxyWorld : public SimpleLayout {
public:
    MapGalaxyWorld();

    /* 0x2C */ s32 _2C;
};

/// Layout "MapGalaxyZoomButton" ("ワールドマップズームボタン", 0x38 bytes).
class MapGalaxyZoomButton : public SimpleLayout {
public:
    MapGalaxyZoomButton();

    /* 0x2C */ s32 _2C;
    /* 0x30 */ s32 _30;
    /* 0x34 */ s32 _34;
};

/// Layout "ワールドマップカウンタ系レイアウト制御" (coin / star piece / life counters, 0x4C bytes).
class MapGalaxyCounters : public LayoutActor {
public:
    MapGalaxyCounters();

    /* 0x2C */ void* _2C[8];
};

/// Layout "ワールドマップギャラクシー情報" / "GalaxyInfo" (0x44 bytes).
class MapGalaxyInfo : public SimpleLayout {
public:
    MapGalaxyInfo(s32 layer);

    /* 0x2C */ s32 _2C;
    /* 0x30 */ s32 _30;
    /* 0x34 */ s32 _34;
    /* 0x38 */ s32 _38;
    /* 0x3C */ s32 _3C;
    /* 0x40 */ bool _40;
};
