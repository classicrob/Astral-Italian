#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultStateMove.hpp"

namespace MR {
    bool testCorePadTriggerA(s32);
    bool testCorePadTriggerB(s32);
}

WorldMapResultStateMove::WorldMapResultStateMove(WorldMapOwner* pHost) : ActorStateBaseInterface("State[移動]") {
    mIsDead = true;
    mHost = pHost;
    mFlagA = false;
    mFlagB = false;
    mTarget = NULL;
}

void WorldMapResultStateMove::init() {
    initNerve(GET_NERVE(WorldMapResultStateMove, WorldMapResultStateMoveMove), 0);
    mIsDead = true;
}

void WorldMapResultStateMove::appear() {
    if (mHost->isGalaxyInfoAppeared() != 0 && mHost->isCurrentGalaxyInfoShown() == 0) {
        mTarget = mHost->getGalaxyInfoName();
    } else {
        mTarget = NULL;
    }

    mFlagB = false;
    mFlagA = mHost->isZoomButtonAppeared();
    mIsDead = false;
    mHost->resetCameraMove();
    setNerve(GET_NERVE(WorldMapResultStateMove, WorldMapResultStateMoveMove));
}

void WorldMapResultStateMove::exeMove() {
    mHost->updateParts();
    mHost->requestAppearWorldName();
    mHost->startAllPointingAnim();

    if (mFlagA) {
        mHost->requestAppearZoomButton();
    }

    if (mTarget != NULL) {
        mHost->showGalaxyInfo(mTarget);
    }

    if (MR::testCorePadTriggerA(0) || MR::testCorePadTriggerB(0)) {
        mFlagB = true;
    }

    if (mHost->updateCameraMove(mFlagB)) {
        kill();
    }
}

WorldMapResultStateMove::~WorldMapResultStateMove() {
}

namespace NrvWorldMapResultStateMove {
    WMR_NERVE_DEF(WorldMapResultStateMoveMove, WorldMapResultStateMove, Move);
};  // namespace NrvWorldMapResultStateMove
