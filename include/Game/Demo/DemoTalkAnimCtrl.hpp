#pragma once

#include "Game/System/NerveExecutor.hpp"
#include <revolution/types.h>

class ActorCameraInfo;
class BckCtrlData;
class JMapInfoIter;
class LiveActor;

class DemoTalkAnimCtrl : public NerveExecutor {
public:
    DemoTalkAnimCtrl(LiveActor*, const char*, const char*);

    void initForScene(const char*, const char*, const JMapInfoIter&);
    void updateCamera();
    void createBckCtrlData(BckCtrlData*, s32) const;
    void initForActor(const char*);
    void startDemo();
    void endDemo();
    void updateStoryBook();
    void pause();
    void resume();
    bool updateDemoPart();
    bool updateDemo();
    void startAnim();
    void startCamera();
    void updateAnim(const BckCtrlData&);
    void updatePause();
    void endCamera();
    void setupStartDemoPart(const char*);

    /* 0x08 */ LiveActor* mActor;
    /* 0x0C */ const char* _C;
    /* 0x10 */ const char* _10;
    /* 0x14 */ const char* _14;
    /* 0x18 */ const char* _18;
    /* 0x1C */ const char* _1C;
    /* 0x20 */ ActorCameraInfo* mCameraInfo;
    /* 0x24 */ s32 _24;
    /* 0x28 */ s32 _28;
    /* 0x2C */ s32 _2C;
    /* 0x30 */ bool _30;
    /* 0x31 */ bool _31;
    /* 0x32 */ bool _32;
    /* 0x34 */ s32 _34;
    /* 0x38 */ s32 _38;
    /* 0x3C */ s32 _3C;
    /* 0x40 */ bool _40;
    /* 0x41 */ bool mHaveCamera;
    /* 0x42 */ bool _42;
    /* 0x43 */ bool mHaveBtk;
    /* 0x44 */ bool mHaveBpk;
    /* 0x45 */ bool mHaveBtp;
    /* 0x46 */ bool mHaveBrk;
    /* 0x47 */ bool mHaveBva;
};
