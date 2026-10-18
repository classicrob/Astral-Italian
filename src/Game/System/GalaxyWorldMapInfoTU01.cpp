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

// ==== TU A (0x804EA540): GalaxyWorldMapInfoHolder, WorldMapLayouts, MiniPartsModel, MiniGalaxyShadow ====
static const char* sGalaxyListName = "Galaxy";

// ---------------------------------------------------------------------------------------------------------------
// GalaxyWorldMapInfoHolder: the galaxy markers of one world (WorldMapResultWorld::mWorldData)
// ---------------------------------------------------------------------------------------------------------------


GalaxyWorldMapInfoHolder::GalaxyWorldMapInfoHolder(WorldMapResultWorld* pWorld) : mWorld(pWorld), mNum(0), mInfos(nullptr) {}

void GalaxyWorldMapInfoHolder::init() {
    s32 num = 0;
    JMapInfo* pCsv = mWorld->createParser(sGalaxyListName);
    if (pCsv != nullptr) {
        num = MR::getCsvDataElementNum(pCsv);
    }
    mNum = num;
    mInfos = new GalaxyWorldMapInfo*[num];
    f32 one = 1.0f;
    for (s32 i = 0; i < mNum; i++) {
        const char* pName = nullptr;
        const char* pModel = "MiniFlowerHighlandGalaxy";
        const char* pType = "Galaxy";
        s32 pointIndex = -1;
        MR::getCsvDataStrOrNULL(&pName, pCsv, "StageName", i);
        MR::getCsvDataStrOrNULL(&pModel, pCsv, "MiniatureName", i);
        MR::getCsvDataS32(&pointIndex, pCsv, "PointPosIndex", i);
        MR::getCsvDataStr(&pType, pCsv, "StageType", i);

        GalaxyWorldMapInfo* pInfo = new GalaxyWorldMapInfo(mWorld, pName, pModel, pType, pointIndex);
        pInfo->initWithoutIter();
        pInfo->makeActorDead();

        if (MR::hasCsvItem(pCsv, "ScaleMin")) {
            f32 scaleMin = one;
            f32 scaleMax = one;
            MR::getCsvDataF32(&scaleMin, pCsv, "ScaleMin", i);
            MR::getCsvDataF32(&scaleMax, pCsv, "ScaleMax", i);
            pInfo->setScaleRange(scaleMin, scaleMax);
        }
        if (MR::hasCsvItem(pCsv, "PosOffsetX")) {
            TVec3f offset;
            MR::getCsvDataF32(&offset.x, pCsv, "PosOffsetX", i);
            MR::getCsvDataF32(&offset.y, pCsv, "PosOffsetY", i);
            MR::getCsvDataF32(&offset.z, pCsv, "PosOffsetZ", i);
            pInfo->mPosOffset = offset;
        }
        if (MR::hasCsvItem(pCsv, "NamePlatePosX")) {
            TVec3f offset;
            MR::getCsvDataF32(&offset.x, pCsv, "NamePlatePosX", i);
            MR::getCsvDataF32(&offset.y, pCsv, "NamePlatePosY", i);
            MR::getCsvDataF32(&offset.z, pCsv, "NamePlatePosZ", i);
            pInfo->mNamePlateOffset = offset;
        }
        mInfos[i] = pInfo;
    }
}

bool GalaxyWorldMapInfoHolder::tryOpen(s32 pointIndex) {
    GalaxyWorldMapInfo* pInfo = find(pointIndex);
    if (pInfo == nullptr) {
        return false;
    }
    return pInfo->tryOpen();
}

void GalaxyWorldMapInfoHolder::tryNewBossAll() {
    for (s32 i = 0; i < mNum; i++) {
        mInfos[i]->tryNewBoss();
    }
}

void GalaxyWorldMapInfoHolder::tryNewAll() {
    for (s32 i = 0; i < mNum; i++) {
        mInfos[i]->tryNew();
    }
}

bool GalaxyWorldMapInfoHolder::isOpened(s32 pointIndex) {
    GalaxyWorldMapInfo* pInfo = find(pointIndex);
    if (pInfo == nullptr) {
        return false;
    }
    return pInfo->isNerveOpened();
}

bool GalaxyWorldMapInfoHolder::isNewOpen(s32 pointIndex) {
    GalaxyWorldMapInfo* pInfo = find(pointIndex);
    if (pInfo == nullptr) {
        return false;
    }
    return pInfo->isNerveNewOpen();
}

const char* GalaxyWorldMapInfoHolder::getGalaxyName(s32 pointIndex) {
    GalaxyWorldMapInfo* pInfo = find(pointIndex);
    if (pInfo != nullptr) {
        return pInfo->mGalaxyName;
    }
    return nullptr;
}

s32 GalaxyWorldMapInfoHolder::getPointingPoint() {
    GalaxyWorldMapInfo* pInfo;
    for (s32 i = 0; i < mNum; i++) {
        pInfo = mInfos[i];
        if (pInfo->isPointing()) {
            return pInfo->mPointIndex;
        }
    }
    return -1;
}

bool GalaxyWorldMapInfoHolder::updatePointing(s32 pointIndex) {
    for (s32 i = 0; i < mNum; i++) {
        GalaxyWorldMapInfo* pInfo = mInfos[i];
        if (pointIndex == pInfo->mPointIndex) {
            return pInfo->updatePointing();
        }
    }
    return false;
}

GalaxyWorldMapInfo* GalaxyWorldMapInfoHolder::find(s32 pointIndex) {
    for (s32 i = 0; i < mNum; i++) {
        GalaxyWorldMapInfo* pInfo = mInfos[i];
        if (pointIndex == pInfo->mPointIndex) {
            return pInfo;
        }
    }
    return nullptr;
}

GalaxyWorldMapInfo* GalaxyWorldMapInfoHolder::findByName(const char* pName) {
    GalaxyWorldMapInfo* pInfo;
    for (s32 i = 0; i < mNum; i++) {
        pInfo = mInfos[i];
        if (MR::isEqualString(pInfo->mGalaxyName, pName)) {
            return pInfo;
        }
    }
    return nullptr;
}

void GalaxyWorldMapInfoHolder::showNamePlate(s32 pointIndex) {
    GalaxyWorldMapInfo* pInfo = find(pointIndex);
    if (pInfo != nullptr) {
        pInfo->showNamePlate(true);
    }
}

void GalaxyWorldMapInfoHolder::startAllPointingAnim() {
    for (s32 i = 0; i < mNum; i++) {
        mInfos[i]->startPointingAnim();
    }
}

void GalaxyWorldMapInfoHolder::appearAll() {
    for (s32 i = 0; i < mNum; i++) {
        if (!mInfos[i]->isNerveHide()) {
            mInfos[i]->makeActorAppeared();
        }
    }
}

void GalaxyWorldMapInfoHolder::killAll() {
    for (s32 i = 0; i < mNum; i++) {
        mInfos[i]->makeActorDead();
    }
}

bool GalaxyWorldMapInfoHolder::isAllCompleted() {
    s32 worldId = mWorld->mWorldId;
    GalaxyWorldMapInfo* pInfo;
    for (s32 i = 0; i < mNum; i++) {
        pInfo = mInfos[i];
        if (!MR::isEqualString(GameDataFunction::getWorldFinalGalaxyName(worldId), pInfo->mName) && !MR::hasAllNormalPowerStarWithoutComet(pInfo->mName)) {
            return false;
        }
    }
    return true;
}

bool GalaxyWorldMapInfoHolder::startOpenSoundAll() {
    GalaxyWorldMapInfo* pInfo;
    bool result = false;
    for (s32 i = 0; i < mNum; i++) {
        pInfo = mInfos[i];
        s32 distance = mWorld->getPathDistance(pInfo->mPointIndex);
        if (distance >= 0) {
            result = (result | pInfo->startOpenSound(distance)) != 0;
        }
    }
    return result;
}

void GalaxyWorldMapInfoHolder::setAllEffectDrawOrder(s32 order) {
    for (s32 i = 0; i < mNum; i++) {
        mInfos[i]->setAllEffectDrawOrder(order);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// WorldMapLayouts: layout actors shared by the whole world map
// ---------------------------------------------------------------------------------------------------------------


WorldMapLayouts::WorldMapLayouts() : mWorldLayout(nullptr), mInfoLayout(nullptr), mZoomButton(nullptr), mCounters(nullptr) {}

void WorldMapLayouts::init() {
    mWorldLayout = new MapGalaxyWorld();
    mWorldLayout->initWithoutIter();
    mWorldLayout->kill();

    mZoomButton = new MapGalaxyZoomButton();
    mZoomButton->initWithoutIter();
    mZoomButton->kill();

    mCounters = new MapGalaxyCounters();
    mCounters->initWithoutIter();
}

void WorldMapLayouts::initInfo() {
    mInfoLayout = new MapGalaxyInfo(-1);
    mInfoLayout->initWithoutIter();
    mInfoLayout->kill();
}

void WorldMapLayouts::killCounters() {
    mCounters->kill();
}

// ---------------------------------------------------------------------------------------------------------------
// MiniPartsModel / MiniGalaxyShadow
// ---------------------------------------------------------------------------------------------------------------

MiniPartsModel::MiniPartsModel(LiveActor* pHost, const char* pName, const char* pModelName, const TVec3f& rPos)
    : PartsModel(pHost, pName, pModelName, nullptr, -1, false) {
    initFixedPosition(rPos, TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    setupCommon(pHost);
}

MiniPartsModel::MiniPartsModel(LiveActor* pHost, const char* pName, const char* pModelName, const char* pArcName, const TVec3f& rPos)
    : PartsModel(pHost, pName, nullptr, nullptr, -1, false) {
    MR::initActor(this, pModelName, pArcName, false);
    initFixedPosition(rPos, TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    setupCommon(pHost);
}

void MiniPartsModel::setupCommon(LiveActor* pHost) {
    mCheckHiddenHost = true;
    MR::setClippingFarMax(this);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 1000.0f);
    MR::tryStartAllAnim(this, MR::getModelResName(pHost));
    makeActorAppeared();
}

void MiniPartsModel::endClipped() {
    LiveActor::endClipped();
    movement();
}

MiniPartsModel::~MiniPartsModel() {}

MiniGalaxyShadow::MiniGalaxyShadow(LiveActor* pHost, const char* pName, const char* pModelName) : DemoObj(pName, nullptr) {
    MR::initActor(this, pModelName, false);
    MR::setPosition(this, pHost->mPosition);
    MR::setClippingFarMax(this);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 1000.0f);
    MR::tryStartAllAnim(this, pModelName);
    makeActorAppeared();
}

MiniGalaxyShadow::~MiniGalaxyShadow() {}
