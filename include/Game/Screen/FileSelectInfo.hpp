#pragma once

#include "Game/Screen/LayoutActor.hpp"
#include "Game/System/NerveExecutor.hpp"

namespace FileSelectInfoSub {
    class SlideState;
    class CharaState;
};  // namespace FileSelectInfoSub

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
    void setInfo(u16*, s32, s32, s32, bool, bool, bool, const wchar_t*, const wchar_t*, s32);
    void change();
    void forceChange();
    void exeAppear();
    void exeDisplay();
    void exeDisappear();
    void reflectInfo();

private:
    /* 0x20 */ s32 mNumber;
    /* 0x24 */ s32 mStarNum;
    /* 0x28 */ s32 mStarPieceNum;
    /* 0x2C */ s32 mNameBufferSize;
    /* 0x30 */ wchar_t* mName;
    /* 0x34 */ wchar_t mDateMessage[32];
    /* 0x74 */ wchar_t mTimeMessage[32];
    /* 0xB4 */ s32 mMissNum;
    /* 0xB8 */ bool mIsSelectedMarioPrev;
    /* 0xB9 */ bool mIsSelectedMario;
    /* 0xBA */ bool mIsViewNormalEnding;
    /* 0xBB */ bool mIsViewCompleteEnding;
    /* 0xBC */ FileSelectInfoSub::SlideState* mSlideState;
    /* 0xC0 */ FileSelectInfoSub::CharaState* mCharaState;
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

    class CharaState : public NerveExecutor {
    public:
        /// @brief Creates a new `CharaState`.
        /// @param pHost TODO.
        CharaState(FileSelectInfo* pHost);

        void exeMario();
        void exeToLuigi();
        void exeLuigi();
        void exeToMario();

    private:
        /* 0x8 */ FileSelectInfo* mHost;
    };
};  // namespace FileSelectInfoSub
