#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultStatePointWarp.hpp"

WorldMapResultStatePointWarp::WorldMapResultStatePointWarp(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ポイントワープ]", pHost) {
}

void WorldMapResultStatePointWarp::init() {
    initNerve(GET_NERVE(WorldMapResultStatePointWarp, WorldMapResultStatePointWarpStart), 0);
    mHost->initCameraSetup("ポイント間ワープイン");
    mHost->initCameraSetup("ポイント間ワープアウト");
    mIsDead = true;
}

void WorldMapResultStatePointWarp::appear() {
    mIsDead = false;
    mHost->startCameraTargetOther("ポイント間ワープイン", -1);
    setNerve(GET_NERVE(WorldMapResultStatePointWarp, WorldMapResultStatePointWarpStart));
}

void WorldMapResultStatePointWarp::kill() {
    mIsDead = true;
    mHost->startNodeCamera(-1, -1);
}

WorldMapResultStatePointWarp::~WorldMapResultStatePointWarp() {
}

void WorldMapResultStatePointWarp::exeStart() {
    if (MR::isFirstStep(this)) {
        mHost->changeRocketAction("EarthenPipeIn");
        MR::startActionSound(mHost->getRocket(), "DokanWarp", -1, -1, -1);
    }

    if (MR::isGreaterEqualStep(this, 0x5A)) {
        setNerve(GET_NERVE(WorldMapResultStatePointWarp, WorldMapResultStatePointWarpWait));
    }
}

void WorldMapResultStatePointWarp::exeWait() {
    if (MR::isFirstStep(this)) {
        mHost->startCameraTargetOther("ポイント間ワープアウト", -1);
        mHost->warpRocketToNode(-1);
    }

    if (MR::isGreaterEqualStep(this, 0x1E)) {
        setNerve(GET_NERVE(WorldMapResultStatePointWarp, WorldMapResultStatePointWarpEnd));
    }
}

void WorldMapResultStatePointWarp::exeEnd() {
    if (MR::isFirstStep(this)) {
        mHost->changeRocketAction("EarthenPipeOut");
        MR::startActionSound(mHost->getRocket(), "DokanWarp", -1, -1, -1);
    }

    if (MR::isGreaterEqualStep(this, 0x3C)) {
        kill();
    }
}

namespace NrvWorldMapResultStatePointWarp {
    WMR_NERVE_DEF(WorldMapResultStatePointWarpStart, WorldMapResultStatePointWarp, End);
    WMR_NERVE_DEF(WorldMapResultStatePointWarpWait, WorldMapResultStatePointWarp, Wait);
    WMR_NERVE_DEF(WorldMapResultStatePointWarpEnd, WorldMapResultStatePointWarp, Start);
};  // namespace NrvWorldMapResultStatePointWarp
