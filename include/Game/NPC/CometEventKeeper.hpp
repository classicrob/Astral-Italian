#pragma once

#include <revolution.h>

class CometEventExecutorTimeLimit;
class GalaxyCometScreenFilter;
class GalaxyTimeLimitInfo;

class CometEventKeeper {
public:
    /// @brief Creates a new `CometEventKeeper`.
    CometEventKeeper();

    void init();
    bool isStartEvent(const char*) const;
    void startCometEventIfExist();
    void endCometEvent();
    void pauseCometTimer();
    void resumeCometTimer();
    void showCometTimer();
    void hideCometTimer();
    void addCometTimer(s32);
    void decCometTimer(s32);
    void initCometStatus();

private:
    /* 0x0 */ CometEventExecutorTimeLimit* mExecutorTimeLimit;
    /* 0x4 */ GalaxyCometScreenFilter* mScreenFilter;
    /* 0x8 */ s32 mCometStateIndex;
    /* 0xC */ const char* mCometName;
};
