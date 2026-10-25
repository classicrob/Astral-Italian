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
#ifdef GALAXY_NATIVE
    TVec3f sMiniEye;
    TVec3f sMiniTarget;
#else
    TVec3f sMiniEye(0.0f, 0.0f, 10000.0f);
    TVec3f sMiniTarget(0.0f, 0.0f, 0.0f);
#endif
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
#ifdef GALAXY_NATIVE
void WorldMapFunction::initMiniCamera() {
    new ((void*)&sMiniEye) TVec3f(0.0f, 0.0f, 10000.0f);
    new ((void*)&sMiniTarget) TVec3f(0.0f, 0.0f, 0.0f);
}

#endif
