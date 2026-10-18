#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapResultStateKoopaCastle.hpp"

// PLACEHOLDER(core): unnamed SMG2 callee (no symbol name yet); resolve when its owner names it
extern "C" {
    void fn_8001C030(s32);
}

namespace MR {
    void startSystemWipeCircleWithCaptureScreen(s32);
}

WorldMapResultStateKoopaCastle::WorldMapResultStateKoopaCastle(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[クッパ城へ]", pHost) {
}

void WorldMapResultStateKoopaCastle::init() {
    initNerve(GET_NERVE(WorldMapResultStateKoopaCastle, WorldMapResultStateKoopaCastleWait), 0);
    mIsDead = true;
}

void WorldMapResultStateKoopaCastle::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateKoopaCastle, WorldMapResultStateKoopaCastleWait));
}

void WorldMapResultStateKoopaCastle::exeWait() {
    if (MR::isFirstStep(this)) {
        fn_8001C030(0x1E);
    }

    if (MR::isStep(this, 0x1E)) {
        GameSequenceFunction::setWorldMapCurrentNode(mHost->getCurrentNode());
        MR::startSystemWipeCircleWithCaptureScreen(0x5A);
        GameSequenceFunction::requestChangeStageMarioFaceShipBeforeKoopaVs3();
    }

    TVec3f pos(1.0f, 0.0f, 0.0f);
    mHost->faceRocket(&pos);
}

WorldMapResultStateKoopaCastle::~WorldMapResultStateKoopaCastle() {
}

namespace NrvWorldMapResultStateKoopaCastle {
    WMR_NERVE_DEF(WorldMapResultStateKoopaCastleWait, WorldMapResultStateKoopaCastle, Wait);
};  // namespace NrvWorldMapResultStateKoopaCastle
