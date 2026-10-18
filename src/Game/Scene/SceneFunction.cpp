#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/NameObj/NameObjListExecutor.hpp"
#include "Game/Scene/SceneDataInitializer.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SystemUtil.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/Scene/IntermissionScene.hpp"
#include "Game/Scene/LogoScene.hpp"
#include "Game/Scene/SceneFactory.hpp"
#include <algorithm>
#include <cstring>
#include "Game/Map/Air.hpp"
#include "Game/MapObj/SpiderThread.hpp"
#include "Game/MapObj/SpinDriverPathDrawer.hpp"
#include "Game/Scene/SceneNameObjMovementController.hpp"
#include "Game/Scene/StopSceneController.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include <JSystem/J3DGraphBase/J3DSys.hpp>

namespace {
    SceneDataInitializer* getSceneDataInitializer() {
        return MR::getSceneObj< SceneDataInitializer >(SceneObj_SceneDataInitializer);
    }
};  // namespace

void SceneFunction::movementStopSceneController() {
    MR::getSceneObj< StopSceneController >(SceneObj_StopSceneController)->movement();
}

void SceneFunction::executeCalcAnimList() {
    MR::loadViewMtx();
    CategoryList::execute(MR::CalcAnimType_Environment);
    CategoryList::execute(MR::CalcAnimType_MapObj);
    CategoryList::execute(MR::CalcAnimType_NPC);
    CategoryList::execute(MR::CalcAnimType_Ride);
    CategoryList::execute(MR::CalcAnimType_Enemy);
    CategoryList::execute(MR::CalcAnimType_Player);
    CategoryList::execute(MR::CalcAnimType_PlayerDecoration);
    CategoryList::execute(MR::CalcAnimType_MapObjDecoration);
    CategoryList::execute(MR::CalcAnimType_Item);
    CategoryList::execute(MR::CalcAnimType_Layout);
    CategoryList::execute(MR::CalcAnimType_LayoutDecoration);
    CategoryList::execute(MR::CalcAnimType_UNK_0x12);
    CategoryList::execute(MR::CalcAnimType_MirrorMapObj);
    CategoryList::execute(MR::MovementType_ShadowControllerHolder);
    CategoryList::execute(MR::CalcAnimType_AnimParticle);
}

void SceneFunction::executeCalcViewAndEntryList() {
    TMtx34f mtx;
    mtx.identity();
    PSMTXCopy(mtx, j3dSys.mViewMtx);
    CategoryList::entryDrawBuffer2D();
    MR::loadViewMtx();
    CategoryList::entryDrawBuffer3D();
}

void SceneFunction::executeCalcViewAndEntryList2D() {
    TMtx34f mtx;
    mtx.identity();
    PSMTXCopy(mtx, j3dSys.mViewMtx);
    CategoryList::entryDrawBuffer2D();
}

void SceneFunction::executeDrawBufferListNormalOpaBeforeVolumeShadow() {
    CategoryList::drawOpa(MR::DrawBufferType_CrystalItem);
    CategoryList::drawXlu(MR::DrawBufferType_CrystalItem);
    CategoryList::drawOpa(MR::DrawBufferType_Crystal);
    CategoryList::drawXlu(MR::DrawBufferType_Crystal);
    CategoryList::drawOpa(MR::DrawBufferType_AstroDomeSky);
    CategoryList::drawXlu(MR::DrawBufferType_AstroDomeSky);

    if (MR::isExistPriorDrawAir()) {
        CategoryList::drawOpa(MR::DrawBufferType_Sky);
        CategoryList::drawOpa(MR::DrawBufferType_Air);
        CategoryList::drawOpa(MR::DrawBufferType_Sun);
        CategoryList::drawXlu(MR::DrawBufferType_Sky);
        CategoryList::drawXlu(MR::DrawBufferType_Air);
        CategoryList::drawXlu(MR::DrawBufferType_Sun);
    }

    if (MR::isDrawSpinDriverPathAtOpa()) {
        MR::reinitGX();
        CategoryList::execute(MR::DrawType_SpinDriverPathDrawer);
    }

    CategoryList::drawOpa(MR::DrawBufferType_Planet);
    CategoryList::drawOpa(MR::DrawBufferType_PlanetLow);
    CategoryList::execute(MR::DrawType_FlexibleSphere);
    CategoryList::drawOpa(MR::DrawBufferType_Environment);
    CategoryList::drawOpa(MR::DrawBufferType_MapObj);
    CategoryList::drawOpa(MR::DrawBufferType_MapObjWeakLight);
    CategoryList::drawOpa(MR::DrawBufferType_MapObjStrongLight);
    CategoryList::drawOpa(MR::DrawBufferType_TripodBoss);
}

void SceneFunction::executeDrawBufferListNormalOpaBeforeSilhouette() {
    CategoryList::drawOpa(MR::DrawBufferType_NoShadowedMapObj);
    CategoryList::drawOpa(MR::DrawBufferType_NoShadowedMapObjStrongLight);
}

void SceneFunction::executeDrawBufferListNormalOpa() {
    CategoryList::drawOpa(MR::DrawBufferType_NoSilhouettedMapObj);
    CategoryList::drawOpa(MR::DrawBufferType_NoSilhouettedMapObjWeakLight);
    CategoryList::drawOpa(MR::DrawBufferType_NoSilhouettedMapObjStrongLight);
    CategoryList::drawOpa(MR::DrawBufferType_NPC);
    CategoryList::drawOpa(MR::DrawBufferType_Ride);
    CategoryList::drawOpa(MR::DrawBufferType_Enemy);
    CategoryList::drawOpa(MR::DrawBufferType_EnemyDecoration);
    CategoryList::drawOpa(MR::DrawBufferType_PlayerDecoration);

    if (!MR::isExistPriorDrawAir()) {
        CategoryList::drawOpa(MR::DrawBufferType_Sky);
        CategoryList::drawOpa(MR::DrawBufferType_Air);
        CategoryList::drawOpa(MR::DrawBufferType_Sun);
        CategoryList::drawXlu(MR::DrawBufferType_Sky);
        CategoryList::drawXlu(MR::DrawBufferType_Air);
        CategoryList::drawXlu(MR::DrawBufferType_Sun);
    }
}

void SceneFunction::executeDrawBufferListNormalXlu() {
    CategoryList::drawXlu(MR::DrawBufferType_Planet);
    CategoryList::drawXlu(MR::DrawBufferType_PlanetLow);
    CategoryList::drawXlu(MR::DrawBufferType_Environment);
    CategoryList::drawXlu(MR::DrawBufferType_EnvironmentStrongLight);
    CategoryList::drawXlu(MR::DrawBufferType_MapObj);
    CategoryList::drawXlu(MR::DrawBufferType_MapObjWeakLight);
    CategoryList::drawXlu(MR::DrawBufferType_MapObjStrongLight);
    CategoryList::drawXlu(MR::DrawBufferType_TripodBoss);
    CategoryList::drawXlu(MR::DrawBufferType_NoShadowedMapObj);
    CategoryList::drawXlu(MR::DrawBufferType_NoShadowedMapObjStrongLight);
    CategoryList::drawXlu(MR::DrawBufferType_NoSilhouettedMapObj);
    CategoryList::drawXlu(MR::DrawBufferType_NoSilhouettedMapObjWeakLight);
    CategoryList::drawXlu(MR::DrawBufferType_NoSilhouettedMapObjStrongLight);
    CategoryList::drawXlu(MR::DrawBufferType_NPC);
    CategoryList::drawXlu(MR::DrawBufferType_Ride);
    CategoryList::drawXlu(MR::DrawBufferType_Enemy);
    CategoryList::drawXlu(MR::DrawBufferType_EnemyDecoration);
    CategoryList::drawXlu(MR::DrawBufferType_PlayerDecoration);
}

void SceneFunction::executeDrawListOpa() {
    CategoryList::execute(MR::DrawType_0x3);
    MR::reinitGX();
    CategoryList::execute(MR::DrawType_EffectDraw3D);
    CategoryList::execute(MR::DrawType_WhirlPoolAccelerator);
    CategoryList::execute(MR::DrawType_OceanRingPipeOutside);
    CategoryList::execute(MR::DrawType_CaptureScreenCamera);
    CategoryList::execute(MR::DrawType_SwingRope);
    CategoryList::execute(MR::DrawType_0x1);
    CategoryList::execute(MR::DrawType_PlantStalk);
    CategoryList::execute(MR::DrawType_Plant);
    CategoryList::execute(MR::DrawType_Creeper);
    CategoryList::execute(MR::DrawType_Trapeze);
    CategoryList::execute(MR::DrawType_SpaceCocoon);
    CategoryList::execute(MR::DrawType_0x17);
    CategoryList::execute(MR::DrawType_WarpPodPath);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevDirect(GX_TEVSTAGE3);
    CategoryList::execute(MR::DrawType_KirairaChain);
    CategoryList::execute(MR::DrawType_WaterPlant);
    CategoryList::execute(MR::DrawType_BigBubbleGoal);
    CategoryList::execute(MR::DrawType_MiiFacePartsHolder);
    CategoryList::execute(MR::DrawType_AstroDomeOrbit);
    MR::loadLightPlayer();
    CategoryList::execute(MR::DrawType_Player);
    CategoryList::execute(MR::DrawType_DynamicMapObj);
    CategoryList::execute(MR::DrawType_Fur);
    CategoryList::execute(MR::DrawType_OceanSphere);
    CategoryList::execute(MR::DrawType_Flag);
    MR::resetTextureCacheSize();
}

void SceneFunction::executeDrawListXlu() {
    MR::reinitGX();
    CategoryList::execute(MR::DrawType_VolumeModel);
    CategoryList::execute(MR::DrawType_ClipFieldFillDraw);
    if (!MR::isDrawSpinDriverPathAtOpa()) {
        CategoryList::execute(MR::DrawType_SpinDriverPathDrawer);
    }
    CategoryList::execute(MR::DrawType_ClipAreaDropLaser);
    CategoryList::execute(MR::DrawType_SpiderThread);
    CategoryList::execute(MR::DrawType_DashRing);
    MR::resetTextureCacheSize();
}

void SceneFunction::executeDrawSilhouetteAndFillShadow() {
    MR::fillSilhouetteColor();
    MR::loadViewMtx();
    MR::loadProjectionMtx();
}

void SceneFunction::executeDrawAlphaShadow() {
    CategoryList::execute(MR::DrawType_AlphaShadow);
    MR::loadViewMtx();
}

void SceneFunction::executeDrawAfterIndirect() {
    CategoryList::drawOpa(MR::DrawBufferType_IndirectPlanet);
    CategoryList::drawOpa(MR::DrawBufferType_IndirectMapObj);
    CategoryList::drawOpa(MR::DrawBufferType_IndirectMapObjStrongLight);
    CategoryList::drawOpa(MR::DrawBufferType_IndirectNpc);
    CategoryList::drawOpa(MR::DrawBufferType_IndirectEnemy);
    CategoryList::drawOpa(MR::DrawBufferType_GlaringLight);
    CategoryList::drawOpa(MR::DrawBufferType_UNK_0x17);
    CategoryList::drawOpa(MR::DrawBufferType_CrystalBox);
    CategoryList::drawXlu(MR::DrawBufferType_IndirectPlanet);
    CategoryList::drawXlu(MR::DrawBufferType_IndirectMapObj);
    CategoryList::drawXlu(MR::DrawBufferType_IndirectMapObjStrongLight);
    CategoryList::drawXlu(MR::DrawBufferType_IndirectNpc);
    CategoryList::drawXlu(MR::DrawBufferType_IndirectEnemy);
    CategoryList::drawXlu(MR::DrawBufferType_GlaringLight);
    CategoryList::drawXlu(MR::DrawBufferType_UNK_0x17);
    CategoryList::drawXlu(MR::DrawBufferType_CrystalBox);
    MR::reinitGX();
    CategoryList::execute(MR::DrawType_0x11);
    CategoryList::execute(MR::DrawType_GCapture);
    CategoryList::execute(MR::DrawType_WaterRoad);
    CategoryList::execute(MR::DrawType_BigBubble);
    CategoryList::execute(MR::DrawType_ElectricRailHolder);
    CategoryList::execute(MR::DrawType_OceanRing);
    CategoryList::execute(MR::DrawType_OceanBowl);
    CategoryList::execute(MR::DrawType_0x20);
    CategoryList::execute(MR::DrawType_EffectDrawIndirect);
    CategoryList::execute(MR::DrawType_EffectDrawAfterIndirect);
    CategoryList::execute(MR::DrawType_OceanRingPipeInside);
    CategoryList::execute(MR::DrawType_WaterCameraFilter);
    MR::resetTextureCacheSize();
}

void SceneFunction::executeDrawImageEffect() {
    if (MR::isNormalBloomOn()) {
        MR::reinitGX();
        CategoryList::execute(MR::DrawType_BloomEffectPreDraw);

        if (MR::isNormalBloomForEachModel()) {
            CategoryList::drawOpa(MR::DrawBufferType_BloomModel);
            CategoryList::drawXlu(MR::DrawBufferType_BloomModel);
            CategoryList::execute(MR::DrawType_BloomModel);
            CategoryList::execute(MR::DrawType_EffectDrawForBloomEffect);
            MR::drawSpiderThreadBloom();
            CategoryList::execute(MR::DrawType_OceanBowlBloomDrawer);
        }

        CategoryList::execute(MR::DrawType_BloomEffectPostDraw);
    }

    CategoryList::execute(MR::DrawType_ImageEffect);
}

void SceneFunction::executeDrawList2DNormal() {
    MR::drawInitFor2DModel();
    CategoryList::drawOpa(MR::DrawBufferType_Model3DFor2D);
    CategoryList::drawXlu(MR::DrawBufferType_Model3DFor2D);
    CategoryList::execute(MR::DrawType_CometScreenFilter);
    CategoryList::execute(MR::DrawType_GalaxyNamePlate);
    CategoryList::execute(MR::DrawType_Layout);
    CategoryList::execute(MR::DrawType_LayoutDecoration);
    CategoryList::execute(MR::DrawType_CinemaFrame);
    CategoryList::execute(MR::DrawType_TalkLayout);
    CategoryList::execute(MR::DrawType_0x44);
    MR::drawInitFor2DModel();
    CategoryList::drawOpa(MR::DrawBufferType_0x25);
    CategoryList::drawXlu(MR::DrawBufferType_0x25);
    CategoryList::execute(MR::DrawType_EffectDraw2D);
    CategoryList::execute(MR::DrawType_EffectDrawFor2DModel);
}

namespace {
    struct Name2CreateFunc {
        bool operator==(const char* pName) const {
            return strcmp(mName, pName) == 0;
        }

        /* 0x0 */ const char* mName;
        /* 0x4 */ Scene* (*mCreateFunc)();
    };

    template < typename T >
    Scene* createScene() {
        return new T();
    }

    const Name2CreateFunc cCreateTable[] = {
        {"Game", createScene< GameScene >},
        {"Intermission", createScene< IntermissionScene >},
        {"Logo", createScene< LogoScene >},
    };
};  // namespace

namespace MR {
    Scene* createScene(const char* pName) {
        const Name2CreateFunc* pIter = std::find(::cCreateTable, ::cCreateTable + ARRAY_SIZE(::cCreateTable), pName);

        if (pIter == &::cCreateTable[ARRAY_SIZE(::cCreateTable)]) {
            return nullptr;
        }

        return (*pIter->mCreateFunc)();
    }
};  // namespace MR

void SceneFunction::startStageFileLoad() {
    MR::createSceneObj(SceneObj_SceneDataInitializer);
    ::getSceneDataInitializer()->startStageFileLoad();
}

void SceneFunction::waitDoneStageFileLoad() {
    ::getSceneDataInitializer()->waitDoneStageFileLoad();
}

void SceneFunction::startActorFileLoadCommon() {
    ::getSceneDataInitializer()->startActorFileLoadCommon();
}

void SceneFunction::startActorFileLoadScenario() {
    ::getSceneDataInitializer()->startActorFileLoadScenario();
}

void SceneFunction::startActorPlacement() {
    ::getSceneDataInitializer()->startActorPlacement();
}

void SceneFunction::initAfterScenarioSelected() {
    ::getSceneDataInitializer()->startStageFileLoadAfterScenarioSelected();
    ::getSceneDataInitializer()->initAfterScenarioSelected();
}

void SceneFunction::initForNameObj() {
    MR::createSceneObj(SceneObj_NameObjExecuteHolder);
    MR::createSceneObj(SceneObj_StopSceneController);
    MR::createSceneObj(SceneObj_SceneNameObjMovementController);
}

void SceneFunction::initForLiveActor() {
    MR::createSceneObj(SceneObj_AllLiveActorGroup);
    MR::createSceneObj(SceneObj_ClippingDirector);
    MR::createSceneObj(SceneObj_DemoDirector);
    MR::createSceneObj(SceneObj_SensorHitChecker);
    MR::createSceneObj(SceneObj_CollisionDirector);
    MR::createSceneObj(SceneObj_MessageSensorHolder);
    MR::createSceneObj(SceneObj_LiveActorGroupArray);
    MR::createSceneObj(SceneObj_MovementOnOffGroupHolder);
    MR::createSceneObj(SceneObj_LightDirector);
    MR::createSceneObj(SceneObj_AreaObjContainer);
    MR::createSceneObj(SceneObj_CaptureScreenActor);
    MR::createSceneObj(SceneObj_StageSwitchContainer);
    MR::createSceneObj(SceneObj_SwitchWatcherHolder);
    MR::createSceneObj(SceneObj_SleepControllerHolder);
    MR::createSceneObj(SceneObj_TalkDirector);
    MR::createSceneObj(SceneObj_NPCDirector);
}

void SceneFunction::initEffectSystem(u32 a1, u32 a2) {
    MR::createSceneObj(SceneObj_EffectSystem);
    MR::getEffectSystem()->entry(MR::getParticleResourceHolder(), a1, a2);
}

void SceneFunction::allocateDrawBufferActorList() {
    NameObjListExecutor* pListExecutor = SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor();

    pListExecutor->allocateDrawBufferActorList();
    MR::initConnectting();
}

void CategoryList::execute(MR::MovementType type) {
    NameObjListExecutor* pListExecutor = SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor();

    pListExecutor->executeMovement(type);
}

void CategoryList::execute(MR::CalcAnimType type) {
    NameObjListExecutor* pListExecutor = SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor();

    pListExecutor->executeCalcAnim(type);
}

void CategoryList::execute(MR::DrawType type) {
    NameObjListExecutor* pListExecutor = SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor();

    pListExecutor->executeDraw(type);
}

void CategoryList::entryDrawBuffer2D() {
    NameObjListExecutor* pListExecutor = SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor();

    pListExecutor->entryDrawBuffer2D();
}

void CategoryList::entryDrawBuffer3D() {
    NameObjListExecutor* pListExecutor = SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor();

    pListExecutor->entryDrawBuffer3D();
}

void CategoryList::entryDrawBufferMirror() {
    NameObjListExecutor* pListExecutor = SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor();

    pListExecutor->entryDrawBufferMirror();
}

void CategoryList::drawOpa(MR::DrawBufferType type) {
    NameObjListExecutor* pListExecutor = SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor();

    pListExecutor->drawOpa(type);
}

void CategoryList::drawXlu(MR::DrawBufferType type) {
    NameObjListExecutor* pListExecutor = SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor();

    pListExecutor->drawXlu(type);
}

void CategoryList::requestMovementOn(MR::MovementType type) {
    MR::requestMovementOnWithCategory(type);
}

void CategoryList::requestMovementOff(MR::MovementType type) {
    MR::requestMovementOffWithCategory(type);
}

void SceneFunction::createHioBasicNode(Scene*) {
}
