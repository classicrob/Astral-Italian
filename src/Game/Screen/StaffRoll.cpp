#include "Game/Screen/StaffRoll.hpp"
#include "Game/Screen/StaffRollPart.hpp"
#include "Game/Screen/StaffRollUtil.hpp"
#include "Game/Demo/StaffRollDemoCamera.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/Language.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <algorithm>
#include <functional.hpp>

namespace {
    const char* cStaffRollMsgId = "StaffList000";
    const char* cPaneNameTitleLogo = "TitleLogo";
    const char* cPaneNameTheEnd = "TheEnd";
    const char* cPaneNameAllRights = "AllRights";
}

namespace NrvStaffRoll {
    NEW_NERVE(StaffRollNrvPrepareBgm, StaffRoll, PrepareBgm);
    NEW_NERVE(StaffRollNrvWaitTitle, StaffRoll, Wait);
    NEW_NERVE(StaffRollNrvTitleAppear, StaffRoll, PaneAppear);
    NEW_NERVE(StaffRollNrvTitleWait, StaffRoll, PaneWait);
    NEW_NERVE(StaffRollNrvTitleDisappear, StaffRoll, PaneDisappear);
    NEW_NERVE(StaffRollNrvWaitPart, StaffRoll, Wait);
    NEW_NERVE(StaffRollNrvPart, StaffRoll, Part);
    NEW_NERVE(StaffRollNrvEndLogoAppear, StaffRoll, PaneAppear);
    NEW_NERVE(StaffRollNrvEndLogoWait, StaffRoll, PaneWait);
    NEW_NERVE(StaffRollNrvEndLogoDisappear, StaffRoll, PaneDisappear);
    NEW_NERVE(StaffRollNrvWaitAllRights, StaffRoll, Wait);
    NEW_NERVE(StaffRollNrvAllRightsAppear, StaffRoll, PaneAppear);
    NEW_NERVE(StaffRollNrvAllRightsWait, StaffRoll, PaneWait);
    NEW_NERVE(StaffRollNrvAllRightsDisappear, StaffRoll, PaneDisappear);
    NEW_NERVE(StaffRollNrvWaitEnd, StaffRoll, Wait);
    NEW_NERVE(StaffRollNrvWaitDemo, StaffRoll, WaitDemo);
    NEW_NERVE(StaffRollNrvDemoStart, StaffRoll, DemoStart);
    NEW_NERVE(StaffRollNrvDemoEnd, StaffRoll, DemoEnd);
    NEW_NERVE(StaffRollNrvEnd, StaffRoll, End);
}

#define STAFF_NERVE(name) GET_NERVE(StaffRoll, StaffRollNrv##name)

StaffRoll::StaffRoll()
    : LayoutActor("スタッフロール", true), mMessage(nullptr), mMessageLineCount(0), mMessageLine(0),
      mTitleLogoPosition(0.0f, 0.0f), mEndLogoPosition(0.0f, 0.0f), mAllRightsPosition(0.0f, 0.0f),
      mLastPartIndex(-1), mPartCount(0), mPartDuration(0), mStartFrame(0), mDemoCamera(nullptr) {
}

void StaffRoll::init(const JMapInfoIter&) {
    MR::connectToSceneLayout(this);
    initLayoutManager("StaffRollTitle", 1);
    StaffRollUtil::initPane(this, &mTitleLogoPosition, cPaneNameTitleLogo);
    StaffRollUtil::initPane(this, &mEndLogoPosition, cPaneNameTheEnd);
    StaffRollUtil::initPane(this, &mAllRightsPosition, cPaneNameAllRights);
    mMessage = MR::getGameMessageDirect(cStaffRollMsgId);
    mMessageLineCount = MR::countMessageLine(mMessage);
    u32 line = 0;
    initActionSound(3, "StaffRoll", nullptr);
    while (line < mMessageLineCount) {
        StaffRollPart* part = StaffRollUtil::createPart(mMessage, &line);
        s32 index = mPartCount++;
        mParts[index] = part;
    }
    const char* region = MR::getCurrentRegionPrefix();
    s32 duration;
    if (MR::isEqualString(region, "Jp")) {
        duration = 500;
    } else if (MR::isEqualString(region, "Us")) {
        duration = 440;
    } else {
        MR::isEqualString(region, "Eu");
        duration = 430;
    }
    mPartDuration = duration;
    std::for_each(mParts, mParts + mPartCount,
                  std::binder2nd<std::mem_fun1_t<void, StaffRollPart, s32>, s32>(std::mem_func(&StaffRollPart::setDuration), duration));
    initNerve(STAFF_NERVE(PrepareBgm));
    kill();
}

void StaffRoll::appear() {
    LayoutActor::appear();
    mCurrentPart = mParts;
    setNerve(STAFF_NERVE(PrepareBgm));
}

void StaffRoll::kill() {
    std::for_each(mParts, mParts + mPartCount, std::mem_func(&StaffRollPart::kill));
    LayoutActor::kill();
}

void StaffRoll::movement() {
    if (MR::isExecuteCalcAnimLayout(this)) {
        LayoutActor::movement();
        std::for_each(mParts, mParts + mPartCount, std::mem_func(&StaffRollPart::movement));
    }
}

void StaffRoll::calcAnim() {
    LayoutActor::calcAnim();
    std::for_each(mParts, mParts + mPartCount, std::mem_func(&StaffRollPart::calcAnim));
}

void StaffRoll::draw() const {
    if (!MR::isDead(this)) {
        GXColor color = {0, 0, 0, 0};
        MR::fillScreen(color);
        LayoutActor::draw();
        std::for_each(mParts, mParts + mPartCount, std::mem_func(&StaffRollPart::draw));
    }
}

bool StaffRoll::isCurrentPartEnd() const {
    if (mCurrentPart == mParts) {
        return false;
    }
    if (mCurrentPart == mParts + mPartCount) {
        return false;
    }
    StaffRollPart* part = *mCurrentPart;
    if (part->mHasEndMarker) {
        return part->isEnd();
    }
    return false;
}

// NONMATCHING: equivalent equality comparison uses reversed register operands.
bool StaffRoll::isLastPartEnd() const {
    if (mCurrentPart == mParts) {
        return false;
    }
    s32 partCount = mPartCount;
    if (mCurrentPart == mParts + partCount) {
        return false;
    }
    if (getCurrentPartIndex() == partCount - 1) {
        return (*mCurrentPart)->isEnd();
    }
    return false;
}

bool StaffRoll::isEnd() const {
    return isNerve(STAFF_NERVE(End));
}

s32 StaffRoll::getCurrentPartIndex() const {
    return mCurrentPart - mParts;
}

s32 StaffRoll::getElapsedFrame() const {
    return GameSequenceFunction::getElapsedStageFrame() - mStartFrame;
}

void StaffRoll::exePrepareBgm() {
    if (MR::isFirstStep(this)) {
        MR::startStageBGM("MBGM_SMG2_STAFFROLL", true);
    }
    if (MR::isPreparedStageBgm()) {
        MR::unlockStageBGM();
        MR::setSoundVolumeSetting(8, 0);
        MR::setSoundField9A4(false);
        mStartFrame = GameSequenceFunction::getElapsedStageFrame();
        setNerve(STAFF_NERVE(WaitTitle));
    }
}

void StaffRoll::exePart() {
    if (MR::isFirstStep(this)) {
        (*mCurrentPart)->appear();
        return;
    }
    if ((*mCurrentPart)->isEnd()) {
        ++mCurrentPart;
        if (mCurrentPart != mParts + mPartCount) {
            setNerve(STAFF_NERVE(Part));
            return;
        }
        setNerve(STAFF_NERVE(WaitDemo));
    }
}

void StaffRoll::exePaneAppear() {
    const char* pane = nullptr;
    const Nerve* next = nullptr;
    if (isNerve(STAFF_NERVE(TitleAppear))) {
        pane = cPaneNameTitleLogo;
        next = STAFF_NERVE(TitleWait);
    } else if (isNerve(STAFF_NERVE(EndLogoAppear))) {
        pane = cPaneNameTheEnd;
        next = STAFF_NERVE(EndLogoWait);
    } else if (isNerve(STAFF_NERVE(AllRightsAppear))) {
        pane = cPaneNameAllRights;
        next = STAFF_NERVE(AllRightsWait);
    }
    StaffRollUtil::updatePaneAppear(this, pane, next);
}

void StaffRoll::exePaneWait() {
    const char* pane = nullptr;
    const Nerve* next = nullptr;
    s32 duration = -1;
    if (isNerve(STAFF_NERVE(TitleWait))) {
        pane = cPaneNameTitleLogo;
        next = STAFF_NERVE(TitleDisappear);
        duration = 729;
    } else if (isNerve(STAFF_NERVE(EndLogoWait))) {
        pane = cPaneNameTheEnd;
        next = STAFF_NERVE(AllRightsAppear);
        duration = 240;
    } else if (isNerve(STAFF_NERVE(AllRightsWait))) {
        pane = cPaneNameAllRights;
        next = STAFF_NERVE(WaitEnd);
        duration = 240;
        if (MR::isStep(this, StaffRollUtil::sThankYouVoiceFrame)) {
            if (MR::isPlayerLuigi()) {
                MR::startActionSound(this, "ThankYouLuigi", -1, -1);
            } else {
                MR::startActionSound(this, "ThankYouMario", -1, -1);
            }
        }
    }
    StaffRollUtil::updatePaneWait(this, pane, duration, next);
}

void StaffRoll::exePaneDisappear() {
    const char* pane = nullptr;
    const Nerve* next = nullptr;
    if (isNerve(STAFF_NERVE(TitleDisappear))) {
        pane = cPaneNameTitleLogo;
        next = STAFF_NERVE(WaitPart);
    } else if (isNerve(STAFF_NERVE(EndLogoDisappear))) {
        pane = cPaneNameTheEnd;
        next = STAFF_NERVE(AllRightsAppear);
    } else if (isNerve(STAFF_NERVE(AllRightsDisappear))) {
        pane = cPaneNameAllRights;
        next = STAFF_NERVE(WaitEnd);
    }
    StaffRollUtil::updatePaneDisappear(this, pane, next);
}

void StaffRoll::exeWait() {
    if (isNerve(STAFF_NERVE(WaitTitle))) {
        if (MR::isGreaterEqualStep(this, 528)) {
            setNerve(STAFF_NERVE(TitleAppear));
        }
    } else if (isNerve(STAFF_NERVE(WaitPart))) {
        if (MR::isGreaterEqualStep(this, 88)) {
            setNerve(STAFF_NERVE(Part));
        }
    } else if (isNerve(STAFF_NERVE(WaitEnd)) && MR::isGreaterEqualStep(this, 90)) {
        setNerve(STAFF_NERVE(End));
    }
}

void StaffRoll::exeWaitDemo() {
    if (static_cast<u32>(getElapsedFrame()) >= 12650) {
        setNerve(STAFF_NERVE(DemoStart));
    }
}

void StaffRoll::exeDemoStart() {
    if (MR::isFirstStep(this)) {
        mDemoCamera->start();
    }
    mDemoCamera->update(false);
    if (isDemoCameraStartFinished()) {
        setNerve(STAFF_NERVE(DemoEnd));
    }
}

void StaffRoll::exeDemoEnd() {
    if (MR::isFirstStep(this)) {
        MR::recoverSoundVolumeSetting(0);
        MR::setSoundVolumeSetting(13, 120);
    }
    mDemoCamera->update(true);
    if (isDemoCameraEndFinished()) {
        setNerve(STAFF_NERVE(EndLogoAppear));
    }
}

void StaffRoll::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::recoverSoundVolumeSetting(0);
        MR::setSoundField9A4(true);
        MR::stopSeCategories(0);
    }
    MR::setSeVolumeSetLevel(13, 0);
}

StaffRoll::~StaffRoll() {}

namespace MR {
    StaffRoll* getStaffRoll() {
        return MR::getSceneObj<StaffRoll>(SceneObj_StaffRoll);
    }
}
