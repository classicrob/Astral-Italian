#pragma once

#include "Game/Screen/LayoutActor.hpp"

class StarPointerGuidance : public LayoutActor {
public:
    /// @brief Creates a new `StarPointerGuidance`
    /// @param pName A pointer to the null-terminated name of the object.
    StarPointerGuidance(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();

    void checkRequest1P();
    void exeAppear1P();
    void exeWait1P();
    void exeEnd1P();
    void exeAppearBlueStarGuide();
    void exeWaitBlueStarGuide();
    void exeEndBlueStarGuide();
    void exeEndWait();
    s32 exeAppearCore(Spine*, const char*, u32, const Nerve*);
    s32 exeWaitCore(Spine*, const char*, u32);
    s32 exeEndCore(Spine*, const char*, u32);
    bool request1PGuidance(const char*, bool);
    bool isExistGuidanceOrFrame();
    bool isExistGuidance();
    bool isExistFrame1P() NO_INLINE;
    void deactive();

    void active() {
        mIsActive = true;
    }

    /* 0x2C */ Spine* mSpineFrame1P;
    /* 0x30 */ Spine* mSpineGuidance;
    /* 0x34 */ s32 mOutScreenTime;
    /* 0x38 */ s32 mRequestTime1P;
    /* 0x3C */ const wchar_t* mGuidanceMessage;
    /* 0x40 */ const wchar_t* mPrevGuidanceMessage;
    /* 0x44 */ bool mIsActiveRequest1P;
    /* 0x45 */ bool mIsActive;
    /* 0x46 */ bool mIsGuidanceDisabled;
    /* 0x48 */ u32 mTextLineNumGuidance;
    /* 0x4C */ u32 mTextLineNumFrame;
    /* 0x50 */ bool mIsTimeOutEnabled;
};
