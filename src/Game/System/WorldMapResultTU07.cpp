#define JGEOMETRY_VEC3_WMR_OUTLINE
#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultStatePointEvent.hpp"
#include "Game/System/WorldMapResultStateTicoRouteCreate.hpp"
#include "Game/System/WorldMapResultStateWorldNews.hpp"
#include "Game/System/WorldMapResultStateWorldWarp.hpp"
#include "Game/System/WorldMapResultStateGalaxyMap.hpp"
#include "Game/System/WorldMapResultStateLeaveHome.hpp"
#include "Game/System/WorldMapResultStateMove.hpp"
#include "Game/System/WorldMapResultStatePointWarp.hpp"
#include "Game/System/WorldMapResultStateOpenGalaxy.hpp"
#include "Game/System/WorldMapResultStateEnterGalaxy.hpp"
#include "Game/System/WorldMapResultStateEnterHome.hpp"
#include "Game/System/WorldMapResultStateKoopaCastle.hpp"
#include "Game/System/WorldMapResultStateGrandGalaxyOpen.hpp"
#include "Game/System/WorldMapResultStateGrandGalaxyEnd.hpp"
#include "Game/System/WorldMapResultStateGreenStarDemo.hpp"

#include "Game/AudioLib/AudInstListMgr.hpp"

static f32 sCameraAngle = 37.86f;
extern const f32 sCameraWidthScale;
extern const f32 sCameraScreenWidth;
extern const f32 sRouteTailRate;

WorldMapResultBox::WorldMapResultBox() : mMin(100000.0f, 100000.0f, 100000.0f) {
    mMax.x = -100000.0f;
    mMax.y = -100000.0f;
    mMax.z = -100000.0f;
}

void WorldMapResultBox::extend(const TVec3f& rPos) {
    if (rPos.x < mMin.x) {
        mMin.x = rPos.x;
    }

    if (mMax.x < rPos.x) {
        mMax.x = rPos.x;
    }

    if (rPos.y < mMin.y) {
        mMin.y = rPos.y;
    }

    if (mMax.y < rPos.y) {
        mMax.y = rPos.y;
    }

    if (rPos.z < mMin.z) {
        mMin.z = rPos.z;
    }

    if (mMax.z < rPos.z) {
        mMax.z = rPos.z;
    }
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void WorldMapResultBox::getCenter(TVec3f* pDst) const {
    pDst->set<f32>((mMax + mMin).scaleInline(0.5f));
    pDst->z -= 2000.0f;
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void WorldMapResultBox::getCameraPos(TVec3f* pDst) const {
    f32 dist = calcCameraDistance();
    TVec3f center;
    TVec3f dir;
    f32 sinAngle = JMASinDegree(sCameraAngle);
    f32 cosAngle = JMACosDegree(sCameraAngle);
    dir.x = 0.0f;
    dir.y = sinAngle;
    dir.z = cosAngle;
    getCenter(&center);
    f32 len = 45000.0f;
    if (len >= dist) {
    } else {
        len = dist;
    }
    TVec3f scaled(dir);
    scaled.scale(len);
    TVec3f pos = center + scaled;
    pDst->set<f32>(pos);
}

f32 WorldMapResultBox::calcCameraDistance() const {
    f32 fov = 0.4537856f;
    f32 tanHalf = JMASinRadian(0.5f * fov) / JMACosRadian(0.5f * fov);
    f32 dist = (0.5f * (f32)MR::getScreenHeight()) / tanHalf;
    f32 width = (dist * (mMax.x - mMin.x)) / (sCameraWidthScale * sCameraScreenWidth);
    f32 depth = (dist * (mMax.z - mMin.z)) / (0.75f * (f32)MR::getScreenHeight());
    width = width >= depth ? width : depth;

    return width;
}

static const char sMapArcFormat[] = "WorldMap%02d.arc";

JMapInfo* WorldMapResultWorld::createParser(const char* pFile) {
    char arc[0x40];
    snprintf(arc, 0x40, sMapArcFormat, mWorldId);
    return MR::createCsvParserFromFile(arc, pFile);
}

s32 WorldMapResultWorld::getNodeNum() const {
    return mMapData->mNum;
}

WorldMapResultNode* WorldMapResultWorld::getNode(s32 index) const {
    return &mMapData->mNodes[index];
}

bool WorldMapResultWorld::isNodeValid(s32 index) const {
    return mMapData->isNodeValid(index);
}

bool WorldMapResultMapData::isNodeValid(s32 index) const {
    return mNodes[index].mIsValid != 0;
}

bool WorldMapResultWorld::isNodeHidden(s32 index) const {
    return mMapData->isNodeHidden(index);
}

bool WorldMapResultMapData::isNodeHidden(s32 index) const {
    return mNodes[index].mIsHidden != 0;
}

bool WorldMapResultWorld::isNodeFlag11(s32 index) const {
    return mMapData->mNodes[index].mFlag11 != 0;
}

bool WorldMapResultWorld::isLinked(s32 a, s32 b) const {
    if (mLinks->isLinked(a, b, 1)) {
        return true;
    }

    return mUnit14->isPipeTarget(a, b);
}

bool WorldMapResultWorld::isLinked14(s32 a, s32 b) const {
    return mUnit14->isPipeTarget(a, b);
}

bool WorldMapResultWorld::isLinkedDirect(s32 a, s32 b) const {
    return mLinks->isLinked(a, b, 1);
}

bool WorldMapResultWorld::isPathReachable(s32 index) const {
    return mPathFinder->isReachable(index);
}

bool WorldMapResultWorld::hasOpenLine(s32 node) {
    return mLinks->hasOpenLine(node);
}

bool WorldMapResultWorld::hasOpenLineNoAlpha(s32 node) {
    return mLinks->hasOpenLineNoAlpha(node);
}

bool WorldMapResultWorld::isNodeOpened(s32 index) const {
    const char* pName = mWorldData->getGalaxyName(index);

    if (pName != nullptr) {
        return GameDataFunction::getPowerStarNumOwned(pName) > 0;
    }

    if (mUnit14->isStarCheckPointPending(index)) {
        return false;
    }

    return !mUnit14->isWarpOpen(index);
}

void WorldMapResultWorld::updatePaths(s32 start) {
    mPathFinder->update(start);
}

s32 WorldMapResultWorld::getPathDistance(s32 index) const {
    return mPathFinder->getDistance(index);
}

s32 WorldMapResultWorld::getPathNode(s32 index, s32 step) const {
    return mPathFinder->getPath(index, step);
}

bool WorldMapResultWorld::setupNode(s32 index, s32 start) {
    s32 target = start;

    if (start < 0) {
        target = index;
    }

    if (mLinks->openFrom(index)) {
        refreshNode(target);
        return true;
    }

    mUnit18->updateAll();
    return false;
}

void WorldMapResultWorld::refreshNode(s32 start) {
    mPathFinder->update(start);
    mUnit18->updateAll();
    mWorldData->tryNewBossAll();
}

void WorldMapResultWorld::refreshNodeFull(s32 start) {
    mPathFinder->update(start);
    mUnit18->updateReachable();
    mUnit18->updateAll();
    mWorldData->tryNewAll();
}

bool WorldMapResultWorld::isNodeNotReady(s32 index) const {
    if (hasWorldData(index)) {
        return hasWorldDataAndComet(index);
    }

    return !mUnit14->isNotStarPieceMine(index);
}

bool WorldMapResultWorld::hasWorldData(s32 index) const {
    return mWorldData->getGalaxyName(index) != nullptr;
}

bool WorldMapResultWorld::hasWorldDataAndComet(s32 index) const {
    GalaxyWorldMapInfo* pInfo = mWorldData->find(index);
    if (pInfo != nullptr && pInfo->isNerveHide()) {
        return true;
    }

    return false;
}

bool WorldMapResultWorld::isWorldDataType(s32 index, s32 type) const {
    GalaxyWorldMapInfo* pInfo = mWorldData->find(index);
    if (pInfo != nullptr && type == pInfo->mKind) {
        return true;
    }

    return false;
}

bool WorldMapResultWorld::isWorldDataType3(s32 index) const {
    return isWorldDataType(index, 3);
}

bool WorldMapResultWorld::isWorldDataType4(s32 index) const {
    return isWorldDataType(index, 4);
}

bool WorldMapResultWorld::isWorldDataType5(s32 index) const {
    return isWorldDataType(index, 5);
}

bool WorldMapResultWorld::callU14_6D0(s32 index) const {
    return mUnit14->isStarCheckPointPending(index);
}

bool WorldMapResultWorld::callU14_450(s32 index) {
    return mUnit14->isTicoCreatorNotFull(index);
}

bool WorldMapResultWorld::callU14_860(s32 index) {
    return mUnit14->isEarthenPipe(index);
}

bool WorldMapResultWorld::callU14_8A0(s32 index) {
    return mUnit14->isWorldWarp(index);
}

bool WorldMapResultWorld::callU14_620(s32 index) {
    return mUnit14->isStarCheckPointReady(index);
}

s32 WorldMapResultWorld::callU14_2B0(s32 index) {
    return mUnit14->getPipeTarget(index);
}

s32 WorldMapResultWorld::callU14_310(s32 index) {
    return mUnit14->getPipeOtherEnd(index);
}


#pragma dont_inline on
void WorldMapResultWorld::setLinkProgress(s32 a, s32 b, f32 t) {
    mLinks->setLineAlpha(a, b, t);

    if (t >= 1.0f) {
        mLinks->deleteLineEffect(a, b);
    } else if (t > 0.0f) {
        mLinks->emitLineEffect(a, b);
    }
}


#pragma dont_inline reset
void WorldMapResultWorld::setLinkProgressFull(s32 a, s32 b) {
    mLinks->setLineHidden(a, b);
    mLinks->setLineEffectTico(a, b);
    mLinks->setLineAlpha(a, b, 0.0f);
}

s32 WorldMapResultWorld::getWorldId() const {
    return mWorldId;
}

void WorldMapResultWorld::disappear() {
    mUnit14->resetStarCheckPoints();
}

// NONMATCHING: regalloc (stack slots, register assignment, or instruction/branch scheduling) (FP order/grouping checked: no FP instructions).
void WorldMapResultRoute::collect(WorldMapResultWorld* pWorld) {
    mCount = 0;
    WorldMapResultMapData* pMap = pWorld->mMapData;
    MiniRouteLineList* pLinks = pWorld->mLinks;
    s32 num = pMap->mNum;

    for (s32 i = 0; i < num; i++) {
        for (s32 j = i + 1; j < num; j++) {
            if (pLinks->isLinked(i, j, 1)) {
                if (pLinks->isDirected(i, j)) {
                    addEdge(j, i);
                } else {
                    addEdge(i, j);
                }
            }
        }
    }
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void WorldMapResultWorld::setupRouteSteps(WorldMapResultRoute* pRoute, s32 node) {
    s32 step = 0;

    for (s32 i = 0; i < pRoute->mCount; i++) {
        WorldMapResultEdge* pEdge = &pRoute->mEdges[i];
        s32 from = pEdge->mFrom;
        s32 to = pEdge->mTo;

        if (from == node || to == node) {
            mLinks->setLineAlpha(from, to, 0.0f);
            pEdge->mStep = 0;

            if (from == node) {
                pEdge->mReverse = 0;
            } else {
                pEdge->mReverse = 1;
            }
        }
    }

    bool found = true;

    while (found && step <= 100) {
        found = false;
        step++;

        for (s32 i = 0; i < pRoute->mCount; i++) {
            WorldMapResultEdge* pEdge = &pRoute->mEdges[i];
            s32 from = pEdge->mFrom;
            s32 to = pEdge->mTo;

            if (pEdge->mStep == -1) {
                for (s32 j = 0; j < pRoute->mCount; j++) {
                    WorldMapResultEdge* pOther = &pRoute->mEdges[j];

                    if (i != j && step - 1 == pOther->mStep) {
                        s32 otherFrom = pOther->mFrom;
                        s32 otherTo = pOther->mTo;
                        s32 adjacent = 0;

                        if (from == otherFrom || from == otherTo || to == otherFrom || to == otherTo) {
                            adjacent = 1;
                        }

                        bool connected = adjacent != 0;
                        adjacent = connected | isLinked14(from, otherFrom);
                        connected = adjacent != 0;
                        adjacent = connected | isLinked14(from, otherTo);
                        connected = adjacent != 0;
                        adjacent = connected | isLinked14(to, otherFrom);
                        connected = adjacent != 0;
                        adjacent = connected | isLinked14(to, otherTo);

                        if (adjacent != 0) {
                            mLinks->setLineAlpha(from, to, 0.0f);
                            pEdge->mStep = step;
                            found = true;

                            if (from == otherFrom || from == otherTo) {
                                pEdge->mReverse = 0;
                            } else {
                                pEdge->mReverse = 1;
                            }

                            break;
                        }
                    }
                }
            }
        }

    }
}

void WorldMapResultWorld::refreshRouteComet() {
    WorldMapResultRoute route;
    route.collect(this);
    GameSequenceFunction::storeWorldMapRoute(&route);
}

void WorldMapResultWorld::startRoutePointOpenSound(s32 pointIndex) {
    mUnit18->find(pointIndex)->startOpenSound();
}

void WorldMapResultWorld::startRoutePointCloseSound(s32 pointIndex) {
    mUnit18->find(pointIndex)->startCloseSound();
}

void WorldMapResultWorld::setupRoute(WorldMapResultRoute* pRoute) {
    s32 node = GameSequenceFunction::getWorldMapCurrentNode();
    mLinks->openFrom(0);
    WorldMapResultRoute route;
    GameSequenceFunction::loadWorldMapRoute(&route);
    pRoute->collect(this);
    pRoute->subtract(&route);
    GameSequenceFunction::clearWorldMapRouteCount();
    mPathFinder->update(node);
    setupRouteSteps(pRoute, node);
    refreshNode(node);
    mUnit14->resetAll();
}

void WorldMapResultWorld::setupRouteFromNode(WorldMapResultRoute* pRoute, s32 node) {
    mLinks->openFrom(0);
    WorldMapResultRoute route;
    MiniRouteLineList* pLinks = mLinks;
    route.mCount = 0;
    s32 num = mMapData->mNum;

    for (s32 i = 0; i < num; i++) {
        for (s32 j = i + 1; j < num; j++) {
            if (pLinks->isLinked(i, j, 1) && !pLinks->hasLineFlag(i, j)) {
                if (pLinks->isDirected(i, j)) {
                    route.addEdge(j, i);
                } else {
                    route.addEdge(i, j);
                }
            }
        }
    }

    pRoute->collect(this);
    pRoute->subtract(&route);
    GameSequenceFunction::clearWorldMapRouteCount();
    mPathFinder->update(node);
    setupRouteSteps(pRoute, node);
    refreshNode(node);
    mUnit14->resetAll();
}

// NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
void WorldMapResultBox::extendByRoute(WorldMapResultWorld* pWorld, WorldMapResultRoute* pRoute) {
    s32 i = 0;
    WorldMapResultMapData* pMap = pWorld->mMapData;

    for (; i < pRoute->mCount; i++) {
        s32 to = pRoute->mEdges[i].mTo;
        extend(pMap->mNodes[pRoute->mEdges[i].mFrom].mPos);
        extend(pMap->mNodes[to].mPos);
    }
}

bool WorldMapResultWorld::updateRouteStep(WorldMapResultRoute* pRoute, s32 step, f32 t) {
    s32 maxStep = 0;

    for (s32 i = 0; i < pRoute->mCount; i++) {
        WorldMapResultEdge* pEdge = &pRoute->mEdges[i];
        s32 from = pEdge->mFrom;
        s32 edgeStep = pEdge->mStep;
        s32 to = pEdge->mTo;

        if (maxStep < edgeStep) {
            maxStep = edgeStep;
        }

        if (edgeStep == step) {
            if (pEdge->mReverse != 0) {
                setLinkProgress(to, from, t);
            } else {
                setLinkProgress(from, to, t);
            }
        }
    }

    if (t >= 1.0f) {
        refreshNodeFull(GameSequenceFunction::getWorldMapCurrentNode());
    }

    bool done = false;

    if (t >= 1.0f && step >= maxStep) {
        done = true;
    }

    return done;
}

// NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
bool WorldMapResultWorld::isRouteNotHeld(WorldMapResultRoute* pRoute) const {
    s32 to;

    GalaxyWorldMapInfoHolder* pData = mWorldData;

    for (s32 i = 0; i < pRoute->mCount; i++) {
        to = pRoute->mEdges[i].mTo;

        if (pData->isNewOpen(pRoute->mEdges[i].mFrom) || pData->isNewOpen(to)) {
            return false;
        }
    }

    return true;
}

void WorldMapResultBox::extendByWorld(WorldMapResultWorld* pWorld) {
    WorldMapResultMapData* pMap = pWorld->mMapData;

    for (s32 i = 0; i < pWorld->mMapData->mNum; i++) {
        WorldMapResultNode* pNode = &pMap->mNodes[i];

        if (pNode->mIsValid != 0 && pNode->mIsHidden == 0) {
            extend(pNode->mPos);
        }
    }
}

// NONMATCHING: FP operand commutation only (grouping checked); register/stack/branch lowering.
void calcRoutePos(TVec3f* pDst, const TVec3f& rFrom, const TVec3f& rTo, s32 step, s32 maxStep) {
    s32 splitStep = maxStep;
    splitStep *= 0.8f;
    f32 rate = MR::getLinerValue(MR::normalizeAndClamp01((f32)step, 0.0f, (f32)splitStep), 0.0f, 1.0f - sRouteTailRate, 1.0f);
    rate += MR::getEaseOutValue(MR::normalizeAndClamp01((f32)step, (f32)splitStep, (f32)maxStep), 0.0f, 0.12f, 1.0f);
    pDst->set<f32>(rFrom.scaleInline(1.0f - rate) + rTo.scaleInline(rate));
}

void WorldMapResultWorld::setNum(s32 num) {
    mWorldData->setAllEffectDrawOrder(num);
    mLinks->setEffectDrawOrder(num);
    mUnit14->setEffectDrawOrder(num);
}

WorldMapResultPathFinder::Entry::Entry() : mDistance(0), mPath(nullptr) {
}

WorldMapResultPathFinder::WorldMapResultPathFinder(WorldMapResultWorld* pWorld) {
    mWorld = pWorld;
    mEntries = nullptr;
    allocate();
    update(0);
}

void WorldMapResultPathFinder::update(s32 start) {
    s32 num = mWorld->getNodeNum();

    for (s32 i = 0; i < num; i++) {
        Entry* pEntry = &mEntries[i];

        for (s32 j = 0; j < num; j++) {
            pEntry->mDistance = -1;
            pEntry->mPath[j] = -1;
        }
    }

    u8 visited[0x100];

    for (s32 i = 0; i < num; i++) {
        visited[i] = 0;
    }

    visited[start] = 1;
    mEntries[start].mDistance = 0;

    for (s32 i = 0; i < num; i++) {
        Entry* pEntry = &mEntries[i];

        if (mWorld->isLinked(start, i)) {
            pEntry->mDistance = 1;
            visited[i] = 1;
            pEntry->mPath[0] = i;
        }
    }

    s32 depth = 1;

    do {
        bool found = false;

        for (s32 i = 0; i < num; i++) {
            if (mWorld->isNodeValid(i) && visited[i] && i == mEntries[i].mPath[depth - 1]) {
                for (s32 j = 0; j < num; j++) {
                    Entry* pOther = &mEntries[j];

                    if (mWorld->isNodeValid(j) && !visited[j] && mWorld->isLinked(i, j)) {
                        pOther->mDistance = depth + 1;
                        visited[j] = 1;
                        found = true;
                        pOther->mPath[depth] = j;

                        for (s32 k = depth - 1; k >= 0; k--) {
                            pOther->mPath[k] = mEntries[i].mPath[k];
                        }
                    }
                }
            }
        }

        if (!found) {
            break;
        }

        depth++;
    } while (depth < 32);
}

bool WorldMapResultPathFinder::isReachable(s32 index) const {
    return mEntries[index].mDistance >= 0;
}

s32 WorldMapResultPathFinder::getDistance(s32 index) const {
    return mEntries[index].mDistance;
}

s32 WorldMapResultPathFinder::getPath(s32 index, s32 step) const {
    return mEntries[index].mPath[step];
}

void WorldMapResultPathFinder::allocate() {
    s32 num = mWorld->getNodeNum();
    mEntries = new Entry[num];

    for (s32 i = 0; i < num; i++) {
        Entry* pEntry = &mEntries[i];
        pEntry->mPath = new s32[num];

        for (s32 j = 0; j < num; j++) {
            pEntry->mDistance = 1;
            pEntry->mPath[j] = -1;
        }
    }
}

WorldMapResultWorld::WorldMapResultWorld(s32 worldId) {
    mWorldId = worldId;
    mMapData = nullptr;
    mLinks = nullptr;
    mWorldData = nullptr;
    _10 = 0;
    mUnit14 = nullptr;
    mUnit18 = nullptr;
    mPathFinder = nullptr;
}

void WorldMapResultWorld::initWorld(s32 flag) {
    mMapData = new WorldMapResultMapData(this);
    mLinks = new MiniRouteLineList(this);
    mLinks->init();
    mWorldData = new GalaxyWorldMapInfoHolder(this);
    mWorldData->init();
    mUnit14 = new PointPartsHolder(this);
    mUnit14->init();
    mUnit18 = new WorldMapPointMarks(this);
    mUnit18->init();
    mPathFinder = new WorldMapResultPathFinder(this);

    if (flag != 0) {
        setupNode(0, -1);
    }

    finish();
}

void WorldMapResultWorld::prepare() {
    mWorldData->appearAll();
    mUnit14->appearAll();
    mUnit18->appearOpened();
    mLinks->appearAll();
}

void WorldMapResultWorld::finish() {
    mWorldData->killAll();
    mUnit14->killAll();
    mUnit18->killAll();
    mLinks->killAll();
}

// ---------------------------------------------------------------------------------------------------------------
// WorldMapSequence: "シーケンス管理", the state machine of the world map
// ---------------------------------------------------------------------------------------------------------------

class WorldMapSequence;

#define WMS_NERVE_DECL(name)                                                                                                                         \
    class name : public Nerve {                                                                                                                      \
    public:                                                                                                                                          \
        virtual void execute(Spine*) const;                                                                                                          \
        static name sInstance;                                                                                                                       \
    }

namespace NrvWorldMapSequence {
    WMS_NERVE_DECL(WorldMapSequenceStart);
    WMS_NERVE_DECL(WorldMapSequenceWait);
    WMS_NERVE_DECL(WorldMapSequencePointMove);
    WMS_NERVE_DECL(WorldMapSequencePointWarp);
    WMS_NERVE_DECL(WorldMapSequencePointEvent);
    WMS_NERVE_DECL(WorldMapSequenceTicoRouteCreator);
    WMS_NERVE_DECL(WorldMapSequenceWorldWarp);
    WMS_NERVE_DECL(WorldMapSequenceOpenGalaxy);
    WMS_NERVE_DECL(WorldMapSequenceEnterGalaxy);
    WMS_NERVE_DECL(WorldMapSequenceEnterHome);
    WMS_NERVE_DECL(WorldMapSequenceEnterKoopaCastle);
    WMS_NERVE_DECL(WorldMapSequenceWorldNews);
    WMS_NERVE_DECL(WorldMapSequenceGalaxyMapEnter);
    WMS_NERVE_DECL(WorldMapSequenceGalaxyMap);
    WMS_NERVE_DECL(WorldMapSequenceGalaxyMapExit);
    WMS_NERVE_DECL(WorldMapSequenceGreenStarDemo);
    WMS_NERVE_DECL(WorldMapSequenceNoMove);
};  // namespace NrvWorldMapSequence

// The canonical WorldMapSequence definition, included here (after its nerve classes) so the
// Wii object keeps the original definition order (vtable order in .data).
#include "Game/System/WorldMapSequence.hpp"

WorldMapSequence::WorldMapSequence(void* pOwner) : NerveExecutor("シーケンス管理") {
    mOwner = static_cast<WorldMapOwner*>(pOwner);
    mStateWait = nullptr;
    mStatePointEvent = nullptr;
    mStateTicoRoute = nullptr;
    mStateWorldNews = nullptr;
    mFlag = false;
}

void WorldMapSequence::init(bool startAtMap) {
    mOwner->initCameraNodeNames();
    initNerve(&NrvWorldMapSequence::WorldMapSequenceStart::sInstance, 0x14);
    mStateWait = new WorldMapResultStateWait(mOwner);
    mStatePointEvent = new WorldMapResultStatePointEvent(mOwner);
    mStateTicoRoute = new WorldMapResultStateTicoRouteCreate(mOwner);
    mStateWorldNews = new WorldMapResultStateWorldNews(mOwner);
    mStateWorldWarp = new WorldMapResultStateWorldWarp(mOwner);
    mStateGalaxyMap = new WorldMapResultStateGalaxyMap(mOwner);
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStateLeaveHome(mOwner)), &NrvWorldMapSequence::WorldMapSequenceStart::sInstance, "Start");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateWait), &NrvWorldMapSequence::WorldMapSequenceWait::sInstance, "Wait");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStateMove(mOwner)), &NrvWorldMapSequence::WorldMapSequencePointMove::sInstance, "PointMove");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStatePointWarp(mOwner)), &NrvWorldMapSequence::WorldMapSequencePointWarp::sInstance, "PointWarp");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStatePointEvent), &NrvWorldMapSequence::WorldMapSequencePointEvent::sInstance, "PointEvent");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateTicoRoute), &NrvWorldMapSequence::WorldMapSequenceTicoRouteCreator::sInstance, "TicoRouteCreator");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStateOpenGalaxy(mOwner)), &NrvWorldMapSequence::WorldMapSequenceOpenGalaxy::sInstance, "OpenGalaxy");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStateEnterGalaxy(mOwner)), &NrvWorldMapSequence::WorldMapSequenceEnterGalaxy::sInstance, "EnterGalaxy");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStateEnterHome(mOwner)), &NrvWorldMapSequence::WorldMapSequenceEnterHome::sInstance, "EnterHome");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStateKoopaCastle(mOwner)), &NrvWorldMapSequence::WorldMapSequenceEnterKoopaCastle::sInstance, "EnterKoopaCastle");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateWorldNews), &NrvWorldMapSequence::WorldMapSequenceWorldNews::sInstance, "WorldNews");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateWorldWarp), &NrvWorldMapSequence::WorldMapSequenceWorldWarp::sInstance, "WorldWarp");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStateGrandGalaxyOpen(mOwner)), &NrvWorldMapSequence::WorldMapSequenceGalaxyMapEnter::sInstance, "GalaxyMapEnter");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(mStateGalaxyMap), &NrvWorldMapSequence::WorldMapSequenceGalaxyMap::sInstance, "GalaxyMap");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStateGrandGalaxyEnd(mOwner)), &NrvWorldMapSequence::WorldMapSequenceGalaxyMapExit::sInstance, "GalaxyMapExit");
    MR::initActorState(this, static_cast< ActorStateBaseInterface* >(new WorldMapResultStateGreenStarDemo(mOwner)), &NrvWorldMapSequence::WorldMapSequenceGreenStarDemo::sInstance, "GreenStarDemo");
    WorldMapCameraHolder* camera = mOwner->getCamera();
    mOwner->warpRocketToNode(camera->mCurrent);
    camera->mRocket->makeActorAppeared();
    MR::startStarPointerModeSphereSelectorOnReaction(this);
    mOwner->startNodeCamera(-1, 0);

    if (startAtMap) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceGalaxyMap::sInstance);
    }

    if (WorldMapFunction::tryOnGreenStarEventFlag()) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceGreenStarDemo::sInstance);
    }
}

void WorldMapSequence::exeStart() {
    if (MR::updateActorState(this)) {
        if (WorldMapFunction::isGreenStarDemoReady()) {
            mOwner->startDemoNoCinemaCheck();
            setNerve(&NrvWorldMapSequence::WorldMapSequenceNoMove::sInstance);
            return;
        }

        if (!checkWorldNews()) {
            setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
        }
    }
}

void WorldMapSequence::exeWait() {
    if (MR::isFirstStep(this)) {
        mFlag = WorldMapOwner::isStickTilted();
    }

    MR::updateActorState(this);

    if (!updatePoint(mStateWait->isZZZ()) && mStateWait->isXXX()) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceGalaxyMapEnter::sInstance);
    }
}

void WorldMapSequence::exePointMove() {
    if (MR::updateActorState(this)) {
        if (mOwner->tryStartPointEvent2(mStatePointEvent)) {
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointEvent::sInstance);
            return;
        }

        if (mOwner->tryWorldWarpFromNext(mStateWorldWarp)) {
            setNerve(&NrvWorldMapSequence::WorldMapSequenceWorldWarp::sInstance);
            return;
        }

        if (mOwner->tryStickWarp()) {
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointWarp::sInstance);
            return;
        }

        if (mOwner->isHomeNotOpened()) {
            mOwner->tryOpenCurrentGalaxy();
            setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
            return;
        }

        if (mOwner->tryOpenKoopaDemoFlag()) {
            setNerve(&NrvWorldMapSequence::WorldMapSequenceEnterKoopaCastle::sInstance);
            return;
        }

        setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
    }
}

void WorldMapSequence::exePointWarp() {
    if (MR::updateActorState(this)) {
        if (mOwner->tryStartPointEvent2(mStatePointEvent)) {
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointEvent::sInstance);
            return;
        }

        if (mOwner->tryApplyNextNode()) {
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointMove::sInstance);
            return;
        }

        setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
    }
}

void WorldMapSequence::exeGreenStarDemo() {
    if (MR::isFirstStep(this)) {
        AudInstListMgr::requestInstrumentRefresh();
    }

    if (MR::updateActorState(this)) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceWait::sInstance);
    }
}

void WorldMapSequence::exeNoMove() {
    if (MR::isGreaterEqualStep(this, 0x3C)) {
        mOwner->endDemo();
        MR::deactivateDefaultGameLayout();
        setNerve(&NrvWorldMapSequence::WorldMapSequenceGalaxyMapEnter::sInstance);
    }
}

bool WorldMapSequence::isGalaxyMap() const {
    return isNerve(&NrvWorldMapSequence::WorldMapSequenceGalaxyMap::sInstance);
}

bool WorldMapSequence::updatePoint(bool canAct) {
    LiveActor* pActor = mOwner->getCamera()->mRocket;

    if (mOwner->tryWorldWarpFromStick(mStateWorldWarp)) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceWorldWarp::sInstance);
        return true;
    }

    s32 point = -1;

    if (!mOwner->isZoomButtonPointing()) {
        point = mOwner->updatePointing();
    }

    if (point > -1) {
        mStateWait->setTarget(point);
    } else {
        mStateWait->clearTarget();
    }

    bool moved = mOwner->tryWarpToNode(point);
    bool warped = mOwner->tryStickMove();

    if (moved) {
        MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
    }

    if (moved || warped) {
        setNerve(&NrvWorldMapSequence::WorldMapSequencePointMove::sInstance);
        return true;
    }

    if (mOwner->isZoomButtonPointing()) {
        return false;
    }

    if (canAct) {
        if (mOwner->tryStartStarCheckEvent(mStatePointEvent, point)) {
            MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
            setNerve(&NrvWorldMapSequence::WorldMapSequencePointEvent::sInstance);
            return true;
        }

        if (mOwner->tryEnterGalaxy(point)) {
            MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
            setNerve(&NrvWorldMapSequence::WorldMapSequenceEnterGalaxy::sInstance);
            return true;
        }

        if (mOwner->tryEnterHome(point)) {
            MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
            setNerve(&NrvWorldMapSequence::WorldMapSequenceEnterHome::sInstance);
            return true;
        }

        if (mOwner->tryStartTicoRoute(mStateTicoRoute, point)) {
            MR::startActionSound(pActor, "PushAButton", -1, -1, -1);
            setNerve(&NrvWorldMapSequence::WorldMapSequenceTicoRouteCreator::sInstance);
            return true;
        }
    }

    if (mOwner->tryWarpByA(point)) {
        setNerve(&NrvWorldMapSequence::WorldMapSequencePointWarp::sInstance);
        MR::startCSSound("CS_CLICK_WMAP", nullptr, 0, 0, 1.0f);
        return true;
    }

    if (!moved && point > 0 && WorldMapOwner::isPressedA()) {
        MR::startActionSound(pActor, "NoMove", -1, -1, -1);
    }

    bool blocked = mOwner->findStickTarget(0.1f) >= 0;

    if (!warped && !mFlag && WorldMapOwner::isStickTilted() && !blocked) {
        MR::startActionSound(pActor, "NoMove", -1, -1, -1);
    }

    mFlag = WorldMapOwner::isStickTilted();
    return false;
}

bool WorldMapSequence::checkWorldNews() {
    if (mStateWorldNews->checkStep()) {
        setNerve(&NrvWorldMapSequence::WorldMapSequenceWorldNews::sInstance);
        return true;
    }

    return false;
}

WorldMapSequence::~WorldMapSequence() {
}

namespace NrvWorldMapSequence {
    WorldMapSequenceStart WorldMapSequenceStart::sInstance;
    WorldMapSequenceWait WorldMapSequenceWait::sInstance;
    WorldMapSequencePointMove WorldMapSequencePointMove::sInstance;
    WorldMapSequencePointWarp WorldMapSequencePointWarp::sInstance;
    WorldMapSequencePointEvent WorldMapSequencePointEvent::sInstance;
    WorldMapSequenceTicoRouteCreator WorldMapSequenceTicoRouteCreator::sInstance;
    WorldMapSequenceWorldWarp WorldMapSequenceWorldWarp::sInstance;
    WorldMapSequenceOpenGalaxy WorldMapSequenceOpenGalaxy::sInstance;
    WorldMapSequenceEnterGalaxy WorldMapSequenceEnterGalaxy::sInstance;
    WorldMapSequenceEnterHome WorldMapSequenceEnterHome::sInstance;
    WorldMapSequenceEnterKoopaCastle WorldMapSequenceEnterKoopaCastle::sInstance;
    WorldMapSequenceWorldNews WorldMapSequenceWorldNews::sInstance;
    WorldMapSequenceGalaxyMapEnter WorldMapSequenceGalaxyMapEnter::sInstance;
    WorldMapSequenceGalaxyMap WorldMapSequenceGalaxyMap::sInstance;
    WorldMapSequenceGalaxyMapExit WorldMapSequenceGalaxyMapExit::sInstance;
    WorldMapSequenceGreenStarDemo WorldMapSequenceGreenStarDemo::sInstance;
    WorldMapSequenceNoMove WorldMapSequenceNoMove::sInstance;

    void WorldMapSequenceNoMove::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exeNoMove();
    }

    void WorldMapSequenceGreenStarDemo::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exeGreenStarDemo();
    }

    void WorldMapSequenceGalaxyMapExit::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            if (!pSeq->checkWorldNews()) {
                pSeq->setNerve(&WorldMapSequenceWait::sInstance);
            }
        }
    }

    void WorldMapSequenceGalaxyMap::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceGalaxyMapExit::sInstance);
        }
    }

    void WorldMapSequenceGalaxyMapEnter::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceGalaxyMap::sInstance);
        }
    }

    void WorldMapSequenceWorldNews::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceEnterKoopaCastle::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceEnterHome::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceEnterGalaxy::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceOpenGalaxy::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequenceWorldWarp::execute(Spine* pSpine) const {
        MR::updateActorState(reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor));
    }

    void WorldMapSequenceTicoRouteCreator::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequencePointEvent::execute(Spine* pSpine) const {
        WorldMapSequence* pSeq = reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor);

        if (MR::updateActorState(pSeq)) {
            pSeq->setNerve(&WorldMapSequenceWait::sInstance);
        }
    }

    void WorldMapSequencePointWarp::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exePointWarp();
    }

    void WorldMapSequencePointMove::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exePointMove();
    }

    void WorldMapSequenceWait::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exeWait();
    }

    void WorldMapSequenceStart::execute(Spine* pSpine) const {
        reinterpret_cast< WorldMapSequence* >(pSpine->mExecutor)->exeStart();
    }
};  // namespace NrvWorldMapSequence

const f32 sCameraWidthScale = 0.75f;
const f32 sCameraScreenWidth = 608.0f;
const f32 sRouteTailRate = 0.12f;
