#pragma once

#include "Game/Boss/PoltaActionBase.hpp"

class Polta;

class PoltaDemo : public PoltaActionBase {
public:
    PoltaDemo(Polta*);

    virtual ~PoltaDemo();
    virtual void init();
    virtual void control();

    void startDemoSetting();
    void startOpeningDemo();
    void startPowerUpDemo();
    void startDownDemo();
    void startCameraEndDemo(const char*);
    void startTryDemo(const char*, const Nerve*);

    void exeTryStartDemo();
    void exeOpeningDemo();
    void exePowerUpDemo();
    void exeDownDemo();
    void exeCameraEndDemo();

    /* 0x14 */ const Nerve* mCurDemoNerve;
    /* 0x18 */ const char* mCurDemoName;
    /* 0x1C */ const char* mCameraName;
};
