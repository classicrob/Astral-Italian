#pragma once

#include <revolution/types.h>

class ModelManager;

// One row of the ActionScreenBlurCtrl csv (0x20 bytes).
class ActionScreenBlurInfo {
public:
    ActionScreenBlurInfo();

    /* 0x00 */ const char* mBckName;
    /* 0x04 */ f32 mStartFrame;
    /* 0x08 */ s32 mTotalStep;
    /* 0x0C */ f32 mOffset;
    /* 0x10 */ s32 mAlpha;
    /* 0x14 */ s32 mFadeInStep;
    /* 0x18 */ s32 mFadeOutStep;
    /* 0x1C */ u8 mIsEnabled;
};

// SMG2-only controller that starts a center screen blur while the model plays a matching bck (no Petari counterpart).
class ActionScreenBlurCtrl {
public:
    ActionScreenBlurCtrl(const ModelManager*);

    static ActionScreenBlurCtrl* tryCreate(const ModelManager*);

    void update();
    void updateInfoEnabled();   // fn_8022A830

    /* 0x00 */ const ModelManager* mModelManager;
    /* 0x04 */ const char* mPlayingBckName;
    /* 0x08 */ s32 mInfoNum;
    /* 0x0C */ ActionScreenBlurInfo* mInfos;
};
