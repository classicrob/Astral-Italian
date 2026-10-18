#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ModelObj;
class SimpleLayout;

class OpeningDemoObj : public LiveActor {
public:
    OpeningDemoObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual ~OpeningDemoObj();

    void exeStart();
    void exeWaitAnim();
    void exeMain();
    void exeTalk();
    void exeFinish();
    void exeFall();
    void exeEnd();
    void updateActionSound(s32 step);

    /* 0x90 */ ModelObj* mModelA;
    /* 0x94 */ ModelObj* mModelB;
    /* 0x98 */ SimpleLayout* mLayoutA;
    /* 0x9C */ SimpleLayout* mLayoutB;
    /* 0xA0 */ ModelObj* mModelC;
};
