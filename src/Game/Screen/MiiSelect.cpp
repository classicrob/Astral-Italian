#include "Game/Screen/MiiSelect.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/FileSelectFunc.hpp"
#include "Game/Map/FileSelectIconID.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Screen/MiiSelectIcon.hpp"
#include "Game/Util/BitArray.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "revolution/types.h"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <RVLFaceLib.h>


namespace NrvMissSelect {
    class MissSelectNrvAppear : public Nerve { public: MissSelectNrvAppear(); virtual void execute(Spine* spine) const { static_cast<MiiSelect*>(spine->mExecutor)->exeAppear(); } static MissSelectNrvAppear sInstance; };
    MissSelectNrvAppear MissSelectNrvAppear::sInstance ATTRIBUTE_WEAK;
    class MissSelectNrvWait : public Nerve { public: MissSelectNrvWait(); virtual void execute(Spine* spine) const { static_cast<MiiSelect*>(spine->mExecutor)->exeWait(); } static MissSelectNrvWait sInstance; };
    MissSelectNrvWait MissSelectNrvWait::sInstance ATTRIBUTE_WEAK;
    class MissSelectNrvScrollRight : public Nerve { public: MissSelectNrvScrollRight(); virtual void execute(Spine* spine) const { static_cast<MiiSelect*>(spine->mExecutor)->exeScrollRight(); } static MissSelectNrvScrollRight sInstance; };
    MissSelectNrvScrollRight MissSelectNrvScrollRight::sInstance ATTRIBUTE_WEAK;
    class MissSelectNrvScrollLeft : public Nerve { public: MissSelectNrvScrollLeft(); virtual void execute(Spine* spine) const { static_cast<MiiSelect*>(spine->mExecutor)->exeScrollLeft(); } static MissSelectNrvScrollLeft sInstance; };
    MissSelectNrvScrollLeft MissSelectNrvScrollLeft::sInstance ATTRIBUTE_WEAK;
    class MissSelectNrvSelected : public Nerve { public: MissSelectNrvSelected(); virtual void execute(Spine* spine) const { static_cast<MiiSelect*>(spine->mExecutor)->exeSelected(); } static MissSelectNrvSelected sInstance; };
    MissSelectNrvSelected MissSelectNrvSelected::sInstance ATTRIBUTE_WEAK;
    class MissSelectNrvDisappear : public Nerve { public: MissSelectNrvDisappear(); virtual void execute(Spine* spine) const { static_cast<MiiSelect*>(spine->mExecutor)->exeDisappear(); } static MissSelectNrvDisappear sInstance; };
    MissSelectNrvDisappear MissSelectNrvDisappear::sInstance ATTRIBUTE_WEAK;
    class MissSelectNrvDummySelected : public Nerve { public: MissSelectNrvDummySelected(); virtual void execute(Spine* spine) const { static_cast<MiiSelect*>(spine->mExecutor)->exeDummySelected(); } static MissSelectNrvDummySelected sInstance; };
    MissSelectNrvDummySelected MissSelectNrvDummySelected::sInstance ATTRIBUTE_WEAK;

}

namespace {
    typedef void (MiiSelect::*CallbackFuncPtr)();

    typedef bool (MiiSelect::*TriggerFuncPtr)();
    static const TriggerFuncPtr trigger[2] = {&MiiSelect::isLeftTriggered, &MiiSelect::isRightTriggered};

    static const CallbackFuncPtr func[2] = {&MiiSelect::callbackLeft, &MiiSelect::callbackRight};
};  // namespace

MiiSelect::MiiSelect(const char* pName)
    : LayoutActor(pName, true), mFellowAvailableFlags(new MR::BitArray(7)), mFellowIconNum(), mFavoriteMiiNum(), mMiiNum(), mPageStartIndex(), mSelectedIconIndex(), mSelectedMiiTexMap(), mFrontPage(), mBackPage(),
      mIsIconProhibited(), mProhibitedIconID(new FileSelectIconID()) {
    for (int i = 0; i < ARRAY_SIZE(mPageButtons); i++) {
        mPageButtons[i] = nullptr;
    }

    validateAllSpecialMii();
}

void MiiSelect::init(const JMapInfoIter& rIter) {
    initLayoutManager("MiiSelect", 1);
    MR::invalidateParentAnim(this);
    createButtons();
    MR::connectToSceneLayoutTemporarily(this);
    initNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvAppear));
    createPage();
    kill();
}

void MiiSelect::appear() {
    LayoutActor::appear();
    setNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvAppear));
    refresh();
    setCurrentPageGroupA();
    appearButtons();
    updateButtons();
    mFrontPage->invalidateAllIcon();
    mBackPage->invalidateAllIcon();
    MR::setTextBoxMessageRecursive(this, "TxtName", L"");
}

void MiiSelect::disappear() {
    mFrontPage->invalidateAllIcon();
    mBackPage->invalidateAllIcon();
    disappearButtons();
    setNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvDisappear));
}

void MiiSelect::calcAnim() {
    LayoutActor::calcAnim();
    mFrontPage->calcAnim();
    mBackPage->calcAnim();
}

bool MiiSelect::isAppearing() const {
    return isNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvAppear));
}

bool MiiSelect::isSelected() {
    return isNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvSelected));
}

bool MiiSelect::isDummySelected() {
    return isNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvDummySelected));
}

void MiiSelect::getSelectedID(FileSelectIconID* pSelectedID) {
    getIconID(pSelectedID, mSelectedIconIndex);
}

nw4r::lyt::TexMap* MiiSelect::getSelectedMiiTexMap() {
    return mSelectedMiiTexMap;
}

void MiiSelect::admitIcon() {
    mIsIconProhibited = 0;
}

void MiiSelect::prohibitIcon(const FileSelectIconID& rIconID) {
    mIsIconProhibited = 1;

    mProhibitedIconID->set(rIconID);
}

void MiiSelect::invalidateSpecialMii(FileSelectIconID::EFellowID fellowID) {
    if (mFellowAvailableFlags->isOn(fellowID)) {
        mFellowAvailableFlags->set(fellowID, false);

        mFellowIconNum--;
    }
}

void MiiSelect::validateAllSpecialMii() {
    for (int i = 0; i < mFellowAvailableFlags->size(); i++) {
        mFellowAvailableFlags->set(i, true);
    }

    mFellowIconNum = mFellowAvailableFlags->size();
}

void MiiSelect::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Appear", 0);
        MR::startSystemSE("SE_SY_FILE_SEL_MIISEL_OPEN");
        setCurrentPageNum();
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvWait));
    }
}

void MiiSelect::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Wait", 0);
        validateAllIcon();
        setCurrentPageGroupB();

        for (int i = 0; i < ARRAY_SIZE(mPageButtons); i++) {
            if (mPageButtons[i]->isHidden()) {
                continue;
            }

            mPageButtons[i]->mIsActive = true;
            mPageButtons[i]->forceToWait();
        }

        setCurrentPageNum();
    }

    for (int i = 0; i < ARRAY_SIZE(mPageButtons); i++) {
        mPageButtons[i]->trySelect();
        if ((this->*::trigger[i])() == true) {
            mPageButtons[i]->trySelectImmediately();
        }
        if (mPageButtons[i]->mIsSelected) {
            (this->*::func[i])();
            return;
        }
    }
}

void MiiSelect::exeScrollLeft() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "PreviousMii", 0);

        mPageStartIndex -= 8;

        if (mPageStartIndex < 0) {
            mPageStartIndex = ((getIconNum() - 1) / 8) * 8;
        }

        flipPage();
        setCurrentPageGroupB();
        refresh();
        updateButtons();
        mFrontPage->invalidateAllIcon();
        mBackPage->invalidateAllIcon();
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvWait));
    }
}

void MiiSelect::exeScrollRight() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "NextMii", 0);

        mPageStartIndex += 8;

        if (mPageStartIndex >= getIconNum()) {
            mPageStartIndex = 0;
        }

        flipPage();
        setCurrentPageGroupB();
        refresh();
        updateButtons();
        mFrontPage->invalidateAllIcon();
        mBackPage->invalidateAllIcon();
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvWait));
    }
}

void MiiSelect::exeSelected() {
}

void MiiSelect::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "End", 0);
    }

    if (MR::isAnimStopped(this, 0)) {
        kill();
    }
}

void MiiSelect::exeDummySelected() {
}

void MiiSelect::control() {
    for (int i = 0; i < ARRAY_SIZE(mPageButtons); i++) {
        mPageButtons[i]->update();
    }

    J3DFrameCtrl* pAnimCtrl;

    if (mPageButtons[0]->isFirstStepWait() && !mPageButtons[1]->isFirstStepWait() && mPageButtons[1]->isWait()) {
        pAnimCtrl = MR::getPaneAnimCtrl(this, mPageButtons[0]->mPaneName, 0);

#ifdef GALAXY_NATIVE
        if (pAnimCtrl)
#endif
        pAnimCtrl->setFrame(MR::getPaneAnimFrame(this, mPageButtons[1]->mPaneName, 0));
    }

    if (mPageButtons[1]->isFirstStepWait() && !mPageButtons[0]->isFirstStepWait() && mPageButtons[0]->isWait()) {
        pAnimCtrl = MR::getPaneAnimCtrl(this, mPageButtons[1]->mPaneName, 0);

#ifdef GALAXY_NATIVE
        if (pAnimCtrl)
#endif
        pAnimCtrl->setFrame(MR::getPaneAnimFrame(this, mPageButtons[0]->mPaneName, 0));
    }

    mFrontPage->movement();
}

void MiiSelect::createButtons() {
    static const char* pane[] = {
        "Left",
        "Right",
    };
    static const char* bound[] = {
        "PicArrowL",
        "PicArrowR",
    };

    for (int i = 0; i < ARRAY_SIZE(mPageButtons); i++) {
        MR::createAndAddPaneCtrl(this, pane[i], 1);

        mPageButtons[i] = new ButtonPaneController(this, pane[i], bound[i], 0, true);
        mPageButtons[i]->mDisappearOnDecide = false;
    }
}

void MiiSelect::callbackLeft() {
    MR::startSystemSE("SE_SY_FILE_SEL_MIISEL_SCRL");
    setNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvScrollLeft));
}

void MiiSelect::callbackRight() {
    MR::startSystemSE("SE_SY_FILE_SEL_MIISEL_SCRL");
    setNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvScrollRight));
}

bool MiiSelect::isLeftTriggered() {
    return !mPageButtons[0]->isHidden() && (MR::testCorePadTriggerLeft(WPAD_CHAN0) || MR::testSubPadStickTriggerLeft(WPAD_CHAN0));
}

bool MiiSelect::isRightTriggered() {
    return !mPageButtons[1]->isHidden() && (MR::testCorePadTriggerRight(WPAD_CHAN0) || MR::testSubPadStickTriggerRight(WPAD_CHAN0));
}

void MiiSelect::appearButtons() {
    for (int i = 0; i < ARRAY_SIZE(mPageButtons); i++) {
        mPageButtons[i]->appear();
    }
}

void MiiSelect::disappearButtons() {
    for (int i = 0; i < ARRAY_SIZE(mPageButtons); i++) {
        mPageButtons[i]->disappear();
    }
}

void MiiSelect::updateButtons() {
    if (getIconNum() > 8) {
        if (mPageButtons[0]->isHidden()) {
            mPageButtons[0]->appear();
        }

        if (mPageButtons[1]->isHidden()) {
            mPageButtons[1]->appear();
        }
    } else {
        if (mPageStartIndex - 8 < 0) {
            if (!mPageButtons[0]->isHidden()) {
                mPageButtons[0]->disappear();
            }
        } else if (mPageButtons[0]->isHidden()) {
            mPageButtons[0]->appear();
        }

        if (mPageStartIndex + 8 >= getIconNum()) {
            if (!mPageButtons[1]->isHidden()) {
                mPageButtons[1]->disappear();
            }
        } else if (mPageButtons[1]->isHidden()) {
            mPageButtons[1]->appear();
        }
    }
}

void MiiSelect::collectValidMiiIndex() {
    for (u32 i = 0; i < RFL_DB_CHAR_MAX; i++) {
        if (!RFLIsAvailableOfficialData(i)) {
            continue;
        }

        RFLAdditionalInfo additionalInfo;

        if (RFLGetAdditionalInfoWithSanitizedNames(&additionalInfo, RFLDataSource_Official, nullptr, i) != RFLErrcode_Success) {
            continue;
        }

        if (additionalInfo.favorite == 1) {
            mFavoriteMiiIndices[mFavoriteMiiNum] = static_cast< u16 >(i);
            mFavoriteMiiNum++;
        } else {
            mMiiIndices[mMiiNum] = static_cast< u16 >(i);
            mMiiNum++;
        }
    }
}

void MiiSelect::createPage() {
    mFrontPage = new MiiSelectSub::Page(this);
    mBackPage = new MiiSelectSub::Page(this);
}

void MiiSelect::flipPage() {
    MiiSelectSub::Page* pPage = mFrontPage;

    mFrontPage = mBackPage;
    mBackPage = pPage;
}

void MiiSelect::setCurrentPageGroupA() {
    mFrontPage->mIsGroupA = 1;
    mBackPage->mIsGroupA = 0;
}

void MiiSelect::setCurrentPageGroupB() {
    mFrontPage->mIsGroupA = 0;
    mBackPage->mIsGroupA = 1;
}

void MiiSelect::setCurrentPageNum() {
    s32 v2 = (getIconNum() - 1) / 8;
    s32 v1 = mPageStartIndex / 8;

    MR::setTextBoxFormatRecursive(this, "TxtPage", L"%d/%d", v1 + 1, v2 + 1);
}

void MiiSelect::validateAllIcon() {
    mFrontPage->validateAllIcon();
    mBackPage->validateAllIcon();

    if (mIsIconProhibited) {
        mFrontPage->prohibitIcon(*mProhibitedIconID);
        mBackPage->prohibitIcon(*mProhibitedIconID);
    }
}

void MiiSelect::refresh() {
    mFrontPage->refresh(mPageStartIndex);

    if (mIsIconProhibited) {
        mFrontPage->prohibitIcon(*mProhibitedIconID);
        mBackPage->prohibitIcon(*mProhibitedIconID);
    }
}

void MiiSelect::getIconID(FileSelectIconID* pIconID, s32 param2) const {
    if (param2 < mFavoriteMiiNum) {
        pIconID->setMiiIndex(mFavoriteMiiIndices[param2]);
    } else if (param2 < mMiiNum + mFavoriteMiiNum) {
        pIconID->setMiiIndex(mMiiIndices[param2 - mFavoriteMiiNum]);
    } else {
        s32 fellowIndex = param2 - (mMiiNum + mFavoriteMiiNum);
        s32 enabledIndex = 0;
        for (int i = 0; i < mFellowAvailableFlags->size(); i++) {
            if (mFellowAvailableFlags->isOn(i)) {
                if (enabledIndex == fellowIndex) {
                    pIconID->setFellowID(static_cast<FileSelectIconID::EFellowID>(i));
                    break;
                }
                enabledIndex++;
            }
        }
    }
}

void MiiSelect::onSelect(s32 param1, nw4r::lyt::TexMap* pTexMap) {
#ifdef GALAXY_NATIVE
    if (!pTexMap) return;
#endif
    mSelectedIconIndex = param1;
    mSelectedMiiTexMap = pTexMap;

    mFrontPage->invalidateAllIcon();
    mBackPage->invalidateAllIcon();
    disappearButtons();
    setNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvSelected));
}

void MiiSelect::onSelectDummy() {
    mFrontPage->invalidateAllIcon();
    mBackPage->invalidateAllIcon();
    disappearButtons();
    setNerve(GET_NERVE_DIRECT(NrvMissSelect, MissSelectNrvDummySelected));
}

namespace MiiSelectSub {
    Page::Page(MiiSelect* pHost) : mHost(pHost), mIsGroupA(true) {
        for (u32 i = 0; i < ARRAY_SIZE(mIconArray); i++) {
            mIconArray[i] = new MiiSelectIcon(-1, -1, 0x51, "Miiセレクト用アイコン");
        }
    }

    void Page::refresh(s32 baseIndex) {
        mBaseIndex = baseIndex;

        s32 iconNum = mHost->getIconNum();

        for (s32 i = 0; i < ARRAY_SIZE(mIconArray); i++) {
            if (mHost->isExistMiiIcon()) {
                if (baseIndex + i >= iconNum) {
                    mIconArray[i]->kill();
                    continue;
                }
            } else if (baseIndex + i == iconNum) {
                mIconArray[i]->appearMiiDummy();
                continue;
            } else if (baseIndex + i > iconNum) {
                mIconArray[i]->kill();
                continue;
            }

            FileSelectIconID iconID;
            mHost->getIconID(&iconID, baseIndex + i);
            mIconArray[i]->appear(iconID);
        }
    }

    inline void setPointedMiiName(MiiSelect* pHost, s32 index) {
        wchar_t name[RFL_NAME_LEN + 1];
        FileSelectFunc::copyMiiName(reinterpret_cast< u16* >(name), pHost->makeIconID(index));
        MR::setTextBoxMessageRecursive(pHost, "TxtName", name);
    }

    void Page::movement() {
        bool isPointingAny = false;

        for (s32 i = 0; i < ARRAY_SIZE(mIconArray); i++) {
            if (!MR::isDead(mIconArray[i])) {
                mIconArray[i]->movement();

                if (mIconArray[i]->isMiiDummy()) {
                    if (mIconArray[i]->isSelected()) {
                        mHost->onSelectDummy();
                        mIconArray[i]->invalidate();
                    }
                } else if (mIconArray[i]->isSelected()) {
                    mHost->onSelect(mBaseIndex + i, mIconArray[i]->getTexMap());
                    mIconArray[i]->invalidate();
                } else if (mIconArray[i]->isPointing()) {
                    setPointedMiiName(mHost, mBaseIndex + i);

                    isPointingAny = true;
                }
            }
        }

        if (!isPointingAny) {
            MR::setTextBoxMessageRecursive(mHost, "TxtName", L"");
        }
    }

    void Page::calcAnim() {
        s32 v = 9;

        if (mIsGroupA) {
            v = 1;
        }

        for (s32 i = 0; i < ARRAY_SIZE(mIconArray); i++) {
            char paneName[128];

            snprintf(paneName, sizeof(paneName), "Mii%02d", v + i);
            MR::setLayoutPosAtPaneTrans(mIconArray[i], mHost, paneName);
            mIconArray[i]->calcAnim();
        }
    }

    void Page::invalidateAllIcon() {
        for (int i = 0; i < ARRAY_SIZE(mIconArray); i++) {
            mIconArray[i]->invalidate();
        }
    }

    void Page::validateAllIcon() {
        for (int i = 0; i < ARRAY_SIZE(mIconArray); i++) {
            mIconArray[i]->validate();
        }
    }

    void Page::prohibitIcon(const FileSelectIconID& rIconID) {
        for (int i = 0; i < ARRAY_SIZE(mIconArray); i++) {
            if (MR::isDead(mIconArray[i])) {
                continue;
            }

            if (mIconArray[i]->isMiiDummy()) {
                continue;
            }

            FileSelectIconID iconID = FileSelectIconID();

            mIconArray[i]->copyIconID(&iconID);

            if (iconID != rIconID) {
                continue;
            }

            mIconArray[i]->prohibit();
        }
    }
};  // namespace MiiSelectSub

s32 MiiSelect::getIconNum() {
    return mMiiNum + mFavoriteMiiNum + mFellowIconNum;
}

namespace NrvMissSelect {
    MissSelectNrvAppear::MissSelectNrvAppear() {}
    MissSelectNrvWait::MissSelectNrvWait() {}
    MissSelectNrvScrollRight::MissSelectNrvScrollRight() {}
    MissSelectNrvScrollLeft::MissSelectNrvScrollLeft() {}
    MissSelectNrvSelected::MissSelectNrvSelected() {}
    MissSelectNrvDisappear::MissSelectNrvDisappear() {}
    MissSelectNrvDummySelected::MissSelectNrvDummySelected() {}
}
