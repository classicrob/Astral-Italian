#pragma once

#include <revolution/hbm.h>
#include <revolution/types.h>

class WPad;

class WPadReadDataInfo {
public:
    WPadReadDataInfo();

    KPADStatus* getKPadStatus(u32) const;
    KPADStatus* getReadBuffer();
    u32 getValidStatusCount() const;
    void copy(const WPadReadDataInfo*);

    /* 0x00 */ KPADStatus* mStatusArray;
    /* 0x04 */ u32 mValidStatusCount;
};

class WPadHolder {
public:
    enum Mode {
        MODE_HOME_BUTTON = 0,
        MODE_GAME = 1,
    };

    WPadHolder();

    virtual void readPadData();

    void updateReadDataOnly();
    void updateProjectPadData();
    void updateRumbleAndInfoChecker();
    void updateInGame();
    void update();
    void initSensorBarPosition();
    WPad* getWPad(s32);
    static void setConnectCallback();

    // PLACEHOLDER(core): Petari-only, SMG2 has no WPadHolder::resetPad; resolve when GameSystemSceneController (MR::resetWPad caller) is ported from the target
    void resetPad();

    /* 0x04 */ WPad* mPad[2];
    /* 0x0C */ WPadReadDataInfo* mRawReadDataInfoArray;
    /* 0x10 */ WPadReadDataInfo* mReadDataInfoArray;
    /* 0x14 */ KPADUnifiedWpadStatus* mUnifiedStatusBuffer;
    /* 0x18 */ s32 mMode;
};

namespace MR {
    WPad* getWPad(s32);
    void setWPadHolderModeHomeButton();
    void setWPadHolderModeGame();
    u32 getHBMKPadData(HBMKPadData*, s32);

    // PLACEHOLDER(core): Petari-only, absent from SMG2; resolve when GameSystemSceneController is ported from the target
    void resetWPad();
    // PLACEHOLDER(core): Petari-only, absent from SMG2; resolve when GameSystemFunction::setAutoSleepTimeWiiRemote is ported from the target
    void setAutoSleepTimeWiiRemote(bool);
};  // namespace MR
