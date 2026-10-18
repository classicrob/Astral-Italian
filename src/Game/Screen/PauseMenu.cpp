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
    : LayoutActor("ポーズメニュー", true), _20(), _24(), _38(), mAllStarList(), mUpButtonPos(0.0f, 0.0f),
      mDownButtonPos(0.0f, 0.0f), mListButtonPos(0.0f, 0.0f), _30(), _54(true), mNoPowerStars(false), mSelectedResult() {
}

void PauseMenu::init(const JMapInfoIter& iter) {
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
    _20 = new ButtonPaneController(this, "NBackUp", "BoxButton1", 0, true);
    _20->_22 = false;
    MR::setFollowPos(&mUpButtonPos, this, "UpButton");
    MR::setFollowTypeReplace(this, "UpButton");
    if (showDown) {
        _24 = new ButtonPaneController(this, "NBackDown", "BoxButton2", 0, true);
        _24->_22 = false;
        MR::setFollowPos(&mDownButtonPos, this, "DownButton");
        MR::setFollowTypeReplace(this, "DownButton");
        MR::setTextBoxGameMessageRecursive(this, "NBackDown", isInvalidBackMarioFaceShipOrWorldMap() ? "PauseMenu_ButtonEndGame" : "PauseMenu_ButtonBackWorldMap");
    } else MR::hidePaneRecursive(this, "DownButton");
    if (showList) {
        _38 = new ButtonPaneController(this, "StarListButton", "BoxButton3", 0, true);
        _38->_22 = false;
        MR::setFollowPos(&mListButtonPos, this, "ListButton");
        MR::setFollowTypeReplace(this, "ListButton");
    } else MR::hidePaneRecursive(this, "ListButton");
    bool hideStage = false;
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
    _30 = MR::createSysInfoWindow();
    MR::connectToSceneLayoutOnPause(_30);
    initNerve(GET_NERVE(PauseMenu, PauseMenuNrvSelecting));
    mAllStarList = new AllStarList("オールスターリスト", true);
    mAllStarList->initWithoutIter();
}

void PauseMenu::appear() {
    s32 coinNum = MR::getCoinNum();
    s32 starPieceNum = MR::getStarPieceNum();
    s32 medalNum = GameDataFunction::calcTicoCoinNum();
    _54 = MR::isActiveDefaultGameLayout();
    if (_54) MR::deactivateDefaultGameLayout();
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
    LayoutActor::kill();

    if (_54) {
        MR::activateDefaultGameLayout();
    }
}

void PauseMenu::control() {
    MR::copyPaneTrans(&mUpButtonPos, this, "UpButtonPos");
    MR::copyPaneTrans(&mDownButtonPos, this, "DownButtonPos");
    MR::copyPaneTrans(&mListButtonPos, this, "ListButtonPos");
    _20->update();
    if (_24 != nullptr) _24->update();
    if (_38 != nullptr) _38->update();
}

// NONMATCHING: local buffer size and pointer register allocation differ.
void PauseMenu::updateStarPane() {
    if (isInvalidBackMarioFaceShipOrWorldMap()) return;
    if (isRestrictedStage()) return;
    GalaxyStatusAccessor accessor = MR::makeCurrentGalaxyStatusAccessor();
    wchar_t buffer[70];
    if (!mNoPowerStars) {
        wchar_t* end = buffer + MR::makeGalaxyCompletionMark(buffer, 70, accessor.getName(), false);
        MR::makeGalaxyStarList(end, 70 - (end - buffer), accessor.getName(), false, false);
    } else MR::addPictureFontCode(buffer, 0x6E);
    MR::setTextBoxMessageRecursive(this, "ShaStars", buffer);
}

// NONMATCHING: boolean temporary allocation and literal addressing differ.
void PauseMenu::startPaneAnimWithoutButton(const char* animation) {
    MR::startAnim(this, animation, 0);
    MR::startPaneAnim(this, "BG", animation, 0);
    MR::startPaneAnim(this, "StageInfo", animation, 0);
    f32 frame;
    if (isRestrictedStage()) frame = 0.0f;
    else if (isInvalidBackMarioFaceShipOrWorldMap()) {
        bool noSave = false;
        if (MR::isStageMarioFaceShipOrWorldMap()) noSave = GameSequenceFunction::isNoSaveMode();
        frame = noSave ? 4.0f : 1.0f;
    } else frame = mNoPowerStars ? 3.0f : 2.0f;
    MR::startPaneAnimAndSetFrameAndStop(this, "ButtonPosition", "ButtonPosition", frame, 0);
}

bool PauseMenu::isPaneAnimStoppedWithoutButton() const {
    return MR::isPaneAnimStopped(this, "BG", 0) && MR::isPaneAnimStopped(this, "StageInfo", 0);
}

void PauseMenu::forceToWaitAllButton() {
    _20->forceToWait();

    if (_24 != nullptr) {
        _24->forceToWait();
    }

    if (_38 != nullptr) {
        _38->forceToWait();
    }
}

void PauseMenu::exeSelecting() {
    registerPointer();
    bool isPointingTrigger = _20->isPointingTrigger() || (_24 != nullptr && _24->isPointingTrigger()) ||
                             (_38 != nullptr && _38->isPointingTrigger());

    if (isPointingTrigger) {
        MR::startSystemSE("SE_SY_SELECT_PAUSE_ITEM");
    }

    if (_20->trySelect() || (_24 != nullptr && _24->trySelect()) || (_38 != nullptr && _38->trySelect())) {
        setNerve(GET_NERVE(PauseMenu, PauseMenuNrvDecided));
        return;
    }

    bool isAnimStopped = isPaneAnimStoppedWithoutButton() && !_20->isAppearing() && (_24 == nullptr || !_24->isAppearing()) &&
                         (_38 == nullptr || !_38->isAppearing());

    if (isAnimStopped && (MR::testCorePadTriggerPlus(0) || MR::testCorePadTriggerMinus(0))) {
        MR::startSystemSE("SE_SY_PAUSE_OFF");
        MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        setNerve(GET_NERVE(PauseMenu, PauseMenuNrvDisappear));
    }
}

void PauseMenu::exeDecided() {
    if (_20->mIsSelected) {
        if (_20->isTimingForSelectedSe()) {
            MR::startSystemSE("SE_SY_PAUSE_OFF");
            MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        }

        if (_20->isDecidedWait()) {
            mSelectedResult = 0;
            setNerve(GET_NERVE(PauseMenu, PauseMenuNrvDisappear));
        }
    } else if (_24 != nullptr && _24->mIsSelected) {
        if (_24->isTimingForSelectedSe()) {
            MR::startSystemSE("SE_SY_PAUSE_OFF");
            MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        }

        if (_24->isDecidedWait()) {
            if (isInvalidBackMarioFaceShipOrWorldMap()) {
                setNerve(GET_NERVE(PauseMenu, PauseMenuNrvGameDataSave));
            } else {
                setNerve(GET_NERVE(PauseMenu, PauseMenuNrvConfirm));
            }
        }
    } else {
        if (_38 != nullptr && _38->mIsSelected) {
            if (_38->isTimingForSelectedSe()) {
                MR::startSystemSE("SE_SY_FILE_SEL_UPPER_DECIDE");
                MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
            }

            if (_38->isDecidedWait()) {
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
        _20->disappear();

        if (_24 != nullptr) {
            _24->disappear();
        }

        if (_38 != nullptr) {
            _38->disappear();
        }
    }

    if (isPaneAnimStoppedWithoutButton() && _20->isHidden() && (_24 == nullptr || _24->isHidden()) && (_38 == nullptr || _38->isHidden())) {
        if (mSelectedResult == 1) setNerve(GET_NERVE(PauseMenu, PauseMenuNrvLuigiLetter));
        else kill();
    }
}

void PauseMenu::exeConfirm() {
    if (MR::isFirstStep(this)) {
        const char* pMessageId = isInvalidBackMarioFaceShipOrWorldMap() ? "PauseMenu_ConfirmEndGame" : "PauseMenu_ConfirmBackWorldMap";

        _30->appear(pMessageId, SysInfoWindow::Type_YesNo, SysInfoWindow::TextPos_Center, SysInfoWindow::MessageType_System);
    }

    if (MR::isDead(_30)) {
        if (!_30->isSelectedYes()) {
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

void PauseMenu::restartSelecting() {
    startPaneAnimWithoutButton("Appear");
    _20->appear();
    if (_24) _24->appear();
    if (_38) _38->appear();
    setNerve(GET_NERVE(PauseMenu, PauseMenuNrvSelecting));
    mSelectedResult = 0;
}

void PauseMenu::registerPointer() {
    if (StarPointerUtil::beginMovePositionRegistration(this, true)) {
        StarPointerUtil::addStarPointerMovePositionFromPane(this, "BoxButton1", StarPointerUtil::getDefaultButtonOffsetVec2());
        if (_24) StarPointerUtil::addStarPointerMovePositionFromPane(this, "BoxButton2", StarPointerUtil::getDefaultButtonOffsetVec2());
        if (_38) StarPointerUtil::addStarPointerMovePositionFromPane(this, "BoxButton3", StarPointerUtil::getDefaultButtonOffsetVec2());
        if (_24) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton1", "BoxButton2");
        else if (_38) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton1", "BoxButton3");
        if (_38) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton3", "BoxButton1");
        else if (_24) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton2", "BoxButton1");
        if (_24 && _38) StarPointerUtil::setConnectionMovePositionDown2Way("BoxButton2", "BoxButton3");
        StarPointerUtil::setDefaultAllMovePosition("BoxButton1");
    }
    StarPointerUtil::refreshMovePositionsFromLayout(this);
    StarPointerUtil::endMovePositionRegistration(this);
}
