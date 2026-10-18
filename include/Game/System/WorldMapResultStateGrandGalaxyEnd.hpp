#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStateGrandGalaxyEnd {
    WMR_NERVE_DECL(WorldMapResultStateGrandGalaxyEndZoomOut);
    WMR_NERVE_DECL(WorldMapResultStateGrandGalaxyEndWait);
};  // namespace NrvWorldMapResultStateGrandGalaxyEnd

/// Result-map step that zooms the map back out at the end of the Grand Galaxy map ("State[グランドギャラクシーマップ終了]").
class WorldMapResultStateGrandGalaxyEnd : public ActorStateBase< WorldMapOwner > {
public:
    WorldMapResultStateGrandGalaxyEnd(WorldMapOwner*);
    virtual ~WorldMapResultStateGrandGalaxyEnd();

    virtual void init();
    virtual void appear();
    void exeZoomOut();

    void exeWait() {
        setNerve(GET_NERVE(WorldMapResultStateGrandGalaxyEnd, WorldMapResultStateGrandGalaxyEndWait));
    }


        /* 0x10 */ f32 mFovy;
};
