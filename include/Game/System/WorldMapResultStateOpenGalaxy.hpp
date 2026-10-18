#pragma once

#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultStateOpenGalaxy {
    WMR_NERVE_DECL(WorldMapResultStateOpenGalaxyOpen);
    WMR_NERVE_DECL(WorldMapResultStateOpenGalaxyWait);
    WMR_NERVE_DECL(WorldMapResultStateOpenGalaxyClose);
};  // namespace NrvWorldMapResultStateOpenGalaxy

/// Result-map step that opens the galaxy ("State[ギャラクシーをオープンする]").
class WorldMapResultStateOpenGalaxy : public ActorStateBaseInterface {
public:
    WorldMapResultStateOpenGalaxy(WorldMapOwner*);
    virtual ~WorldMapResultStateOpenGalaxy();

    virtual void init();
    virtual void appear();
    virtual void kill();
    void exeOpen();
    void exeWait() {
        if (MR::isGreaterEqualStep(this, 0x96)) {
            setNerve(GET_NERVE(WorldMapResultStateOpenGalaxy, WorldMapResultStateOpenGalaxyClose));
        }
    }
    void exeClose() {
        if (MR::isGreaterEqualStep(this, 0x1E)) {
            setNerve(GET_NERVE(WorldMapResultStateOpenGalaxy, WorldMapResultStateOpenGalaxyWait));
        }
    }


    /* 0x0C */ WorldMapOwner* mHost;
    /* 0x10 */ f32 _10;
    /* 0x14 */ f32 _14;
    /* 0x18 */ f32 _18;
};
