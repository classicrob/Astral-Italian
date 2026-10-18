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

// ==== TU B (0x804EB2F0): WorldMapCameraHolder ====
// ---------------------------------------------------------------------------------------------------------------
// WorldMapCameraHolder (0x68 bytes): rocket, camera target and camera info of the world map
// ---------------------------------------------------------------------------------------------------------------


WorldMapCameraHolder::WorldMapCameraHolder() {
    mCurrent = 0;
    mNext = -1;
    mRocket = nullptr;
    mCameraTarget = nullptr;
    mCameraInfo = nullptr;
    mCameraId = -1;
    mIsChanging = false;
    mHasNext = false;
    mPrevious = -1;
}

void WorldMapCameraHolder::init(s32 current) {
    mCurrent = current;
    mRocket = new MiniPlayerRocket();
    mRocket->initWithoutIter();
    mCameraTarget = new LiveActor("カメラターゲット");
    mCameraTarget->initWithoutIter();
    mCameraTarget->makeActorAppeared();
    MR::invalidateClipping(mCameraTarget);
    mCameraId = MR::getCameraParamId("WorldMapCamera", "CameraParam");
    mCameraInfo = new ActorCameraInfo(0, 0);
}

void WorldMapCameraHolder::setNext(s32 next) {
    mHasNext = true;
    mPrevious = mNext;
    mNext = next;
}

bool WorldMapCameraHolder::tryApplyNext() {
    s32 previous = mPrevious;
    mHasNext = false;
    if (previous == mCurrent) {
        mPrevious = -1;
        return false;
    }
    mNext = previous;
    mPrevious = -1;
    return true;
}

// ==== TU C (0x804EB450): MiniRouteLine ====
// ---------------------------------------------------------------------------------------------------------------
// MiniRouteLine: the glowing line between two map nodes
// ---------------------------------------------------------------------------------------------------------------

MiniRouteLine::MiniRouteLine(const char* pName, WorldMapResultWorld*) : LiveActor(pName) {
    mFrom = 0;
    mTo = 0;
    mCondStage = nullptr;
    mCondStarNum = -1;
    mCondFlag = nullptr;
    mUnkA4 = false;
    mColorChange = false;
    mValid = true;
    mOpened = false;
    mHidden = false;
    mAlpha = 1.0f;
    mCenter.x = 0.0f;
    mCenter.y = 0.0f;
    mCenter.z = 0.0f;
    mEffectPos.x = 0.0f;
    mEffectPos.y = 0.0f;
    mEffectPos.z = 0.0f;
    mEffectName = "Progress";
    mShadowActor = nullptr;
    mMtxSetter = nullptr;
}

// NONMATCHING: the three scaled copies of the side vector are low-slot temporaries in the target (chained scaleInline inlines too much)
void MiniRouteLine::init(const JMapInfoIter&) {
    if (!MR::initActor(this, "MiniRouteLine", true)) {
        MR::connectToScene(this, -1, 20, 39, -1);
    }
    mMtxSetter = MR::initDLMakerProjmapEffectMtxSetter(this);
    mMtxSetter->updateMtxUseBaseMtx();
    MR::newDifferedDLBuffer(this);
    MR::invalidateClipping(this);
    makeActorDead();

    mEffectActor = new LiveActor("リンク作成中");
    if (!MR::initActor(mEffectActor, "MiniRouteLine", false)) {
        mEffectActor->initEffectKeeper(1, nullptr, false);
        MR::setEffectHostSRT(mEffectActor, "Progress", &mEffectPos, &mEffectActor->mRotation, nullptr);
        MR::setEffectHostSRT(mEffectActor, "ProgressTico", &mEffectPos, &mEffectActor->mRotation, nullptr);
        MR::connectToScene(mEffectActor, -1, 20, 39, -1);
    }
    MR::invalidateClipping(mEffectActor);
    mEffectActor->makeActorDead();

    mShadowActor = new LiveActor("リンク影");
    if (!MR::initActor(mShadowActor, "MiniRouteLineShadow", false)) {
        MR::connectToScene(mShadowActor, -1, 20, 39, -1);
    }
    MR::invalidateClipping(mShadowActor);
    mShadowActor->makeActorDead();

    TVec3f sideVec;
    MR::calcSideVec(&sideVec, this);
    f32 scaleX = mScale.x;
    TVec3f a(sideVec);
    a.scale(scaleX);
    TVec3f b(a);
    b.scale(0.5f);
    TVec3f c(b);
    c.scale(1000.0f);
    mCenter.set(mPosition + c);
    MR::setClippingTypeSphere(this, 1000.0f * (mScale.x * 0.5f), &mCenter);
    MR::setClippingTypeSphere(mShadowActor, 1000.0f * (mScale.x * 0.5f), &mCenter);
    MR::startAction(this, "Close");
    MR::startAction(mEffectActor, "Open");
    MR::startBrk(mEffectActor, "Normal");
    MR::copyTransRotateScale(this, mShadowActor);
    MR::copyTransRotateScale(this, mEffectActor);
    updateShadow(0, mAlpha);
    updateEffect(0, mAlpha);
}

void MiniRouteLine::appear() {
    LiveActor::appear();
    if (mHidden) {
        MR::hideModel(this);
    } else {
        MR::showModel(this);
    }
    updateShadow(0, mAlpha);
    updateEffect(0, mAlpha);
}

void MiniRouteLine::kill() {
    LiveActor::kill();
    mShadowActor->kill();
    mEffectActor->kill();
}

// NONMATCHING: register numbers of the paired-single matrix copy (f0..f5 are assigned in reverse)
void MiniRouteLine::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    TPos3f mtx;
    MR::makeMtxTR(mtx, this);
    mtx.invert(mtx);
    mMtxSetter->mEffectMtx.set(mtx);
}

void MiniRouteLine::setCondition(const char* pStage, s32 starNum, const char* pFlag) {
    mCondStage = pStage;
    mCondStarNum = starNum;
    mCondFlag = pFlag;
    if (isLocked()) {
        mHidden = true;
    }
}

bool MiniRouteLine::isLocked() const {
    if (!MR::isNullOrEmptyString(mCondStage)) {
        if (mCondStarNum == 0) {
            if (GameDataFunction::getPowerStarNumOwned(mCondStage) == 0) {
                return true;
            }
        } else if (!GameDataFunction::hasPowerStar(mCondStage, mCondStarNum)) {
            return true;
        }
    }
    if (!MR::isNullOrEmptyString(mCondFlag) && !GameDataFunction::isOnGameEventFlag(mCondFlag)) {
        return true;
    }
    return false;
}

bool MiniRouteLine::hasFlag() const {
    return !MR::isNullOrEmptyString(mCondFlag);
}

void MiniRouteLine::setup(const TVec3f& rFrom, const TVec3f& rTo) {
    TQuat4f quat;
    TVec3f dir(rTo);
    dir.sub(rFrom);
    mScale.set<f32>(dir.length() / 1000.0f, 1.0f, 1.0f);
    MR::normalize(&dir);
    MR::makeQuatSideUp(&quat, dir, TVec3f(0.0f, 1.0f, 0.0f));
    MR::setRotation(this, quat);
    MR::setPosition(this, TVec3f(rFrom.x, rFrom.y, rFrom.z));
}

// NONMATCHING: target tests the argument with clrlwi. instead of cmpwi
void MiniRouteLine::setColorChange(u8 flag) {
    mColorChange = flag;
    if (flag) {
        MR::startBrk(mEffectActor, "TicoBuild");
    } else {
        MR::startBrk(mEffectActor, "Normal");
    }
}

void MiniRouteLine::setAlpha(s32 flag, f32 alpha) {
    mAlpha = alpha;
    updateShadow(flag, alpha);
    updateEffect(flag, alpha);
    updateEffectPos();
}

void MiniRouteLine::updateShadow(s32 doTransform, f32 alpha) {
    if (!mOpened || alpha == 0.0f) {
        mShadowActor->makeActorDead();
        return;
    }
    mShadowActor->makeActorAppeared();
    MR::copyTransRotateScale(this, mShadowActor);
    mShadowActor->mScale.x *= alpha;
    if (doTransform != 0) {
        updateTransform(mShadowActor);
    }
    mShadowActor->mPosition.y += -700.0f;
}

void MiniRouteLine::updateEffect(s32 doTransform, f32 alpha) {
    if (!mOpened || alpha == 0.0f) {
        mEffectActor->makeActorDead();
        return;
    }
    mEffectActor->makeActorAppeared();
    MR::copyTransRotateScale(this, mEffectActor);
    mEffectActor->mScale.x *= alpha;
    if (doTransform != 0) {
        updateTransform(mEffectActor);
    }
}

void MiniRouteLine::updateTransform(LiveActor* pPart) {
    TVec3f xDir;
    MR::extractMtxXDir(getBaseMtx(), &xDir);
    f32 scaleX = mScale.x;
    MR::setPosition(pPart, mPosition + xDir.scaleInline(scaleX).scaleInline(1000.0f));
    f32 angle = 180.0f + mRotation.y;
    wrapAngle(angle, 0.0f);
    MR::setRotation(pPart, mRotation.x, angle, mRotation.z);
}

void MiniRouteLine::updateEffectPos() {
    TVec3f xDir;
    MR::extractMtxXDir(mEffectActor->getBaseMtx(), &xDir);
    LiveActor* pEffect = mEffectActor;
    f32 length = 1000.0f * pEffect->mScale.x;
    mEffectPos.set(pEffect->mPosition + xDir.scaleInline(length));
}

void MiniRouteLine::emitProgressEffect() {
    MR::emitEffect(mEffectActor, mEffectName);
}

void MiniRouteLine::deleteProgressEffect() {
    MR::deleteEffect(mEffectActor, mEffectName);
}

void MiniRouteLine::setEffectDrawOrder(s32 order) {
    MR::setAllEffectDrawOrder(mEffectActor, order);
}

bool MiniRouteLine::getNextNode(s32 node, s32* pOther) {
    *pOther = -1;
    if (!mValid) {
        return false;
    }
    if (mOpened) {
        return false;
    }
    if (mAlpha < 1.0f) {
        return false;
    }
    if (isLocked()) {
        return false;
    }
    if (mFrom == node) {
        *pOther = mTo;
    }
    if (mTo == node) {
        *pOther = mFrom;
    }
    if (*pOther < 0) {
        return false;
    }
    return true;
}

bool MiniRouteLine::open() {
    mOpened = true;
    appear();
    return true;
}

bool MiniRouteLine::isBetween(s32 a, s32 b, bool strict) {
    if (!mValid) {
        return false;
    }
    if (strict) {
        if (!mOpened) {
            return false;
        }
        if (mAlpha < 1.0f) {
            return false;
        }
        if (isLocked()) {
            return false;
        }
    }
    if (mFrom == a && mTo == b) {
        return true;
    }
    if (mFrom == b && mTo == a) {
        return true;
    }
    return false;
}

bool MiniRouteLine::getOther(s32 node, s32* pOther) const {
    *pOther = -1;
    if (mFrom == node) {
        *pOther = mTo;
    }
    if (mTo == node) {
        *pOther = mFrom;
    }
    if (*pOther < 0) {
        return false;
    }
    return true;
}

bool MiniRouteLine::isBetweenNodes(s32 a, s32 b) const {
    if (mFrom == a && mTo == b) {
        return true;
    }
    if (mFrom == b && mTo == a) {
        return true;
    }
    return false;
}

bool MiniRouteLine::isDirected(s32 a, s32 b) const {
    if (mFrom == b && mTo == a) {
        return true;
    }
    return false;
}

MiniRouteLine::~MiniRouteLine() {}

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
            const char* pFlag = "";
            const char* pColorChange = "";
            const char* pSubRoute;
            s32 pointA;
            s32 pointB;
            const char* pCloseStage;
            s32 closeScenario;
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
        for (s32 j = 0; j < mCount; j++) {
            if (pOther->mEdges[i].mFrom == mEdges[j].mFrom && pOther->mEdges[i].mTo == mEdges[j].mTo) {
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

void PointPartsHolder::init() {
    JMapInfo* pCsv = mWorld->createParser(sPointPartsCsv);
    if (pCsv != nullptr) {
        mUnkC = WorldMapFunction::getGrandStarNum();
        s32 worldId = mWorld->getWorldId();
        s32 num = MR::getCsvDataElementNum(pCsv);
        mNum = num;
        mParts = new PointPart[num];
        PointPart* pPart;
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
            if (!isStarRoadWarpPoint(pPart->mType)) {
                MR::invalidateClipping(pPart->mActor);
            }
        }
    }
}

bool PointPartsHolder::getWorldWarp(s32 pointIndex, PartsWarpInfo* pOut) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    if (!isWorldWarpPoint(pPart->mType)) {
        return false;
    }
    pOut->mParam0 = pPart->mParam0;
    pOut->mParam1 = pPart->mParam1;
    pOut->mPointIndex = pointIndex;
    return true;
}

bool PointPartsHolder::getStarRoadWarp(s32 pointIndex, PartsWarpInfo* pOut) {
    PointPart* pPart = find(pointIndex);
    if (pPart == nullptr) {
        return false;
    }
    if (!isStarRoadWarpPoint(pPart->mType)) {
        return false;
    }
    pOut->mParam0 = pPart->mParam0;
    pOut->mParam1 = pPart->mParam1;
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
    return *reinterpret_cast<s32*>(reinterpret_cast<u8*>(find(pointIndex)->mActor) + 0x94);
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
    if (!isStarRoadWarpPoint(pPart->mType)) {
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
    WorldMapResultWorld* pWorld = mWorld;
    s32 count = 0;
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
            pMark->mFlag11 = mWorld->isNodeFlag11(i);
            index++;
        }
    }
}
