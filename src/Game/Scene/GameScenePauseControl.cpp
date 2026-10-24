#include "Game/Scene/GameScenePauseControl.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/Screen/GamePauseSequence.hpp"
#include "Game/System/PauseButtonCheckerInGame.hpp"
#include "Game/Util/SceneUtil.hpp"

namespace NrvGameScenePauseControl {
    NEW_NERVE(GameScenePauseControlNrvNormal, GameScenePauseControl, Normal);
};  // namespace

GameScenePauseControl::GameScenePauseControl(GameScene* pScene) : NerveExecutor("GameSceneポーズ制御") {
    mScene = pScene;
    mPauseChecker = nullptr;
    mPauseMenuOff = false;
    mPauseMenuNerve = nullptr;
    initNerve(&NrvGameScenePauseControl::GameScenePauseControlNrvNormal::sInstance);
    mPauseChecker = new PauseButtonCheckerInGame();
}

void GameScenePauseControl::registerNervePauseMenu(const Nerve* pNerve) {
    mPauseMenuNerve = pNerve;
}

void GameScenePauseControl::requestPauseMenuOff() {
    mPauseMenuOff = true;
}

void GameScenePauseControl::exeNormal() {
    tryStartPauseMenu();

    if (mPauseMenuOff) {
        AudWrap::getSystem()->exitPauseMenu();
        mScene->setNerveAfterPauseMenu();
        mPauseMenuOff = false;
        mScene->mPauseSeq->deactivate();
    }
}

bool GameScenePauseControl::tryStartPauseMenu() {
    if (!mScene->permitsStartPauseMenu() || MR::isStageDisablePauseMenu()) {
        return false;
    }

    mPauseChecker->update();

    if (mPauseChecker->isPermitToMinusPause()) {
        startPauseMenu();
        return true;
    }

    if (mPauseChecker->isPermitToPlusPause()) {
        startPauseMenu();
        return true;
    }

    return false;
}

void GameScenePauseControl::startPauseMenu() {
    mScene->mPauseSeq->startPause(GamePauseSequence::ActivePause);
    mScene->setNerve(mPauseMenuNerve);
}

GameScenePauseControl::~GameScenePauseControl() {
}
