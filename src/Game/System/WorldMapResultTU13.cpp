#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Effect/ParticleCalcExecutor.hpp"

/// Stage-info block returned by fn_804E8300: the step value at 0x3C.
struct WorldMapResultStageInfo {
    /* 0x00 */ u8 _0[0x3C];
    /* 0x3C */ s32 mValue;
};

extern "C" {
    void fn_804E17D0(void*);
    void fn_804E17E0(void*);
    void fn_804E1930(void*, s32);
    void fn_804E86E0(WorldMapResultCommand*, s32);
    void fn_804F6DF0(WorldMapResultCommand*);
    void fn_804F6E40(WorldMapResultCommand*);
    WorldMapResultStageInfo* fn_804E8300(WorldMapResultCommand*);
    LiveActor* fn_804E8470(WorldMapResultCommand*);
    void* fn_804E8950(WorldMapResultCommand*);
    void* fn_804F60D0(WorldMapResultCommand*);
    void* fn_804DFD30(void*);
    s32 fn_804DFD50(void*, void*);
    s32 fn_804DFE70(void*);
    s32 fn_804DFE80(void*);
    void fn_804DFCC0(void*);
    bool fn_80059330();
}

/// Galaxy-map step of the result sequence ("State[ギャラクシーマップ]").
WMR_STATE_DECL(WorldMapResultStateGalaxyMap, virtual void control(); void exeStart(); void exeWait(); void exeEnd();
               /* 0x10 */ s32 mInt10; /* 0x14 */ s32 mInt14;);

namespace NrvWorldMapResultStateGalaxyMap {
    WMR_NERVE_DECL(WorldMapResultStateGalaxyMapStart);
    WMR_NERVE_DECL(WorldMapResultStateGalaxyMapWait);
    WMR_NERVE_DECL(WorldMapResultStateGalaxyMapEnd);
};  // namespace NrvWorldMapResultStateGalaxyMap

static ParticleCalcExecutor* getGalaxyMapCalc() {
    return *reinterpret_cast< ParticleCalcExecutor** >(reinterpret_cast< u8* >(MR::getEffectSystem()) + 0x20);
}

WorldMapResultStateGalaxyMap::WorldMapResultStateGalaxyMap(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[ギャラクシーマップ]", pHost) {
    mInt10 = -1;
    mInt14 = -1;
}

void WorldMapResultStateGalaxyMap::init() {
    initNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapStart), 0);
    void* pController = fn_804DFD30(mHost->mController);
    mInt10 = fn_804E8300(mHost)->mValue;
    mInt14 = fn_804DFD50(pController, fn_804E8950(mHost));
    mIsDead = true;
}

void WorldMapResultStateGalaxyMap::appear() {
    mIsDead = false;
    mInt10 = fn_804E8300(mHost)->mValue;
    fn_804E1930(fn_804F60D0(mHost), mInt14);
    MR::tryForceDeleteEffectAll(fn_804E8470(mHost));
    fn_804E17D0(fn_804F60D0(mHost));
    fn_804F6E40(mHost);
    setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapStart));
}

void WorldMapResultStateGalaxyMap::kill() {
    mIsDead = true;
}

void WorldMapResultStateGalaxyMap::control() {
    fn_804DFCC0(fn_804F60D0(mHost));
}

WorldMapResultStateGalaxyMap::~WorldMapResultStateGalaxyMap() {
}

void WorldMapResultStateGalaxyMap::exeStart() {
    MR::setStageBGMStateBit(0x100);
    if (fn_804DFE70(fn_804F60D0(mHost)) != 0) {
        setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapWait));
    }
}

void WorldMapResultStateGalaxyMap::exeWait() {
    if (fn_804DFE80(fn_804F60D0(mHost)) != 0) {
        fn_804E17E0(fn_804F60D0(mHost));
        fn_804F6DF0(mHost);
        mInt14 = fn_804E8300(mHost)->mValue;
        fn_804E86E0(mHost, mInt10);
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
    WMR_NERVE_DEF(WorldMapResultStateGalaxyMapStart, WorldMapResultStateGalaxyMap, Start);
    WMR_NERVE_DEF(WorldMapResultStateGalaxyMapWait, WorldMapResultStateGalaxyMap, Wait);
    WMR_NERVE_DEF(WorldMapResultStateGalaxyMapEnd, WorldMapResultStateGalaxyMap, End);
};  // namespace NrvWorldMapResultStateGalaxyMap
