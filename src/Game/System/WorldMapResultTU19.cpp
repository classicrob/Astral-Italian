#include "Game/System/WorldMapResultLocal.hpp"

extern "C" {
    void fn_804F7580(WorldMapResultCommand*, const char*);
    void fn_804F80B0(WorldMapResultCommand*, const char*);
    void fn_804F8220(WorldMapResultCommand*, const char*, s32);
    void fn_804F82C0(WorldMapResultCommand*, s32, s32);
    void fn_804E86E0(WorldMapResultCommand*, s32);
    LiveActor* fn_804E8470(WorldMapResultCommand*);
}

/// Result-map step that warps the cursor through a pipe ("State[ポイントワープ]").
WMR_STATE_DECL(WorldMapResultStatePointWarp, void exeStart(); void exeWait(); void exeEnd(););

namespace NrvWorldMapResultStatePointWarp {
    WMR_NERVE_DECL(WorldMapResultStatePointWarpStart);
    WMR_NERVE_DECL(WorldMapResultStatePointWarpWait);
    WMR_NERVE_DECL(WorldMapResultStatePointWarpEnd);
};  // namespace NrvWorldMapResultStatePointWarp

WorldMapResultStatePointWarp::WorldMapResultStatePointWarp(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[ポイントワープ]", pHost) {
}

void WorldMapResultStatePointWarp::init() {
    initNerve(GET_NERVE(WorldMapResultStatePointWarp, WorldMapResultStatePointWarpStart), 0);
    fn_804F80B0(mHost, "ポイント間ワープイン");
    fn_804F80B0(mHost, "ポイント間ワープアウト");
    mIsDead = true;
}

void WorldMapResultStatePointWarp::appear() {
    mIsDead = false;
    fn_804F8220(mHost, "ポイント間ワープイン", -1);
    setNerve(GET_NERVE(WorldMapResultStatePointWarp, WorldMapResultStatePointWarpStart));
}

void WorldMapResultStatePointWarp::kill() {
    mIsDead = true;
    fn_804F82C0(mHost, -1, -1);
}

WorldMapResultStatePointWarp::~WorldMapResultStatePointWarp() {
}

void WorldMapResultStatePointWarp::exeStart() {
    if (MR::isFirstStep(this)) {
        fn_804F7580(mHost, "EarthenPipeIn");
        MR::startActionSound(fn_804E8470(mHost), "DokanWarp", -1, -1, -1);
    }

    if (MR::isGreaterEqualStep(this, 0x5A)) {
        setNerve(GET_NERVE(WorldMapResultStatePointWarp, WorldMapResultStatePointWarpWait));
    }
}

void WorldMapResultStatePointWarp::exeWait() {
    if (MR::isFirstStep(this)) {
        fn_804F8220(mHost, "ポイント間ワープアウト", -1);
        fn_804E86E0(mHost, -1);
    }

    if (MR::isGreaterEqualStep(this, 0x1E)) {
        setNerve(GET_NERVE(WorldMapResultStatePointWarp, WorldMapResultStatePointWarpEnd));
    }
}

void WorldMapResultStatePointWarp::exeEnd() {
    if (MR::isFirstStep(this)) {
        fn_804F7580(mHost, "EarthenPipeOut");
        MR::startActionSound(fn_804E8470(mHost), "DokanWarp", -1, -1, -1);
    }

    if (MR::isGreaterEqualStep(this, 0x3C)) {
        kill();
    }
}

namespace NrvWorldMapResultStatePointWarp {
    WMR_NERVE_DEF(WorldMapResultStatePointWarpStart, WorldMapResultStatePointWarp, Start);
    WMR_NERVE_DEF(WorldMapResultStatePointWarpWait, WorldMapResultStatePointWarp, Wait);
    WMR_NERVE_DEF(WorldMapResultStatePointWarpEnd, WorldMapResultStatePointWarp, End);
};  // namespace NrvWorldMapResultStatePointWarp
