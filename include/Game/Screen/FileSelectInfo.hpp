#pragma once

#include "Game/Screen/LayoutActor.hpp"
#include "Game/System/NerveExecutor.hpp"

namespace FileSelectInfoSub { class SlideState; }

struct FileSelectFileInfo {
    /* 0x00 */ u16* mName;
    /* 0x04 */ s32 mFileId;
    /* 0x08 */ s32 mPowerStarNum;
    /* 0x0C */ s32 mGrandStarNum;
    /* 0x10 */ bool mHas120Stars;
    /* 0x11 */ bool mIsViewed120StarsEnding;
    /* 0x12 */ bool mIsComplete;
    /* 0x13 */ bool mHas240Stars;
    /* 0x14 */ const wchar_t* mDate;
    /* 0x18 */ const wchar_t* mTime;
    /* 0x1C */ s32 mMissCount;
    /* 0x20 */ const wchar_t* mPlayTime;
};

class FileSelectInfo : public LayoutActor {
public:
    /// @brief Creates a new `FileSelectInfo`.
    /// @param nameBufferSize The number of wide characters to allocate memory for in the file name buffer.
    /// @param pName The pointer to the null-terminated name of the object.
    FileSelectInfo(s32 nameBufferSize, const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void control();

    void disappear();
    void slide();
    void slideBack();
    void setInfo(const FileSelectFileInfo&);
    void exeAppear();
    void exeDisplay();
    void exeDisappear();
    void reflectInfo();
    void setMissCounter();
    void setCompleteness();
    void setMessage();

private:
    /* 0x2C */ s32 mNumber;
    /* 0x30 */ s32 mStarNum;
    /* 0x34 */ s32 mNameBufferSize;
    /* 0x38 */ wchar_t* mName;
    /* 0x3C */ wchar_t mDateMessage[32];
    /* 0x7C */ wchar_t mTimeMessage[32];
    /* 0xBC */ s32 mMissNum;
    /* 0xC0 */ s32 mGrandStarNum;
    /* 0xC4 */ bool mHas120Stars;
    /* 0xC5 */ bool mIsViewed120StarsEnding;
    /* 0xC6 */ bool mIsComplete;
    /* 0xC7 */ bool mHas240Stars;
    /* 0xC8 */ bool mShowMissCounter;
    /* 0xC9 */ bool mShow120StarsMessage;
    /* 0xCA */ wchar_t mPlayTimeMessage[32];
    /* 0x10C */ FileSelectInfoSub::SlideState* mSlideState;
};

namespace FileSelectInfoSub {
    class SlideState : public NerveExecutor {
    public:
        /// @brief Creates a new `SlideState`.
        /// @param pHost TODO.
        SlideState(FileSelectInfo* pHost);

        void exeNormalPos();
        void exeSliding();
        void exeSlidePos();
        void exeSlidingBack();

    private:
        /* 0x8 */ FileSelectInfo* mHost;
    };

}
