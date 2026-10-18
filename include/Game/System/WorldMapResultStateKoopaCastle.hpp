#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStateKoopaCastle {
    WMR_NERVE_DECL(WorldMapResultStateKoopaCastleWait);
};  // namespace NrvWorldMapResultStateKoopaCastle

/// Result-map step that leaves for Koopa's castle (name "State[クッパ城へ]").
class WorldMapResultStateKoopaCastle : public ActorStateBaseInterface {
public:
    WorldMapResultStateKoopaCastle(WorldMapOwner*);
    virtual ~WorldMapResultStateKoopaCastle();

    virtual void init();
    virtual void appear();
    void exeWait();

    /* 0x0C */ WorldMapOwner* mHost;
};
