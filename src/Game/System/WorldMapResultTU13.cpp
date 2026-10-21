#ifdef GALAXY_NATIVE
#include <petari/wm_route_scene_diag.hpp>
#endif
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Effect/ParticleCalcExecutor.hpp"
#include "Game/System/WorldMapResultStateGalaxyMap.hpp"
#include "Game/Util/ScreenUtil.hpp"

static ParticleCalcExecutor* getGalaxyMapCalc() {
    return *reinterpret_cast< ParticleCalcExecutor** >(reinterpret_cast< u8* >(MR::getEffectSystem()) + 0x20);
}

WorldMapResultStateGalaxyMap::WorldMapResultStateGalaxyMap(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ギャラクシーマップ]", pHost) {
    mInt10 = -1;
    mInt14 = -1;
}

void WorldMapResultStateGalaxyMap::init() {
    initNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapStart), 0);
    WorldMapResultWorld* pController = static_cast< GrandGalaxyMap* >(mHost->mMainLayout)->getWorld();
    mInt10 = mHost->getCamera()->mCurrent;
    mInt14 = GrandGalaxyMap::findWorldNode(pController, mHost->getWorldId());
    mIsDead = true;
}

void WorldMapResultStateGalaxyMap::appear() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::body("WorldMapResultStateGalaxyMap::appear", 0);
#endif
    mIsDead = false;
    mInt10 = mHost->getCamera()->mCurrent;
    mHost->getMainLayout()->warpRocketToNode(mInt14);
    MR::tryForceDeleteEffectAll(mHost->getRocket());
    mHost->getMainLayout()->activate();
    mHost->finishWorld();
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::checkpoint(mHost, "GalaxyMap::appear-after-finish-local");
#endif
    setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapStart));
}

void WorldMapResultStateGalaxyMap::kill() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::body("WorldMapResultStateGalaxyMap::kill", 0);
#endif
    mIsDead = true;
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::checkpoint(mHost, "GalaxyMap::kill-after-dead-write");
#endif
}

void WorldMapResultStateGalaxyMap::control() {
    mHost->getMainLayout()->update();
}

WorldMapResultStateGalaxyMap::~WorldMapResultStateGalaxyMap() {
}

void WorldMapResultStateGalaxyMap::exeStart() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::bodyState("WorldMapResultStateGalaxyMap::exeStart", this);
#endif
    MR::setStageBGMStateBit(0x100);
    if (mHost->getMainLayout()->isWorldWarp() != 0) {
        setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapWait));
    }
}

void WorldMapResultStateGalaxyMap::exeWait() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::bodyState("WorldMapResultStateGalaxyMap::exeWait", this);
#endif
    if (mHost->getMainLayout()->isWorldWarpEnd() != 0) {
        mHost->getMainLayout()->deactivate();
        mHost->prepareWorld();
        mInt14 = mHost->getCamera()->mCurrent;
        mHost->warpRocketToNode(mInt10);
#ifdef GALAXY_NATIVE
        PetariWmRouteDiag::body("GalaxyMap::exeWait-restoration-complete", 0);
        PetariWmRouteDiag::checkpoint(mHost, "GalaxyMap::exeWait-restoration-complete");
#endif
        setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapEnd));
    }
}

void WorldMapResultStateGalaxyMap::exeEnd() {
#ifdef GALAXY_NATIVE
    PetariWmRouteDiag::bodyState("WorldMapResultStateGalaxyMap::exeEnd", this);
#endif
    for (s32 i = 0; i < 15; i++) {
        getGalaxyMapCalc()->movementCheckUpdate();
        getGalaxyMapCalc()->movementNormal();
    }

    if (MR::isGreaterEqualStep(this, 6) && MR::tryEndModelWipe()) {
        kill();
    }
}

namespace NrvWorldMapResultStateGalaxyMap {
    WorldMapResultStateGalaxyMapStart WorldMapResultStateGalaxyMapStart::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateGalaxyMapWait WorldMapResultStateGalaxyMapWait::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateGalaxyMapEnd WorldMapResultStateGalaxyMapEnd::sInstance ATTRIBUTE_WEAK;

    void WorldMapResultStateGalaxyMapEnd::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateGalaxyMap*>(pSpine->mExecutor)->exeEnd();
    }

    void WorldMapResultStateGalaxyMapWait::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateGalaxyMap*>(pSpine->mExecutor)->exeWait();
    }

    void WorldMapResultStateGalaxyMapStart::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateGalaxyMap*>(pSpine->mExecutor)->exeStart();
    }
};
