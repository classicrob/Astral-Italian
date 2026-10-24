#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/System/MiniRouteLine.hpp"
#include "Game/System/WorldMapCameraHolder.hpp"
#include "Game/System/WorldMapPointParts.hpp"
#include "Game/System/WorldMapResultStateWorldWarp.hpp"
#include "Game/System/WorldMapPointMarks.hpp"
#include "Game/System/MiniPartsActors.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/MiniStarCheckPoint.hpp"
#include "Game/System/MiniTicoMaster.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/Util.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/VectorUtil.hpp"
#include <cmath>

// ==== TU E (0x804ECBC0): WorldMapResultRoute members, PointPartsHolder ====

WorldMapResultRoute::WorldMapResultRoute() {
    mCount = 0;
}

void WorldMapResultRoute::addEdge(s32 from, s32 to) {
    mEdges[mCount].mFrom = from;
    mEdges[mCount].mTo = to;
    mEdges[mCount].mStep = -1;
    mEdges[mCount].mReverse = 0;
    mCount++;
}

void WorldMapResultRoute::subtract(WorldMapResultRoute* pOther) {
    for (s32 i = 0; i < pOther->mCount; i++) {
        s32 from = pOther->mEdges[i].mFrom;
        s32 to = pOther->mEdges[i].mTo;
        for (s32 j = 0; j < mCount; j++) {
            if (from == mEdges[j].mFrom && to == mEdges[j].mTo) {
                mCount--;
                mEdges[j] = mEdges[mCount];
            }
        }
    }
}

bool isWorldWarpPoint(const char* pName) NO_INLINE;
bool isStarRoadWarpPoint(const char* pName) NO_INLINE;
bool isStarCheckPointPart(const char* pName) NO_INLINE;
bool isStarPieceMinePart(const char* pName) NO_INLINE;
bool isEarthenPipePart(const char* pName) NO_INLINE;
bool isTicoRouteCreatorPart(const char* pName) NO_INLINE;

bool isWorldWarpPoint(const char* pName) {
    return MR::isEqualString(pName, "WorldWarpPoint");
}

bool isStarRoadWarpPoint(const char* pName) {
    return MR::isEqualString(pName, "StarRoadWarpPoint");
}

bool isStarCheckPointPart(const char* pName) {
    return MR::isEqualString(pName, "StarCheckPoint");
}

bool isStarPieceMinePart(const char* pName) {
    return MR::isEqualString(pName, "StarPieceMine");
}

bool isEarthenPipePart(const char* pName) {
    return MR::isEqualString(pName, "EarthenPipe");
}

bool isTicoRouteCreatorPart(const char* pName) {
    return MR::isEqualString(pName, "TicoRouteCreator");
}



PointPart::PointPart() {
    mType = nullptr;
    mPointIndex = 0;
    mPartsIndex = 0;
    mParam0 = -1;
    mParam1 = -1;
    mParam2 = -1;
    mActor = nullptr;
}

static const char* sPointPartsCsv = "PointParts";
static TVec3f sPartsOffset(0.0f, 300.0f, 300.0f);


PointPartsHolder::PointPartsHolder(WorldMapResultWorld* pWorld) : mWorld(pWorld), mParts(nullptr), mNum(0), mUnkC(0) {}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void PointPartsHolder::init() {
    PointPart* pPart;
    JMapInfo* pCsv = mWorld->createParser(sPointPartsCsv);
    if (pCsv != nullptr) {
        mUnkC = WorldMapFunction::getGrandStarNum();
        s32 worldId = mWorld->getWorldId();
        s32 num = MR::getCsvDataElementNum(pCsv);
        mNum = num;
        mParts = new PointPart[num];
        for (s32 i = 0; i < mNum; i++) {
            pPart = &mParts[i];
            MR::getCsvDataStr(&pPart->mType, pCsv, "PartsTypeName", i);
            MR::getCsvDataS32(&pPart->mPointIndex, pCsv, "PointIndex", i);
            MR::getCsvDataS32(&pPart->mPartsIndex, pCsv, "PartsIndex", i);
            MR::getCsvDataS32(&pPart->mParam0, pCsv, "Param00", i);
            MR::getCsvDataS32(&pPart->mParam1, pCsv, "Param01", i);
            if (MR::hasCsvItem(pCsv, "Param02")) {
                MR::getCsvDataS32(&pPart->mParam2, pCsv, "Param02", i);
            }
            if (isWorldWarpPoint(pPart->mType)) {
                pPart->mActor = new MiniWorldWarpPoint(mWorld, pPart->mPointIndex, worldId < pPart->mParam0, pPart->mParam0);
                MR::setPosition(pPart->mActor, mWorld->getNode(pPart->mPointIndex)->mPos);
                pPart->mActor->mPosition.add(sPartsOffset);
                pPart->mActor->initWithoutIter();
            } else if (isStarRoadWarpPoint(pPart->mType)) {
                pPart->mActor = new MiniStarWorldPoint(mWorld, pPart->mPointIndex, pPart->mParam0);
                MR::setPosition(pPart->mActor, mWorld->getNode(pPart->mPointIndex)->mPos);
                pPart->mActor->initWithoutIter();
            } else if (isStarCheckPointPart(pPart->mType)) {
                pPart->mActor = new MiniStarCheckPoint(pPart->mType, pPart->mParam0, pPart->mPartsIndex, pPart->mParam1, worldId);
                MR::setPosition(pPart->mActor, mWorld->getNode(pPart->mPointIndex)->mPos);
                pPart->mActor->initWithoutIter();
            } else if (isEarthenPipePart(pPart->mType)) {
                pPart->mActor = new MiniEarthenPipe();
                MR::setPosition(pPart->mActor, mWorld->getNode(pPart->mPointIndex)->mPos);
                pPart->mActor->initWithoutIter();
            } else if (isTicoRouteCreatorPart(pPart->mType)) {
                pPart->mActor = new MiniTicoMaster(mWorld, pPart->mPartsIndex, &pPart->mParam0, pPart->mPointIndex, pPart->mParam1);
                MR::setPosition(pPart->mActor, mWorld->getNode(pPart->mPointIndex)->mPos);
                pPart->mActor->initWithoutIter();
            } else {
                pPart->mActor = new MiniStarPieceMine(worldId, pPart->mPointIndex);
                MR::setPosition(pPart->mActor, mWorld->getNode(pPart->mPointIndex)->mPos);
                pPart->mActor->initWithoutIter();
            }
            MR::setClippingFarMax(pPart->mActor);
            pPart->mActor->makeActorDead();
            if (isStarRoadWarpPoint(pPart->mType) == false) {
                MR::invalidateClipping(pPart->mActor);
            }
        }
    }
}

bool PointPartsHolder::getWorldWarp(s32 pointIndex, WorldMapResultStateWorldWarp* pOut) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    if (!isWorldWarpPoint(pPart->mType)) {
        return false;
    }
    const s32 param1 = pPart->mParam1;
    const s32 param0 = pPart->mParam0;
    pOut->mInt10 = param0;
    pOut->mInt14 = param1;
    pOut->mInt18 = pointIndex;
    return true;
}

bool PointPartsHolder::getStarRoadWarp(s32 pointIndex, PartsWarpInfo* pOut) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    if (isStarRoadWarpPoint(pPart->mType) == false) {
        return false;
    }
    const s32 param1 = pPart->mParam1;
    const s32 param0 = pPart->mParam0;
    pOut->mParam0 = param0;
    pOut->mParam1 = param1;
    return true;
}

s32 PointPartsHolder::getWarpTarget(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return -1;
    }
    if (!isWorldWarpPoint(pPart->mType) && !isStarRoadWarpPoint(pPart->mType)) {
        return -1;
    }
    return pPart->mParam0;
}

s32 PointPartsHolder::getPipeTarget(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return -1;
    }
    if (isEarthenPipePart(pPart->mType)) {
        return pPart->mParam0;
    }
    return -1;
}

s32 PointPartsHolder::getPipeOtherEnd(s32 pointIndex) {
    PointPart* pPart;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        if (isEarthenPipePart(pPart->mType)) {
            if (pPart->mPointIndex == pointIndex) {
                return pPart->mParam0;
            }
            if (pPart->mParam0 == pointIndex) {
                return pPart->mPointIndex;
            }
        }
    }
    return -1;
}

LiveActor* PointPartsHolder::getTicoCreator(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart != nullptr && isTicoRouteCreatorPart(pPart->mType)) {
        return pPart->mActor;
    }
    return nullptr;
}

LiveActor* PointPartsHolder::getWorldWarpActor(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart != nullptr && isWorldWarpPoint(pPart->mType)) {
        return pPart->mActor;
    }
    return nullptr;
}

bool PointPartsHolder::isTicoCreatorNotFull(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart != nullptr && isTicoRouteCreatorPart(pPart->mType)) {
        return static_cast<MiniTicoMaster*>(pPart->mActor)->isFull() == false;
    }
    return false;
}

LiveActor* PointPartsHolder::getActor(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart != nullptr) {
        return pPart->mActor;
    }
    return nullptr;
}

bool PointPartsHolder::isPipeTarget(s32 pointIndex, s32 target) {
    return getPipeTarget(pointIndex) == target;
}

bool PointPartsHolder::isNotStarPieceMine(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    return isStarPieceMinePart(pPart->mType) == false;
}

bool PointPartsHolder::isActorAlive(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    return MR::isDead(pPart->mActor) == false;
}

void PointPartsHolder::callStarCheckPoint(s32 pointIndex) {
    for (s32 i = 0; i < mNum; i++) {
        PointPart* pPart = &mParts[i];
        if (pPart->mPointIndex == pointIndex) {
            static_cast<MiniStarCheckPoint*>(pPart->mActor)->startOpen();
            return;
        }
    }
}

s32 PointPartsHolder::getActorField94(s32 pointIndex) {
#ifdef GALAXY_NATIVE
    return static_cast<MiniStarCheckPoint*>(find(pointIndex)->mActor)->mStarNum;
#else
    return *reinterpret_cast<s32*>(reinterpret_cast<u8*>(find(pointIndex)->mActor) + 0x94);
#endif
}

bool PointPartsHolder::isStarCheckPointReady(s32 pointIndex) {
    PointPart* pPart;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        if (pPart->mPointIndex == pointIndex) {
            bool result;
            if (isStarCheckPointPart(pPart->mType) && static_cast<MiniStarCheckPoint*>(pPart->mActor)->isStandby()) {
                result = true;
            } else {
                result = false;
            }
            if (result) {
                return true;
            }
        }
    }
    return false;
}

bool PointPartsHolder::isStarCheckPointPending(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    if (isStarCheckPointPart(pPart->mType) && !static_cast<MiniStarCheckPoint*>(pPart->mActor)->isDone()) {
        return true;
    }
    return false;
}

bool PointPartsHolder::isAnyStarCheckPointPending() {
    PointPart* pPart;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        if (isStarCheckPointPart(pPart->mType)) {
            MiniStarCheckPoint* pActor = static_cast<MiniStarCheckPoint*>(pPart->mActor);
            if (!pActor->isDone() && !pActor->isStandby()) {
                return true;
            }
        }
    }
    return false;
}

bool PointPartsHolder::isWarpOpen(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    if (isStarRoadWarpPoint(pPart->mType) == false) {
        return false;
    }
    bool result = mUnkC <= pPart->mParam0;
    return result;
}

bool PointPartsHolder::isEarthenPipe(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    return isEarthenPipePart(pPart->mType);
}

bool PointPartsHolder::isWorldWarp(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    return isWorldWarpPoint(pPart->mType);
}

bool PointPartsHolder::tryTalkTico(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart != nullptr && isTicoRouteCreatorPart(pPart->mType)) {
        LiveActor* pActor = pPart->mActor;
        if (!MR::isDead(pActor)) {
            static_cast<MiniTicoMaster*>(pActor)->tryTalk();
            return true;
        }
    }
    return false;
}

bool PointPartsHolder::tryTalkStarCheckPoint(s32 pointIndex) {
    PointPart* pPart = find(pointIndex);
    if (pPart != nullptr && isStarCheckPointPart(pPart->mType)) {
        LiveActor* pActor = pPart->mActor;
        if (!MR::isDead(pActor)) {
            static_cast<MiniStarCheckPoint*>(pActor)->talk();
            return true;
        }
    }
    return false;
}

void PointPartsHolder::appearAll() {
    for (s32 i = 0; i < mNum; i++) {
        mParts[i].mActor->appear();
    }
}

void PointPartsHolder::killAll() {
    for (s32 i = 0; i < mNum; i++) {
        mParts[i].mActor->kill();
    }
}

void PointPartsHolder::makeActorAppearedAll() {
    for (s32 i = 0; i < mNum; i++) {
        mParts[i].mActor->makeActorAppeared();
    }
}

void PointPartsHolder::makeActorDeadAll() {
    for (s32 i = 0; i < mNum; i++) {
        mParts[i].mActor->makeActorDead();
    }
}

void PointPartsHolder::resetAll() {
    makeActorDeadAll();
    makeActorAppearedAll();
}

void PointPartsHolder::resetStarCheckPoints() {
    PointPart* pPart;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        if (isStarCheckPointPart(pPart->mType)) {
            static_cast<MiniStarCheckPoint*>(pPart->mActor)->resetOpen();
        }
    }
}

PointPart* PointPartsHolder::find(s32 pointIndex) {
    for (s32 i = 0; i < mNum; i++) {
        PointPart* pPart = &mParts[i];
        if (pPart->mPointIndex == pointIndex) {
            return pPart;
        }
    }
    return nullptr;
}

s32 PointPartsHolder::findActivePoint() {
    PointPart* pPart;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        if (isTicoRouteCreatorPart(pPart->mType)) {
            if (static_cast<MiniTicoMaster*>(pPart->mActor)->isPointing()) {
                return pPart->mPointIndex;
            }
        } else if (isStarCheckPointPart(pPart->mType)) {
            if (static_cast<MiniStarCheckPoint*>(pPart->mActor)->isOpening()) {
                return pPart->mPointIndex;
            }
        } else if (isEarthenPipePart(pPart->mType) && static_cast<MiniEarthenPipe*>(pPart->mActor)->isUsable()) {
            return pPart->mPointIndex;
        }
    }
    return -1;
}

bool PointPartsHolder::tryTico72C0(s32 pointIndex) {
    PointPart* pPart;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        if (pPart->mPointIndex == pointIndex && isTicoRouteCreatorPart(pPart->mType) && static_cast<MiniTicoMaster*>(pPart->mActor)->tryPointTalk()) {
            return true;
        }
    }
    return false;
}

bool PointPartsHolder::tryStarCheck6000(s32 pointIndex) {
    PointPart* pPart;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        if (pPart->mPointIndex == pointIndex && isStarCheckPointPart(pPart->mType) && static_cast<MiniStarCheckPoint*>(pPart->mActor)->isReadyToOpen()) {
            return true;
        }
    }
    return false;
}

bool PointPartsHolder::tryPipe3420(s32 pointIndex) {
    PointPart* pPart;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        if (pPart->mPointIndex == pointIndex && isEarthenPipePart(pPart->mType) && static_cast<MiniEarthenPipe*>(pPart->mActor)->isUsableStrict()) {
            return true;
        }
    }
    return false;
}

bool PointPartsHolder::updateAll() {
    PointPart* pPart;
    WorldMapResultPathFinder* pFinder = mWorld->mPathFinder;
    bool result = false;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        s32 distance = pFinder->getDistance(pPart->mPointIndex);
        if (isStarCheckPointPart(pPart->mType)) {
            result = (result | static_cast<MiniStarCheckPoint*>(pPart->mActor)->update(distance)) != 0;
        } else if (isWorldWarpPoint(pPart->mType)) {
            result = (result | static_cast<MiniWorldWarpPoint*>(pPart->mActor)->updateDistance(distance)) != 0;
        }
    }
    return result;
}

void PointPartsHolder::setEffectDrawOrder(s32 order) {
    PointPart* pPart;
    for (s32 i = 0; i < mNum; i++) {
        pPart = &mParts[i];
        if (isWorldWarpPoint(pPart->mType)) {
            static_cast<MiniWorldWarpPoint*>(pPart->mActor)->setEffectDrawOrder(order);
        } else if (isStarCheckPointPart(pPart->mType)) {
            static_cast<MiniStarCheckPoint*>(pPart->mActor)->setEffectDrawOrder(order);
        } else if (isStarPieceMinePart(pPart->mType)) {
            static_cast<MiniStarPieceMine*>(pPart->mActor)->setEffectDrawOrder(order);
        } else if (isTicoRouteCreatorPart(pPart->mType)) {
            static_cast<MiniTicoMaster*>(pPart->mActor)->setEffectDrawOrder(order);
        }
    }
}
