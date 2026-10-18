#pragma once
#include <revolution/types.h>
class CometEventExecutor {
public:
    virtual void init(s32) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pauseCometTimer() = 0;
    virtual void resumeCometTimer() = 0;
    virtual void showCometTimer() = 0;
    virtual void hideCometTimer() = 0;
    virtual void addCometTimer(s32) = 0;
    virtual void readdCometTimer() = 0;
    virtual void decCometTimer(s32) = 0;
};
