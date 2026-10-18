#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JMapInfoIter;

// Demo object that switches the camera back from the end start position (SMG2 layout: LiveActor only, size 0x90).
class DemoEndStartPosCameraSetter : public LiveActor {
public:
    DemoEndStartPosCameraSetter(const char*);

    virtual ~DemoEndStartPosCameraSetter();
    virtual void init(const JMapInfoIter&);

    void registerDemo(const char*, const JMapInfoIter&);
    void endStartPosCamera();   // fn_80136920
};
