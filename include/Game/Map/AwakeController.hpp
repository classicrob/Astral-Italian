#pragma once

#include <revolution/types.h>

class JMapInfoIter;
class SwitchEventListener;
class SwitchIdInfo;

class AwakeController {
public:
    /// @brief Creates a new `AwakeController`.
    /// @param rIter The map object iterator that holds the switch settings.
    /// @param pListener The listener notified when the awake switch turns on or off.
    AwakeController(const JMapInfoIter& rIter, SwitchEventListener* pListener);

    void initSync();
    void update();

    /* 0x00 */ SwitchIdInfo* mSwitchIdInfo;
    /* 0x04 */ SwitchEventListener* mSwitchEventListener;
    /* 0x08 */ bool mIsOnSwitchByIdInfo;
};
