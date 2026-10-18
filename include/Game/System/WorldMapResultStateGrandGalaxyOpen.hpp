#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStateGrandGalaxyOpen {
    WMR_NERVE_DECL(WorldMapResultStateGrandGalaxyOpenWait);
};  // namespace NrvWorldMapResultStateGrandGalaxyOpen

/// Result-map step that zooms the map in for the Grand Galaxy start ("State[グランドギャラクシーマップ開始]").
class WorldMapResultStateGrandGalaxyOpen : public ActorStateBase< WorldMapOwner > {
public:
    WorldMapResultStateGrandGalaxyOpen(WorldMapOwner*);
    virtual ~WorldMapResultStateGrandGalaxyOpen();

    virtual void init();
    virtual void appear();
    void exeWait();


    /* 0x10 */ f32 mFovy;
};
