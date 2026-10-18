#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

class GalaxyConfirmLayout;
class MiniTicoMaster;

namespace NrvWorldMapResultStateTicoRouteCreate {
    WMR_NERVE_DECL(WorldMapResultStateTicoRouteCreateZoomIn);
    WMR_NERVE_DECL(WorldMapResultStateTicoRouteCreateWait);
    WMR_NERVE_DECL(WorldMapResultStateTicoRouteCreateReturn);
    WMR_NERVE_DECL(WorldMapResultStateTicoRouteCreateZoomOut);
};  // namespace NrvWorldMapResultStateTicoRouteCreate

/// Step that creates the tico route ("State[チコのルート作り]").
WMR_OWNER_STATE_DECL(WorldMapResultStateTicoRouteCreate, virtual void control(); void exeZoomIn(); void exeWait(); void exeReturn(); void exeZoomOut(); void updateRoutePosition(f32);
               /* 0x10 */ TVec3f mPos; /* 0x1C */ GalaxyConfirmLayout* mLayout; /* 0x20 */ MiniTicoMaster* mTicoCreator;);
