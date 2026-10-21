// "Ready for play": the game's own test (launch_stage.cpp) held for kPlayFrames frames, shared by the GX app's
// "[play ready]" line / pop-in gating and by tape recording and the headless replay (--sync-play), so all three anchor
// on the same seam. SDK side: game headers only.
#include <algorithm>
#include <cstring>

#include "Game/AudioLib/AudSystem.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"

#include "host.hpp"

namespace PetariNative::App::Host {

namespace {
bool rawPlayerInPlay() {
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
    return pScene->isExecAction() && pScene->permitsStartPauseMenu() && !pScene->isExecScenarioStarter() &&
           !MR::isDemoActive() && !MR::isSystemTalking() && !pauseMenuActive();
}

// The test can pass for a frame at a stage's start before the opening begins, so
// play counts only once it has held for kPlayFrames consecutive frames (0.1 s).
constexpr int kPlayFrames = 6;
int sPlayFrames = 0;
}  // namespace

void updatePlayerInPlay() {
    sPlayFrames = rawPlayerInPlay() ? std::min(sPlayFrames + 1, kPlayFrames) : 0;
}

bool playerInPlay() {
    return sPlayFrames >= kPlayFrames;
}

bool pauseMenuActive() {
    const AudSystem* pAudSystem = AudSystem::get();
    return pAudSystem != nullptr && pAudSystem->isPauseMenuActive();
}

}  // namespace PetariNative::App::Host
