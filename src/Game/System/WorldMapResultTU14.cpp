#ifdef GALAXY_NATIVE
#include <petari/wm_route_scene_diag.hpp>
#endif
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapResultStateGrandGalaxyEnd.hpp"

namespace MR {
    f32 getEaseOutValue(f32, f32, f32, f32);
    f32 getFovy();
    void setFovy(f32);
}

WorldMapResultStateGrandGalaxyEnd::WorldMapResultStateGrandGalaxyEnd(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[グランドギャラクシーマップ終了]", pHost), mFovy(0.0f) {
}

void WorldMapResultStateGrandGalaxyEnd::init() {
    initNerve(GET_NERVE(WorldMapResultStateGrandGalaxyEnd, WorldMapResultStateGrandGalaxyEndWait), 0);
    mIsDead = true;
}

void WorldMapResultStateGrandGalaxyEnd::appear() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::body("WorldMapResultStateGrandGalaxyEnd::appear", 0);
    PetariWmRouteDiag::checkpoint(mHost, "GrandGalaxyEnd::appear-before-local-camera");
#endif
    mIsDead = false;
    mHost->changeRocketActionBck("Wait");
    mHost->startNodeCamera(-1, 0);
    setNerve(GET_NERVE(WorldMapResultStateGrandGalaxyEnd, WorldMapResultStateGrandGalaxyEndZoomOut));
}

void WorldMapResultStateGrandGalaxyEnd::exeZoomOut() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::bodyState("WorldMapResultStateGrandGalaxyEnd::exeZoomOut", this);
#endif
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
#ifdef GALAXY_NATIVE
    void WorldMapResultStateGrandGalaxyEndWait::execute(Spine* spine) const {
        auto* state = reinterpret_cast<WorldMapResultStateGrandGalaxyEnd*>(spine->mExecutor);
        PetariWmRouteDiag::bodyState("WorldMapResultStateGrandGalaxyEnd::exeWait", state);
        state->exeWait();
    }
    WorldMapResultStateGrandGalaxyEndWait WorldMapResultStateGrandGalaxyEndWait::sInstance ATTRIBUTE_WEAK;
#else
    WMR_NERVE_DEF(WorldMapResultStateGrandGalaxyEndWait, WorldMapResultStateGrandGalaxyEnd, Wait);
#endif
};  // namespace NrvWorldMapResultStateGrandGalaxyEnd
