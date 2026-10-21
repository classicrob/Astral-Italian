// The game's phase for the frame-time statistics (host.hpp). SDK side: no
// Aurora or SDL headers here. Reads the same scene-controller values as the
// smoke observation (smoke_game.cpp), on the game thread while it holds the CPU.

#include <cstring>

#include "Game/AudioLib/AudSystem.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"

#include "host.hpp"

namespace PetariNative::App::Host {

FrameStats::Phase framePhase() {
    static bool sceneReadyOnce = false;
    const GameSystem* pGameSystem = SingletonHolder< GameSystem >::get();
    const GameSystemSceneController* pController = pGameSystem != nullptr ? pGameSystem->mSceneController : nullptr;
    if (pController == nullptr || !pController->isSceneInitializeState(SceneInitializeState_End)) {
        return sceneReadyOnce ? FrameStats::Phase::Loading : FrameStats::Phase::Startup;
    }
    sceneReadyOnce = true;
    const SceneControlInfo& info = pController->mCurrSceneControlInfo;
    if (std::strcmp(info.mScene, "Game") == 0 && info.mStage[0] != '\0' && std::strcmp(info.mStage, "FileSelect") != 0) {
        return FrameStats::Phase::Gameplay;
    }
    return FrameStats::Phase::Menu;
}

bool playerInPlay() {
    // The game's own "ready for play" test (launch_stage.cpp), on a stage that is
    // not the file select or a world map, with no pause menu or system talk.
    const GameSystem* pGameSystem = SingletonHolder< GameSystem >::get();
    const GameSystemSceneController* pController = pGameSystem != nullptr ? pGameSystem->mSceneController : nullptr;
    if (pController == nullptr || pController->mScene == nullptr || !pController->isSceneInitializeState(SceneInitializeState_End)) {
        return false;
    }
    const SceneControlInfo& info = pController->mCurrSceneControlInfo;
    if (std::strcmp(info.mScene, "Game") != 0 || info.mStage[0] == '\0' || std::strcmp(info.mStage, "FileSelect") == 0 ||
        std::strncmp(info.mStage, "WorldMap", 8) == 0) {
        return false;
    }
    const GameScene* pScene = static_cast< const GameScene* >(pController->mScene);
    return pScene->isExecAction() && pScene->permitsStartPauseMenu() && !MR::isDemoActive() && !MR::isSystemTalking() &&
           !pauseMenuActive();
}

bool pauseMenuActive() {
    const AudSystem* pAudSystem = AudSystem::get();
    return pAudSystem != nullptr && pAudSystem->isPauseMenuActive();
}

}  // namespace PetariNative::App::Host
