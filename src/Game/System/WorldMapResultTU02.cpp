#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/MiniTicoMaster.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

// Result-map helpers of WorldMapResultCommand that drive the worlds of TU07 (route opening, comet and Tico road effects).

/// The item travelling along a route is a MiniTicoMaster: from and to node indices at 0xA4 and 0xA8.
typedef MiniTicoMaster WorldMapResultLinkItem;

static const char* sResultCameraName = "リザルトマップカメラ";
static TVec3f sPosOffset(0.0f, 0.0f, 0.0f);
static TVec3f sRotation(0.0f, 0.0f, 0.0f);

bool updateTicoRoad(WorldMapResultCommand*, f32);

MiniTicoMaster* WorldMapResultCommand::getCurrentTicoItem() {
    s32 index = GameDataFunction::getCurrentWorldId() - 1;
    return static_cast< WorldMapResultLinkItem* >(mWorlds[index]->mUnit14->getTicoCreator(GameSequenceFunction::getWorldMapCurrentNode()));
}

WorldMapResultWorld* WorldMapResultCommand::getCurrentWorld() {
    s32 index = GameDataFunction::getCurrentWorldId() - 1;
    return mWorlds[index];
}

s32 getResultEndStep() {
    return 5;
}

void WorldMapResultCommand::appearRocketAt(const TVec3f& rPos) {
    LiveActor* pActor = mRocket;
    pActor->mPosition.x = rPos.x;
    pActor->mPosition.y = rPos.y;
    pActor->mPosition.z = rPos.z;
    pActor->makeActorAppeared();
    MR::startAction(pActor, "Wait");
    MR::startBckNoInterpole(pActor, "Wait");
}

void WorldMapResultCommand::appearRocketAtNode(s32 worldId, s32 nodeIndex) {
    WorldMapResultNode* pNode = &mWorlds[worldId - 1]->mMapData->mNodes[nodeIndex];
    Vec offset;

    if (getCurrentWorld()->mWorldData->getGalaxyName(nodeIndex) != nullptr) {
        offset = *WorldMapFunction::getRocketOffsetGalaxy();
    } else {
        offset = *WorldMapFunction::getRocketOffsetPoint();
    }

    appearRocketAt(TVec3f(pNode->mPos.x + offset.x, pNode->mPos.y + offset.y, pNode->mPos.z + offset.z));
}

// NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
void WorldMapResultCommand::setupTicoRouteCreate() {
    s32 from;
    s32 to;
    WorldMapResultLinkItem* pItem = getCurrentTicoItem();
    from = pItem->mUnkA4;
    to = pItem->mUnkA8;
    getCurrentWorld()->setLinkProgress(from, to, 1.0f);
    getCurrentWorld()->mLinks->openFrom(from);
    updateTicoRoad(this, 0.0f);
    pItem->makeActorDead();
    WorldMapResultBox box;
    box.extend(getCurrentWorld()->getNode(from)->mPos);
    box.extend(getCurrentWorld()->getNode(to)->mPos);
    TVec3f eye;
    TVec3f target;
    box.getCenter(&target);
    box.getCameraPos(&eye);
    setCameraTarget(target);
    setCameraEye(eye);
    appearRocketAtNode(GameDataFunction::getCurrentWorldId(), from);
}

bool updateTicoRoad(WorldMapResultCommand* pCommand, f32 rate) {
    WorldMapResultLinkItem* pItem = pCommand->getCurrentTicoItem();
    pCommand->getCurrentWorld()->setLinkProgress(pItem->mUnkA4, pItem->mUnkA8, rate);

    if (rate >= 1.0f) {
        pItem->makeActorDead();
    } else {
        pItem->setPosOnRoute(rate);
    }

    return rate >= 1.0f;
}

void WorldMapResultCommand::refreshTicoStartNode() {
    WorldMapResultLinkItem* pItem = getCurrentTicoItem();

    if (getCurrentWorld()->isNodeOpened(pItem->mUnkA4)) {
        getCurrentWorld()->refreshNodeFull(pItem->mUnkA4);
    }
}

void WorldMapResultCommand::updateCurrentWorldUnits() {
    s32 index = GameDataFunction::getCurrentWorldId() - 1;
    mWorlds[index]->mUnit18->updateAll();
}

void WorldMapResultCommand::setupRouteOpen(s32 worldId, WorldMapResultRoute* pRoute) {
    mWorlds[worldId - 1]->setupRoute(pRoute);
    getCurrentWorld()->finish();
}

// NONMATCHING: register copy order at entry (FP order/grouping checked: no FP instructions).
void WorldMapResultCommand::setupRouteOpenFromSwitchMoveBlock(s32 worldId, WorldMapResultRoute* pRoute) {
    WorldMapResultWorld* pWorld = mWorlds[worldId - 1];
    GalaxyWorldMapInfo* pInfo = pWorld->mWorldData->findByName("SwitchMoveBlockGalaxy");
    pWorld->setupRouteFromNode(pRoute, pInfo->mPointIndex);
    getCurrentWorld()->finish();
    pWorld->finish();
}

void WorldMapResultCommand::setRouteCamera(s32 worldId, WorldMapResultRoute* pRoute) {
    WorldMapResultBox box;
    box.extendByRoute(mWorlds[worldId - 1], pRoute);
    TVec3f eye;
    TVec3f target;
    box.getCenter(&target);
    box.getCameraPos(&eye);
    setCameraTarget(target);
    setCameraEye(eye);

    if (worldId == GameDataFunction::getCurrentWorldId()) {
        s32 node = GameSequenceFunction::getWorldMapCurrentNode();
        appearRocketAtNode(GameDataFunction::getCurrentWorldId(), node);
    }
}

bool updateRouteExtend(WorldMapResultCommand* pCommand, s32 worldId, WorldMapResultRoute* pRoute, s32 step, f32 rate) {
    return pCommand->mWorlds[worldId - 1]->updateRouteStep(pRoute, step, rate);
}

bool WorldMapResultCommand::isRouteNotHeld(s32 worldId, WorldMapResultRoute* pRoute) {
    return mWorlds[worldId - 1]->isRouteNotHeld(pRoute);
}

TVec3f* WorldMapResultCommand::getGalaxyPosition(const char* pName) {
    s32 worldId = WorldMapFunction::getGalaxyWorldNo(pName);
    WorldMapResultWorld* pWorld = mWorlds[worldId - 1];
    return &pWorld->mWorldData->findByName(pName)->mPosition;
}

void WorldMapResultCommand::endGalaxyPointingAnim(const char* pName, s32) {
    s32 worldId = WorldMapFunction::getGalaxyWorldNo(pName);
    WorldMapResultWorld* pWorld = mWorlds[worldId - 1];
    pWorld->mWorldData->findByName(pName)->endPointingAnim();
}

void WorldMapResultCommand::clearGalaxyComet(const char* pName) {
    s32 worldId = WorldMapFunction::getGalaxyWorldNo(pName);
    WorldMapResultWorld* pWorld = mWorlds[worldId - 1];
    pWorld->mWorldData->findByName(pName)->setComet(false);
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void WorldMapResultCommand::appearCometAtGalaxy(const char* pName, LiveActor* pActor) {
    TVec3f pos = *getGalaxyPosition(pName) + sPosOffset;
    MR::setPosition(pActor, pos);
    MR::setRotation(pActor, sRotation);
    pActor->appear();
}

void WorldMapResultCommand::setupCometGalaxy(const char* pName) {
    s32 worldId = WorldMapFunction::getGalaxyWorldNo(pName);
    mWorlds[worldId - 1]->setupNode(0, -1);

    if (worldId == GameDataFunction::getCurrentWorldId()) {
        appearRocketAtNode(worldId, GameSequenceFunction::getWorldMapCurrentNode());
    } else {
        mRocket->makeActorDead();
    }
}

// NONMATCHING: stack slot order of target/eye
bool WorldMapResultCommand::updateCometCamera(const char* pName, TVec3f* pFrom, TVec3f* pTo, f32 rate) {
    WorldMapResultBox box;
    box.extend(*getGalaxyPosition(pName));
    TVec3f target;
    box.getCenter(&target);
    TVec3f eye;
    box.getCameraPos(&eye);
    target.x = MR::getEaseInOutValue(rate, pFrom->x, target.x, 1.0f);
    target.y = MR::getEaseInOutValue(rate, pFrom->y, target.y, 1.0f);
    target.z = MR::getEaseInOutValue(rate, pFrom->z, target.z, 1.0f);
    eye.x = MR::getEaseInOutValue(rate, pTo->x, eye.x, 1.0f);
    eye.y = MR::getEaseInOutValue(rate, pTo->y, eye.y, 1.0f);
    eye.z = MR::getEaseInOutValue(rate, pTo->z, eye.z, 1.0f);
    setCameraTarget(target);
    setCameraEye(eye);
    MR::setProgrammableCameraParamFovy(sResultCameraName, 26.0f);
    return rate >= 1.0f;
}

// NONMATCHING: target normalizes the first flag before the call (ori r4,r3,0)
bool WorldMapResultCommand::updateRouteOpenSound(s32 worldId) {
    WorldMapResultWorld* pWorld = mWorlds[worldId - 1];
    bool hasValue = pWorld->mWorldData->startOpenSoundAll();
    return (hasValue | pWorld->mUnit14->updateAll()) != 0;
}
