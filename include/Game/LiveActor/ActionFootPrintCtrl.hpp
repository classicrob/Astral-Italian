#pragma once

#include <revolution/types.h>

class LiveActor;

// One row of the ActionFootPrintCtrl csv (0x10 bytes). Its ctor is compiled in the ActionFlagCtrl TU in the target.
class ActionFootPrintInfo {
public:
    ActionFootPrintInfo();

    /* 0x00 */ const char* mBckName;
    /* 0x04 */ f32 mStartFrame;
    /* 0x08 */ const char* mJointName;
    /* 0x0C */ u8 mIsRight;
    /* 0x0D */ u8 mIsEnabled;
};

// SMG2-only controller that prints footprints while the actor plays a matching bck (no Petari counterpart).
class ActionFootPrintCtrl {
public:
    ActionFootPrintCtrl(LiveActor*);

    static ActionFootPrintCtrl* tryCreate(LiveActor*);

    void update();
    void updateInfoEnabled();   // fn_80229F80

    /* 0x00 */ LiveActor* mActor;
    /* 0x04 */ const char* mPlayingBckName;
    /* 0x08 */ s32 mHandle;
    /* 0x0C */ s32 mInfoNum;
    /* 0x10 */ ActionFootPrintInfo* mInfos;
};
