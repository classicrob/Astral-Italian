#include "Game/Screen/StaffRollLine.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/Language.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    const StaffRollLineRegionInfo cRegionInfo[] = {
        {1.0f, 27.5f, 36.0f},
        {1.0f, 24.5f, 29.5f},
        {1.0f, 23.5f, 30.5f},
        {1.0f, 24.0f, 30.5f}
    };
}

namespace NrvStaffRollLine {
    NEW_NERVE(StaffRollLineNrvAppear, StaffRollLine, Appear);
    NEW_NERVE(StaffRollLineNrvWait, StaffRollLine, Wait);
    NEW_NERVE(StaffRollLineNrvDisappear, StaffRollLine, Disappear);
}

StaffRollLine::StaffRollLine(s32 type, s32 index)
    : LayoutActor("スタッフロール一行", true), mType(type), mPosition(0.0f, 0.0f), mLineIndex(index), mWaitFrames(360) {
}

StaffRollLine::~StaffRollLine() {
}

void StaffRollLine::init(const JMapInfoIter&) {
    initLayoutManager("StaffRoll", 1);
    MR::createAndAddPaneCtrl(this, getPaneName(), 1);
    MR::setFollowPos(&mPosition, this, getPaneName());
    const char* pane = getPaneName();
    f32 scaleY = getRegionInfo()->mScale;
    MR::setPaneScale(this, pane, getRegionInfo()->mScale, scaleY);
    MR::startPaneAnim(this, getPaneName(), "End", 0);
    MR::setAnimFrameAndStopAtEnd(this, 0);
    initNerve(GET_NERVE(StaffRollLine, StaffRollLineNrvAppear));
    kill();
}

void StaffRollLine::appear() {
    LayoutActor::appear();
    setNerve(GET_NERVE(StaffRollLine, StaffRollLineNrvAppear));
}

void StaffRollLine::kill() {
    LayoutActor::kill();
}

void StaffRollLine::setDuration(s32 duration) {
    mWaitFrames = duration - 64;
}

const char* StaffRollLine::getPaneName() const {
    if (mType == 0) {
        return "Title";
    }
    if (mType == 1) {
        return "Name";
    }
    return nullptr;
}

void StaffRollLine::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(this, getPaneName(), "Appear", 0);
    }
    if (MR::isPaneAnimStopped(this, getPaneName(), 0)) {
        setNerve(GET_NERVE(StaffRollLine, StaffRollLineNrvWait));
    }
}

void StaffRollLine::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(this, getPaneName(), "Wait", 0);
    }
    if (MR::isGreaterEqualStep(this, mWaitFrames)) {
        setNerve(GET_NERVE(StaffRollLine, StaffRollLineNrvDisappear));
    }
}

void StaffRollLine::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(this, getPaneName(), "End", 0);
    }
    if (MR::isPaneAnimStopped(this, getPaneName(), 0)) {
        kill();
    }
}

const StaffRollLineRegionInfo* StaffRollLine::getRegionInfo() {
    const char* region = MR::getCurrentRegionPrefix();
    if (MR::isEqualString(region, "Jp")) {
        return &cRegionInfo[0];
    }
    if (MR::isEqualString(region, "Us")) {
        return &cRegionInfo[1];
    }
    if (MR::isEqualString(region, "Eu")) {
        return &cRegionInfo[2];
    }
    if (MR::isEqualString(region, "Kr")) {
        return &cRegionInfo[3];
    }
    return nullptr;
}
