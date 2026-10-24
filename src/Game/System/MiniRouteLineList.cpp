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

// ==== TU D (0x804EC230): MiniRouteLineList ====
// ---------------------------------------------------------------------------------------------------------------
// MiniRouteLineList: the route lines of one world (WorldMapResultWorld::mLinks)
// ---------------------------------------------------------------------------------------------------------------

static const char* sPointLinkCsv = "PointLink";


MiniRouteLineList::MiniRouteLineList(WorldMapResultWorld* pWorld) : mWorld(pWorld), mNum(0), mLines(nullptr) {}

void MiniRouteLineList::init() {
    load(mWorld->getWorldId());
    killAll();
}

void MiniRouteLineList::appearAll() {
    MiniRouteLine* pLine;
    for (s32 i = 0; i < mNum; i++) {
        pLine = mLines[i];
        if (MR::isDead(pLine)) {
            pLine->appear();
        }
    }
}

void MiniRouteLineList::killAll() {
    MiniRouteLine* pLine;
    for (s32 i = 0; i < mNum; i++) {
        pLine = mLines[i];
        if (!MR::isDead(pLine)) {
            pLine->kill();
        }
    }
}

void MiniRouteLineList::setEffectDrawOrder(s32 order) {
    for (s32 i = 0; i < mNum; i++) {
        mLines[i]->setEffectDrawOrder(order);
    }
}

bool MiniRouteLineList::isLinked(s32 a, s32 b, bool strict) {
    for (s32 i = 0; i < mNum; i++) {
        if (mLines[i]->isBetween(a, b, strict)) {
            return true;
        }
    }
    return false;
}

bool MiniRouteLineList::isDirected(s32 a, s32 b) {
    MiniRouteLine* pLine = findLine(a, b);
    if (pLine == nullptr) {
        return false;
    }
    return pLine->isDirected(a, b);
}

bool MiniRouteLineList::openFrom(s32 node) {
    if (!mWorld->isNodeOpened(node)) {
        return false;
    }
    MiniRouteLine* pLine;
    bool result = false;
    for (s32 i = 0; i < mNum; i++) {
        pLine = mLines[i];
        s32 other = -1;
        if (pLine->getNextNode(node, &other)) {
            result = pLine->open();
            if (mWorld->isNodeOpened(other)) {
                openFrom(other);
            }
            s32 linked = mWorld->callU14_310(other);
            if (linked >= 0 && mWorld->isNodeOpened(linked)) {
                openFrom(linked);
            }
        }
    }
    return result;
}

bool MiniRouteLineList::hasOpenLine(s32 node) {
    f32 one = 1.0f;
    MiniRouteLine* pLine;
    for (s32 i = 0; i < mNum; i++) {
        pLine = mLines[i];
        s32 other = -1;
        if (pLine->getOther(node, &other)) {
            if (!pLine->mHidden || ((pLine->mAlpha < one) == false && !pLine->isLocked() && pLine->mOpened)) {
                return true;
            }
        }
    }
    return false;
}

bool MiniRouteLineList::hasOpenLineNoAlpha(s32 node) {
    MiniRouteLine* pLine;
    for (s32 i = 0; i < mNum; i++) {
        pLine = mLines[i];
        s32 other = -1;
        if (pLine->getOther(node, &other)) {
            if (!pLine->mHidden || (!pLine->isLocked() && pLine->mOpened)) {
                return true;
            }
        }
    }
    return false;
}

bool MiniRouteLineList::hasLineFlag(s32 a, s32 b) {
    MiniRouteLine* pLine = findLine(a, b);
    if (pLine == nullptr) {
        return false;
    }
    return pLine->hasFlag();
}

void MiniRouteLineList::setLineAlpha(s32 a, s32 b, f32 alpha) {
    MiniRouteLine* pLine = findLine(a, b);
    if (pLine != nullptr) {
        pLine->setAlpha(pLine->isDirected(a, b), alpha);
    }
}

void MiniRouteLineList::emitLineEffect(s32 a, s32 b) {
    MiniRouteLine* pLine = findLine(a, b);
    if (pLine != nullptr) {
        pLine->emitProgressEffect();
    }
}

void MiniRouteLineList::deleteLineEffect(s32 a, s32 b) {
    MiniRouteLine* pLine = findLine(a, b);
    if (pLine != nullptr) {
        pLine->deleteProgressEffect();
    }
}

void MiniRouteLineList::setLineEffectTico(s32 a, s32 b) {
    MiniRouteLine* pLine = findLine(a, b);
    if (pLine != nullptr) {
        pLine->mEffectName = "ProgressTico";
    }
}

void MiniRouteLineList::setLineHidden(s32 a, s32 b) {
    MiniRouteLine* pLine = findLine(a, b);
    if (pLine != nullptr) {
        pLine->mHidden = true;
    }
}

void MiniRouteLineList::load(s32 worldId) {
    s32 num = 0;
    JMapInfo* pCsv = mWorld->createParser(sPointLinkCsv);
    if (pCsv != nullptr) {
        num = MR::getCsvDataElementNum(pCsv);
    }
    mNum = num;
    mLines = new MiniRouteLine*[num];
    if (mNum > 0) {
        for (s32 i = 0; i < mNum; i++) {
            MiniRouteLine* pLine = new MiniRouteLine("ポイントリンク", mWorld);
            s32 pointA;
            s32 pointB;
            s32 closeScenario;
            const char* pCloseStage;
            const char* pSubRoute;
            const char* pColorChange = "";
            const char* pFlag = "";
            MR::getCsvDataS32(&pointA, pCsv, "PointIndexA", i);
            MR::getCsvDataS32(&pointB, pCsv, "PointIndexB", i);
            MR::getCsvDataStrOrNULL(&pCloseStage, pCsv, "CloseStageName", i);
            MR::getCsvDataS32(&closeScenario, pCsv, "CloseStageScenarioNo", i);
            if (MR::hasCsvItem(pCsv, "CloseGameFlag")) {
                MR::getCsvDataStr(&pFlag, pCsv, "CloseGameFlag", i);
            }
            if (MR::hasCsvItem(pCsv, "IsColorChange")) {
                MR::getCsvDataStr(&pColorChange, pCsv, "IsColorChange", i);
            }
            MR::getCsvDataStr(&pSubRoute, pCsv, "IsSubRoute", i);
            pLine->mFrom = pointA;
            pLine->mTo = pointB;
            WorldMapResultNode* pNodeB = mWorld->getNode(pointB);
            WorldMapResultNode* pNodeA = mWorld->getNode(pointA);
            pLine->setup(pNodeA->mPos, pNodeB->mPos);
            pLine->setCondition(pCloseStage, closeScenario, pFlag);
            if (*pSubRoute == 'o') {
                pLine->mHidden = true;
                pLine->mUnkA4 = true;
            }
            pLine->initWithoutIter();
            if (*pColorChange == 'o') {
                pLine->setColorChange(true);
            }
            mLines[i] = pLine;
        }
    }
}

MiniRouteLine* MiniRouteLineList::findLine(s32 a, s32 b) {
    MiniRouteLine* pLine;
    for (s32 i = 0; i < mNum; i++) {
        pLine = mLines[i];
        if (pLine->isBetweenNodes(a, b)) {
            return pLine;
        }
    }
    return nullptr;
}
