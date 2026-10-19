#pragma auto_inline off
#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Camera/CameraShaker.hpp"
#include "Game/Effect/EffectSystemUtil.hpp"
#include "Game/LiveActor/AllLiveActorGroup.hpp"
#include "Game/LiveActor/ClippingJudge.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Map/NamePosHolder.hpp"
#include "Game/Map/SwitchWatcherHolder.hpp"
#include "Game/Map/WaterInfo.hpp"
#include "Game/MapObj/BenefitItemLifeUp.hpp"
#include "Game/MapObj/BenefitItemOneUp.hpp"
#include "Game/MapObj/CoinHolder.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/NPC/StageStateKeeper.hpp"
#include "Game/NameObj/MovementOnOffGroupHolder.hpp"
#include "Game/NameObj/NameObj.hpp"
#include "Game/NameObj/NameObjAdaptor.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/NameObj/NameObjFinder.hpp"
#include "Game/NameObj/NameObjListExecutor.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Scene/StopSceneController.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/ResourceHolderManager.hpp"
#include "Game/System/WPadRumble.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SwitchEventFunctorListener.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TUtil.hpp>
#include <cstdio>
#include <va_list.h>



namespace {
    static const TVec3f sDefaultPlayerPos = TVec3f(0, 0, 0);
};  // namespace

bool MR::isJudgedToClipFrustum(const TVec3f& rParam1, f32 param2) {
    return MR::getClippingJudge()->isJudgedToClipFrustum(rParam1, param2);
}


bool MR::isJudgedToClipFrustum50m(const TVec3f& rParam1, f32 param2) {
    return MR::getClippingJudge()->isJudgedToClipFrustum(rParam1, param2, 7);
}

bool MR::isJudgedToClipFrustum300m(const TVec3f& rParam1, f32 param2) {
    return MR::getClippingJudge()->isJudgedToClipFrustum(rParam1, param2, 4);
}

bool MR::isJudgedToNearClip(const TVec3f& rPos, f32 distance) {
    TVec3f camPos = MR::getCamPos();

    return rPos.distance(camPos) < distance;
}

void MR::initialize(NameObj* pObj, const JMapInfoIter& rIter) {
    pObj->setLinkedInfo(rIter);
    MR::initLiveActorSystemInfo(rIter);
    pObj->init(rIter);
    MR::initLiveActorSystemInfo(rIter);
}

bool MR::isInWater(const TVec3f& rPos) {
    WaterInfo waterInfo = WaterInfo();

    return MR::getWaterAreaObj(&waterInfo, rPos);
}

bool MR::isInDeath(const TVec3f& rPos) {
    return MR::isInAreaObj("DeathArea", rPos);
}

bool MR::isInDarkMatter(const TVec3f& rPos) {
    return MR::isInAreaObj("DarkMatterArea", rPos);
}

void MR::connectToScene(LiveActor* pActor, int movementType, int calcAnimType, int drawBufferType, int drawType) {
    MR::registerNameObjToExecuteHolder(pActor, movementType, calcAnimType, drawBufferType, drawType);
}

void MR::connectToScene(NameObj* pObj, int movementType, int calcAnimType, int drawBufferType, int drawType) {
    MR::registerNameObjToExecuteHolder(pObj, movementType, calcAnimType, drawBufferType, drawType);
    MR::connectToSceneTemporarily(pObj);
    MR::connectToDrawTemporarily(pObj);
}

void MR::connectToSceneCollisionMapObj(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x1d, 0x2, 0x9, -0x1);
}


void MR::connectToSceneCollisionMapObjWeakLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x1d, 0x2, 0xa, -0x1);
}

void MR::connectToSceneCollisionMapObjStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x1d, 0x2, 0xb, -0x1);
}

void MR::connectToSceneCollisionEnemy(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x1e, 0x3, 0x13, -0x1);
}


void MR::connectToSceneCollisionEnemyStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x1e, 0x3, 0xb, -0x1);
}

void MR::connectToSceneCollisionEnemyNoShadowedMapObjStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x1e, 0x3, 0xd, -0x1);
}

void MR::connectToSceneNpc(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x27, 0x6, 0x11, -0x1);
}

void MR::connectToSceneNpcMovement(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x27, -0x1, -0x1, -0x1);
}

void MR::connectToSceneRide(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x28, 0x7, 0x12, -0x1);
}

void MR::connectToSceneEnemy(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x29, 0x8, 0x13, -0x1);
}

void MR::connectToSceneEnemyMovement(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_Enemy, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneMapObj(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, 0x5, 0x9, -0x1);
}

void MR::connectToSceneMapObjMovement(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_MapObj, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneMapObjMovementCalcAnim(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_MapObj, MR::CalcAnimType_MapObj, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneMapObjNoMovement(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, -0x1, 0x5, 0x9, -0x1);
}

void MR::connectToSceneMapObjNoCalcAnim(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, -0x1, 0x9, -0x1);
}

void MR::connectToSceneMapObjNoCalcAnimStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, -0x1, 0xb, -0x1);
}

void MR::connectToSceneMapObjDecoration(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x22, 0xb, 0x9, -0x1);
}

void MR::connectToSceneMapObjDecorationStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x22, 0xb, 0xb, -0x1);
}

void MR::connectToSceneMapObjDecorationMovement(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_MapObjDecoration, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneMapObjStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, 0x5, 0xb, -0x1);
}


void MR::connectToScenePlanet(LiveActor* pActor) {
    if (MR::isExistIndirectTexture(pActor)) {
        MR::connectToScene(pActor, MR::MovementType_Planet, MR::CalcAnimType_Planet, MR::DrawBufferType_IndirectPlanet, MR::DrawType_None);
    } else {
        MR::connectToScene(pActor, MR::MovementType_Planet, MR::CalcAnimType_Planet, MR::DrawBufferType_Planet, MR::DrawType_None);
    }
}

void MR::connectToSceneEnvironment(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x20, 0x4, 0x7, -0x1);
}

void MR::connectToSceneEnvironmentStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x20, 0x4, 0x8, -0x1);
}

void MR::connectToClippedMapParts(LiveActor* pActor) {
    MR::connectToScene(pActor, MR::MovementType_ClippedMapParts, MR::CalcAnimType_ClippedMapParts, MR::DrawBufferType_ClippedMapParts,
                       MR::DrawType_None);
}

void MR::connectToSceneEnemyDecoration(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x2a, 0xb, 0x14, -0x1);
}

void MR::connectToSceneEnemyDecorationMovement(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_EnemyDecoration, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneEnemyDecorationMovementCalcAnim(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_EnemyDecoration, MR::CalcAnimType_MapObjDecoration, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneItem(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x2b, 0xf, 0xe, -0x1);
}

void MR::connectToSceneItemStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x2b, 0xf, 0x10, -0x1);
}

void MR::connectToSceneIndirectEnemy(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x29, 0x8, 0x1d, -0x1);
}

void MR::connectToSceneIndirectNpc(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x27, 0x6, 0x1c, -0x1);
}

void MR::connectToSceneIndirectMapObj(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, 0x5, 0x1a, -0x1);
}

void MR::connectToSceneIndirectMapObjStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, 0x5, 0x1b, -0x1);
}

void MR::connectToSceneScreenEffectMovement(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_ScreenEffect, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneAreaObj(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_AreaObj, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToScene3DModelFor2D(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0xe, 0xd, 0x29, -0x1);
}

void MR::connectToSceneLayout(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_Layout, MR::CalcAnimType_Layout, MR::DrawBufferType_None, MR::DrawType_Layout);
}

void MR::connectToSceneLayoutTemporarily(NameObj* pObj) {
    MR::connectToScene(pObj, 0xe, 0xd, -0x1, 0x50);
}

void MR::connectToSceneLayoutMovementCalcAnim(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_Layout, MR::CalcAnimType_Layout, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneLayoutDecoration(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_LayoutDecoration, MR::CalcAnimType_LayoutDecoration, MR::DrawBufferType_None,
                       MR::DrawType_LayoutDecoration);
}

void MR::connectToSceneLayoutFileSelect(NameObj* pObj) {
    MR::connectToScene(pObj, 0xe, 0xd, -0x1, 0x51);
}

void MR::connectToSceneTalkLayout(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_Layout, MR::CalcAnimType_Layout, MR::DrawBufferType_None, MR::DrawType_TalkLayout);
}


void MR::connectToSceneWipeLayout(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_WipeLayout, MR::CalcAnimType_Layout, MR::DrawBufferType_None, MR::DrawType_WipeLayout);
}

void MR::connectToSceneLayoutOnPause(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_LayoutOnPause, MR::CalcAnimType_Layout, MR::DrawBufferType_None, MR::DrawType_LayoutOnPause);
}



void MR::connectToSceneLayoutMovement(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_Layout, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneMirrorMapObj(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, 0xc, 0x2c, -0x1);
}

void MR::connectToSceneMirrorMapObjDecoration(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x22, 0xc, 0x2c, -0x1);
}


void MR::connectToSceneCamera(NameObj* pObj) {
    MR::connectToScene(pObj, MR::MovementType_Camera, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

void MR::connectToSceneNoShadowedMapObj(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, 0x5, 0xc, -0x1);
}

void MR::connectToSceneNoShadowedMapObjStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, 0x5, 0xd, -0x1);
}

void MR::connectToSceneNoSilhouettedMapObj(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, 0x5, 0xe, -0x1);
}

void MR::connectToSceneNoSilhouettedMapObjStrongLight(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x21, 0x5, 0x10, -0x1);
}


void MR::connectToSceneNoSilhouettedMapObjStrongLightCalcAnim(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, -0x1, 0x5, 0x10, -0x1);
}

void MR::connectToSceneNoSilhouettedMapObjWeakLightCalcAnim(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, -0x1, 0x5, 0xf, -0x1);
}

void MR::connectToSceneSwitchingScreenEffect(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x10, 0xd, 0x2b, 0x46);
}

void MR::connectToSceneSky(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x23, 0x5, 0x2, -0x1);
}

void MR::connectToSceneAir(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x23, 0x5, 0x3, -0x1);
}

void MR::connectToSceneSun(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x23, 0x5, 0x4, -0x1);
}



void MR::connectToSceneWorldMapMiniObj(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x2f, 0x14, 0x27, -0x1);
}

void MR::connectToSceneWorldMapIcon(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x30, 0x15, 0x28, -0x1);
}

void MR::connectToSceneSuperDreamer(LiveActor* pActor) {
    MR::registerNameObjToExecuteHolder(pActor, 0x2d, 0x6, 0x11, -0x1);
}

void MR::requestMovementOn(NameObj* pObj) {
    NameObjFunction::requestMovementOn(pObj);
}

void MR::requestMovementOn(LiveActor* pActor) {
    NameObjFunction::requestMovementOn(pActor);

    if (MR::isExistEffectKeeper(pActor)) {
        MR::Effect::requestMovementOn(pActor->mEffectKeeper);
    }
}

void MR::requestMovementOn(LayoutActor* pActor) {
    NameObjFunction::requestMovementOn(pActor);
}

void MR::requestMovementOff(NameObj* pObj) {
    NameObjFunction::requestMovementOff(pObj);
}

void MR::requestMovementOff(LiveActor* pActor) {
    NameObjFunction::requestMovementOff(pActor);

    if (MR::isExistEffectKeeper(pActor)) {
        MR::Effect::requestMovementOff(pActor->mEffectKeeper);
    }
}

void MR::requestMovementOff(LayoutActor* pActor) {
    NameObjFunction::requestMovementOff(pActor);
}

NameObjGroup* MR::joinToNameObjGroup(NameObj* pObj, const char* pGroupName) {
    NameObjGroup* pObjGroup;

    pObjGroup = static_cast< NameObjGroup* >(NameObjFinder::find(pGroupName));
    pObjGroup->registerObj(pObj);

    return pObjGroup;
}



void MR::registerPreDrawFunction(const MR::FunctorBase& rFunc, int drawType) {
    SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor()->registerPreDrawFunction(rFunc, drawType);
}

void MR::registerPreDrawFunctionIfNotExist(const MR::FunctorBase& rFunc, int drawType) {
    if (!SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor()->isRegisteredDrawFunction(drawType)) {
        SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor()->registerPreDrawFunction(rFunc, drawType);
    }
}

NameObjAdaptor* MR::createDrawAdaptor(const char* pName, int drawType, const MR::FunctorBase& rFunctor) {
    NameObjAdaptor* pAdaptor;

    pAdaptor = new NameObjAdaptor(pName);
    pAdaptor->connectToDraw(rFunctor);
    MR::connectToScene(pAdaptor, -0x1, -0x1, -0x1, drawType);

    return pAdaptor;
}

NameObjAdaptor* MR::createAdaptorAndConnectToDrawBloomModel(const char* pName, const MR::FunctorBase& rFunctor) {
    NameObjAdaptor* pAdaptor;

    pAdaptor = new NameObjAdaptor(pName);
    pAdaptor->connectToDraw(rFunctor);

    MR::connectToScene(pAdaptor, MR::MovementType_None, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_BloomModel);

    return pAdaptor;
}

void MR::listenNameObjStageSwitchOnAppear(const NameObj* pObj, const StageSwitchCtrl* pSwitchCtrl, const MR::FunctorBase& rOnFunctor) {
    SwitchEventFunctorListener* pListener;

    pListener = new SwitchEventFunctorListener();
    pListener->setOnFunctor(rOnFunctor);

    MR::getSwitchWatcherHolder()->joinSwitchEventListenerAppear(pSwitchCtrl, pListener);
}

void MR::listenNameObjStageSwitchOnOffAppear(const NameObj* pObj, const StageSwitchCtrl* pSwitchCtrl, const MR::FunctorBase& rOnFunctor,
                                             const MR::FunctorBase& rOffFunctor) {
    SwitchEventFunctorListener* pListener;

    pListener = new SwitchEventFunctorListener();
    pListener->setOnFunctor(rOnFunctor);
    pListener->setOffFunctor(rOffFunctor);

    MR::getSwitchWatcherHolder()->joinSwitchEventListenerAppear(pSwitchCtrl, pListener);
}

void MR::listenNameObjStageSwitchOnA(const NameObj* pObj, const StageSwitchCtrl* pSwitchCtrl, const MR::FunctorBase& rOnFunctor) {
    SwitchEventFunctorListener* pListener;

    pListener = new SwitchEventFunctorListener();
    pListener->setOnFunctor(rOnFunctor);

    MR::getSwitchWatcherHolder()->joinSwitchEventListenerA(pSwitchCtrl, pListener);
}

void MR::listenNameObjStageSwitchOnOffA(const NameObj* pObj, const StageSwitchCtrl* pSwitchCtrl, const MR::FunctorBase& rOnFunctor,
                                        const MR::FunctorBase& rOffFunctor) {
    SwitchEventFunctorListener* pListener;

    pListener = new SwitchEventFunctorListener();
    pListener->setOnFunctor(rOnFunctor);
    pListener->setOffFunctor(rOffFunctor);

    MR::getSwitchWatcherHolder()->joinSwitchEventListenerA(pSwitchCtrl, pListener);
}

void MR::listenNameObjStageSwitchOnB(const NameObj* pObj, const StageSwitchCtrl* pSwitchCtrl, const MR::FunctorBase& rOnFunctor) {
    SwitchEventFunctorListener* pListener;

    pListener = new SwitchEventFunctorListener();
    pListener->setOnFunctor(rOnFunctor);

    MR::getSwitchWatcherHolder()->joinSwitchEventListenerB(pSwitchCtrl, pListener);
}

void MR::listenNameObjStageSwitchOffB(const NameObj* pObj, const StageSwitchCtrl* pSwitchCtrl, const MR::FunctorBase& rOffFunctor) {
    SwitchEventFunctorListener* pListener;

    pListener = new SwitchEventFunctorListener();
    pListener->setOffFunctor(rOffFunctor);

    MR::getSwitchWatcherHolder()->joinSwitchEventListenerB(pSwitchCtrl, pListener);
}

void MR::listenNameObjStageSwitchOnOffB(const NameObj* pObj, const StageSwitchCtrl* pSwitchCtrl, const MR::FunctorBase& rOnFunctor,
                                        const MR::FunctorBase& rOffFunctor) {
    SwitchEventFunctorListener* pListener;

    pListener = new SwitchEventFunctorListener();
    pListener->setOnFunctor(rOnFunctor);
    pListener->setOffFunctor(rOffFunctor);

    MR::getSwitchWatcherHolder()->joinSwitchEventListenerB(pSwitchCtrl, pListener);
}

ResourceHolder* MR::createAndAddResourceHolder(const char* pResource) {
    const char* pName = pResource;
    char buf[0x80];
    if (!MR::isEqualSubString(pResource, ".arc")) {
        snprintf(buf, 0x80, "%s.arc", pResource);
        pName = buf;
    }
    return SingletonHolder< ResourceHolderManager >::get()->createAndAdd(pName, nullptr);
}



ResourceHolder* MR::createAndAddResourceHolderStationed(const char* pResource) {
    return SingletonHolder< ResourceHolderManager >::get()->createAndAddRawData(pResource);
}

ResTIMG* MR::loadTexFromArcHelper(const char* pArchive, const char* pFile, void* pHolderOut) {
    ResourceHolder* pHolder = MR::createAndAddResourceHolder(pArchive);
    if (pHolderOut != nullptr) {
        *static_cast< ResourceHolder** >(pHolderOut) = pHolder;
    }

    return static_cast< ResTIMG* >(pHolder->mFileInfoTable->getRes(pFile));
}

const ResTIMG* MR::loadTexFromArc(const char* pArchive, const char* pFile, ResTIMG* pHolderOut) {
    return MR::loadTexFromArcHelper(pArchive, pFile, pHolderOut);
}

const ResTIMG* MR::loadTexFromArc(const char* pName, const ResourceHolder* pHolder) {
    char archive[0x100];
    char file[0x100];
    snprintf(archive, 0x100, "%s.arc", pName);
    snprintf(file, 0x100, "%s.bti", pName);

    return MR::loadTexFromArcHelper(archive, file, const_cast< ResourceHolder* >(pHolder));
}

const ResTIMG* MR::getTextureResource(const ResourceHolder* pHolder, const char* pName) {
    return static_cast< const ResTIMG* >(pHolder->mFileInfoTable->getRes(pName));
}

namespace {
    bool isExistFileInArcLocal(const ResourceHolder* pHolder, const char* pArchive, va_list pFormat) NO_INLINE {
        char buf[0x100];
        vsnprintf(buf, sizeof(buf), pArchive, pFormat);

        return pHolder->mFileInfoTable->isExistRes(buf) != false;
    }
};  // namespace

bool MR::isExistFileInArc(const ResourceHolder* pHolder, const char* pFormat, ...) {
    va_list list;
    va_start(list, pFormat);

    return ::isExistFileInArcLocal(pHolder, pFormat, list);
}

namespace {
    JMapInfo* tryCreateCsvParserLocal(const ResourceHolder* pHolder, const char* pArchive, va_list pFormat) NO_INLINE {
        char buf[0x100];
        vsnprintf(buf, sizeof(buf), pArchive, pFormat);
        if (!MR::isEqualSubString(buf, ".bcsv")) {
            snprintf(buf, sizeof(buf), "%s.bcsv", buf);
        }

        if (!pHolder->mFileInfoTable->isExistRes(buf)) {
            return nullptr;
        }

        JMapInfo* pInfo = new JMapInfo();
        pInfo->attach(pHolder->mFileInfoTable->getRes(buf));

        return pInfo;
    }
};  // namespace

JMapInfo* MR::createCsvParser(const ResourceHolder* pHolder, const char* pFormat, ...) {
    va_list list;
    va_start(list, pFormat);

    return ::tryCreateCsvParserLocal(pHolder, pFormat, list);
}

JMapInfo* MR::createCsvParser(const char* pArchive, const char* pFormat, ...) {
    ResourceHolder* pResourceHolder;
    if (pArchive[0] == '/') {
        pResourceHolder = SingletonHolder<ResourceHolderManager>::get()->createAndAddRawData(pArchive);
    } else {
        pResourceHolder = MR::createAndAddResourceHolder(pArchive);
    }

    return MR::createCsvParser(pResourceHolder, pFormat);
}

JMapInfo* MR::tryCreateCsvParser(const LiveActor* pActor, const char* pFormat, ...) {
    va_list list;
    va_start(list, pFormat);

    return ::tryCreateCsvParserLocal(getResourceHolder(pActor), pFormat, list);
}

JMapInfo* MR::tryCreateCsvParser(const ResourceHolder* pHolder, const char* pFormat, ...) {
    va_list list;
    va_start(list, pFormat);

    return ::tryCreateCsvParserLocal(pHolder, pFormat, list);
}

JMapInfo* MR::createCsvParserFromFile(const char* pArchive, const char* pFormat, ...) {
    va_list list;
    va_start(list, pFormat);

    ResourceHolder* pResourceHolder = MR::createAndAddResourceHolder(pArchive);
    return ::tryCreateCsvParserLocal(pResourceHolder, pFormat, list);
}

// NONMATCHING: equivalent common null-result branch arrangement
const JMapInfo* MR::tryCreateStageCsvParser(const char* pStage, const char* pSuffix, const char* pFormat, va_list list) {
        char fileName[0x100];
        char archive[0x100];
        vsnprintf(fileName, 0x100, pFormat, list);
        if (!MR::isEqualSubString(fileName, ".bcsv")) {
            snprintf(fileName, 0x100, "%s.bcsv", fileName);
        }

        snprintf(archive, 0x100, "/StageData/%s/%s%s.arc", pStage, pStage, pSuffix);
        if (!MR::isFileExist(archive, false)) {
            return nullptr;
        }

        JKRArchive* pArchive = MR::mountArchive(archive, nullptr, true);
        void* pEntry = pArchive->getResource(QUESTIONMARK_MAGIC, fileName);
        if (pEntry == nullptr) {
            return nullptr;
        }

        JMapInfo* pInfo = new JMapInfo();
        pInfo->attach(pEntry);
        return pInfo;
}

const JMapInfo* MR::tryCreateStageZoneInfoCsvParser(const char* pFormat, ...) {
    va_list list;
    va_start(list, pFormat);

    const char* suffix = "ZoneInfo";
    return MR::tryCreateStageCsvParser(MR::getCurrentStageName(), suffix, pFormat, list);
}

const JMapInfo* MR::tryCreateStageUseResourceCsvParser(const char* pStage, const char* pFormat, ...) {
    va_list list;
    va_start(list, pFormat);

    return MR::tryCreateStageCsvParser(pStage, "UseResource", pFormat, list);
}

s32 MR::getCsvDataElementNum(const JMapInfo* pMapInfo) {
    if (pMapInfo->mData != nullptr) {
        return pMapInfo->mData->mNumEntries;
    }

    return 0;
}


void MR::getCsvDataStrOrNULL(const char** pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    getCsvDataStr(pOut, pMapInfo, pKey, idx);

    if (*pOut[0] == 0) {
        *pOut = nullptr;
    }
}

void MR::getCsvDataS32(s32* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    pMapInfo->getValue< s32 >(idx, pKey, pOut);
}

template <>
const bool JMapInfo::getValue<u32>(int idx, const char* pKey, u32* pOut) const {
    s32 item = searchItemInfo(pKey);
    if (item < 0) return false;
    return getValueFast(idx, item, pOut);
}

void MR::getCsvDataS16(s16* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    s32 value = 0;
    pMapInfo->getValue< s32 >(idx, pKey, &value);
    *pOut = static_cast< s16 >(value);
}

void MR::getCsvDataU8(u8* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) NO_INLINE {
    s32 val = 0;
    pMapInfo->getValue< s32 >(idx, pKey, &val);
    *pOut = val;
}

void MR::getCsvDataF32(f32* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    pMapInfo->getValue(idx, pKey, pOut);
}

void MR::getCsvDataBool(bool* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    pMapInfo->getValue(idx, pKey, pOut);
}

void MR::getCsvDataVec(Vec* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    char key[256];
    snprintf(key, sizeof(key), "%sX", pKey);
    getCsvDataF32(&pOut->x, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sY", pKey);
    getCsvDataF32(&pOut->y, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sZ", pKey);
    getCsvDataF32(&pOut->z, pMapInfo, key, idx);
}

void MR::getCsvDataColor(GXColor* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    char key[256];
    snprintf(key, sizeof(key), "%sR", pKey);
    getCsvDataU8(&pOut->r, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sG", pKey);
    getCsvDataU8(&pOut->g, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sB", pKey);
    getCsvDataU8(&pOut->b, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sA", pKey);
    getCsvDataU8(&pOut->a, pMapInfo, key, idx);
}

bool MR::hasCsvItem(const JMapInfo* pMapInfo, const char* pItemName) {
    return pMapInfo->searchItemInfo(pItemName) >= 0;
}

template <>
JMapInfoIter JMapInfo::findElement<s32>(const char* key, s32 search, int start) const NO_INLINE {
    s32 value;
    for (int i = start; i < getNumEntries(); i++) {
        getValue<s32>(i, key, &value);
        if (value == search) return JMapInfoIter(this, i);
    }
    return end();
}

template <>
JMapInfoIter JMapInfo::findElement<u32>(const char* key, u32 search, int start) const NO_INLINE {
    u32 value;
    for (int i = start; i < getNumEntries(); i++) {
        getValue<u32>(i, key, &value);
        if (value == search) return JMapInfoIter(this, i);
    }
    return end();
}

bool MR::isExistElement(const JMapInfo* pMapInfo, const char* pKey, const char* pValue) {
    JMapInfoIter iter = pMapInfo->findElement< const char* >(pKey, pValue, 0);
    return !(pMapInfo->end() == iter);
}

JMapInfoIter MR::getCsvDataStrByElement(const char** pOut, const JMapInfo* pInfo, const char* pKey, const char* value, const char* pColumn) {
    JMapInfoIter iter = pInfo->findElement<const char*>(pKey, value, 0);
    if (pInfo->end() == iter) return iter;
    iter.getValue<const char*>(pColumn, pOut);
    return iter;
}

JMapInfoIter MR::getCsvDataStrByElement(const char** pOut, const JMapInfo* pInfo, const char* pKey, s32 value, const char* pColumn) {
    JMapInfoIter iter = pInfo->findElement<s32>(pKey, value, 0);
    if (pInfo->end() == iter) return iter;
    iter.getValue<const char*>(pColumn, pOut);
    return iter;
}

JMapInfoIter MR::getCsvDataStrByElement(const char** pOut, const JMapInfo* pInfo, const char* pKey, u32 value, const char* pColumn) {
    JMapInfoIter iter = pInfo->findElement<u32>(pKey, value, 0);
    if (pInfo->end() == iter) return iter;
    iter.getValue<const char*>(pColumn, pOut);
    return iter;
}

JMapInfoIter MR::getCsvDataS32ByElement(s32* pOut, const JMapInfo* pInfo, const char* pKey, const char* value, const char* pColumn) {
    JMapInfoIter iter = pInfo->findElement<const char*>(pKey, value, 0);
    if (pInfo->end() == iter) return iter;
    iter.getValue<s32>(pColumn, pOut);
    return iter;
}

JMapInfoIter MR::getCsvDataU32ByElement(u32* pOut, const JMapInfo* pInfo, const char* pKey, const char* value, const char* pColumn) {
    JMapInfoIter iter = pInfo->findElement<const char*>(pKey, value, 0);
    if (pInfo->end() == iter) return iter;
    iter.getValue<u32>(pColumn, pOut);
    return iter;
}

bool MR::isStageStateScenarioOpeningCamera() {
    return GameSceneFunction::isExecScenarioOpeningCamera();
}

bool MR::isStageStatePowerStarAppeared() {
    return GameSequenceFunction::isResultPowerStarAppeared();
}

void MR::declarePowerStar(const NameObj* pObj) {
    declareEventPowerStar(pObj, -1);
}

void MR::declarePowerStar(const NameObj* pObj, s32 id) {
    declareEventPowerStar(pObj, id);
}

void MR::declarePowerStarCoin100() {
    declareEventPowerStar("１００枚コイン", -1);
}

void MR::appearPowerStarContinueCurrentDemo(const NameObj* pObj, const TVec3f& rParam2) {
    appearEventPowerStar(pObj->getName(), -1, &rParam2, true, false, false);
}

void MR::appearPowerStarWithoutDemo(const NameObj* pObj) {
    appearEventPowerStar(pObj->getName(), -1, nullptr, false, true, false);
}

void MR::requestAppearPowerStar(const NameObj* pObj) {
    TVec3f pos(0.0f);
    const char* name = "パワースター出現位置";
    if (getNamePosHolder()->find(name, &pos, nullptr)) {
        appearEventPowerStar(pObj->getName(), -1, &pos, false, false, false);
    } else {
        appearEventPowerStar(pObj->getName(), -1, nullptr, false, false, false);
    }
}

void MR::requestAppearPowerStar(const NameObj* pObj, const TVec3f& rParam2) {
    appearEventPowerStar(pObj->getName(), -1, &rParam2, false, false, false);
}

void MR::requestAppearPowerStar(const NameObj* pObj, s32 param2, const TVec3f& rParam3) {
    appearEventPowerStar(pObj->getName(), param2, &rParam3, false, false, false);
}

void MR::requestAppearPowerStar(const NameObj* pObj, const LiveActor* pActor, f32 flt) {
    MtxPtr baseMtx = pActor->getBaseMtx();
    TVec3f baseMtxY(baseMtx[1][0], baseMtx[1][1], baseMtx[1][2]);
    baseMtxY.scale(flt);
    baseMtxY.add(pActor->mPosition);
    appearEventPowerStar(pObj->getName(), -1, &baseMtxY, false, false, false);
}

void MR::requestAppearPowerStarFromBelow(const NameObj* pObj) {
    appearEventPowerStar(pObj->getName(), -1, nullptr, false, false, true);
}

void MR::requestAppearPowerStarCoin100() {
    appearEventPowerStar("１００枚コイン", -1, nullptr, false, false, false);
}

bool MR::isEndPowerStarAppearDemo(const NameObj* pObj) {
    return isEndEventPowerStarAppearDemo(pObj->getName());
}

ModelObj* MR::createPowerStarDemoModel(const NameObj* pObj, const char* pName, MtxPtr pMtx) {
    ModelObj* pPowerStarDemoModel;
    if (isProductGameScene() && isGrandStarInCurrentStageWithDeclarer(pObj->getName(), -1)) {
        pPowerStarDemoModel = createModelObjIndirectMapObj(pName, "GrandStar", pMtx);
    } else {
        pPowerStarDemoModel = createModelObjNoSilhouettedMapObjStrongLight(pName, "PowerStar", pMtx);
    }

    PowerStar::setupColor(pPowerStarDemoModel, pObj, -1);

    return pPowerStarDemoModel;
}

void MR::declareCoin(const NameObj* pObj, s32 param2) {
    if (param2 <= 0) {
        return;
    }

    MR::createCoinHolder();
    MR::getCoinHolder()->declare(pObj, param2);
}

s32 MR::getDeclareRemnantCoinCount(const NameObj* pObj) {
    return MR::getCoinHolder()->getDeclareRemnantCoinCount(pObj);
}

s32 MR::getUncollectedCoinCount(const NameObj* pObj) {
    return MR::getCoinHolder()->getUncollectedCoinCount(pObj);
}

void MR::hopCoin(const NameObj* pObj, const TVec3f& rParam2, const TVec3f& rParam3) {
    MR::getCoinHolder()->hopCoin(pObj, rParam2, rParam3);
}


void MR::appearCoinPop(const NameObj* pObj, const TVec3f& rParam2, s32 param3) {
    MR::getCoinHolder()->appearCoinPop(pObj, rParam2, param3);
}

void MR::appearCoinPopToDirection(const NameObj* pObj, const TVec3f& rParam2, const TVec3f& rParam3, s32 param4) {
    MR::getCoinHolder()->appearCoinPopToDirection(pObj, rParam2, rParam3, param4);
}

bool MR::appearCoinAgainstGravity(const NameObj* pObj, const TVec3f& rPosition, s32 count) {
    return MR::getCoinHolder()->appearCoinAgainstGravity(pObj, rPosition, count);
}

void MR::appearCoinToVelocity(const NameObj* pObj, const TVec3f& rParam2, const TVec3f& rParam3, s32 param4) {
    MR::getCoinHolder()->appearCoinToVelocity(pObj, rParam2, rParam3, param4);
}

void MR::appearCoinCircle(const NameObj* pObj, const TVec3f& rParam2, s32 param3) {
    MR::getCoinHolder()->appearCoinCircle(pObj, rParam2, param3);
}

void MR::declareStarPiece(const NameObj* pObj, s32 num) {
    getStarPieceDirector()->declare(pObj, num);
}

s32 MR::getDeclareRemnantStarPieceCount(const NameObj* pObj) {
    return getStarPieceDirector()->getDeclareRemnantStarPieceCount(pObj);
}

s32 MR::getDeclareRemnantStarPieceCountNotExist(const NameObj* pObj) {
    return getStarPieceDirector()->getDeclareRemnantStarPieceCountNotExist(pObj);
}

void MR::declareStarPieceReceiver(const NameObj* pObj, s32 num) {
    getStarPieceDirector()->declareReceiveNum(pObj, num);
}

void MR::clearGotCountStarPieceReceiver(const NameObj* pObj) {
    getStarPieceDirector()->clearGotCountReceiver(pObj);
}

void MR::hopStarPiece(const NameObj* pObj, const TVec3f& rVec1, const TVec3f& rVec2) {
    getStarPieceDirector()->hopPiece(pObj, rVec1, rVec2);
}

bool MR::appearStarPiece(const NameObj* pObj, const TVec3f& rVec1, s32 a1, f32 f1, f32 f2, bool a2) {
    return getStarPieceDirector()->appearPiece(pObj, rVec1, a1, f1, f2, false, a2);
}

bool MR::appearStarPieceToDirection(const NameObj* pObj, const TVec3f& rVec1, const TVec3f& rVec2, s32 a1, f32 f1, f32 f2, bool a2) {
    return getStarPieceDirector()->appearPieceToDirection(pObj, rVec1, rVec2, a1, f1, f2, false, a2);
}

bool MR::appearStarPieceToVelocity(const NameObj* pHost, const TVec3f& pos, const TVec3f& velocity, s32 count, bool a, bool b, bool c, f32 angle) {
    return getStarPieceDirector()->appearPieceToDirectionRandom(pHost, pos, velocity, count, angle, a, b, c);
}

void MR::initStarPieceGetCSSound() {
    StarPieceDirector* pDirector = getStarPieceDirector();

    if (pDirector != nullptr) {
        pDirector->initCSSound();
    }
}

BenefitItemOneUp* MR::createKinokoOneUp() {
    BenefitItemOneUp* pKinokoOneUp;

    pKinokoOneUp = new BenefitItemOneUp("１ＵＰキノコ(create)");
    pKinokoOneUp->initWithoutIter();
    pKinokoOneUp->makeActorDead();

    return pKinokoOneUp;
}


// NONMATCHING: floating register allocation and matrix load ordering
void MR::appearKinokoOneUp(BenefitItemObj* pKinoko, MtxPtr pMtx) {
    TPos3f mtx(pMtx);
    MtxPtr m = mtx.toMtxPtr();
    TVec3f angle;

    if (m[2][0] - 1.0f >= -3.814697265625e-6f) {
        angle.x = JMath::sAtanTable.atan2_(-m[0][1], m[1][1]);
        angle.y = -1.5707964f;
        angle.z = 0.0f;
    } else if (m[2][0] + 1.0f <= 3.814697265625e-6f) {
        angle.x = JMath::sAtanTable.atan2_(m[0][1], m[1][1]);
        angle.y = 1.5707964f;
        angle.z = 0.0f;
    } else {
        angle.x = JMath::sAtanTable.atan2_(m[2][1], m[2][2]);
        angle.z = JMath::sAtanTable.atan2_(m[1][0], m[0][0]);
        angle.y = JGeometry::TUtil< f32 >::asin(-m[2][0]);
    }

    TVec3f rot(angle);
    rot.scale(57.29578f);
    setRotation(pKinoko, rot);

    TVec3f pos;
    mtx.getTrans(pos);
    setPosition(pKinoko, pos);
}

void MR::appearKinokoOneUpPop(BenefitItemObj* pKinoko, MtxPtr pMtx) {
    MR::appearKinokoOneUp(pKinoko, pMtx);
    pKinoko->appearPop();
}

void MR::appearKinokoOneUpHop(BenefitItemObj* pKinoko, MtxPtr pMtx) {
    MR::appearKinokoOneUp(pKinoko, pMtx);
    pKinoko->appearHop();
}

BenefitItemLifeUp* MR::createKinokoSuper() {
    BenefitItemLifeUp* pKinokoSuper;

    pKinokoSuper = new BenefitItemLifeUp("スーパーキノコ");
    pKinokoSuper->initWithoutIter();
    pKinokoSuper->makeActorDead();

    return pKinokoSuper;
}


void MR::appearKinokoSuperPop(BenefitItemLifeUp* pKinoko, MtxPtr pMtx) {
    MR::appearKinokoOneUp(pKinoko, pMtx);
    pKinoko->appearPop();
}

void MR::stopScene(s32 frame) {
    MR::getSceneObj< StopSceneController >(SceneObj_StopSceneController)->requestStopScene(frame);
}

void MR::stopSceneForDefaultHit(s32 frame) {
    MR::getSceneObj< StopSceneController >(SceneObj_StopSceneController)->requestStopSceneDelay(frame, 2);
}

bool MR::tryRumblePad(const void* pParam1, const char* pPatternName, s32 channel) {
    return WPadFunction::getWPadRumble(channel)->vibratePatternIfNotExist(pParam1, pPatternName);
}


bool MR::tryRumblePadVeryStrong(const void* pParam1, s32 channel) {
    return WPadFunction::getWPadRumble(channel)->vibratePatternIfNotExist(pParam1, "最強");
}

bool MR::tryRumblePadStrong(const void* pParam1, s32 channel) {
    return WPadFunction::getWPadRumble(channel)->vibratePatternIfNotExist(pParam1, "強");
}

bool MR::tryRumblePadMiddle(const void* pParam1, s32 channel) {
    return WPadFunction::getWPadRumble(channel)->getRumbleInstance()->vibratePatternIfNotExist(pParam1, "中");
}

bool MR::tryRumblePadWeak(const void* pParam1, s32 channel) {
    return WPadFunction::getWPadRumble(channel)->vibratePatternIfNotExist(pParam1, "弱");
}

bool MR::tryRumblePadVeryWeak(const void* pParam1, s32 channel) {
    return WPadFunction::getWPadRumble(channel)->vibratePatternIfNotExist(pParam1, "微弱");
}

// NONMATCHING: equivalent shared true-return block arrangement
bool MR::tryRumblePadWeakAndVeryWeak(const void* pHost, s32 channel) {
    if (WPadFunction::getWPadRumble(channel)->hasRumblePattern(pHost, "微弱")) {
        return false;
    }

    if (WPadFunction::getWPadRumble(channel)->vibratePatternIfNotExist(pHost, "弱") ||
        WPadFunction::getWPadRumble(channel)->hasRumblePattern(pHost, "弱")) {
        return true;
    }
    return WPadFunction::getWPadRumble(channel)->vibratePatternIfNotExist(pHost, "微弱");
}

bool MR::tryRumbleDefaultHit(const void* pParam1, s32 channel) {
    return WPadFunction::getWPadRumble(channel)->vibratePatternIfNotExist(pParam1, "強");
}

void MR::shakeCameraVeryStrong() {
    getCameraDirector()->mShaker->shakeVertical(CameraShaker::SINGLY_VERTICAL_POWER_VERY_STRONG);
}

void MR::shakeCameraStrong() {
    getCameraDirector()->mShaker->shakeVertical(CameraShaker::SINGLY_VERTICAL_POWER_STRONG);
}

void MR::shakeCameraNormalStrong() {
    getCameraDirector()->mShaker->shakeVertical(CameraShaker::SINGLY_VERTICAL_POWER_NORMAL_STRONG);
}

void MR::shakeCameraNormal() {
    getCameraDirector()->mShaker->shakeVertical(CameraShaker::SINGLY_VERTICAL_POWER_NORMAL);
}

void MR::shakeCameraNormalWeak() {
    getCameraDirector()->mShaker->shakeVertical(CameraShaker::SINGLY_VERTICAL_POWER_NORMAL_WEAK);
}

void MR::shakeCameraWeak() {
    getCameraDirector()->mShaker->shakeVertical(CameraShaker::SINGLY_VERTICAL_POWER_WEAK);
}

void MR::shakeCameraVeryWeak() {
    getCameraDirector()->mShaker->shakeVertical(CameraShaker::SINGLY_VERTICAL_POWER_VERY_WEAK);
}

void MR::shakeCameraInfinity(NameObj* pObj, f32 intensity, f32 speed) {
    getCameraDirector()->mShaker->shakeInfinity(pObj, intensity, 15.0f / speed);
}

void MR::stopShakingCamera(NameObj* pObj) {
    getCameraDirector()->mShaker->stopShakingInfinity(pObj);
}

bool MR::isName(const NameObj* pObj, const char* pName) {
    return strcmp(pObj->getName(), pName) == 0;
}

bool MR::isSame(const NameObj* pObj1, const NameObj* pObj2) {
    return pObj1 == pObj2;
}


bool MR::registerNamePosLinkObj(const NameObj* pLinkedObj, const JMapInfoIter& rIter, const char* pPosName) {
    return MR::getNamePosHolder()->tryRegisterLinkObj(pLinkedObj, rIter, pPosName);
}

bool MR::tryRegisterNamePosLinkObj(const NameObj* pLinkedObj, const JMapInfoIter& rIter, const char* pPosName) {
    return MR::getNamePosHolder()->tryRegisterLinkObj(pLinkedObj, rIter, pPosName);
}

bool MR::findNamePos(const char* pName, MtxPtr pMtx) {
    return MR::tryFindNamePos(pName, pMtx);
}

bool MR::findNamePos(const char* pName, TVec3f* pPosition, TVec3f* pRotation) {
    return getNamePosHolder()->find(pName, pPosition, pRotation);
}

// NONMATCHING: vector temporary stack slots and register allocation
void MR::findNamePosOnGround(const char* pName, MtxPtr pMtx) {
    Triangle triangle;
    TPos3f mtx;
    TVec3f hitPos;
    TVec3f position;
    TVec3f gravity;
    TVec3f front;
    tryFindNamePos(pName, mtx.toMtxPtr());
    mtx.getTrans(position);
    mtx.getZDir(front);
    calcGravityVector(nullptr, position, &gravity, nullptr, 0);

    TVec3f offset(gravity);
    offset.scale(100.0f);
    TVec3f start(position);
    start -= offset;
    TVec3f ray(gravity);
    ray.scale(1000.0f);
    if (getFirstPolyOnLineToMap(&hitPos, &triangle, start, ray)) {
        makeMtxUpFrontPos(&mtx, -gravity, front, hitPos);
    }

    PSMTXCopy(mtx.toMtxPtr(), pMtx);
}

bool MR::tryFindNamePos(const char* pName, MtxPtr pMtx) {
    TVec3f position(0.0f, 0.0f, 0.0f);
    TVec3f rotation(0.0f, 0.0f, 0.0f);
    if (getNamePosHolder()->find(pName, &position, &rotation)) {
        makeMtxTR(pMtx, position, rotation);
        return true;
    }
    return false;
}

bool MR::tryFindNamePos(const char* pName, TVec3f* pParam2, TVec3f* pParam3) {
    return getNamePosHolder()->find(pName, pParam2, pParam3);
}

void MR::findLinkNamePos(const NameObj* pObj, const char* pName, MtxPtr pMtx) {
    MR::tryFindLinkNamePos(pObj, pName, pMtx);
}

bool MR::tryFindLinkNamePos(const NameObj* pObj, const char* pName, MtxPtr pMtx) {
    TVec3f position(0.0f, 0.0f, 0.0f);
    TVec3f rotation(0.0f, 0.0f, 0.0f);

    if (getNamePosHolder()->find(pObj, pName, &position, &rotation)) {
        makeMtxTR(pMtx, position, rotation);
        return true;
    }

    return false;
}

bool MR::tryFindLinkNamePos(const NameObj* pObj, const char* pName, TVec3f* pParam3, TVec3f* pParam4) {
    return getNamePosHolder()->find(pObj, pName, pParam3, pParam4);
}

bool LiveActor::receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*) {
    return false;
}

bool LiveActor::receiveMsgPush(HitSensor*, HitSensor*) {
    return false;
}

void LiveActor::updateHitSensor(HitSensor*) {
}



void MR::getCsvDataStr(const char** pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    pMapInfo->getValue< const char* >(idx, pKey, pOut);
}
