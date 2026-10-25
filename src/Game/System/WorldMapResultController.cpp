#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

namespace NrvWorldMapResultController {
    NEW_NERVE(WorldMapResultControllerWait, WorldMapResultController, Wait);
    NEW_NERVE(WorldMapResultControllerResultMapStart, WorldMapResultController, ResultMapStart);
    NEW_NERVE(WorldMapResultControllerResultMapEnd, WorldMapResultController, ResultMapEnd);
    NEW_NERVE(WorldMapResultControllerRouteOpen, WorldMapResultController, RouteOpen);
    NEW_NERVE(WorldMapResultControllerCometAppear, WorldMapResultController, CometAppear);
    NEW_NERVE(WorldMapResultControllerRouteCreateTico, WorldMapResultController, RouteCreateTico);
};  // namespace NrvWorldMapResultController

WorldMapResultController::WorldMapResultController(WorldMapResultCommand* pCommand)
    : NerveExecutor("シーケンス管理"), mCommand(pCommand), mCometAppear(nullptr), mRouteOpen(nullptr) {
}

void WorldMapResultController::init() {
    initNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerWait), 8);
    mCometAppear = new WorldMapResultStateCometAppear(mCommand);
    mRouteOpen = new WorldMapResultStateRouteOpen(mCommand);
    MR::initActorState(this, new WorldMapResultStateStart(mCommand), GET_NERVE(WorldMapResultController, WorldMapResultControllerResultMapStart),
                       "ResultMapStart");
    MR::initActorState(this, new WorldMapResultStateEnd(mCommand), GET_NERVE(WorldMapResultController, WorldMapResultControllerResultMapEnd),
                       "ResultMapEnd");
    MR::initActorState(this, mRouteOpen, GET_NERVE(WorldMapResultController, WorldMapResultControllerRouteOpen), "RouteOpen");
    MR::initActorState(this, mCometAppear, GET_NERVE(WorldMapResultController, WorldMapResultControllerCometAppear), "CometAppear");
    MR::initActorState(this, new WorldMapResultStateCreateTico(mCommand),
                       GET_NERVE(WorldMapResultController, WorldMapResultControllerRouteCreateTico), "RouteCreateTico");
}

void WorldMapResultController::update() {
    updateNerve();
}

bool WorldMapResultController::isRouteOpenReady() const {
    return mRouteOpen->isXXX();
}

void WorldMapResultController::startResultMapStart() {
    setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerResultMapStart));
}

void WorldMapResultController::startResultMapEnd() {
    setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerResultMapEnd));
}

bool WorldMapResultController::tryRouteOpen() {
    if (mRouteOpen->isXXX()) {
        setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerRouteOpen));
        return true;
    }

    return false;
}

bool WorldMapResultController::tryCometAppear() {
    if (mCometAppear->hasComets()) {
        setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerCometAppear));
        return true;
    }

    return false;
}

void WorldMapResultController::startRouteCreateTico() {
    setNerve(GET_NERVE(WorldMapResultController, WorldMapResultControllerRouteCreateTico));
}
