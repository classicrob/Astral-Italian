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
    /* 0x0C */ const char* mDemoName;
    /* 0x10 */ const char* mTargetPartName;
    /* 0x14 */ const char* mCurrentPartName;
    /* 0x18 */ const char* mAnimName;
    /* 0x1C */ const char* mCameraName;
    /* 0x20 */ ActorCameraInfo* mCameraInfo;
    /* 0x24 */ s32 mAnimFrame;
    /* 0x28 */ s32 mPartStep;
    /* 0x2C */ s32 mTotalStep;
    /* 0x30 */ bool mIsTargetPart;
    /* 0x31 */ bool mIsTimeKeep;
    /* 0x32 */ bool mIsWaitUserInput;
    /* 0x34 */ s32 mInterpole;
    /* 0x38 */ s32 mCameraStartFrame;
    /* 0x3C */ s32 mCameraEndFrame;
    /* 0x40 */ bool mHaveDemoPlayerBck;
    /* 0x41 */ bool mHaveCamera;
    /* 0x42 */ bool mHaveBck;
    /* 0x43 */ bool mHaveBtk;
    /* 0x44 */ bool mHaveBpk;
    /* 0x45 */ bool mHaveBtp;
    /* 0x46 */ bool mHaveBrk;
    /* 0x47 */ bool mHaveBva;
};
