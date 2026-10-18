#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStatePointWarp {
    WMR_NERVE_DECL(WorldMapResultStatePointWarpStart);
    WMR_NERVE_DECL(WorldMapResultStatePointWarpWait);
    WMR_NERVE_DECL(WorldMapResultStatePointWarpEnd);
};  // namespace NrvWorldMapResultStatePointWarp

/// Result-map step that warps the cursor through a pipe ("State[ポイントワープ]").
WMR_OWNER_STATE_DECL(WorldMapResultStatePointWarp, void exeStart(); void exeWait(); void exeEnd(););
