#include "Game/Demo/DemoTalkAnimCtrl.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoFunction.hpp"
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
    : NerveExecutor(a3), mActor(pActor), _C(a3), _10(a4), _14(nullptr), _18(""), _1C(""), mCameraInfo(nullptr), _24(0), _28(0), _2C(0),
      _30(true), _31(false), _32(false), _34(0), _38(0), _3C(0), _40(false), mHaveCamera(false), _42(false), mHaveBtk(false), mHaveBpk(false),
      mHaveBtp(false), mHaveBrk(false), mHaveBva(false) {
}

void DemoTalkAnimCtrl::initForActor(const char* a1) {
    _31 = false;
    _30 = true;
    mHaveCamera = false;
    _40 = false;
    _18 = a1;
    _14 = nullptr;
    _24 = 0;
    _28 = 0;
    _2C = 0;
    _32 = false;
    mCameraInfo = nullptr;
    _1C = "";
    _42 = MR::isExistBck(mActor, a1);
    mHaveBtk = MR::isExistBtk(mActor, _18);
    mHaveBpk = MR::isExistBpk(mActor, _18);
    mHaveBtp = MR::isExistBtp(mActor, _18);
    mHaveBrk = MR::isExistBrk(mActor, _18);
    mHaveBva = MR::isExistBva(mActor, _18);
}


void DemoTalkAnimCtrl::initForScene(const char* a1, const char* a2, const JMapInfoIter& rIter) {
    char buf[0x100];
    snprintf(buf, sizeof(buf), "%s.canm", a2);
    initForActor(a1);
    _31 = true;
    _30 = _10 == nullptr;
    mHaveCamera = MR::getResourceHolder(mActor)->mFileInfoTable->isExistRes(buf);
    _40 = MR::isExistBck(MR::getPlayerDemoActor(), a1);
    if (mHaveCamera) {
        _1C = a2;
        mCameraInfo = MR::createActorCameraInfo(rIter);
        MR::initAnimCamera(mActor, mCameraInfo, _1C);
    }
}

void DemoTalkAnimCtrl::startDemo() {
    _14 = nullptr;
    _24 = 0;
    _28 = 0;
    _2C = 0;
    _32 = false;
    if (mHaveCamera && _30) startCamera();
}

bool DemoTalkAnimCtrl::updateDemo() {
    if (!_30) return false;
    updateDemoPart();
    if (_31 && _32) updatePause();
    if (mHaveCamera) updateCamera();
    if (DemoFunction::isDemoSuspendOrLastPartLastStep()) {
        if (mHaveCamera) endCamera();
        return false;
    }
    if (!DemoFunction::isPauseTimeKeepDemo(_C)) {
        _2C++;
        _28++;
    }
    return true;
}

void DemoTalkAnimCtrl::endDemo() {
    if (mHaveCamera && _30) MR::endAnimCamera(mActor, mCameraInfo, _1C, 0, true);
}

void DemoTalkAnimCtrl::updateStoryBook() {
    updateDemo();
    startAnim();
    MR::setAllAnimFrameAndStop(mActor, _18, _24);
    if (MR::isStoryBookEndingDemoName(_C)) MR::fn_8000A320(mActor);
    if (_40) {
        BckCtrlData data;
        data.mPlayFrame = 0;
        data.mStartFrame = _24;
        MR::startDemoBckPlayer(_18, data);
    }
}

void DemoTalkAnimCtrl::pause() {
    if (_31) DemoFunction::pauseTimeKeepDemo(_C);
    if (mHaveCamera) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s%s", mActor->mName, _1C);
        MR::pauseOnAnimCamera(mCameraInfo, buf);
    }
}

void DemoTalkAnimCtrl::resume() {
    if (_31) DemoFunction::resumeTimeKeepDemo(_C);
}

void DemoTalkAnimCtrl::startAnim() {
    MR::startAction(mActor, _18);
    if (MR::isStoryBookEndingDemoName(_C)) MR::fn_8000A400(mActor);
}

void DemoTalkAnimCtrl::startCamera() {
    MR::startAnimCameraTargetSelf(mActor, mCameraInfo, _1C, _38, false, 1.0f);
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
    const char* name = MR::getCurrentDemoPartNameMain(_C);
    if (DemoFunction::isPauseTimeKeepDemo(_C)) {
        if (!MR::isSystemTalking() && !InformationObserverFunction::isActive() && !MR::isStoryBookTextAppeared()) {
            DemoFunction::resumeTimeKeepDemo(_C);
        }
    } else if (MR::isDemoPartStep(name, MR::getDemoPartTotalStep(name) - 2)) {
        DemoFunction::pauseTimeKeepDemo(_C);
    }
}

void DemoTalkAnimCtrl::updateCamera() {
    char buf[0x100];
    snprintf(buf, sizeof(buf), "%s%s", mActor->mName, _1C);
    if (DemoFunction::isPauseTimeKeepDemo(_C)) {
        MR::pauseOnAnimCamera(mCameraInfo, buf);
    } else {
        MR::pauseOffAnimCamera(mCameraInfo, buf);
    }
}

void DemoTalkAnimCtrl::endCamera() {
    MR::endAnimCamera(mActor, mCameraInfo, _1C, _3C, true);
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



void DemoTalkAnimCtrl::createBckCtrlData(BckCtrlData* data, s32 totalSteps) const {
    s16 start = MR::max(_24 - 1, 0);
    u8 loop = 2;
    if (DemoFunction::isStoryBookActionActive()) loop = 0;
    data->mLoopMode = loop;
    data->mStartFrame = start + 1;
    data->mRepeatFrame = start + 1;
    bool extra = false;
    if (MR::isDemoSubPartLastStep() || !_32) extra = true;
    s16 end = start + totalSteps;
    if (extra) end = start + totalSteps + 1;
    data->mEndFrame = end;
    data->mInterpole = _34;
    if (_32) {
        s32 step = calcDemoPartStep(_C, _14);
        if (step > 0) data->mRepeatFrame = start + step;
    }
}

bool DemoTalkAnimCtrl::updateDemoPart() {
    if (!_14) {
        startAnim();
        _14 = "";
    }
    const char* name = MR::getCurrentDemoPartNameMain(_C);
    bool first;
    if (MR::isEqualString(_14, "")) first = MR::isDemoPartStep(name, 1);
    else first = MR::isDemoPartFirstStep(name);
    if (!first) return false;
    _32 = DemoFunction::isTalkWaitUserInputFunction();
    _14 = name;
    BckCtrlData data;
    s32 steps = MR::getDemoPartTotalStep(name);
    createBckCtrlData(&data, steps);
    if (_42 && !DemoFunction::isDemoActionAnimRegistered(mActor)) {
        startAnim();
        MR::reflectBckCtrlData(mActor, data);
    }
    updateAnim(data);
    if (_40 && !DemoFunction::isDemoPlayerAnimRegistered()) MR::startDemoBckPlayer(_18, data);
    _28 = 0;
    _24 += steps;
    return true;
}

void DemoTalkAnimCtrl::setupStartDemoPart(const char* pPartName) {
    if (_10 != nullptr) {
        _30 = MR::isEqualStringCase(pPartName, _10);
    }
}
