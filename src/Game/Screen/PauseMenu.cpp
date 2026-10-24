#include "Game/Screen/PauseMenu.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Screen/AllStarList.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Screen/SysInfoWindow.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

#ifdef GALAXY_NATIVE
// Progress telemetry and the automated smoke run (native/app); observation only.
#include <petari/milestone.hpp>
#define NATIVE_MILESTONE(name) petari_milestone(name)
#else
#define NATIVE_MILESTONE(name)
#endif

namespace {
    const s32 cBackSequenceFadeFrame = 90;

    inline bool isRestrictedStage() {
        return MR::isStageNoPauseMenuStars() || MR::isStageEpilogueDemo() || MR::isStageStoryBook();
    }

    bool isInvalidBackMarioFaceShipOrWorldMap() {
        if (isRestrictedStage()) return true;
        return MR::isStageMarioFaceShipOrWorldMap();
    }
};  // namespace

namespace NrvPauseMenu {
    NEW_NERVE(PauseMenuNrvSelecting, PauseMenu, Selecting);
    NEW_NERVE(PauseMenuNrvDecided, PauseMenu, Decided);
    NEW_NERVE(PauseMenuNrvDisappear, PauseMenu, Disappear);
    NEW_NERVE(PauseMenuNrvConfirm, PauseMenu, Confirm);
    NEW_NERVE(PauseMenuNrvGameDataSave, PauseMenu, GameDataSave);
    NEW_NERVE(PauseMenuNrvLuigiLetter, PauseMenu, AllStarList);
};  // namespace NrvPauseMenu

namespace {
    const TVec2f cUnusedOffsetA(19.0f, 24.0f);
    const TVec2f cUnusedOffsetB(19.0f, 1.0f);
}

PauseMenu::PauseMenu()
    : LayoutActor("ポーズメニュー", true), mUpButton(), mDownButton(), mListButton(), mAllStarList(), mUpButtonPos(0.0f, 0.0f),
      mDownButtonPos(0.0f, 0.0f), mListButtonPos(0.0f, 0.0f), mSysInfoWindow(), mWasDefaultLayoutActive(true), mNoPowerStars(false), mSelectedResult() {
}

void PauseMenu::init(const JMapInfoIter& iter) {
    bool hideStage;
    initLayoutManager("PauseMenu", 1);
    MR::createAndAddPaneCtrl(this, "BG", 1);
    MR::createAndAddPaneCtrl(this, "StageInfo", 1);
    MR::createAndAddPaneCtrl(this, "ButtonPosition", 1);
    MR::createAndAddPaneCtrl(this, "UpButton", 1);
    MR::createAndAddPaneCtrl(this, "DownButton", 1);
    MR::createAndAddPaneCtrl(this, "ListButton", 1);
    MR::connectToSceneLayoutOnPause(this);
    mNoPowerStars = GameDataFunction::calcCurrentPowerStarNum() == 0;
    bool showDown = true;
    bool showList = true;
    if (!isInvalidBackMarioFaceShipOrWorldMap()) {
        if (mNoPowerStars) { showDown = false; showList = false; }
        else { showDown = true; showList = true; }
    }
    if (isRestrictedStage()) { showDown = false; showList = false; }
    mUpButton = new ButtonPaneController(this, "NBackUp", "BoxButton1", 0, true);
    mUpButton->mDisappearOnDecide = false;
    MR::setFollowPos(&mUpButtonPos, this, "UpButton");
    MR::setFollowTypeReplace(this, "UpButton");
    if (showDown) {
        mDownButton = new ButtonPaneController(this, "NBackDown", "BoxButton2", 0, true);
        mDownButton->mDisappearOnDecide = false;
        MR::setFollowPos(&mDownButtonPos, this, "DownButton");
        MR::setFollowTypeReplace(this, "DownButton");
        MR::setTextBoxGameMessageRecursive(this, "NBackDown", isInvalidBackMarioFaceShipOrWorldMap() ? "PauseMenu_ButtonEndGame" : "PauseMenu_ButtonBackWorldMap");
    } else MR::hidePaneRecursive(this, "DownButton");
    if (showList) {
        mListButton = new ButtonPaneController(this, "StarListButton", "BoxButton3", 0, true);
        mListButton->mDisappearOnDecide = false;
        MR::setFollowPos(&mListButtonPos, this, "ListButton");
        MR::setFollowTypeReplace(this, "ListButton");
    } else MR::hidePaneRecursive(this, "ListButton");
    hideStage = false;
    if (MR::isStageNoPauseMenuStars() || MR::isStageMarioFaceShipOrWorldMap() || MR::isStageStoryBook()) hideStage = true;
    if (hideStage) {
        MR::hidePaneRecursive(this, "StageInfo");
    } else {
        MR::setTextBoxMessageRecursive(this, "StageTitle", MR::getCurrentGalaxyNameOnCurrentLanguage());
        MR::setTextBoxMessageRecursive(this, "ScenarioTitle", MR::getCurrentScenarioNameOnCurrentLanguage());
        updateStarPane();
    }
    if (isRestrictedStage()) MR::hidePaneRecursive(this, "World");
    else {
        s32 world = GameDataFunction::getCurrentWorldId();
        if (world == 8) world = 1;
        MR::setTextBoxMessageRecursive(this, "World", MR::getWorldNameOnCurrentLanguage(world));
    }
    mSysInfoWindow = MR::createSysInfoWindow();
    MR::connectToSceneLayoutOnPause(mSysInfoWindow);
    initNerve(GET_NERVE(PauseMenu, PauseMenuNrvSelecting));
    mAllStarList = new AllStarList("オールスターリスト", true);
    mAllStarList->initWithoutIter();
}

void PauseMenu::appear() {
    s32 coinNum = MR::getCoinNum();
    s32 starPieceNum = MR::getStarPieceNum();
    s32 medalNum = GameDataFunction::calcTicoCoinNum();
    mWasDefaultLayoutActive = MR::isActiveDefaultGameLayout();
    if (mWasDefaultLayoutActive) MR::deactivateDefaultGameLayout();
    MR::hideWorldMapInfoLayout();
    if (MR::isStageNoPauseMenuStars()) {
        MR::hidePaneRecursive(this, "ShaStar");
        MR::hidePaneRecursive(this, "ShaMedal");
    } else {
        if (MR::isProductGameScene() && GameSequenceFunction::isCollectTicoCoinSaved()) medalNum++;
        MR::setTextBoxArgNumberRecursive(this, "ShaStar", MR::getPowerStarNum(), 0);
        MR::setTextBoxGameMessageRecursive(this, "ShaMedal", "PauseMenu_Medal");
        MR::setTextBoxArgNumberRecursive(this, "ShaMedal", medalNum, 0);
    }
    MR::setTextBoxArgNumberRecursive(this, "ShaMario", MR::getPlayerLeft(), 0);
    MR::setTextBoxGameMessageRecursive(this, "ShaCoin", MR::isStageMarioFaceShipOrWorldMap() ? "PauseMenu_TotalCoin" : "PauseMenu_Coin");
    MR::setTextBoxArgNumberRecursive(this, "ShaCoin", coinNum, 0);
    MR::setTextBoxGameMessageRecursive(this, "ShaPiece", MR::isStageMarioFaceShipOrWorldMap() ? "PauseMenu_TotalPiece" : "PauseMenu_Piece");
    MR::setTextBoxArgNumberRecursive(this, "ShaPiece", starPieceNum, 0);
    updateStarPane();
    forceToWaitAllButton();
    control();
    restartSelecting();
    LayoutActor::appear();
    MR::incrementPauseBlurDelay();
}

void PauseMenu::kill() {
#ifdef GALAXY_NATIVE
    // Only a menu that was shown closes; init also kills the menu once.
    const bool isClosing = !MR::isDead(this);
#endif
    LayoutActor::kill();
#ifdef GALAXY_NATIVE
    if (isClosing) {
        NATIVE_MILESTONE("PauseMenu.Close");
    }
#endif

    if (mWasDefaultLayoutActive) {
        MR::activateDefaultGameLayout();
    }
}

void PauseMenu::control() {
    MR::copyPaneTrans(&mUpButtonPos, this, "UpButtonPos");
    MR::copyPaneTrans(&mDownButtonPos, this, "DownButtonPos");
    MR::copyPaneTrans(&mListButtonPos, this, "ListButtonPos");
    mUpButton->update();
    if (mDownButton != nullptr) mDownButton->update();
    if (mListButton != nullptr) mListButton->update();
}

void PauseMenu::updateStarPane() {
    if (isInvalidBackMarioFaceShipOrWorldMap()) return;
    if (isRestrictedStage()) return;
    GalaxyStatusAccessor accessor = MR::makeCurrentGalaxyStatusAccessor();
    wchar_t storage[42];
    wchar_t* end;
    wchar_t* buffer = storage;
    if (!mNoPowerStars) {
        end = buffer + MR::makeGalaxyCompletionMark(buffer, 70, accessor.getName(), false);
        MR::makeGalaxyStarList(end, 70 - (end - buffer), accessor.getName(), false, false);
    } else MR::addPictureFontCode(buffer, 0x6E);
    MR::setTextBoxMessageRecursive(this, "ShaStars", storage);
}

void PauseMenu::startPaneAnimWithoutButton(const char* animation) {
    MR::startAnim(this, animation, 0);
    MR::startPaneAnim(this, "BG", animation, 0);
    MR::startPaneAnim(this, "StageInfo", animation, 0);
    f32 frame;
    if (isRestrictedStage()) frame = 0.0f;
    else if (isInvalidBackMarioFaceShipOrWorldMap()) {
        bool noSave = MR::isStageMarioFaceShipOrWorldMap() ? GameSequenceFunction::isNoSaveMode() : false;
        frame = noSave ? 4.0f : 1.0f;
    } else frame = mNoPowerStars ? 3.0f : 2.0f;
    MR::startPaneAnimAndSetFrameAndStop(this, "ButtonPosition", "ButtonPosition", frame, 0);
}

bool PauseMenu::isPaneAnimStoppedWithoutButton() const {
    return MR::isPaneAnimStopped(this, "BG", 0) && MR::isPaneAnimStopped(this, "StageInfo", 0);
}

void PauseMenu::forceToWaitAllButton() {
    mUpButton->forceToWait();

    if (mDownButton != nullptr) {
        mDownButton->forceToWait();
    }

    if (mListButton != nullptr) {
        mListButton->forceToWait();
    }
}

void PauseMenu::restartSelecting() {
    startPaneAnimWithoutButton("Appear");
    mUpButton->appear();
    if (mDownButton) mDownButton->appear();
    if (mListButton) mListButton->appear();
    setNerve(GET_NERVE(PauseMenu, PauseMenuNrvSelecting));
    mSelectedResult = 0;
}

void PauseMenu::exeSelecting() {
    registerPointer();
    bool isPointingTrigger = mUpButton->isPointingTrigger() || (mDownButton != nullptr && mDownButton->isPointingTrigger()) ||
                             (mListButton != nullptr && mListButton->isPointingTrigger());

    if (isPointingTrigger) {
        MR::startSystemSE("SE_SY_SELECT_PAUSE_ITEM");
    }

    if (mUpButton->trySelect() || (mDownButton != nullptr && mDownButton->trySelect()) || (mListButton != nullptr && mListButton->trySelect())) {
        setNerve(GET_NERVE(PauseMenu, PauseMenuNrvDecided));
        return;
    }

    bool isAnimStopped = isPaneAnimStoppedWithoutButton() && !mUpButton->isAppearing() && (mDownButton == nullptr || !mDownButton->isAppearing()) &&
                         (mListButton == nullptr || !mListButton->isAppearing());

    if (isAnimStopped && (MR::testCorePadTriggerPlus(0) || MR::testCorePadTriggerMinus(0))) {
        MR::startSystemSE("SE_SY_PAUSE_OFF");
        MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        setNerve(GET_NERVE(PauseMenu, PauseMenuNrvDisappear));
    }
}

void PauseMenu::exeDecided() {
    if (mUpButton->mIsSelected) {
        if (mUpButton->isTimingForSelectedSe()) {
            MR::startSystemSE("SE_SY_PAUSE_OFF");
            MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        }

        if (mUpButton->isDecidedWait()) {
            mSelectedResult = 0;
            setNerve(GET_NERVE(PauseMenu, PauseMenuNrvDisappear));
        }
    } else if (mDownButton != nullptr && mDownButton->mIsSelected) {
        if (mDownButton->isTimingForSelectedSe()) {
            MR::startSystemSE("SE_SY_PAUSE_OFF");
            MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        }

        if (mDownButton->isDecidedWait()) {
            if (isInvalidBackMarioFaceShipOrWorldMap()) {
                setNerve(GET_NERVE(PauseMenu, PauseMenuNrvGameDataSave));
            } else {
                setNerve(GET_NERVE(PauseMenu, PauseMenuNrvConfirm));
            }
        }
    } else {
        if (mListButton != nullptr && mListButton->mIsSelected) {
            if (mListButton->isTimingForSelectedSe()) {
                MR::startSystemSE("SE_SY_PAUSE_DECIDE_S_RESULT");
                MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
            }

            if (mListButton->isDecidedWait()) {
                mSelectedResult = 1;
                setNerve(GET_NERVE(PauseMenu, PauseMenuNrvDisappear));
            }
        }
    }
}

void PauseMenu::exeDisappear() {
    if (MR::isFirstStep(this)) {
        if (mSelectedResult != 1) MR::decrementPauseBlurDelay();
        startPaneAnimWithoutButton("End");
        mUpButton->disappear();

        if (mDownButton != nullptr) {
            mDownButton->disappear();
        }

        if (mListButton != nullptr) {
            mListButton->disappear();
        }
    }

    if (isPaneAnimStoppedWithoutButton() && mUpButton->isHidden() && (mDownButton == nullptr || mDownButton->isHidden()) && (mListButton == nullptr || mListButton->isHidden())) {
        if (mSelectedResult == 1) setNerve(GET_NERVE(PauseMenu, PauseMenuNrvLuigiLetter));
        else kill();
    }
}

void PauseMenu::exeConfirm() {
    if (MR::isFirstStep(this)) {
        const char* pMessageId = isInvalidBackMarioFaceShipOrWorldMap() ? "PauseMenu_ConfirmEndGame" : "PauseMenu_ConfirmBackWorldMap";

        mSysInfoWindow->appear(pMessageId, SysInfoWindow::Type_YesNo, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
    }

    if (MR::isDead(mSysInfoWindow)) {
        if (!mSysInfoWindow->isSelectedYes()) {
            forceToWaitAllButton();
            setNerve(GET_NERVE(PauseMenu, PauseMenuNrvSelecting));
            return;
        }
        MR::startSystemWipeCircleWithCaptureScreen(::cBackSequenceFadeFrame);
        if (isInvalidBackMarioFaceShipOrWorldMap()) GameSequenceFunction::notifyToGameSequenceProgressToEndScene();
        else GameSequenceFunction::requestChangeStageWorldMap();
    }
}

void PauseMenu::exeGameDataSave() {
    if (MR::isFirstStep(this)) {
        GameSequenceFunction::startGameDataSaveSequence(true, true, false);
    }

    if (GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        return;
    }

    if (GameSequenceFunction::isSuccessSaveDataHandleSequence()) {
        setNerve(GET_NERVE(PauseMenu, PauseMenuNrvConfirm));
    } else {
        forceToWaitAllButton();
        setNerve(GET_NERVE(PauseMenu, PauseMenuNrvSelecting));
    }
}

void PauseMenu::exeAllStarList() {
    if (MR::isFirstStep(this)) mAllStarList->appear();
    if (MR::isDead(mAllStarList)) {
        if (mAllStarList->mResult == 0) restartSelecting();
        else { mSelectedResult = 0; kill(); }
    }
}


void PauseMenu::registerPointer() {
    if (StarPointerUtil::beginMovePositionRegistration(this, true)) {
        StarPointerUtil::addStarPointerMovePositionFromPane(this, "BoxButton1", StarPointerUtil::getDefaultButtonOffsetVec2());
        if (mDownButton) StarPointerUtil::addStarPointerMovePositionFromPane(this, "BoxButton2", StarPointerUtil::getDefaultButtonOffsetVec2());
        if (mListButton) StarPointerUtil::addStarPointerMovePositionFromPane(this, "BoxButton3", StarPointerUtil::getDefaultButtonOffsetVec2());
        if (mDownButton) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton1", "BoxButton2");
        else if (mListButton) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton1", "BoxButton3");
        if (mListButton) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton3", "BoxButton1");
        else if (mDownButton) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton2", "BoxButton1");
        if (mDownButton && mListButton) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton2", "BoxButton3");
        StarPointerUtil::setDefaultAllMovePosition("BoxButton1");
    }
    StarPointerUtil::refreshMovePositionsFromLayout(this);
    StarPointerUtil::endMovePositionRegistration(this);
}
