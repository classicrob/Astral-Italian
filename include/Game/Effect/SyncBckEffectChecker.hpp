#pragma once

#include <revolution/types.h>

class SyncBckEffectInfo;
class XanimePlayer;

class SyncBckEffectChecker {
public:
    SyncBckEffectChecker(XanimePlayer* pPlayer, s32 index);

    void updateBefore();
    void updateAfter();
    void reset();
    bool isCreate(const SyncBckEffectInfo* pInfo, bool checkFrame) const;
    bool isDelete(const SyncBckEffectInfo* pInfo) const;
    bool checkPass(f32 frame) const;
    bool checkPassIfRate0(f32 frame) const;

    /* 0x00 */ XanimePlayer* mPlayer;
    /* 0x04 */ s32 mIndex;
    /* 0x08 */ f32 mPrevFrame;
    /* 0x0C */ bool mIsReset;
    /* 0x0D */ bool mIsCreateEnabled;
    /* 0x10 */ const char* mCurrentBckName;
    /* 0x14 */ const char* mPrevBckName;
    /* 0x18 */ const char* _18;
    /* 0x1C */ const char* _1C;
};
