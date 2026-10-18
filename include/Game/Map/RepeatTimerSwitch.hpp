#pragma once

#include "Game/NameObj/NameObj.hpp"

class ActionSoundKeeper;
class StageSwitchCtrl;

class RepeatTimerSwitch : public NameObj {
public:
    /// @brief Creates a new `RepeatTimerSwitch`.
    /// @param pName A pointer to the null-terminated name of the object.
    RepeatTimerSwitch(const char* pName);

    virtual ~RepeatTimerSwitch();
    virtual void init(const JMapInfoIter&);
    virtual void movement();

    /* 0x14 */ StageSwitchCtrl* mStageSwitchCtrl;
    /* 0x18 */ s32 mTimeLimit;
    /* 0x1C */ s32 mTimeOffset;
    /* 0x20 */ s32 mTimer;
    /* 0x24 */ ActionSoundKeeper* mSound;
};
