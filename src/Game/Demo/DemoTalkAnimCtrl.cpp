#include "Game/Demo/DemoTalkAnimCtrl.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/Demo/DemoSubPartKeeper.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/ResourceInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>

#include "Game/Screen/InformationObserver.hpp"

DemoTalkAnimCtrl::DemoTalkAnimCtrl(LiveActor* pActor, const char* a3, const char* a4)
    : NerveExecutor(a3), mActor(pActor), mDemoName(a3), mTargetPartName(a4), mCurrentPartName(nullptr), mAnimName(""), mCameraName(""), mCameraInfo(nullptr), mAnimFrame(0), mPartStep(0), mTotalStep(0),
      mIsTargetPart(true), mIsTimeKeep(false), mIsWaitUserInput(false), mInterpole(0), mCameraStartFrame(0), mCameraEndFrame(0), mHaveDemoPlayerBck(false), mHaveCamera(false), mHaveBck(false), mHaveBtk(false), mHaveBpk(false),
      mHaveBtp(false), mHaveBrk(false), mHaveBva(false) {
}

void DemoTalkAnimCtrl::initForActor(const char* a1) {
    mIsTimeKeep = false;
    mIsTargetPart = true;
    mHaveCamera = false;
    mHaveDemoPlayerBck = false;
    mAnimName = a1;
    mCurrentPartName = nullptr;
    mAnimFrame = 0;
    mPartStep = 0;
    mTotalStep = 0;
    mIsWaitUserInput = false;
    mCameraInfo = nullptr;
    mCameraName = "";
    mHaveBck = MR::isExistBck(mActor, a1);
    mHaveBtk = MR::isExistBtk(mActor, mAnimName);
    mHaveBpk = MR::isExistBpk(mActor, mAnimName);
    mHaveBtp = MR::isExistBtp(mActor, mAnimName);
    mHaveBrk = MR::isExistBrk(mActor, mAnimName);
    mHaveBva = MR::isExistBva(mActor, mAnimName);
}


void DemoTalkAnimCtrl::initForScene(const char* a1, const char* a2, const JMapInfoIter& rIter) {
    char buf[0x100];
    snprintf(buf, sizeof(buf), "%s.canm", a2);
    initForActor(a1);
    mIsTimeKeep = true;
    mIsTargetPart = mTargetPartName == nullptr;
    mHaveCamera = MR::getResourceHolder(mActor)->mFileInfoTable->isExistRes(buf);
    mHaveDemoPlayerBck = MR::isExistBck(MR::getPlayerDemoActor(), a1);
    if (mHaveCamera) {
        mCameraName = a2;
        mCameraInfo = MR::createActorCameraInfo(rIter);
        MR::initAnimCamera(mActor, mCameraInfo, mCameraName);
    }
}

void DemoTalkAnimCtrl::startDemo() {
    mCurrentPartName = nullptr;
    mAnimFrame = 0;
    mPartStep = 0;
    mTotalStep = 0;
    mIsWaitUserInput = false;
    if (mHaveCamera && mIsTargetPart) startCamera();
}

bool DemoTalkAnimCtrl::updateDemo() {
    if (!mIsTargetPart) return false;
    updateDemoPart();
    if (mIsTimeKeep && mIsWaitUserInput) updatePause();
    if (mHaveCamera) updateCamera();
    if (DemoFunction::isDemoSuspendOrLastPartLastStep()) {
        if (mHaveCamera) endCamera();
        return false;
    }
    if (!DemoFunction::isPauseTimeKeepDemo(mDemoName)) {
        mTotalStep++;
        mPartStep++;
    }
    return true;
}

void DemoTalkAnimCtrl::endDemo() {
    if (mHaveCamera && mIsTargetPart) MR::endAnimCamera(mActor, mCameraInfo, mCameraName, 0, true);
}

void DemoTalkAnimCtrl::updateStoryBook() {
    updateDemo();
    startAnim();
    MR::setAllAnimFrameAndStop(mActor, mAnimName, mAnimFrame);
    if (MR::isStoryBookEndingDemoName(mDemoName)) MR::stopAllAnim(mActor);
    if (mHaveDemoPlayerBck) {
        BckCtrlData data;
        data.mPlayFrame = 0;
        data.mStartFrame = mAnimFrame;
        MR::startDemoBckPlayer(mAnimName, data);
    }
}

void DemoTalkAnimCtrl::pause() {
    if (mIsTimeKeep) DemoFunction::pauseTimeKeepDemo(mDemoName);
    if (mHaveCamera) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s%s", mActor->mName, mCameraName);
        MR::pauseOnAnimCamera(mCameraInfo, buf);
    }
}

void DemoTalkAnimCtrl::resume() {
    if (mIsTimeKeep) DemoFunction::resumeTimeKeepDemo(mDemoName);
}

void DemoTalkAnimCtrl::startAnim() {
    MR::startAction(mActor, mAnimName);
    if (MR::isStoryBookEndingDemoName(mDemoName)) MR::resumeAllAnim(mActor);
}

void DemoTalkAnimCtrl::startCamera() {
    MR::startAnimCameraTargetSelf(mActor, mCameraInfo, mCameraName, mCameraStartFrame, false, 1.0f);
}

void DemoTalkAnimCtrl::updateAnim(const BckCtrlData& rBck) {
    J3DFrameCtrl* ctrls[5];

    ctrls[0] = mHaveBtk ? MR::getBtkCtrl(mActor) : nullptr;
    ctrls[1] = mHaveBpk ? MR::getBpkCtrl(mActor) : nullptr;
    ctrls[2] = mHaveBtp ? MR::getBtpCtrl(mActor) : nullptr;
    ctrls[3] = mHaveBrk ? MR::getBrkCtrl(mActor) : nullptr;
    ctrls[4] = mHaveBva ? MR::getBvaCtrl(mActor) : nullptr;

    for (int i = 0; i < 5; i++) {
        J3DFrameCtrl* ctrl = ctrls[i];
        if (ctrl != nullptr) {
            ctrl->mAttribute = rBck.mLoopMode;
            ctrl->mStart = rBck.mStartFrame;
            ctrl->mFrame = (f32)rBck.mStartFrame;
            ctrl->mLoop = rBck.mRepeatFrame;
            ctrl->mFrame = (f32)rBck.mStartFrame;
            ctrl->mEnd = rBck.mEndFrame;
            ctrl->mRate = 1.0f;
        }
    }

}


void DemoTalkAnimCtrl::updatePause() {
    const char* name = MR::getCurrentDemoPartNameMain(mDemoName);
    if (DemoFunction::isPauseTimeKeepDemo(mDemoName)) {
        if (!MR::isSystemTalking() && !InformationObserverFunction::isActive() && !MR::isStoryBookTextAppeared()) {
            DemoFunction::resumeTimeKeepDemo(mDemoName);
        }
    } else if (MR::isDemoPartStep(name, MR::getDemoPartTotalStep(name) - 2)) {
        DemoFunction::pauseTimeKeepDemo(mDemoName);
    }
}

void DemoTalkAnimCtrl::updateCamera() {
    char buf[0x100];
    snprintf(buf, sizeof(buf), "%s%s", mActor->mName, mCameraName);
    if (DemoFunction::isPauseTimeKeepDemo(mDemoName)) {
        MR::pauseOnAnimCamera(mCameraInfo, buf);
    } else {
        MR::pauseOffAnimCamera(mCameraInfo, buf);
    }
}

void DemoTalkAnimCtrl::endCamera() {
    MR::endAnimCamera(mActor, mCameraInfo, mCameraName, mCameraEndFrame, true);
}

namespace {
    s32 calcDemoPartStep(const char* pExecutorName, const char* pMainPartName) {
        DemoExecutor* exec = DemoFunction::findDemoExecutor(pExecutorName);

        DemoSubPartInfo* subpart;
        DemoSubPartKeeper* subpartkeeper = exec->mSubPartKeeper;
        for (int i = 0; i < subpartkeeper->mNumSubPartInfos; i++) {
            subpart = &subpartkeeper->mSubPartInfos[i];
            if (MR::isEqualString(pMainPartName, subpart->mMainPartName) && MR::isEqualSubString(subpart->mSubPartName, "会話アニメループ")) {
                return subpart->mMainPartStep;
            }
        }
        return 0;
    }
};  // namespace



// NONMATCHING: regalloc
void DemoTalkAnimCtrl::createBckCtrlData(BckCtrlData* data, s32 totalSteps) const {
    s16 start = MR::max(mAnimFrame - 1, 0);
    bool storyBook = DemoFunction::isStoryBookActionActive();
    u8 loop = 2;
    if (storyBook) loop = 0;
    data->mLoopMode = loop;
    data->mStartFrame = start + 1;
    data->mRepeatFrame = start + 1;
    bool extra = MR::isDemoSubPartLastStep() || !mIsWaitUserInput;
    s16 end = start + totalSteps;
    if (extra) end = start + totalSteps + 1;
    data->mEndFrame = end;
    data->mInterpole = mInterpole;
    if (mIsWaitUserInput) {
        s32 step = calcDemoPartStep(mDemoName, mCurrentPartName);
        if (step > 0) data->mRepeatFrame = start + step;
    }
}

bool DemoTalkAnimCtrl::updateDemoPart() {
    if (!mCurrentPartName) {
        startAnim();
        mCurrentPartName = "";
    }
    const char* name = MR::getCurrentDemoPartNameMain(mDemoName);
    bool first;
    if (MR::isEqualString(mCurrentPartName, "")) first = MR::isDemoPartStep(name, 1);
    else first = MR::isDemoPartFirstStep(name);
    if (!first) return false;
    mIsWaitUserInput = DemoFunction::isTalkWaitUserInputFunction(name);
    mCurrentPartName = name;
    BckCtrlData data;
    s32 steps = MR::getDemoPartTotalStep(name);
    createBckCtrlData(&data, steps);
    if (mHaveBck && !DemoFunction::isDemoActionAnimRegistered(mActor)) {
        startAnim();
        MR::reflectBckCtrlData(mActor, data);
    }
    updateAnim(data);
    if (mHaveDemoPlayerBck && !DemoFunction::isDemoPlayerAnimRegistered()) MR::startDemoBckPlayer(mAnimName, data);
    mPartStep = 0;
    mAnimFrame += steps;
    return true;
}

void DemoTalkAnimCtrl::setupStartDemoPart(const char* pPartName) {
    if (mTargetPartName != nullptr) {
        mIsTargetPart = MR::isEqualStringCase(pPartName, mTargetPartName);
    }
}
