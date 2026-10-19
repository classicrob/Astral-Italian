#pragma once

#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class ChipBase;
class StageSwitchCtrl;

class ChipGroup : public NameObj {
public:
    ChipGroup(const char*, s32);

    struct ChipEntry {
        /* 0x0 */ ChipBase* mChip;
        /* 0x4 */ bool mIsGotten;
    };

    virtual ~ChipGroup();
    virtual void init(const JMapInfoIter&);
    virtual void movement();

    void updateUIRange();
    void registerChip(ChipBase*);
    void noticeEndCompleteDemo();
    s32 getGotCount() const;
    bool isComplete() const;
    void noticeGet(ChipBase*);
    void receiveAppearRequest();

    /* 0x14 */ ChipEntry mChips[5];
    /* 0x3C */ StageSwitchCtrl* mSwitchCtrl;
    /* 0x40 */ TVec3f mUIPos;
    /* 0x4C */ s32 mGotCount;
    /* 0x50 */ s32 mTotalCount;
    /* 0x54 */ s32 mStageSwitchArg;
    /* 0x58 */ s32 mChipType;
    /* 0x5C */ s32 mFlashFrame;
    /* 0x60 */ s32 mCounterTimer;
    /* 0x64 */ f32 mUIRange;
    /* 0x68 */ f32 mUIRangeFixed;
    /* 0x6C */ bool mIsDeactivated;
    /* 0x70 */ int mAlreadyDoneFlag;
    /* 0x74 */ u8 mIsListeningAppear;
    /* 0x75 */ u8 mIsCounterVisible;
};

class BlueChipGroup : public ChipGroup {
public:
    BlueChipGroup(const char*);

    virtual ~BlueChipGroup();
};

class YellowChipGroup : public ChipGroup {
public:
    YellowChipGroup(const char*);

    virtual ~YellowChipGroup();
};

void updateChipGroupUIRange(ChipGroup*, ChipBase*);
