#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Effect/ParticleCalcExecutor.hpp"
#include "Game/System/WorldMapResultStateGalaxyMap.hpp"

extern "C" {
    bool fn_80059330();
}

static ParticleCalcExecutor* getGalaxyMapCalc() {
    return *reinterpret_cast< ParticleCalcExecutor** >(reinterpret_cast< u8* >(MR::getEffectSystem()) + 0x20);
}

WorldMapResultStateGalaxyMap::WorldMapResultStateGalaxyMap(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ギャラクシーマップ]", pHost) {
    mInt10 = -1;
    mInt14 = -1;
}

void WorldMapResultStateGalaxyMap::init() {
    initNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapStart), 0);
    WorldMapResultWorld* pController = mHost->getMainLayout()->getWorld();
    mInt10 = mHost->getCamera()->mCurrent;
    mInt14 = GrandGalaxyMap::findWorldNode(pController, mHost->getWorldId());
    mIsDead = true;
}

void WorldMapResultStateGalaxyMap::appear() {
    mIsDead = false;
    mInt10 = mHost->getCamera()->mCurrent;
    mHost->getMainLayout()->warpRocketToNode(mInt14);
    MR::tryForceDeleteEffectAll(mHost->getRocket());
    mHost->getMainLayout()->activate();
    mHost->finishWorld();
    setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapStart));
}

void WorldMapResultStateGalaxyMap::kill() {
    mIsDead = true;
}

void WorldMapResultStateGalaxyMap::control() {
    mHost->getMainLayout()->update();
}

WorldMapResultStateGalaxyMap::~WorldMapResultStateGalaxyMap() {
}

void WorldMapResultStateGalaxyMap::exeStart() {
    MR::setStageBGMStateBit(0x100);
    if (mHost->getMainLayout()->isWorldWarp() != 0) {
        setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapWait));
    }
}

void WorldMapResultStateGalaxyMap::exeWait() {
    if (mHost->getMainLayout()->isWorldWarpEnd() != 0) {
        mHost->getMainLayout()->deactivate();
        mHost->prepareWorld();
        mInt14 = mHost->getCamera()->mCurrent;
        mHost->warpRocketToNode(mInt10);
        setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapEnd));
    }
}

void WorldMapResultStateGalaxyMap::exeEnd() {
    for (s32 i = 0; i < 15; i++) {
        getGalaxyMapCalc()->movementCheckUpdate();
        getGalaxyMapCalc()->movementNormal();
    }

    if (MR::isGreaterEqualStep(this, 6) && fn_80059330()) {
        kill();
    }
}

namespace NrvWorldMapResultStateGalaxyMap {
    WMR_NERVE_DEF(WorldMapResultStateGalaxyMapStart, WorldMapResultStateGalaxyMap, End);
    WMR_NERVE_DEF(WorldMapResultStateGalaxyMapWait, WorldMapResultStateGalaxyMap, Wait);
    WMR_NERVE_DEF(WorldMapResultStateGalaxyMapEnd, WorldMapResultStateGalaxyMap, Start);
};  