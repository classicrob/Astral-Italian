#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapResultStateGrandGalaxyEnd.hpp"

namespace MR {
    f32 getEaseOutValue(f32, f32, f32, f32);
    f32 getFovy();
    void setFovy(f32);
}

WorldMapResultStateGrandGalaxyEnd::WorldMapResultStateGrandGalaxyEnd(WorldMapOwner*) : ActorStateBaseInterface("State[グランドギャラクシーマップ終了]") {
}

void WorldMapResultStateGrandGalaxyEnd::init() {
    initNerve(GET_NERVE(WorldMapResultStateGrandGalaxyEnd, WorldMapResultStateGrandGalaxyEndWait), 0);
    mIsDead = true;
}

void WorldMapResultStateGrandGalaxyEnd::appear() {
    setNerve(GET_NERVE(WorldMapResultStateGrandGalaxyEnd, WorldMapResultStateGrandGalaxyEndZoomOut));
}

void WorldMapResultStateGrandGalaxyEnd::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        mFovy = MR::getFovy();
    }

    s32 steps = 0x14;
    f32 r = MR::getEaseOutValue((f32)getNerveStep() / (f32)steps, 90.0f, mFovy, 1.0f);
    MR::setFovy(r);

    if (MR::isStep(this, 0x14)) {
        kill();
    }
}

WorldMapResultStateGrandGalaxyEnd::~WorldMapResultStateGrandGalaxyEnd() {
}

namespace NrvWorldMapResultStateGrandGalaxyEnd {
    WMR_NERVE_DEF(WorldMapResultStateGrandGalaxyEndZoomOut, WorldMapResultStateGrandGalaxyEnd, ZoomOut);
    WMR_NERVE_DEF(WorldMapResultStateGrandGalaxyEndWait, WorldMapResultStateGrandGalaxyEnd, Wait);
};  // namespace NrvWorldMapResultStateGrandGalaxyEnd
