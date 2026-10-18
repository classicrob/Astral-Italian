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
#include "Game/Util/LightUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/J3DGraphBase/J3DSys.hpp>

namespace {
    SceneDataInitializer* getSceneDataInitializer() {
        return MR::getSceneObj< SceneDataInitializer >(SceneObj_SceneDataInitializer);
    }
};  // namespace

void SceneFunction::movementStopSceneController() {
    MR::getSceneObj< StopSceneController >(SceneObj_StopSceneController)->movement();
}

void SceneFunction::executeMovementList() {
    if (!(MR::getSceneObj< StopSceneController >(SceneObj_StopSceneController)->isSceneStopped())) {
        MR::getSceneNameObjMovementController()->movement();
        MR::executeRequirementDisconnectDrawDelay();
        CategoryList::execute(static_cast< MR::MovementType >(0x1));
        CategoryList::execute(static_cast< MR::MovementType >(0x2));
        CategoryList::execute(static_cast< MR::MovementType >(0x14));
        MR::setStarPointerCameraMtxAtGameScene();
        MR::loadViewMtx();
        CategoryList::execute(static_cast< MR::MovementType >(0x4));
        MR::executeRequirementConnectMovement();
        MR::executeRequirementConnectDraw();
        MR::executeRequirementDisconnectMovement();
        MR::executeRequirementDisconnectDraw();
        CategoryList::execute(static_cast< MR::MovementType >(0x3));
        CategoryList::execute(static_cast< MR::MovementType >(0x5));
        CategoryList::execute(static_cast< MR::MovementType >(0x6));
        CategoryList::execute(static_cast< MR::MovementType >(0x7));
        CategoryList::execute(static_cast< MR::MovementType >(0x12));
        CategoryList::execute(static_cast< MR::MovementType >(0xa));
        CategoryList::execute(static_cast< MR::MovementType >(0xb));
        CategoryList::execute(static_cast< MR::MovementType >(0xc));
        CategoryList::execute(static_cast< MR::MovementType >(0x1b));
        CategoryList::execute(static_cast< MR::MovementType >(0x1c));
        CategoryList::execute(static_cast< MR::MovementType >(0x1d));
        CategoryList::execute(static_cast< MR::MovementType >(0x1e));
        CategoryList::execute(static_cast< MR::CalcAnimType >(0x0));
        CategoryList::execute(static_cast< MR::CalcAnimType >(0x1));
        CategoryList::execute(static_cast< MR::CalcAnimType >(0x2));
        CategoryList::execute(static_cast< MR::CalcAnimType >(0x3));
        CategoryList::execute(static_cast< MR::MovementType >(0x1f));
        CategoryList::execute(static_cast< MR::MovementType >(0x20));
        CategoryList::execute(static_cast< MR::MovementType >(0x21));
        CategoryList::execute(static_cast< MR::MovementType >(0x22));
        CategoryList::execute(static_cast< MR::MovementType >(0x13));
        CategoryList::execute(static_cast< MR::MovementType >(0x27));
        CategoryList::execute(static_cast< MR::MovementType >(0x28));
        CategoryList::execute(static_cast< MR::MovementType >(0x2d));
        CategoryList::execute(static_cast< MR::MovementType >(0x24));
        CategoryList::execute(static_cast< MR::MovementType >(0x25));
        CategoryList::execute(static_cast< MR::MovementType >(0x29));
        CategoryList::execute(static_cast< MR::MovementType >(0x2a));
        CategoryList::execute(static_cast< MR::MovementType >(0x2b));
        CategoryList::execute(static_cast< MR::MovementType >(0x26));
        CategoryList::execute(static_cast< MR::MovementType >(0xd));
        CategoryList::execute(static_cast< MR::MovementType >(0xe));
        CategoryList::execute(static_cast< MR::MovementType >(0xf));
        CategoryList::execute(static_cast< MR::MovementType >(0x10));
        CategoryList::execute(static_cast< MR::MovementType >(0x23));
        CategoryList::execute(static_cast< MR::MovementType >(0x15));
        CategoryList::execute(static_cast< MR::MovementType >(0x16));
        CategoryList::execute(static_cast< MR::MovementType >(0x17));
        CategoryList::execute(static_cast< MR::MovementType >(0x9));
        CategoryList::execute(static_cast< MR::MovementType >(0x18));
        CategoryList::execute(static_cast< MR::MovementType >(0x19));
        MR::executeRequirementDisconnectMovement();
        MR::executeRequirementDisconnectDraw();
    }
}

void MR::movementWorldMapBig() {
    CategoryList::execute(static_cast< MR::MovementType >(0x2f));
}

void MR::movementWorldMapMini() {
    CategoryList::execute(static_cast< MR::MovementType >(0x2e));
    CategoryList::execute(static_cast< MR::MovementType >(0x30));
}

void SceneFunction::executeCalcAnimList() {
    MR::loadViewMtx();
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x4));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x5));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x6));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x7));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x8));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x9));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xa));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xb));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xf));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xd));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xe));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x11));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xc));
    CategoryList::execute(static_cast< MR::MovementType >(0x2c));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x12));
}

void MR::calcAnimWorldMapBig() {
    MR::loadViewMtx();
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x14));
}

void MR::calcAnimWorldMapMini() {
    MR::loadViewMtx();
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x15));
}

void SceneFunction::executeCalcAnimListWithWorldMap() {
    MR::loadViewMtx();
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x4));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x5));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x6));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x7));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x8));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x9));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xa));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xb));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xf));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x14));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xd));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xe));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x11));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0xc));
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x12));
}

void MR::entryWorldMapBig() {
    MR::loadViewMtx();
    CategoryList::entryDrawBufferWorldMapBig();
}

void MR::entryWorldMapMini() {
    MR::loadViewMtx();
    CategoryList::entryDrawBufferWorldMapSmall();
}

void SceneFunction::executeDrawAfterImageEffect() {
    MR::clearZBuffer();
    CategoryList::execute(static_cast< MR::DrawType >(0x45));
}

void MR::drawWorldMapBig() {
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x27));
    MR::reinitGX();
    CategoryList::execute(static_cast< MR::DrawType >(0x44));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x27));
}

void MR::drawWorldMapMiniEffects() {
    CategoryList::execute(static_cast< MR::DrawType >(0x55));
}

void MR::drawWorldMapMini() {
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x28));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x28));
    CategoryList::execute(static_cast< MR::DrawType >(0x55));
}

void MR::drawWorldMapInfo() {
    CategoryList::execute(static_cast< MR::DrawType >(0x4f));
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
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x22));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x22));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x21));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x21));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x24));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x24));
    if (!(MR::isExistSceneObj(0x27))) {
        CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x2));
        CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x3));
        CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x4));
        CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x2));
        CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x3));
        CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x4));
    }
    if (!(!MR::isDrawSpinDriverPathAtOpa())) {
        MR::reinitGX();
        CategoryList::execute(static_cast< MR::DrawType >(0x12));
    }
    MR::loadPointLightSub();
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x5));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x6));
    CategoryList::execute(static_cast< MR::DrawType >(0x19));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x7));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x9));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0xa));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0xb));
    CategoryList::execute(static_cast< MR::DrawType >(0x23));
    MR::reinitGX();
    CategoryList::execute(static_cast< MR::DrawType >(0x3));
}

void SceneFunction::executeDrawBufferListNormalOpaBeforeSilhouette() {
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0xc));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0xd));
}

void SceneFunction::executeDrawBufferListNormalOpa() {
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0xe));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0xf));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x10));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x1));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x11));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x12));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x13));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x14));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x16));
    if (!(!MR::isExistSceneObj(0x27))) {
        CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x2));
        CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x3));
        CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x4));
        CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x2));
        CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x3));
        CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x4));
    }
}

void SceneFunction::executeDrawBufferListNormalXlu() {
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x5));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x6));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x7));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x8));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x9));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0xa));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0xb));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0xc));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0xd));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0xe));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0xf));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x10));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x11));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x12));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x13));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x14));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x16));
}

void SceneFunction::executeDrawListOpa() {
    MR::reinitGX();
    CategoryList::execute(static_cast< MR::DrawType >(0xd));
    CategoryList::execute(static_cast< MR::DrawType >(0xa));
    CategoryList::execute(static_cast< MR::DrawType >(0x37));
    CategoryList::execute(static_cast< MR::DrawType >(0x0));
    CategoryList::execute(static_cast< MR::DrawType >(0x1));
    CategoryList::execute(static_cast< MR::DrawType >(0x4));
    CategoryList::execute(static_cast< MR::DrawType >(0x5));
    CategoryList::execute(static_cast< MR::DrawType >(0x2));
    CategoryList::execute(static_cast< MR::DrawType >(0x6));
    CategoryList::execute(static_cast< MR::DrawType >(0x16));
    CategoryList::execute(static_cast< MR::DrawType >(0x17));
    CategoryList::execute(static_cast< MR::DrawType >(0x18));
    GXSetTevDirect(static_cast< GXTevStageID >(0x0));
    GXSetTevDirect(static_cast< GXTevStageID >(0x1));
    GXSetTevDirect(static_cast< GXTevStageID >(0x2));
    GXSetTevDirect(static_cast< GXTevStageID >(0x3));
    CategoryList::execute(static_cast< MR::DrawType >(0x1a));
    CategoryList::execute(static_cast< MR::DrawType >(0x1b));
    CategoryList::execute(static_cast< MR::DrawType >(0x10));
    CategoryList::execute(static_cast< MR::DrawType >(0x28));
    CategoryList::execute(static_cast< MR::DrawType >(0x20));
    CategoryList::execute(static_cast< MR::DrawType >(0x43));
    CategoryList::execute(static_cast< MR::DrawType >(0x41));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x25));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x20));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x20));
    MR::reinitGX();
    MR::loadLightPlayer();
    CategoryList::execute(static_cast< MR::DrawType >(0x25));
    CategoryList::execute(static_cast< MR::DrawType >(0x26));
    CategoryList::execute(static_cast< MR::DrawType >(0x36));
    CategoryList::execute(static_cast< MR::DrawType >(0xb));
    CategoryList::execute(static_cast< MR::DrawType >(0x1e));
    CategoryList::execute(static_cast< MR::DrawType >(0x29));
    MR::reinitGX();
    CategoryList::execute(static_cast< MR::DrawType >(0x1d));
}

void SceneFunction::executeDrawListXlu() {
    MR::reinitGX();
    CategoryList::execute(static_cast< MR::DrawType >(0x35));
    if (!(MR::isDrawSpinDriverPathAtOpa())) {
        CategoryList::execute(static_cast< MR::DrawType >(0x12));
    }
    CategoryList::execute(static_cast< MR::DrawType >(0x22));
    CategoryList::execute(static_cast< MR::DrawType >(0x14));
    CategoryList::execute(static_cast< MR::DrawType >(0x15));
    CategoryList::execute(static_cast< MR::DrawType >(0x3d));
    MR::resetTextureCacheSize();
}

void SceneFunction::executeDrawSilhouetteAndFillShadow() {
    MR::fillSilhouetteColor();
    MR::loadViewMtx();
    MR::loadProjectionMtx();
}

void SceneFunction::executeDrawAlphaShadow() {
    CategoryList::execute(static_cast< MR::DrawType >(0x2d));
    MR::loadViewMtx();
}

void SceneFunction::executeDrawAfterIndirect() {
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x1e));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x1a));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x1b));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x1c));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x1d));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x23));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x18));
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x17));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x1e));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x1a));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x1b));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x1c));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x1d));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x23));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x18));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x17));
    MR::reinitGX();
    CategoryList::execute(static_cast< MR::DrawType >(0x11));
    CategoryList::execute(static_cast< MR::DrawType >(0x13));
    CategoryList::execute(static_cast< MR::DrawType >(0xc));
    CategoryList::execute(static_cast< MR::DrawType >(0xf));
    CategoryList::execute(static_cast< MR::DrawType >(0xe));
    CategoryList::execute(static_cast< MR::DrawType >(0x8));
    CategoryList::execute(static_cast< MR::DrawType >(0x7));
    CategoryList::execute(static_cast< MR::DrawType >(0x21));
    CategoryList::execute(static_cast< MR::DrawType >(0x53));
    CategoryList::execute(static_cast< MR::DrawType >(0x54));
    CategoryList::execute(static_cast< MR::DrawType >(0x9));
    CategoryList::execute(static_cast< MR::DrawType >(0x3f));
    MR::resetTextureCacheSize();
}

void SceneFunction::executeDrawImageEffect() {
    if (!(!MR::isNormalBloomOn())) {
        MR::reinitGX();
        CategoryList::execute(static_cast< MR::DrawType >(0x39));
        if (!(!MR::isNormalBloomForEachModel())) {
            CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x1f));
            CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x1f));
            CategoryList::execute(static_cast< MR::DrawType >(0x3b));
            CategoryList::execute(static_cast< MR::DrawType >(0x58));
            MR::drawSpiderThreadBloom();
            CategoryList::execute(static_cast< MR::DrawType >(0x24));
        }
        CategoryList::execute(static_cast< MR::DrawType >(0x3a));
    }
    CategoryList::execute(static_cast< MR::DrawType >(0x3c));
}

void SceneFunction::executeDrawList2DNormal() {
    MR::drawInitFor2DModel();
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x29));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x29));
    CategoryList::execute(static_cast< MR::DrawType >(0x49));
    CategoryList::execute(static_cast< MR::DrawType >(0x46));
    CategoryList::execute(static_cast< MR::DrawType >(0x4e));
    CategoryList::execute(static_cast< MR::DrawType >(0x47));
    CategoryList::execute(static_cast< MR::DrawType >(0x4d));
    CategoryList::execute(static_cast< MR::DrawType >(0x4b));
    CategoryList::execute(static_cast< MR::DrawType >(0x4c));
    MR::drawInitFor2DModel();
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x2a));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x2a));
    CategoryList::execute(static_cast< MR::DrawType >(0x56));
    CategoryList::execute(static_cast< MR::DrawType >(0x57));
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

void SceneFunction::startActorFileLoadPostCreation() {
    ::getSceneDataInitializer()->startActorFileLoadPostCreation();
}

void SceneFunction::startActorPlacement() {
    ::getSceneDataInitializer()->startActorPlacement();
}

void SceneFunction::initAfterScenarioSelected() {
    ::getSceneDataInitializer()->initAfterScenarioSelected();
}

void SceneFunction::initForNameObj() {
    MR::createSceneObj(SceneObj_NameObjExecuteHolder);
    MR::createSceneObj(SceneObj_StopSceneController);
    MR::createSceneObj(SceneObj_SceneNameObjMovementController);
}

void SceneFunction::initForLiveActor() {
    MR::createSceneObj(0x39);
    MR::createSceneObj(0x2);
    MR::createSceneObj(0x0);
    MR::createSceneObj(0x1);
    MR::createSceneObj(0x9);
    MR::createSceneObj(0xe);
    MR::createSceneObj(0xf);
    MR::createSceneObj(0x6);
    MR::createSceneObj(0xd);
    MR::createSceneObj(0x10);
    MR::createSceneObj(0xa);
    MR::createSceneObj(0xb);
    MR::createSceneObj(0xc);
    MR::createSceneObj(0x1a);
    MR::createSceneObj(0x2b);
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

void CategoryList::entryDrawBufferWorldMapBig() {
    SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor()->entryDrawBufferWorldMapBig();
}

void CategoryList::entryDrawBufferWorldMapSmall() {
    SingletonHolder< GameSystem >::get()->mSceneController->getNameObjListExecutor()->entryDrawBufferWorldMapSmall();
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
