#pragma once

#include "Game/Boss/BossStinkBugActionBase.hpp"

class DemoPositionController;
class JMapInfoIter;

class BossStinkBugAngryDemo : public BossStinkBugActionBase {
public:
    BossStinkBugAngryDemo(BossStinkBug* pHost, const JMapInfoIter& rIter);

    virtual void appear();

    void exeDemo();
    void exeTryStart();
    void exeDemoEnd();

    /* 0x64 */ DemoPositionController* mDemoPositionController;
};
