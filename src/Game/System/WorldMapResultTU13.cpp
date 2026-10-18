#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Effect/ParticleCalcExecutor.hpp"
#include "Game/System/WorldMapResultStateGalaxyMap.hpp"

extern "C" {
    void fn_804E17D0(void*);
    void fn_804E17E0(void*);
    void fn_804E1930(void*, s32);
    void* fn_804DFD30(void*);
    s32 fn_804DFD50(void*, s32);
    s32 fn_804DFE70(void*);
    s32 fn_804DFE80(void*);
    void fn_804DFCC0(void*);
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
    void* pController = fn_804DFD30(mHost->mMainLayout);
    mInt10 = mHost->getCamera()->mCurrent;
    mInt14 = fn_804DFD50(pController, mHost->getWorldId());
    mIsDead = true;
}

void WorldMapResultStateGalaxyMap::appear() {
    mIsDead = false;
    mInt10 = mHost->getCamera()->mCurrent;
    fn_804E1930(mHost->getMainLayout(), mInt14);
    MR::tryForceDeleteEffectAll(mHost->getRocket());
    fn_804E17D0(mHost->getMainLayout());
    mHost->finishWorld();
    setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapStart));
}

void WorldMapResultStateGalaxyMap::kill() {
    mIsDead = true;
}

void WorldMapResultStateGalaxyMap::control() {
    fn_804DFCC0(mHost->getMainLayout());
}

WorldMapResultStateGalaxyMap::~WorldMapResultStateGalaxyMap() {
}

void WorldMapResultStateGalaxyMap::exeStart() {
    MR::setStageBGMStateBit(0x100);
    if (fn_804DFE70(mHost->getMainLayout()) != 0) {
        setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapWait));
    }
}

void WorldMapResultStateGalaxyMap::exeWait() {
    if (fn_804DFE80(mHost->getMainLayout()) != 0) {
        fn_804E17E0(mHost->getMainLayout());
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