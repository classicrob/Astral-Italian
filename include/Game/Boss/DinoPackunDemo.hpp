#pragma once

#include "Game/Boss/DinoPackunAction.hpp"

class DinoPackunDemo : public DinoPackunAction {
public:
    DinoPackunDemo(DinoPackun*);

    virtual ~DinoPackunDemo();
    virtual void init();
    virtual void control();

    void startOpeningDemo();
    void startCryDemo();
    void startAngryDemo();
    void startDownDemo();
    void startTryDemo(const char*, const Nerve*);
    void startCameraEndDemo(const char*);
    void exeTryStartDemo();
    void exeOpeningDemo();
    void exeCryDemo();
    void exeAngryDemo();
    void exeDownDemo();
    void exeAppearPowerStar();
    void exeCameraEndDemo();

    /* 0x14 */ const Nerve* _14;
    /* 0x18 */ const char* mDemoName;
    /* 0x1C */ const char* mCameraName;
};
