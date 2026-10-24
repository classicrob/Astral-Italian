#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/System/MiniRouteLine.hpp"
#include "Game/System/WorldMapCameraHolder.hpp"
#include "Game/System/WorldMapPointParts.hpp"
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

// ==== TU F (0x804EE100): WorldMapResultMapData, WorldMapPointMarks ====

WorldMapResultNode::WorldMapResultNode() : mPos(0.0f) {
    mLayer = 0;
    mIsHidden = 0;
    mFlag11 = 0;
    mIsValid = 0;
}

static const char* sPointPosCsv = "PointPos";

WorldMapResultMapData::WorldMapResultMapData(WorldMapResultWorld* pWorld) {
    mWorld = pWorld;
    mNum = 0;
    mNodes = nullptr;
    load();
}

void WorldMapResultMapData::getLayerRange(s32* pMin, s32* pMax) const {
    *pMin = 0;
    *pMax = 0;
    for (s32 i = 0; i < mNum; i++) {
        WorldMapResultNode* pNode = &mNodes[i];
        if (pNode->mLayer < *pMin) {
            *pMin = pNode->mLayer;
        }
        if (mNodes[i].mLayer > *pMax) {
            *pMax = mNodes[i].mLayer;
        }
    }
}

void WorldMapResultMapData::load() {
    s32 num = 0;
    JMapInfo* pCsv = mWorld->createParser(sPointPosCsv);
    if (pCsv != nullptr) {
        num = MR::getCsvDataElementNum(pCsv);
    }
    mNum = num;
    mNodes = new WorldMapResultNode[num];
    for (s32 i = 0; i < mNum; i++) {
        const char* pValue = nullptr;
        WorldMapResultNode* pNode = &mNodes[i];
        MR::getCsvDataStr(&pValue, pCsv, "Valid", i);
        if (*pValue == 'o') {
            pNode->mIsValid = 1;
        }
        MR::getCsvDataStr(&pValue, pCsv, "SubPoint", i);
        if (*pValue == 'o') {
            pNode->mIsHidden = 1;
        }
        if (MR::hasCsvItem(pCsv, "ColorChange")) {
            MR::getCsvDataStr(&pValue, pCsv, "ColorChange", i);
            if (*pValue == 'o') {
                pNode->mFlag11 = 1;
            }
        }
        MR::getCsvDataVec(&pNode->mPos, pCsv, "PointPos", i);
        MR::getCsvDataS32(&pNode->mLayer, pCsv, "LayerNo", i);
    }
}



WorldMapPointMarks::WorldMapPointMarks(WorldMapResultWorld* pWorld) : mWorld(pWorld), mNum(0), mMarks(nullptr) {}

void WorldMapPointMarks::init() {
    s32 count;
    WorldMapResultWorld* pWorld = mWorld;
    count = 0;
    for (s32 i = 0; i < pWorld->getNodeNum(); i++) {
        if (pWorld->isNodeValid(i) && !pWorld->isNodeHidden(i)) {
            count++;
        }
    }
    mNum = count;
    mMarks = new MiniRoutePoint*[count];
    for (s32 i = 0; i < mNum; i++) {
        MiniRoutePoint* pMark = new MiniRoutePoint();
        pMark->initWithoutIter();
        MR::startAction(pMark, "Close");
        mMarks[i] = pMark;
        MR::invalidateClipping(pMark);
    }
    setupMarks();
}

s32 WorldMapPointMarks::findReachablePoint() {
    MiniRoutePoint* pMark;
    for (s32 i = 0; i < mNum; i++) {
        pMark = mMarks[i];
        if (mWorld->isPathReachable(pMark->mPointIndex) && pMark->isOpening()) {
            return pMark->mPointIndex;
        }
    }
    return -1;
}

bool WorldMapPointMarks::tryUseMark(s32 pointIndex) {
    MiniRoutePoint* pMark;
    for (s32 i = 0; i < mNum; i++) {
        pMark = mMarks[i];
        if (pointIndex == pMark->mPointIndex && mWorld->isPathReachable(pMark->mPointIndex) && pMark->tryOpen()) {
            return true;
        }
    }
    return false;
}

void WorldMapPointMarks::emitGoEffect(s32 pointIndex) {
    for (s32 i = 0; i < mNum; i++) {
        MiniRoutePoint* pMark = mMarks[i];
        if (pointIndex == pMark->mPointIndex) {
            MR::emitEffect(pMark, "Go");
        }
    }
}

MiniRoutePoint* WorldMapPointMarks::find(s32 pointIndex) {
    for (s32 i = 0; i < mNum; i++) {
        MiniRoutePoint* pMark = mMarks[i];
        if (pointIndex == pMark->mPointIndex) {
            return pMark;
        }
    }
    return nullptr;
}

void WorldMapPointMarks::appearOpened() {
    MiniRoutePoint* pMark;
    for (s32 i = 0; i < mNum; i++) {
        pMark = mMarks[i];
        if (mWorld->hasOpenLine(pMark->mPointIndex)) {
            pMark->makeActorAppeared();
        }
    }
}

void WorldMapPointMarks::killAll() {
    for (s32 i = 0; i < mNum; i++) {
        mMarks[i]->makeActorDead();
    }
}

void WorldMapPointMarks::updateReachable() {
    MiniRoutePoint* pMark;
    for (s32 i = 0; i < mNum; i++) {
        pMark = mMarks[i];
        if (mWorld->hasOpenLine(pMark->mPointIndex) && mWorld->isPathReachable(pMark->mPointIndex) && !pMark->isStarting()) {
            pMark->start();
        }
    }
}

void WorldMapPointMarks::updateAll() {
    MiniRoutePoint* pMark;
    for (s32 i = 0; i < mNum; i++) {
        pMark = mMarks[i];
        pMark->mNotReady = mWorld->isNodeNotReady(pMark->mPointIndex);
        if (!mWorld->hasOpenLine(pMark->mPointIndex)) {
            pMark->makeActorDead();
        } else {
            if (MR::isDead(pMark)) {
                pMark->makeActorAppeared();
            }
            if (mWorld->isPathReachable(pMark->mPointIndex)) {
                pMark->setReachable(true);
            } else {
                pMark->setReachable(false);
            }
        }
    }
}

void WorldMapPointMarks::setupMarks() {
    s32 index = 0;
    MiniRoutePoint* pMark;
    for (s32 i = 0; i < mWorld->getNodeNum(); i++) {
        if (mWorld->isNodeValid(i) && !mWorld->isNodeHidden(i)) {
            pMark = mMarks[index];
            pMark->setPosition(mWorld->getNode(i)->mPos);
            pMark->mPointIndex = i;
            pMark->makeActorDead();
            pMark->mNotReady = mWorld->isNodeNotReady(i);
            pMark->mIsTicoBuild = mWorld->isNodeFlag11(i);
            index++;
        }
    }
}
