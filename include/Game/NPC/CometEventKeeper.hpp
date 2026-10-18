#pragma once

#include <revolution/types.h>

class CometEventExecutor;
class GalaxyCometScreenFilter;
class CometEventKeeper {
public:
    CometEventKeeper();
    void init();
    bool isStartEvent(const char*) const;
    void startCometEventIfExist();
    void endCometEvent();
    void stopCometEvent();
    void pauseCometTimer();
    void resumeCometTimer();
    void showCometTimer();
    void hideCometTimer();
    void addCometTimer(s32);
    void decCometTimer(s32);
    s32 getTimeLimitFromScenarioData();
    void initCometStatus();

    /* 0x00 */ CometEventExecutor* mExecutor;
    /* 0x04 */ GalaxyCometScreenFilter* mScreenFilter;
    /* 0x08 */ u32 _8;
    /* 0x0C */ const char* mCometName;
};
