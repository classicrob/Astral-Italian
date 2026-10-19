#include "Game/Screen/FileSelectInfo.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>

namespace {
    static const char* sMessages[] = {
        "FileInfo_Message_00", "FileInfo_Message_01", "FileInfo_Message_02", "FileInfo_Message_03",
        "FileInfo_Message_04", "FileInfo_Message_05", "FileInfo_Message_06"
    };

    static const f32 sDisappearAnimRate = 0.25f;
};  // namespace

namespace {
    extern char sDisplayWaitAnimation[];
}

inline void FileSelectInfo::exeDisplay() {
    if (MR::isFirstStep(this)) MR::startAnim(this, sDisplayWaitAnimation, 0);
}

namespace NrvFileSelectInfo {
    NEW_NERVE(FileSelectInfoNrvAppear, FileSelectInfo, Appear);
    NEW_NERVE(FileSelectInfoNrvDisplay, FileSelectInfo, Display);
    NEW_NERVE(FileSelectInfoNrvDisappear, FileSelectInfo, Disappear);
};  // namespace

namespace FileSelectInfoSub {
    NEW_NERVE(SlideStateNrvNormalPos, SlideState, NormalPos);
    NEW_NERVE(SlideStateNrvSliding, SlideState, Sliding);
    NEW_NERVE(SlideStateNrvSlidePos, SlideState, SlidePos);
    NEW_NERVE(SlideStateNrvSlidingBack, SlideState, SlidingBack);
};  // namespace FileSelectInfoSub

FileSelectInfo::FileSelectInfo(s32 nameBufferSize, const char* pName)
    : LayoutActor(pName, true), mNumber(0), mStarNum(0), mNameBufferSize(nameBufferSize), mName(new wchar_t[nameBufferSize]),
      mMissNum(-1), mGrandStarNum(0), mHas120Stars(false), mIsViewed120StarsEnding(false), mIsComplete(false), mHas240Stars(false),
      mShowMissCounter(false), mShow120StarsMessage(false) {
    mSlideState = new FileSelectInfoSub::SlideState(this);
    MR::zeroMemory(mName, nameBufferSize * sizeof(wchar_t));
}

void FileSelectInfo::init(const JMapInfoIter& rIter) {
    initLayoutManager("FileInfo", 3);
    MR::connectToSceneLayout(this);
    initNerve(GET_NERVE_DIRECT(NrvFileSelectInfo, FileSelectInfoNrvAppear));
}

void FileSelectInfo::appear() {
    f32 animFrame;

    if (!MR::isDead(this) && isNerve(GET_NERVE_DIRECT(NrvFileSelectInfo, FileSelectInfoNrvDisappear))) {
        animFrame = MR::getAnimFrame(this, 0);

        MR::startAnim(this, "Appear", 0);
        MR::setAnimFrame(this, animFrame, 0);
    } else {
        MR::startAnim(this, "Appear", 0);
    }

    setNerve(GET_NERVE_DIRECT(NrvFileSelectInfo, FileSelectInfoNrvAppear));
    LayoutActor::appear();
}

void FileSelectInfo::disappear() {
    f32 animFrame;

    if (!MR::isDead(this) && !isNerve(GET_NERVE_DIRECT(NrvFileSelectInfo, FileSelectInfoNrvDisappear))) {
        if (isNerve(GET_NERVE_DIRECT(NrvFileSelectInfo, FileSelectInfoNrvAppear))) {
            animFrame = MR::getAnimFrame(this, 0);
        } else {
            MR::startAnim(this, "Appear", 0);

            animFrame = MR::getAnimCtrl(this, 0)->getEnd() - 1.0f;
        }

        MR::startAnim(this, "Appear", 0);
        MR::setAnimFrame(this, animFrame, 0);
        MR::setAnimRate(this, 0.0f, 0);
        setNerve(GET_NERVE_DIRECT(NrvFileSelectInfo, FileSelectInfoNrvDisappear));
    }
}

void FileSelectInfo::slide() {
    mSlideState->setNerve(GET_NERVE_DIRECT(FileSelectInfoSub, SlideStateNrvSliding));
}

void FileSelectInfo::slideBack() {
    mSlideState->setNerve(GET_NERVE_DIRECT(FileSelectInfoSub, SlideStateNrvSlidingBack));
}

void FileSelectInfo::setInfo(const FileSelectFileInfo& info) {
    mNumber = info.mFileId;
    mStarNum = info.mPowerStarNum;
    MR::copyMemory(mName, info.mName, mNameBufferSize * sizeof(u16));
    mName[11] = 0;
    mGrandStarNum = info.mGrandStarNum;
    mHas120Stars = info.mHas120Stars;
    mIsViewed120StarsEnding = info.mIsViewed120StarsEnding;
    mIsComplete = info.mIsComplete;
    mHas240Stars = info.mHas240Stars;
    mShowMissCounter = info.mHas240Stars;
    mShow120StarsMessage = info.mIsViewed120StarsEnding;
    MR::copyString(mDateMessage, info.mDate, ARRAY_SIZE(mDateMessage));
    MR::copyString(mTimeMessage, info.mTime, ARRAY_SIZE(mTimeMessage));
    mMissNum = info.mMissCount;
    MR::copyString(mPlayTimeMessage, info.mPlayTime, ARRAY_SIZE(mPlayTimeMessage));
}

void FileSelectInfo::exeAppear() {
    if (MR::isFirstStep(this)) {
        reflectInfo();
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_DIRECT(NrvFileSelectInfo, FileSelectInfoNrvDisplay));
    }
}



void FileSelectInfo::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::setAnimRate(this, -::sDisappearAnimRate, 0);
    }

    if (MR::getAnimFrame(this, 0) - ::sDisappearAnimRate <= 0.0f) {
        kill();
    }
}

void FileSelectInfo::control() {
    mSlideState->updateNerve();
}

namespace {
    char sDisplayWaitAnimation[] = "Wait";
}

void FileSelectInfo::reflectInfo() {
    MR::setTextBoxNumberRecursive(this, "FileNumber", mNumber);
    MR::setTextBoxMessageRecursive(this, "FileName", mName);
    MR::setTextBoxArgNumberRecursive(this, "Star", mStarNum, 0);
    MR::setTextBoxMessageRecursive(this, "TxtDay", mDateMessage);
    MR::setTextBoxMessageRecursive(this, "TxtTime", mTimeMessage);
    setMissCounter();
    setCompleteness();
    setMessage();
}

void FileSelectInfo::setMissCounter() {
    if (mShowMissCounter) {
        MR::setTextBoxNumberRecursive(this, "MissCounter", mMissNum);
        MR::showPane(this, "MissCounter");
    } else MR::hidePane(this, "MissCounter");
}

void FileSelectInfo::setCompleteness() {
    MR::showPane(this, "Complete");
    if (mHas240Stars) MR::setTextBoxGameMessageRecursive(this, "Complete", "FileInfo_Complete_02");
    else if (mIsComplete) MR::setTextBoxGameMessageRecursive(this, "Complete", "FileInfo_Complete_01");
    else if (mHas120Stars) MR::setTextBoxGameMessageRecursive(this, "Complete", "FileInfo_Complete_00");
    else MR::hidePane(this, "Complete");
}

void FileSelectInfo::setMessage() {
    if (mShowMissCounter) MR::setTextBoxGameMessageRecursive(this, "Message", "FileInfo_Message_08");
    else if (mShow120StarsMessage) MR::setTextBoxGameMessageRecursive(this, "Message", "FileInfo_Message_07");
    else MR::setTextBoxGameMessageRecursive(this, "Message", sMessages[mGrandStarNum]);
}

namespace FileSelectInfoSub {
    SlideState::SlideState(FileSelectInfo* pHost) : NerveExecutor("スライド状態"), mHost(pHost) {
        initNerve(GET_NERVE_GLOBAL(SlideStateNrvNormalPos));
    }

    void SlideState::exeNormalPos() {
        if (MR::isFirstStep(this)) {
            MR::startAnim(mHost, "ButtonAppear", 1);
            MR::stopAnim(mHost, 1);
        }
    }

    void SlideState::exeSliding() {
        if (MR::isFirstStep(this)) {
            MR::startAnim(mHost, "ButtonAppear", 1);
        }

        if (MR::isAnimStopped(mHost, 1)) {
            setNerve(GET_NERVE_GLOBAL(SlideStateNrvSlidePos));
        }
    }

    void SlideState::exeSlidePos() {
        if (MR::isFirstStep(this)) {
            MR::startAnim(mHost, "ButtonEnd", 1);
            MR::stopAnim(mHost, 1);
        }
    }

    void SlideState::exeSlidingBack() {
        if (MR::isFirstStep(this)) {
            MR::startAnim(mHost, "ButtonEnd", 1);
        }

        if (MR::isAnimStopped(mHost, 1)) {
            setNerve(GET_NERVE_GLOBAL(SlideStateNrvNormalPos));
        }
    }
};  // namespace FileSelectInfoSub

