#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

WorldMapResultStateRouteOpen::WorldMapResultStateRouteOpen(WorldMapResultCommand* pHost)
    : ActorStateBase< WorldMapResultCommand >("State[ルートオープン]", pHost) {
    mRoute = nullptr;
    mCount = 0;
    mWorldId = 0;
}

void WorldMapResultStateRouteOpen::init() {
    mRoute = new WorldMapResultRoute();
    initNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenStart), 0);

    if (GameSequenceFunction::hasStageResultSequence() && GameSequenceFunction::isClearedAllPowerStarAtResult()) {
        mWorldId = 7;
        mHost->setupRouteOpenFromSwitchMoveBlock(7, mRoute);
    } else {
        mWorldId = GameDataFunction::getCurrentWorldId();
        mHost->setupRouteOpen(mWorldId, mRoute);
    }

    kill();
}

void WorldMapResultStateRouteOpen::appear() {
    mCount = 0;
    mIsDead = false;
    mHost->startResult(mWorldId);
    mHost->setRouteCamera(mWorldId, mRoute);
    setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenStart));
}

void WorldMapResultStateRouteOpen::control() {
    mHost->updateRouteOpenSound(mWorldId);
}

bool WorldMapResultStateRouteOpen::isXXX() const {
    return mRoute->mCount > 0;
}

void WorldMapResultStateRouteOpen::exeExtend() {
    WorldMapResultCommand* pHost = mHost;
    f32 rate = MR::calcNerveRate(this, 30);
    bool done = updateRouteExtend(pHost, mWorldId, mRoute, mCount, rate);
    MR::startActionSound(mHost->mRocket, "RoadExtend", -1, -1, -1);

    if (MR::isGreaterStep(this, 30)) {
        if (done) {
            setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenMessage));
        } else {
            mCount++;
            setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenExtend));
        }
    }
}

void WorldMapResultStateRouteOpen::exeWaitNext() {
    if (mHost->isRouteNotHeld(mWorldId, mRoute)) {
        setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenMessage));
    }
}

void WorldMapResultStateRouteOpen::exeMessage() {
    if (MR::isStep(this, 12)) {
        MR::appearInformationMessageDown("System_ResultWorldMapRouteOpen", true);
        MR::startSubBGM("BGM_JG_NEW_GALAXY", false);
    }

    if (!MR::isLessEqualStep(this, 12) && MR::isDeadInformationMessage()) {
        setNerve(GET_NERVE(WorldMapResultStateRouteOpen, WorldMapResultStateRouteOpenEnd));
    }
}

inline void WorldMapResultStateRouteOpen::exeEnd() {
    if (MR::isStep(this, 12)) {
        kill();
    }
}

WorldMapResultStateRouteOpen::~WorldMapResultStateRouteOpen() {
}

namespace NrvWorldMapResultStateRouteOpen {
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenStart, WorldMapResultStateRouteOpen, Start);
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenExtend, WorldMapResultStateRouteOpen, Message);
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenWaitNext, WorldMapResultStateRouteOpen, WaitNext);
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenMessage, WorldMapResultStateRouteOpen, Extend);
    WMR_NERVE_DEF(WorldMapResultStateRouteOpenEnd, WorldMapResultStateRouteOpen, End);
};  // namespace NrvWorldMapResultStateRouteOpen
