#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/System/GalaxyMoveArgument.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SystemUtil.hpp"

namespace MR {
    bool isStageStateScenarioOpeningCamera() {
        return GameSceneFunction::isExecScenarioOpeningCamera();
    }

    bool isStageStatePowerStarAppeared() {
        return GameSequenceFunction::isResultPowerStarAppeared();
    }



    inline void requestChangeStageInGameMoving(const char* pStageName, s32 scenarioNo) {
        requestChangeStageInGameMoving(pStageName, scenarioNo, getInitializeStartIdInfo());
    }







    bool isExecScenarioStarter() {
        return GameSceneFunction::isExecScenarioStarter();
    }

    void requestPowerStarGetDemo() {
        GameSceneFunction::requestPowerStarGetDemo();
    }

    void requestGrandStarGetDemo() {
        GameSceneFunction::requestGrandStarGetDemo();
    }

    void requestStartGameOverDemo() {
        GameSceneFunction::requestStartGameOverDemo();
    }

    void requestEndGameOverDemo() {
        GameSceneFunction::requestEndGameOverDemo();
    }

    void requestEndMissDemo() {
        GameSceneFunction::requestEndMissDemo();
    }


    void executeOnWelcomeAndRetry() {
        startGalaxyCometEvent();
        requestForceAppearHPMeter();
    }


};  // namespace MR
