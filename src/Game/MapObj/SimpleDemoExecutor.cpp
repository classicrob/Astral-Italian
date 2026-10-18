#pragma auto_inline off
#include "Game/MapObj/SimpleDemoExecutor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Util/JMapLinkInfo.hpp"
#include "Game/Util.hpp"
#include <cstdio>
#include <cstring>
namespace {
const char* sCameraName = "簡易デモ";
}
namespace NrvSimpleDemoExecutor {
NEW_NERVE(SimpleDemoExecutorNrvWait, SimpleDemoExecutor, Wait);
NEW_NERVE(SimpleDemoExecutorNrvDemo, SimpleDemoExecutor, Demo);
}
SimpleDemoExecutor::SimpleDemoExecutor(const char* name) : LiveActor(name), mTarget(0), mLinkInfo(0), mDemoName(0), mCameraFrames(-1), mSwitchFrame(60), mCameraInfo(0) {}
void SimpleDemoExecutor::init(const JMapInfoIter& iter) {
    MR::initDefaultPos(this, iter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    MR::syncStageSwitchAppearAndSync(this, iter);
    mLinkInfo = new JMapLinkInfo(iter, false);
    s32 cameraId = -1;
    MR::getJMapInfoCameraSetID(iter, &cameraId);
    char name[256];
    snprintf(name, sizeof(name), "%s%02d", sCameraName, cameraId);
    u32 length = strlen(name) + 1;
    mDemoName = new char[length];
    snprintf(mDemoName, length, "%s", name);
    MR::getJMapInfoArg1NoInit(iter, &mSwitchFrame);
    MR::initUseStageSwitchWriteA(this, iter);
    MR::initMultiActorCamera(this, iter, &mCameraInfo, sCameraName);
    initNerve(&NrvSimpleDemoExecutor::SimpleDemoExecutorNrvWait::sInstance);
    MR::registerDemoSimpleCastAll(this);
    makeActorDead();
}
void SimpleDemoExecutor::initAfterPlacement() {
    if (mLinkInfo->isValid()) mTarget = MR::getLinkedActor(mLinkInfo);
    if (!mTarget) mTarget = this;
    mCameraFrames = MR::getMultiActorCameraFrames(this, mCameraInfo, sCameraName);
}
void SimpleDemoExecutor::exeWait() {
    if (MR::tryStartDemo(mTarget, mDemoName)) {
        MR::startMultiActorCameraTargetOther(this, mCameraInfo, sCameraName, CameraTargetArg(mTarget), mCameraFrames);
        setNerve(&NrvSimpleDemoExecutor::SimpleDemoExecutorNrvDemo::sInstance);
    }
}
void SimpleDemoExecutor::exeDemo() {
    if (MR::isStep(this, mSwitchFrame) && MR::isValidSwitchA(this)) MR::onSwitchA(this);
    if (MR::isGreaterEqualStep(this, mCameraFrames)) {
        MR::endMultiActorCamera(this, mCameraInfo, sCameraName, false, -1);
        MR::endDemo(mTarget, mDemoName);
        kill();
    }
}
SimpleDemoExecutor::~SimpleDemoExecutor() {}
