#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapResultStateKoopaCastle.hpp"
#include "Game/Util/BgmUtil.hpp"

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
        MR::stopStageAndSubBGM(0x1E);
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
