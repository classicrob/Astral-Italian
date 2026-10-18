#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

// Result-map helpers of WorldMapResultCommand that drive the worlds of TU07 (route opening, comet and Tico road effects).

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" {
    const Vec* fn_804E8530();
    const Vec* fn_804E8540();
    void fn_804E71F0(LiveActor*, f32);
}

/// Item (a mini actor) that travels along a route: from and to node indices at 0xA4 and 0xA8.
class WorldMapResultLinkItem : public LiveActor {
public:
    u8 _90[0x14];
    s32 mFrom;
    s32 mTo;
};

static const char* sResultCameraName = "リザルトマップカメラ";
static TVec3f sPosOffset(0.0f, 0.0f, 0.0f);
static TVec3f sRotation(0.0f, 0.0f, 0.0f);

bool updateTicoRoad(WorldMapResultCommand*, f32);

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" {
    WorldMapResultLinkItem* fn_804F0500(WorldMapResultCommand*) NO_INLINE;
    WorldMapResultWorld* fn_804F0560(WorldMapResultCommand*) NO_INLINE;
    void fn_804F05B0(WorldMapResultCommand*, const TVec3f&) NO_INLINE;
    void fn_804F0630(WorldMapResultCommand*, s32, s32) NO_INLINE;
    TVec3f* fn_804F0B40(WorldMapResultCommand*, const char*) NO_INLINE;
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" WorldMapResultLinkItem* fn_804F0500(WorldMapResultCommand* pCommand) {
    s32 index = GameDataFunction::getCurrentWorldId() - 1;
    return static_cast< WorldMapResultLinkItem* >(pCommand->mWorlds[index]->mUnit14->getTicoCreator(GameSequenceFunction::getWorldMapCurrentNode()));
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" WorldMapResultWorld* fn_804F0560(WorldMapResultCommand* pCommand) {
    s32 index = GameDataFunction::getCurrentWorldId() - 1;
    return pCommand->mWorlds[index];
}

s32 getResultEndStep() {
    return 5;
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F05B0(WorldMapResultCommand* pCommand, const TVec3f& rPos) {
    LiveActor* pActor = pCommand->mRocket;
    pActor->mPosition.x = rPos.x;
    pActor->mPosition.y = rPos.y;
    pActor->mPosition.z = rPos.z;
    pActor->makeActorAppeared();
    MR::startAction(pActor, "Wait");
    MR::startBckNoInterpole(pActor, "Wait");
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F0630(WorldMapResultCommand* pCommand, s32 worldId, s32 nodeIndex) {
    WorldMapResultNode* pNode = &pCommand->mWorlds[worldId - 1]->mMapData->mNodes[nodeIndex];
    Vec offset;

    if (fn_804F0560(pCommand)->mWorldData->getGalaxyName(nodeIndex) != nullptr) {
        offset = *fn_804E8530();
    } else {
        offset = *fn_804E8540();
    }

    fn_804F05B0(pCommand, TVec3f(pNode->mPos.x + offset.x, pNode->mPos.y + offset.y, pNode->mPos.z + offset.z));
}

// NONMATCHING: callee-saved register assignment and temp order
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F0700(WorldMapResultCommand* pCommand) {
    WorldMapResultLinkItem* pItem = fn_804F0500(pCommand);
    s32 from = pItem->mFrom;
    s32 to = pItem->mTo;
    fn_804F0560(pCommand)->setLinkProgress(from, to, 1.0f);
    fn_804F0560(pCommand)->mLinks->openFrom(from);
    updateTicoRoad(pCommand, 0.0f);
    pItem->makeActorDead();
    WorldMapResultBox box;
    box.extend(fn_804F0560(pCommand)->getNode(from)->mPos);
    box.extend(fn_804F0560(pCommand)->getNode(to)->mPos);
    TVec3f eye;
    TVec3f target;
    box.getCenter(&target);
    box.getCameraPos(&eye);
    pCommand->setCameraTarget(target);
    pCommand->setCameraEye(eye);
    fn_804F0630(pCommand, GameDataFunction::getCurrentWorldId(), from);
}

bool updateTicoRoad(WorldMapResultCommand* pCommand, f32 rate) {
    WorldMapResultLinkItem* pItem = fn_804F0500(pCommand);
    fn_804F0560(pCommand)->setLinkProgress(pItem->mFrom, pItem->mTo, rate);

    if (rate >= 1.0f) {
        pItem->makeActorDead();
    } else {
        fn_804E71F0(pItem, rate);
    }

    return rate >= 1.0f;
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F08C0(WorldMapResultCommand* pCommand) {
    WorldMapResultLinkItem* pItem = fn_804F0500(pCommand);

    if (fn_804F0560(pCommand)->isNodeOpened(pItem->mFrom)) {
        fn_804F0560(pCommand)->refreshNodeFull(pItem->mFrom);
    }
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F0920(WorldMapResultCommand* pCommand) {
    s32 index = GameDataFunction::getCurrentWorldId() - 1;
    pCommand->mWorlds[index]->mUnit18->updateAll();
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F0970(WorldMapResultCommand* pCommand, s32 worldId, WorldMapResultRoute* pRoute) {
    pCommand->mWorlds[worldId - 1]->setupRoute(pRoute);
    fn_804F0560(pCommand)->finish();
}

// NONMATCHING: register copy order at entry
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F09C0(WorldMapResultCommand* pCommand, s32 worldId, WorldMapResultRoute* pRoute) {
    WorldMapResultWorld* pWorld = pCommand->mWorlds[worldId - 1];
    GalaxyWorldMapInfo* pInfo = pWorld->mWorldData->findByName("SwitchMoveBlockGalaxy");
    pWorld->setupRouteFromNode(pRoute, pInfo->mPointIndex);
    fn_804F0560(pCommand)->finish();
    pWorld->finish();
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F0A40(WorldMapResultCommand* pCommand, s32 worldId, WorldMapResultRoute* pRoute) {
    WorldMapResultBox box;
    box.extendByRoute(pCommand->mWorlds[worldId - 1], pRoute);
    TVec3f eye;
    TVec3f target;
    box.getCenter(&target);
    box.getCameraPos(&eye);
    pCommand->setCameraTarget(target);
    pCommand->setCameraEye(eye);

    if (worldId == GameDataFunction::getCurrentWorldId()) {
        s32 node = GameSequenceFunction::getWorldMapCurrentNode();
        fn_804F0630(pCommand, GameDataFunction::getCurrentWorldId(), node);
    }
}

bool updateRouteExtend(WorldMapResultCommand* pCommand, s32 worldId, WorldMapResultRoute* pRoute, s32 step, f32 rate) {
    return pCommand->mWorlds[worldId - 1]->updateRouteStep(pRoute, step, rate);
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" bool fn_804F0B20(WorldMapResultCommand* pCommand, s32 worldId, WorldMapResultRoute* pRoute) {
    return pCommand->mWorlds[worldId - 1]->isRouteNotHeld(pRoute);
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" TVec3f* fn_804F0B40(WorldMapResultCommand* pCommand, const char* pName) {
    s32 worldId = fn_804E8960(pName);
    WorldMapResultWorld* pWorld = pCommand->mWorlds[worldId - 1];
    return &pWorld->mWorldData->findByName(pName)->mPosition;
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F0BA0(WorldMapResultCommand* pCommand, const char* pName, s32) {
    s32 worldId = fn_804E8960(pName);
    WorldMapResultWorld* pWorld = pCommand->mWorlds[worldId - 1];
    pWorld->mWorldData->findByName(pName)->endPointingAnim();
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F0C00(WorldMapResultCommand* pCommand, const char* pName) {
    s32 worldId = fn_804E8960(pName);
    WorldMapResultWorld* pWorld = pCommand->mWorlds[worldId - 1];
    pWorld->mWorldData->findByName(pName)->setComet(false);
}

// NONMATCHING: scheduling of the sPosOffset address load / actor register
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F0C60(WorldMapResultCommand* pCommand, const char* pName, LiveActor* pActor) {
    TVec3f pos = *fn_804F0B40(pCommand, pName) + sPosOffset;
    MR::setPosition(pActor, pos);
    MR::setRotation(pActor, sRotation);
    pActor->appear();
}

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_804F0D00(WorldMapResultCommand* pCommand, const char* pName) {
    s32 worldId = fn_804E8960(pName);
    pCommand->mWorlds[worldId - 1]->setupNode(0, -1);

    if (worldId == GameDataFunction::getCurrentWorldId()) {
        fn_804F0630(pCommand, worldId, GameSequenceFunction::getWorldMapCurrentNode());
    } else {
        pCommand->mRocket->makeActorDead();
    }
}

// NONMATCHING: stack slot order of target/eye
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" bool fn_804F0D90(WorldMapResultCommand* pCommand, const char* pName, TVec3f* pFrom, TVec3f* pTo, f32 rate) {
    WorldMapResultBox box;
    box.extend(*fn_804F0B40(pCommand, pName));
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
    pCommand->setCameraTarget(target);
    pCommand->setCameraEye(eye);
    MR::setProgrammableCameraParamFovy(sResultCameraName, 26.0f);
    return rate >= 1.0f;
}

// NONMATCHING: target normalizes the first flag before the call (ori r4,r3,0)
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" bool fn_804F0EE0(WorldMapResultCommand* pCommand, s32 worldId) {
    WorldMapResultWorld* pWorld = pCommand->mWorlds[worldId - 1];
    bool hasValue = pWorld->mWorldData->startOpenSoundAll();
    return (hasValue | pWorld->mUnit14->updateAll()) != 0;
}
