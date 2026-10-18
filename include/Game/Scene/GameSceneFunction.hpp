#pragma once

namespace GameSceneFunction {
    void notifyEndScenarioStarter();
    void requestPlayMovieDemo();
    void requestStartGameOverDemo();
    void requestEndGameOverDemo();
    void requestEndMissDemo();
    void requestPowerStarGetDemo();
    void requestGrandStarGetDemo();
    void requestShowGalaxyMap();
    void requestStaffRoll();
    bool isExecScenarioOpeningCamera();
    bool isExecScenarioStarter();
    bool isExecStageClearDemo();
    void loadScenarioWaveData();
    bool isLoadDoneScenarioWaveData();
    void activateDraw3D();
    void deactivateDraw3D();
    void startStagePlayFirst();
    void startStagePlayRetry();
    bool isExecAction();
    bool isExecPauseMenu();
};  // namespace GameSceneFunction

namespace GameSceneFunction { bool isExecSuperDreamerTalk(); }

namespace GameSceneFunction {
    void requestStartSuperDreamerTalk();
    void requestEndSuperDreamerTalk();
}
