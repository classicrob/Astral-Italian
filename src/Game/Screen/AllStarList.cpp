#include "Game/Screen/AllStarList.hpp"
#include "Game/Screen/BackButton.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataConst.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <wstring.h>

namespace {
    inline bool hasStars(const GalaxyStatusAccessor& accessor) { return accessor.getPowerStarNum(); }
    const char* cGalaxyPanes[] = {"Galaxy1", "Galaxy2", "Galaxy3", "Galaxy4", "Galaxy5", "Galaxy6", "Galaxy7"};
}

namespace NrvAllStarList {
    NEW_NERVE(AllStarListNrvAppear, AllStarList, Appear);
    NEW_NERVE(AllStarListNrvWait, AllStarList, Wait);
    NEW_NERVE(AllStarListNrvSelected, AllStarList, Selected);
    NEW_NERVE(AllStarListNrvPreviousPage, AllStarList, PreviousPage);
    NEW_NERVE(AllStarListNrvPreviousPageOver, AllStarList, PreviousPageOver);
    NEW_NERVE(AllStarListNrvNextPage, AllStarList, NextPage);
    NEW_NERVE(AllStarListNrvNextPageOver, AllStarList, NextPageOver);
    NEW_NERVE(AllStarListNrvDisappear, AllStarList, Disappear);
}

AllStarList::AllStarList(const char* name, bool onPause)
    : LayoutActor(name, true), mRightButton(nullptr), mLeftButton(nullptr), mBackButton(nullptr), mWorldId(0), mResult(0), mOnPause(onPause) {
}

AllStarList::~AllStarList() {
}

void AllStarList::init(const JMapInfoIter&) {
    initLayoutManager("AllStarList", 2);
    MR::createAndAddPaneCtrl(this, "AllStarList", 1);
    MR::createAndAddPaneCtrl(this, "BG", 1);
    if (mOnPause) {
        MR::connectToSceneLayoutOnPause(this);
    } else {
        MR::connectToScene(this, 14, 13, -1, 72);
    }
    mRightButton = new ButtonPaneController(this, "Right", "BoxArrowR", 0, true);
    mRightButton->_22 = false;
    mLeftButton = new ButtonPaneController(this, "Left", "BoxArrowL", 0, true);
    mLeftButton->_22 = false;
    mBackButton = new BackButtonCancelB("オールスターリスト戻るボタン", false);
    if (mOnPause) {
        MR::connectToSceneLayoutOnPause(mBackButton);
    } else {
        MR::connectToScene(mBackButton, 14, 13, -1, 72);
    }
    mBackButton->initWithoutIter();
    mWorldId = 0;
    initNerve(GET_NERVE(AllStarList, AllStarListNrvAppear));
    kill();
}

void AllStarList::appear() {
    MR::startAnim(this, "Appear", 0);
    if (!mOnPause) {
        MR::startStarPointerModeChooseYesNo(this);
    }
    mWorldId = GameDataFunction::getCurrentWorldId();
    if (mWorldId == 8) mWorldId = 1;
    mRightButton->forceToHide();
    mLeftButton->forceToHide();
    updatePageButtons(mWorldId);
    updateContents(mWorldId);
    mBackButton->appear();
    setNerve(GET_NERVE(AllStarList, AllStarListNrvAppear));
    LayoutActor::appear();
    mResult = 0;
    if (!mOnPause) MR::incrementPauseBlurDelay();
}

void AllStarList::control() {
    mRightButton->update();
    mLeftButton->update();
    if (mRightButton->isWait() && mRightButton->getNerveStep() == 1 && mLeftButton->isWait()) {
        MR::setPaneAnimFrame(this, "Right", MR::getPaneAnimFrame(this, "Left", 0), 0);
    }
    if (mLeftButton->isWait() && mLeftButton->getNerveStep() == 1 && mRightButton->isWait()) {
        MR::setPaneAnimFrame(this, "Left", MR::getPaneAnimFrame(this, "Right", 0), 0);
    }
    MR::setSeVolumeSetLevelForMenu();
}

void AllStarList::requestMovementOn() {
    MR::requestMovementOn(this);
    MR::requestMovementOn(mBackButton);
    MR::requestPauseBlurMovementOn();
}

void AllStarList::exeAppear() {
    if (MR::isAnimStopped(this, 0)) startWaiting();
}

// NONMATCHING: literal-pool base and temporary use r30/r31 in the opposite order.
void AllStarList::exeWait() {
    bool pointing;
    if (MR::isFirstStep(this)) {
        const char* anim = "Wait";
        MR::startPaneAnim(this, "AllStarList", anim, 0);
        MR::startPaneAnim(this, "BG", anim, 0);
    }
    pointing = false;
    if (mRightButton->isPointingTrigger() || mLeftButton->isPointingTrigger()) pointing = true;
    if (pointing) {
        MR::startSystemSE("SE_SY_SELECT_PAUSE_ITEM", -1, -1);
    }
    if (mOnPause && MR::testCorePadOnlyMinus(0)) {
        mResult = 1;
        setNerve(GET_NERVE(AllStarList, AllStarListNrvDisappear));
        MR::startSystemSE("SE_SY_PAUSE_OFF", -1, -1);
        MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
        return;
    }
    if (!mBackButton->isAppearing() && !mBackButton->isHidden() && mBackButton->isSelected()) {
        setNerve(GET_NERVE(AllStarList, AllStarListNrvSelected));
        return;
    }
    if (MR::testSystemTriggerLeft()) {
        if (mLeftButton->_24) {
            if (mLeftButton->trySelectImmediately()) MR::startSystemSE("SE_SY_STAR_RESULT_PANEL_SLIDE", -1, -1);
        } else {
            setNerve(GET_NERVE(AllStarList, AllStarListNrvPreviousPageOver));
            MR::startSystemSE("SE_SY_STAR_RESULT_PANEL_NG", -1, -1);
        }
    }
    if (MR::testSystemTriggerRight()) {
        if (mRightButton->_24) {
            if (mRightButton->trySelectImmediately()) MR::startSystemSE("SE_SY_STAR_RESULT_PANEL_SLIDE", -1, -1);
        } else {
            setNerve(GET_NERVE(AllStarList, AllStarListNrvNextPageOver));
            MR::startSystemSE("SE_SY_STAR_RESULT_PANEL_NG", -1, -1);
        }
    }
    if ((!mRightButton->isHidden() && mRightButton->trySelect()) || mRightButton->mIsSelected ||
        (!mLeftButton->isHidden() && mLeftButton->trySelect()) || mLeftButton->mIsSelected) {
        MR::startSystemSE("SE_SY_STAR_RESULT_PANEL_SLIDE", -1, -1);
        setNerve(GET_NERVE(AllStarList, AllStarListNrvSelected));
    }
}

void AllStarList::exeSelected() {
    if (mRightButton->mIsSelected) {
        s32 world = mWorldId;
        bool valid = false;
        if (world + 1 >= 1 && world + 1 <= GameDataFunction::calcLastEnteredWorldNo()) valid = true;
        if (valid) {
            setNerve(GET_NERVE(AllStarList, AllStarListNrvNextPage));
        } else {
            setNerve(GET_NERVE(AllStarList, AllStarListNrvNextPageOver));
        }
        return;
    }
    if (mLeftButton->mIsSelected) {
        s32 world = mWorldId;
        bool valid = false;
        if (world - 1 >= 1 && world - 1 <= GameDataFunction::calcLastEnteredWorldNo()) valid = true;
        if (valid) {
            setNerve(GET_NERVE(AllStarList, AllStarListNrvPreviousPage));
        } else {
            setNerve(GET_NERVE(AllStarList, AllStarListNrvPreviousPageOver));
        }
        return;
    }
    if (mBackButton->isSelected() && mBackButton->isHidden()) {
        setNerve(GET_NERVE(AllStarList, AllStarListNrvDisappear));
    }
}

void AllStarList::exePreviousPage() {
    if (MR::isFirstStep(this)) MR::startPaneAnim(this, "AllStarList", "PreviousPage", 0);
    if (MR::isStep(this, 10)) {
        --mWorldId;
        updatePageButtons(mWorldId);
        updateContents(mWorldId);
    }
    if (MR::isPaneAnimStopped(this, "AllStarList", 0)) startWaiting();
}

void AllStarList::exePreviousPageOver() {
    if (MR::isFirstStep(this)) {
        const char* anim = "PreviousPageOver";
        MR::startPaneAnim(this, "AllStarList", anim, 0);
        MR::startPaneAnim(this, "BG", anim, 0);
    } else if (MR::isPaneAnimStopped(this, "AllStarList", 0)) {
        startWaiting();
    }
}

void AllStarList::exeNextPage() {
    if (MR::isFirstStep(this)) MR::startPaneAnim(this, "AllStarList", "NextPage", 0);
    if (MR::isStep(this, 10)) {
        ++mWorldId;
        updatePageButtons(mWorldId);
        updateContents(mWorldId);
    }
    if (MR::isPaneAnimStopped(this, "AllStarList", 0)) startWaiting();
}

void AllStarList::exeNextPageOver() {
    if (MR::isFirstStep(this)) {
        const char* anim = "NextPageOver";
        MR::startPaneAnim(this, "AllStarList", anim, 0);
        MR::startPaneAnim(this, "BG", anim, 0);
    }
    if (MR::isPaneAnimStopped(this, "AllStarList", 0)) startWaiting();
}

void AllStarList::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "End", 0);
        mBackButton->disappear();
        if (!mOnPause) {
            MR::decrementPauseBlurDelay();
            MR::endStarPointerMode(this);
        } else if (mResult == 1) {
            MR::decrementPauseBlurDelay();
        }
    }
    if (MR::isAnimStopped(this, 0) && mBackButton->isHidden()) kill();
}

void AllStarList::updatePageButtons(s32 world) {
    bool next = false;
    if (world + 1 >= 1 && world + 1 <= GameDataFunction::calcLastEnteredWorldNo()) next = true;
    if (next) {
        if (!mRightButton->_24 || mRightButton->isHidden()) mRightButton->appear();
        bool more = false;
        if (world + 2 >= 1 && world + 2 <= GameDataFunction::calcLastEnteredWorldNo()) more = true;
        if (more) mRightButton->_22 = false; else mRightButton->_22 = true;
    } else mRightButton->forceToHide();
    bool previous = false;
    if (world - 1 >= 1 && world - 1 <= GameDataFunction::calcLastEnteredWorldNo()) previous = true;
    if (previous) {
        if (!mLeftButton->_24 || mLeftButton->isHidden()) mLeftButton->appear();
        else mLeftButton->forceToWait();
        bool more = false;
        if (world >= 1 && world <= GameDataFunction::calcLastEnteredWorldNo()) more = true;
        if (more) mLeftButton->_22 = false; else mLeftButton->_22 = true;
    } else mLeftButton->forceToHide();
}

// NONMATCHING: register allocation and the two accessor temporary stack slots differ.
void AllStarList::updateContents(s32 world) {
    GalaxyStatusAccessor galaxies[210];
    GalaxyStatusAccessor candidates[210];
    s32 count = 0;
    for (ScenarioDataIter iter = MR::makeBeginScenarioDataIter(); !iter.isEnd(); iter.goNext()) {
        if (hasStars(iter.makeAccessor())) {
            bool skip = false;
            if (world != 0) {
                if (world != iter.makeAccessor().getWorldNo()) skip = true;
            }
            if (!skip) candidates[count++] = iter.makeAccessor();
        }
    }
    for (GalaxyStatusAccessor* it = candidates + 1; it != candidates + count; ++it) {
        for (GalaxyStatusAccessor* current = it; current != candidates && GameDataConst::getWorldNo(current->getName()) < GameDataConst::getWorldNo(current[-1].getName()); --current) {
            GalaxyStatusAccessor temporary = *current;
            *current = current[-1];
            current[-1] = temporary;
        }
    }
    s32 galaxyCount = 0;
    for (s32 i = 0; i < count; ++i) galaxies[galaxyCount++] = candidates[i];
    MR::setTextBoxMessageRecursive(this, "Title", MR::getWorldNameOnCurrentLanguage(world));
    wchar_t completion[70];
    wchar_t* output = completion;
    for (s32 i = 0; i < galaxyCount; ++i) {
        if (galaxies[i].isCompletedNoGreen()) output = MR::addPictureFontCode(output, 0x50);
        else if (galaxies[i].isCompletedWithGreen()) output = MR::addPictureFontCode(output, 0x6B);
        else output = MR::addPictureFontCode(output, 0x52);
        if (i < galaxyCount - 1) output = MR::addNewLine(output);
    }
    MR::setTextBoxMessageRecursive(this, "Comp", completion);
    for (s32 i = 0; i < galaxyCount; ++i) {
        if (galaxies[i].isOpened()) {
            MR::setTextBoxMessageRecursive(this, cGalaxyPanes[i], MR::getGalaxyNameShortOnCurrentLanguage(galaxies[i].getName()));
        } else {
            MR::setTextBoxGameMessageRecursive(this, cGalaxyPanes[i], "AllStarList_GlaxyName_Invalid");
        }
    }
    wchar_t stars[210];
    swprintf(stars, 420, L"");
    output = stars;
    for (s32 i = 0; i < galaxyCount; ++i) {
        if (galaxies[i].isOpened()) {
            wchar_t row[80];
            s32 length = MR::makeGalaxyStarList(row, 160, galaxies[i].getName(), false, false);
            swprintf(stars, 420, L"%ls%ls", stars, row);
            output += length;
        }
        if (i < galaxyCount - 1) output = MR::addNewLine(output);
    }
    MR::setTextBoxMessageRecursive(this, "Star", stars);
    wchar_t medals[210];
    swprintf(medals, 420, L"");
    output = medals;
    for (s32 i = 0; i < galaxyCount; ++i) {
        if (galaxies[i].isOpened()) {
            if (GameDataFunction::isOnGalaxyFlagTicoCoin(galaxies[i].getName())) output = MR::addPictureFontCode(output, 0x6A);
            else output = MR::addPictureFontCode(output, 0x6F);
        }
        if (i < galaxyCount - 1) output = MR::addNewLine(output);
    }
    MR::setTextBoxMessageRecursive(this, "Medal", medals);
}

void AllStarList::startWaiting() {
    if (!mRightButton->isWait()) mRightButton->forceToWait();
    if (!mLeftButton->isWait()) mLeftButton->forceToWait();
    setNerve(GET_NERVE(AllStarList, AllStarListNrvWait));
}
