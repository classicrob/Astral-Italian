#include "Game/Scene/GameSceneScenarioOpeningCameraState.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/GameAudio/AudStageBgmTable.hpp"
#include "Game/Demo/ScenarioStarter.hpp"
#include "Game/NameObj/NameObjFinder.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/ScenarioTitle.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace NrvGameSceneScenarioOpeningCameraState {
    NEW_NERVE(GameSceneScenarioOpeningCameraStateNrvWait, GameSceneScenarioOpeningCameraState, Wait);
    NEW_NERVE(GameSceneScenarioOpeningCameraStateNrvPlay, GameSceneScenarioOpeningCameraState, Play);
};  // namespace

GameSceneScenarioOpeningCameraState::GameSceneScenarioOpeningCameraState() : NerveExecutor("シナリオ開始カメラ再生") {
    mBaseMtx.identity();
    initNerve(GET_NERVE(GameSceneScenarioOpeningCameraState, GameSceneScenarioOpeningCameraStateNrvPlay));
    mScenarioTitle = new ScenarioTitle();
    mScenarioTitle->initWithoutIter();
    mScenarioTitle->kill();
}

void GameSceneScenarioOpeningCameraState::update() {
    updateNerve();
}

bool GameSceneScenarioOpeningCameraState::isDone() const {
    return isNerve(GET_NERVE(GameSceneScenarioOpeningCameraState, GameSceneScenarioOpeningCameraStateNrvWait));
}

void GameSceneScenarioOpeningCameraState::start() {
    const char* stage;
    s32 scenario;
    MR::stopSceneForScenarioOpeningCamera();
    MR::hidePlayer();
    MR::startStartAnimCamera();
    MR::startStarPointerModeDemo(this);
    MR::forceOpenWipeFade();
    MR::forceToFrameCinemaFrame();
    MR::startSubBGM("BGM_JG_START_CAMERA_DEMO", false);
    if (AudStageBgmTable::isFirstVisitStart(MR::getCurrentStageName(), MR::getCurrentScenarioNo())) {
        scenario = MR::getCurrentScenarioNo();
        stage = MR::getCurrentStageName();
        // The target passes the scene controller in the ignored first argument.
        MR::startStageBGMFromStageName(reinterpret_cast<const char*>(MR::getCurrentGameSystemSceneController()), stage, scenario, false, true);
    }
    PSMTXCopy(MR::getPlayerBaseMtx(), mBaseMtx);

    TVec3f namePos;

    if (MR::tryFindNamePos("オープニングカメラマリオ座標", &namePos, nullptr)) {
        MR::setPlayerPos("オープニングカメラマリオ座標");
    } else {
        ScenarioStarter* starter = static_cast<ScenarioStarter*>(NameObjFinder::find("シナリオスターター"));
        if (starter != nullptr) {
            TVec3f pos;
            starter->getStartPos(&pos);
            MR::setPlayerPos(pos);
        }
    }
}

void GameSceneScenarioOpeningCameraState::end() {
    MR::stopSeCategories(30);
    MR::stopSubBGM(10);
    MR::playSceneForScenarioOpeningCamera();
    MR::showPlayer();
    MR::endStartAnimCamera();
    MR::endStarPointerMode(this);
    mScenarioTitle->kill();
    if (NameObjFinder::find("シナリオスターター") == nullptr) {
        MR::setPlayerBaseMtx(mBaseMtx);
    }
}

void GameSceneScenarioOpeningCameraState::exeWait() {
}

void GameSceneScenarioOpeningCameraState::exePlay() {
    if (MR::isFirstStep(this)) {
        mScenarioTitle->start();
        MR::deactivateDefaultGameLayout();
        MR::killDestroyCounter();
        start();
    }

    if (trySkipTrigger()) {
        end();
        MR::forceToBlankCinemaFrame();
        MR::forceOpenSystemWipeFade();
        setNerve(GET_NERVE(GameSceneScenarioOpeningCameraState, GameSceneScenarioOpeningCameraStateNrvWait));
    } else {
        if (MR::isStep(this, MR::getStartAnimCameraFrame() - 60)) {
            MR::tryFrameToBlankCinemaFrame();
            mScenarioTitle->end();
        }

        if (MR::isStep(this, MR::getStartAnimCameraFrame() - 20)) {
            AudWrap::getSystem()->set830(30);
        }

        if (MR::isStartAnimCameraEnd() && MR::isStopCinemaFrame()) {
            end();
            setNerve(GET_NERVE(GameSceneScenarioOpeningCameraState, GameSceneScenarioOpeningCameraStateNrvWait));
        }
    }
}

bool GameSceneScenarioOpeningCameraState::trySkipTrigger() const {
    if (MR::isFirstStep(this)) {
        return false;
    }

    return !MR::isAlreadyVisitedCurrentStageAndScenario() ? false : MR::testSystemPadTriggerDecide();
}
