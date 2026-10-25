#pragma auto_inline off
#include "Game/System/WorldMapModel.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/WorldMapSequence.hpp"
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/MiniMeister.hpp"
#include "Game/System/MiniPartsActors.hpp"
#include "Game/Screen/MapGalaxyLayouts.hpp"
#include "Game/Screen/GalaxyStatusIcon.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include <cstdio>
#include <new>
#pragma use_simple_stdpnew on
#include <revolution/mtx.h>

namespace {
    TVec3f sOffset0;
    TVec3f sOffset1;
    TVec3f sOffset2;
    TVec3f sOffset3;
    TVec3f sOffset4;
    TVec3f sOffset5;
    TVec3f sOffset6;
    TVec3f sOffset7;
    TVec3f sMiniEye;
    TVec3f sMiniTarget;
}

WorldMapModel::WorldMapModel(const char* pName) : LiveActor(pName), mChildren() {}


void WorldMapModel::appearAll() {
    for (LiveActor** it = mChildren.mArr; it != mChildren.mArr + mChildren.mMaxSize; ++it) (*it)->makeActorAppeared();
    makeActorAppeared();
    MR::tryStartAllAnim(this, MR::getModelResName(this));
}
void WorldMapModel::killAll() {
    for (LiveActor** it = mChildren.mArr; it != mChildren.mArr + mChildren.mMaxSize; ++it) {
        (*it)->makeActorDead();
        if (MR::isExistEffectKeeper(*it)) MR::forceDeleteEffectAll(*it);
    }
    makeActorDead();
    if (MR::isExistEffectKeeper(this)) MR::forceDeleteEffectAll(this);
}
void WorldMapModel::initAfterPlacement() { WorldMapOwner::registerModel(this); }
WorldMapModel::~WorldMapModel() {  }

WorldMapOwnerContext::WorldMapOwnerContext() : mWorldId(-1), mCamera(0), mMeister(0), mModel(0) {}
void WorldMapOwnerContext::init(s32 world, s32 node) {
    mWorldId = world;
    mLayouts = new WorldMapLayouts;
    mWorld = new WorldMapResultWorld(world);
    mWorld->initWorld(1);
    mWorld->prepare();
    mLayouts->init();
    mCamera = new WorldMapCameraHolder;
    mCamera->init(node);
    mWorld->updatePaths(node);
    mMeister = new MiniMeister(mCamera);
    mMeister->initWithoutIter();
    mLayouts->initInfo();
}
WorldMapOwner::WorldMapOwner() : NameObj("ワールドマップ"), mContext(0), mSequence(0), mMainLayout(0) {}
void WorldMapOwner::init(const JMapInfoIter&) {
    bool after = GameDataFunction::isOnGameEventFlag("グランドギャラクシーマップ初プレイ") ? GameSequenceFunction::isWorldMapAfterFileSelect() : false;
    if (MR::isEqualStageName("WorldMap01Galaxy")) GameDataFunction::setCurrentWorldId(1);
    if (MR::isEqualStageName("WorldMap02Galaxy")) GameDataFunction::setCurrentWorldId(2);
    if (MR::isEqualStageName("WorldMap03Galaxy")) GameDataFunction::setCurrentWorldId(3);
    if (MR::isEqualStageName("WorldMap04Galaxy")) GameDataFunction::setCurrentWorldId(4);
    if (MR::isEqualStageName("WorldMap05Galaxy")) GameDataFunction::setCurrentWorldId(5);
    if (MR::isEqualStageName("WorldMap06Galaxy")) GameDataFunction::setCurrentWorldId(6);
    if (MR::isEqualStageName("WorldMap07Galaxy")) GameDataFunction::setCurrentWorldId(7);
    if (MR::isEqualStageName("WorldMap08Galaxy")) GameDataFunction::setCurrentWorldId(8);
    MR::connectToSceneMapObjMovement(this);
    MR::registerDemoSimpleCastAll(this);
    mContext = new WorldMapOwnerContext;
    s32 node = GameSequenceFunction::getWorldMapCurrentNode();
    mContext->init(GameDataFunction::getCurrentWorldId(), node);
    mMainLayout = new GrandGalaxyMap(this);
    static_cast<GrandGalaxyMap*>(mMainLayout)->init(after);
    mSequence = new WorldMapSequence(this);
    mSequence->init(after);
    if (GameSequenceFunction::isWorldMapAfterFileSelect()) GameSequenceFunction::clearWorldMapAfterFileSelect();
    GameSequenceFunction::clearWorldMapFlag22D();
    GameSequenceFunction::clearWorldMapFlag22C();
}
void WorldMapModel::init(const JMapInfoIter& rIter) {
    const char* name = "";
    MR::getObjectName(&name, rIter);
    char path[0x80];
    snprintf(path, sizeof(path), "/ObjectData/%s.arc", name);
    if (MR::isFileExist(path, true)) {
        if (!MR::initActor(this, rIter, name, false)) {
            MR::connectToSceneMapObj(this);
            initEffectKeeper(0, 0, false);
        }
        MR::tryStartAllAnim(this, name);
    } else {
        MR::connectToSceneMapObjMovement(this);
    }
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    makeActorAppeared();
    s32 num = MR::getChildObjNum(rIter);
    if (num > 0) {
        mChildren.mArr = new LiveActor*[num];
        mChildren.mMaxSize = num;
        for (LiveActor** it = mChildren.mArr; it != mChildren.mArr + mChildren.mMaxSize; ++it) {
            const char* childName = 0;
            MR::getChildObjName(&childName, rIter, it - mChildren.mArr);
            *it = static_cast<LiveActor*>(NameObjFactory::initChildObj(rIter, it - mChildren.mArr));
        }
    }
}
void WorldMapOwner::movement() { mSequence->updateNerve(); }
WorldMapOwner::~WorldMapOwner() {}

void WorldMapFunction::movementMini() {
    TPos3f saved(MR::getCameraViewMtx());
    f32 fovy = MR::getFovy();
    MR::setFovy(45.0f);
    MR::loadProjectionMtx();
    TPos3f view;
    TVec3f up(0.0f, 1.0f, 0.0f);
    C_MTXLookAt(view, &sMiniEye, &up, &sMiniTarget);
    MR::setCameraViewMtx(view, false, false, TVec3f(0.0f, 0.0f, 0.0f));
    MR::loadViewMtx();
    MR::movementWorldMapMini();
    MR::setCameraViewMtx(saved, false, false, TVec3f(0.0f, 0.0f, 0.0f));
    MR::loadViewMtx();
    MR::setFovy(fovy);
    MR::loadProjectionMtx();
}
void WorldMapFunction::calcAnimMini() {
    TPos3f saved(MR::getCameraViewMtx());
    TPos3f view;
    TVec3f up(0.0f, 1.0f, 0.0f);
    C_MTXLookAt(view, &sMiniEye, &up, &sMiniTarget);
    MR::setCameraViewMtx(view, false, false, TVec3f(0.0f, 0.0f, 0.0f));
    MR::loadViewMtx();
    MR::calcAnimWorldMapMini();
    MR::setCameraViewMtx(saved, false, false, TVec3f(0.0f, 0.0f, 0.0f));
    MR::loadViewMtx();
}
void WorldMapFunction::entryMini() {
    TPos3f saved(MR::getCameraViewMtx());
    TPos3f view;
    TVec3f up(0.0f, 1.0f, 0.0f);
    C_MTXLookAt(view, &sMiniEye, &up, &sMiniTarget);
    MR::setCameraViewMtx(view, false, false, TVec3f(0.0f, 0.0f, 0.0f));
    MR::loadViewMtx();
    MR::entryWorldMapMini();
    MR::setCameraViewMtx(saved, false, false, TVec3f(0.0f, 0.0f, 0.0f));
    MR::loadViewMtx();
}
void WorldMapFunction::drawMini() {
    TPos3f saved(MR::getCameraViewMtx());
    f32 fovy = MR::getFovy();
    MR::reinitGX();
    MR::clearZBuffer();
    MR::setFovy(45.0f);
    MR::loadProjectionMtx();
    TPos3f view;
    TVec3f up(0.0f, 1.0f, 0.0f);
    C_MTXLookAt(view, &sMiniEye, &up, &sMiniTarget);
    MR::setCameraViewMtx(view, false, false, TVec3f(0.0f, 0.0f, 0.0f));
    MR::loadViewMtx();
    MR::drawWorldMapMini();
    MR::setCameraViewMtx(saved, false, false, TVec3f(0.0f, 0.0f, 0.0f));
    MR::loadViewMtx();
    MR::setFovy(fovy);
    MR::loadProjectionMtx();
}
void WorldMapFunction::initMiniCamera() {
    new ((void*)&sMiniEye) TVec3f(0.0f, 0.0f, 10000.0f);
    new ((void*)&sMiniTarget) TVec3f(0.0f, 0.0f, 0.0f);
}
bool WorldMapOwner::isGalaxyMap() const { return mSequence->isGalaxyMap(); }
s32 MR::getProgressWorldNo() {
    if (MR::isOnGameEventFlagNormalEnding()) return 7;
    return GameDataFunction::calcLastEnteredWorldNo();
}
s32 WorldMapFunction::getGrandStarNum() {
    s32 num = 1;
    for (s32 i = 0; i < 6; ++i) if (GameDataFunction::hasGrandStar(i + 1)) ++num;
    return num;
}
WorldMapCameraHolder* WorldMapOwner::getCamera() const { return mContext->mCamera; }
JMapInfo* WorldMapFunction::createWorldParser(s32 world, const char* name) {
    char archive[0x40];
    snprintf(archive, sizeof(archive), "WorldMap%02d.arc", world);
    return MR::createCsvParserFromFile(archive, name);
}
void WorldMapOwner::registerModel(WorldMapModel* model) { MR::getSceneObj<WorldMapOwner*>(0x33)->mContext->mModel = model; }
PartsModel* WorldMapFunction::createPartsModel(LiveActor* host, const char* name, const char* model, const char* joint) {
    PartsModel* part = new PartsModel(host, name, 0, 0, -1, false);
    if (!MR::initActor(part, model, false)) MR::connectToScene(part, 0x2F, 0x14, 0x27, -1);
    part->initFixedPosition(joint);
    part->initWithoutIter();
    return part;
}
MiniPlayerRocket* WorldMapOwner::getRocket() const { return mContext->mCamera->mRocket; }
// NONMATCHING: regalloc (FP order/grouping checked: no FP instructions).
void WorldMapOwner::setRocketPosWithTarget(const TVec3f* pos) const {
    WorldMapCameraHolder* camera = mContext->mCamera;
    MiniPlayerRocket* rocket = camera->mRocket;
    MR::setPosition(rocket, *pos);
    MR::setPosition(camera->mCameraTarget, rocket->mPosition);
}
void WorldMapOwner::setRocketScale(const TVec3f* scale) { MR::setScale(getRocket(), *scale); }
const TVec3f* WorldMapOwner::getRocketPosition() const { return &mContext->mCamera->mRocket->mPosition; }
const TVec3f* WorldMapFunction::getRocketOffsetGalaxy() { return &sOffset0; }
const TVec3f* WorldMapFunction::getRocketOffsetPoint() { return &sOffset7; }
const TVec3f* WorldMapOwner::getNodeOffset(s32 node) {
    WorldMapResultWorld* world = getWorld();
    if (world->isWorldDataType3(node)) return &sOffset1;
    if (world->isWorldDataType4(node)) return &sOffset2;
    if (world->isWorldDataType5(node)) return &sOffset3;
    if (world->hasWorldData(node)) return &sOffset0;
    if (world->callU14_6D0(node)) return &sOffset4;
    if (world->callU14_450(node)) return &sOffset5;
    return world->callU14_860(node) ? &sOffset6 : &sOffset7;
}
void WorldMapOwner::getNodePosition(TVec3f* out, WorldMapOwner* owner, s32 node) {
    const WorldMapResultNode* data = owner->getWorld()->getNode(node);
    out->x = data->mPos.x; out->y = data->mPos.y; out->z = data->mPos.z;
    out->add(*owner->getNodeOffset(node));
}
void WorldMapOwner::warpRocketToNode(s32 node) {
    WorldMapCameraHolder* camera = mContext->mCamera;
    MiniPlayerRocket* rocket = camera->mRocket;
    if (node < 0) node = camera->mNext;
    TVec3f pos(0.0f);
    getNodePosition(&pos, this, node);
    MR::setPosition(rocket, pos);
    MR::zeroVelocity(rocket);
    MR::setPosition(camera->mCameraTarget, pos);
    camera->mCurrent = node;
    camera->mNext = -1;
    updateCurrentPaths();
}
void WorldMapOwner::startDemo() {
    bool stopped = MR::isStopCinemaFrame();
    MR::tryStartDemoMarioPuppetable(mContext->mCamera->mRocket, "ワールドマップデモ");
    mContext->mCamera->mRocket->disablePlayerBind();
    if (!stopped) MR::forceToFrameCinemaFrame();
}
void WorldMapOwner::startDemoNoCinemaCheck() {
    MR::tryStartDemoMarioPuppetable(mContext->mCamera->mRocket, "ワールドマップデモ");
    mContext->mCamera->mRocket->disablePlayerBind();
    MR::forceToFrameCinemaFrame();
}
void WorldMapOwner::endDemo() {
    MR::endDemo(mContext->mCamera->mRocket, "ワールドマップデモ");
    mContext->mCamera->mRocket->enablePlayerBind();
}
#pragma dont_inline on
// NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
void WorldMapOwner::setRocketFrontUpPos(const TVec3f& front, const TVec3f& up, const TVec3f& pos) {
    MiniPlayerRocket* rocket = mContext->mCamera->mRocket;
    if (MR::isSameDirection(front, up, 0.01f) || MR::isSameDirection(front, -up, 0.01f)) return;
    Mtx mtx;
    reinterpret_cast<TMtx34f*>(mtx)->identity();
    MR::makeMtxUpFrontPos(reinterpret_cast<TPos3f*>(mtx), up, front, pos);
    rocket->setPlayerMtx(mtx);
}
#pragma dont_inline reset
s32 WorldMapOwner::getWorldId() const { return mContext->mWorldId; }
s32 WorldMapFunction::getGalaxyWorldNo(const char* name) { return MR::makeGalaxyStatusAccessor(name).getWorldNo(); }
// NONMATCHING: regalloc (FP order/grouping checked: no FP instructions).
bool WorldMapFunction::hasIncompleteGreenStar(const char* name) {
    GalaxyStatusAccessor status = MR::makeGalaxyStatusAccessor(name);
    if (!GameDataFunction::isOnGameEventFlagEnding120Stars()) return false;
    return !status.getGreenStarNum() ? false : !status.isCompletedGreenStars();
}
void WorldMapOwner::showGalaxyInfo(const char* name) { mContext->mLayouts->mInfoLayout->showGalaxy(name, 4); }
bool WorldMapOwner::isGalaxyInfoAppeared() const { return mContext->mLayouts->mInfoLayout->isAppeared(); }
const char* WorldMapOwner::getGalaxyInfoName() const { return mContext->mLayouts->mInfoLayout->mGalaxyName; }
void WorldMapOwner::killCounters() { mContext->mLayouts->killCounters(); }
void WorldMapFunction::setStatusIconKind(GalaxyStatusIcon* icon, const char* name) {
    if (MR::isPostmanTaskInProgressAtStageGroup6To7(name)) { icon->setEventPane(1); return; }
    if (MR::isPostmanTaskInProgressAtStageGroup8To9(name)) { icon->setEventPane(0); return; }
    if (MR::isPostmanTaskInProgressAtStageGroup10To12(name)) { icon->setEventPane(2); return; }
    icon->setEventPane(-1);
}
void MiniStarPieceMine::setGalaxyFlag() { GameSequenceFunction::setWorldMapGalaxyFlag(mWorldId, mPointIndex); }
bool MiniStarPieceMine::isGalaxyFlag() const { return GameSequenceFunction::isWorldMapGalaxyFlag(mWorldId, mPointIndex); }
// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void WorldMapFunction::initDrawFog(LiveActor* actor) {
    MR::initDLMakerFog(actor, true);
    MR::newDifferedDLBuffer(actor);
    MR::setFogType(actor, 0);
    GXColor color;
    color.r = 0; color.g = 0; color.b = 0; color.a = 255;
    MR::setFogColorWithAlpha(actor, GXColor(color), false);
    MR::setFogStartEndZ(actor, 0.0f, 1.0f);
    f32 farZ = MR::getFarZ();
    MR::setFogNearFarZ(actor, MR::getNearZ(), farZ);
}
void WorldMapFunction::setFogRange(LiveActor* actor, f32 scale) {
    f32 distance = MR::calcCameraDistanceZ(actor->mPosition);
    MR::setFogStartEndZ(actor, 0.0f, (1.0f / scale) * distance);
}
void WorldMapFunction::setFogDrawOn(LiveActor* actor) { MR::setFogType(actor, 2); }
void WorldMapFunction::setFogDrawOff(LiveActor* actor) { MR::setFogType(actor, 0); }
bool WorldMapFunction::isPointing(LiveActor* actor) { return MR::isStarPointerPointing1PWithoutCheckZ(actor, "弱", true, false); }
// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void WorldMapFunction::approachScale(LiveActor* actor, f32 speed, f32 max) { f32 scale = speed + actor->mScale.x; if (scale >= max) scale = max; MR::setScale(actor, scale); }
void WorldMapFunction::returnScale(LiveActor* actor, f32 speed, f32 min) { MR::setScale(actor, actor->mScale.x - speed >= min ? actor->mScale.x - speed : min); }
// NONMATCHING: regalloc (FP order/grouping checked: no FP instructions).
bool WorldMapOwner::isWorldEventFlagOff() const {
    s32 world = mContext->mWorldId;
    if (world == 1 && !GameDataFunction::isOnGameEventFlag("ワールド1初プレイ")) return true;
    if (world == 2 && !GameDataFunction::isOnGameEventFlag("ワールド2初プレイ")) return true;
    if (world == 3 && !GameDataFunction::isOnGameEventFlag("ワールド3初プレイ")) return true;
    if (world == 4 && !GameDataFunction::isOnGameEventFlag("ワールド4初プレイ")) return true;
    if (world == 5 && !GameDataFunction::isOnGameEventFlag("ワールド5初プレイ")) return true;
    if (world == 6 && !GameDataFunction::isOnGameEventFlag("ワールド6初プレイ")) return true;
    if (world == 7 && !GameDataFunction::isOnGameEventFlag("ワールド7初プレイ")) return true;
    return false;
}
bool WorldMapFunction::tryOnGreenStarEventFlag() {
    if (GameSequenceFunction::isGreenStarEventOnly()) { GameDataFunction::onGameEventFlag("グリーンスター出現デモ"); return true; }
    return false;
}
bool WorldMapFunction::isMapEventFlagOff() { return !GameDataFunction::isOnGameEventFlag("グランドギャラクシーマップ初プレイ"); }
bool WorldMapFunction::isGreenStarDemoReady() {
    if (!GameDataFunction::isOnGameEventFlag("グランドギャラクシーマップ初プレイ")) {
        if (GameDataFunction::calcLastEnteredWorldNo() >= 2 && MR::getPowerStarNumOwnedInStage("HomeCenterGalaxy") > 0) return true;
    }
    return false;
}
// NONMATCHING: regalloc (FP order/grouping checked: no FP instructions).
bool WorldMapFunction::isWorldNoMid(s32 world) { s32 result = 0; if (2 <= world) { if (world <= 6) result = 1; } return result; }
// NONMATCHING: regalloc (FP order/grouping checked: no FP instructions).
namespace {
    const char* sExcludedResources[] = {"/ObjectData/WorldMap01Sky.arc"};
}
bool WorldMapFunction::mountWorldMapResources() {
    s32 count;
    s32 i;
    if (!GameSequenceFunction::isWorldMapEnterType5()) return false;
    const JMapInfo* parser = MR::tryCreateStageUseResourceCsvParser("MarioFaceShipGalaxy", "scenario_2");
    count = parser->getNumEntries();
    for (i = 0; i < count; ++i) {
        const char* name;
        if (parser->getValue<const char*>(i, "ResourceName", &name) && !MR::isExistString(name, sExcludedResources, 1)) {
            if (MR::isFileExist(name, true)) MR::mountAsyncArchive(name, MR::getWorldMapHeap(), false, true);
        }
    }
    return false;
}

void WorldMapFunction::initRocketOffsets() {
    new ((void*)&sOffset0) TVec3f(-800.0f, 2666.0f, 3000.0f);
    new ((void*)&sOffset1) TVec3f(-800.0f, 2666.0f, 3000.0f);
    new ((void*)&sOffset2) TVec3f(-2400.0f, 2666.0f, 3800.0f);
    new ((void*)&sOffset3) TVec3f(-800.0f, 2666.0f, 3000.0f);
    new ((void*)&sOffset4) TVec3f(-800.0f, 2666.0f, 3000.0f);
    new ((void*)&sOffset5) TVec3f(-800.0f, 2666.0f, 3000.0f);
    new ((void*)&sOffset6) TVec3f(0.0f, 1700.0f, 0.0f);
    new ((void*)&sOffset7) TVec3f(0.0f, 1000.0f, 0.0f);
}

#ifdef GALAXY_NATIVE
namespace {
    struct WorldMapStaticInit {
        WorldMapStaticInit() {
            WorldMapFunction::initMiniCamera();
            WorldMapFunction::initRocketOffsets();
        }
    } sWorldMapStaticInit;
}
#else
__declspec(section ".ctors") void (*sInitMiniCamera)() = WorldMapFunction::initMiniCamera;
__declspec(section ".ctors") void (*sInitRocketOffsets)() = WorldMapFunction::initRocketOffsets;
#endif
