#include "Game/Map/AwakeControllerHolder.hpp"
#include "Game/Map/AwakeController.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Map/SwitchWatcher.hpp"
#include "Game/Util/ObjUtil.hpp"
#include <algorithm>

AwakeController::AwakeController(const JMapInfoIter& rIter, SwitchEventListener* pListener) {
    mSwitchEventListener = pListener;
    mSwitchIdInfo = NULL;
    mIsOnSwitchByIdInfo = false;
    mSwitchIdInfo = StageSwitchFunction::createSwitchIdInfo("SW_AWAKE", rIter, false);
}

void AwakeController::initSync() {
    if (StageSwitchFunction::isOnSwitchBySwitchIdInfo(*mSwitchIdInfo)) {
        mSwitchEventListener->listenSwitchOnEvent();
    } else {
        mSwitchEventListener->listenSwitchOffEvent();
    }
}

void AwakeController::update() {
    bool isOn = StageSwitchFunction::isOnSwitchBySwitchIdInfo(*mSwitchIdInfo);

    if (!mIsOnSwitchByIdInfo && isOn) {
        mSwitchEventListener->listenSwitchOnEvent();
    }

    if (mIsOnSwitchByIdInfo && !isOn) {
        mSwitchEventListener->listenSwitchOffEvent();
    }

    mIsOnSwitchByIdInfo = isOn;
}

AwakeControllerHolder::AwakeControllerHolder() : NameObj("\x83\x49\x83\x75\x83\x57\x83\x46\x8b\x4e\x8f\xb0\x8a\xc7\x97\x9d") {
    MR::connectToSceneMapObjMovement(this);
}

void AwakeControllerHolder::movement() {
    std::for_each(mAwakeControl.begin(), mAwakeControl.end(), std::mem_func(&AwakeController::update));
}

void AwakeControllerHolder::add(NameObj*, const JMapInfoIter& rIter, SwitchEventListener* pListener) {
    mAwakeControl.push_back(new AwakeController(rIter, pListener));
}

void AwakeControllerHolder::initSync() {
    std::for_each(mAwakeControl.begin(), mAwakeControl.end(), std::mem_func(&AwakeController::initSync));
}
