#include "Game/Screen/GamePauseSequence.hpp"
#include "Game/Util.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/PauseMenu.hpp"
#include "Game/System/GalaxyMapController.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"


namespace NrvGamePauseSequence {
    NEW_NERVE(GamePauseSequenceNrvDeactive, GamePauseSequence, Deactive);
    NEW_NERVE(GamePauseSequenceNrvActivePauseMenu, GamePauseSequence, ActivePauseMenu);
    NEW_NERVE(GamePauseSequenceNrvSceneInformation, GamePauseSequence, SceneInformation);
}

namespace {
    const TVec2f sPauseMenuOffset(360.0f, 350.0f);
}


GamePauseSequence::GamePauseSequence() : LayoutActor("ポーズ画面管理", true), mMenuType(ActivePause), mPauseMenu(nullptr), mWindowMenuFunc(nullptr) {
}

void GamePauseSequence::init(const JMapInfoIter& rIter) {
    if (!MR::isStageDisablePauseMenu()) {
        mPauseMenu = new PauseMenu();
        mPauseMenu->initWithoutIter();
    }

    initNerve(GET_NERVE_DIRECT(NrvGamePauseSequence, GamePauseSequenceNrvDeactive));
}

void GamePauseSequence::initWindowMenu(const MR::FunctorBase& rFunc) {
    mWindowMenuFunc = rFunc.clone(nullptr);
}

void GamePauseSequence::startPause(MenuType type) {
    AudWrap::getSystem()->enterPauseMenu();
    mMenuType = type;
    appear();
    MR::startStarPointerModePauseMenu(this);
    GameSystemFunction::onPauseBeginAllRumble();
    MR::startSystemSE("SE_SY_PAUSE_ON");
    MR::startCSSound("CS_CLICK_OPEN", nullptr, 0, 0, 1.0f);

    switch (mMenuType) {
    case ActivePause:
        mPauseMenu->appear();
        setNerve(GET_NERVE_DIRECT(NrvGamePauseSequence, GamePauseSequenceNrvActivePauseMenu));
        break;
    case SceneInformation:
        setNerve(GET_NERVE_DIRECT(NrvGamePauseSequence, GamePauseSequenceNrvSceneInformation));
        break;
    }
}

void GamePauseSequence::deactivate() {
    MR::endStarPointerMode(this);
    kill();
    GameSystemFunction::onPauseEndAllRumble();
    setNerve(GET_NERVE_DIRECT(NrvGamePauseSequence, GamePauseSequenceNrvDeactive));
}

void GamePauseSequence::exeDeactive() {
}

void GamePauseSequence::exeActivePauseMenu() {
    if (MR::isDead(mPauseMenu)) {
        (*mWindowMenuFunc)();
    }
}

void GamePauseSequence::exeSceneInformation() {
}
