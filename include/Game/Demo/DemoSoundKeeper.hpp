#pragma once

#include "Game/Demo/DemoExecutor.hpp"

class ActionSoundKeeper;

class DemoSoundInfo {
public:
    /// @brief Creates a new `DemoSoundInfo`.
    DemoSoundInfo();

    /* 0x00 */ const char* mPartName;
    /* 0x04 */ const char* mBgm;
    /* 0x08 */ const char* mSystemSe;
    /* 0x0C */ const char* mActionSe;
    /* 0x10 */ bool mReturnBgm;
    /* 0x14 */ s32 mBgmWipeoutFrame;
    /* 0x18 */ s32 mAllSoundStopFrame;
};

class DemoSoundKeeper : public DemoSheetKeeperBase, public DemoSheetKeeperInfoHolder< DemoSoundInfo > {
public:
    DemoSoundKeeper(DemoExecutor* pExecutor);

    virtual const char* getName() const;

    virtual const char* getTypeString() const;

    virtual void start();
    virtual void update();

    virtual void executeType(const DemoSoundInfo* pInfo);

    void initActionSoundKeeper();
    bool isPermitBgmChange();

    /* 0x18 */ ActionSoundKeeper* mActionSoundKeeper;
};
