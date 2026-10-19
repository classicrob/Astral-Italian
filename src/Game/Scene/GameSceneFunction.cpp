#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace {
    GameScene* getGameScene() NO_INLINE {
        return static_cast< GameScene* >(SingletonHolder< GameSystem >::get()->mSceneController->mScene);
    }
};  // namespace

namespace GameSceneFunction {
    void notifyEndScenarioStarter() {
        ::getGameScene()->notifyEndScenarioStarter();
    }

    void requestPlayMovieDemo() {
        ::getGameScene()->requestPlayMovieDemo();
    }

    void requestStartGameOverDemo() {
        ::getGameScene()->requestStartGameOverDemo();
    }

    void requestEndGameOverDemo() {
        ::getGameScene()->requestEndGameOverDemo();
    }

    void requestEndMissDemo() {
        ::getGameScene()->requestEndMissDemo();
    }

    void requestPowerStarGetDemo() {
        ::getGameScene()->requestPowerStarGetDemo();
    }

    void requestGrandStarGetDemo() {
        ::getGameScene()->requestGrandStarGetDemo();
    }

    void requestStartSuperDreamerTalk() {
        MR::pauseCometTimer();
        ::getGameScene()->requestStartSuperDreamerTalk();
    }

    void requestEndSuperDreamerTalk() {
        MR::resumeCometTimer();
        ::getGameScene()->requestEndSuperDreamerTalk();
    }

    void requestShowGalaxyMap() {
        ::getGameScene()->requestShowGalaxyMap();
    }

    void requestStaffRoll() {
        ::getGameScene()->requestStaffRoll();
    }

    bool isExecScenarioOpeningCamera() {
        return ::getGameScene()->isExecScenarioOpeningCamera();
    }

    bool isExecScenarioStarter() {
        return ::getGameScene()->isExecScenarioStarter();
    }

    bool isExecStageClearDemo() {
        return ::getGameScene()->isExecStageClearDemo();
    }

    bool isExecSuperDreamerTalk() {
        return ::getGameScene()->isExecSuperDreamerTalk();
    }

    void loadScenarioWaveData(bool loadPlayer) {
        SingletonHolder< GameSystem >::get()->mSceneController->loadScenarioWaveData(loadPlayer);
    }

    bool isLoadDoneScenarioWaveData() {
        return SingletonHolder< GameSystem >::get()->mSceneController->isLoadDoneScenarioWaveData();
    }

    bool isExecPauseMenu() {
        return ::getGameScene()->isExecPauseMenu();
    }

    bool isExecAction() {
        return ::getGameScene()->isExecAction();
    }

    void startStagePlayFirst() {
        ::getGameScene()->startStagePlayFirst();
    }

    void startStagePlayRetry() {
        ::getGameScene()->startStagePlayRetry();
    }

    void startStarPointerModeGame() {
        MR::startStarPointerModeGame(::getGameScene());
    }

    void activateDraw3D() {
        GameScene* pGameScene = ::getGameScene();

        pGameScene->mDraw3D = true;
    }

    void deactivateDraw3D() {
        GameScene* pGameScene = ::getGameScene();

        pGameScene->mDraw3D = false;
    }
};  // namespace GameSceneFunction
