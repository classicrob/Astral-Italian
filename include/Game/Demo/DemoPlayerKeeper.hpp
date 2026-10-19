#pragma once

#include "Game/Demo/DemoExecutor.hpp"

class DemoPlayerInfo {
public:
    DemoPlayerInfo();

    /* 0x0 */ const char* mPartName;
    /* 0x4 */ const char* mPosName;
    /* 0x8 */ const char* mBckName;
    /* 0xC */ s32 mVisible; // -1 when the sheet has no "Visible" column, 0 hide, 1 show
};

class DemoPlayerKeeper {
public:
    DemoPlayerKeeper(const DemoExecutor*);
    const DemoPlayerInfo* findPlayerInfo(const char*) const;
    void start();
    void update();
    bool isExistPosName() const;
    void executePlayer(const DemoPlayerInfo*) const;

    /* 0x0 */ const DemoExecutor* mExecutor;
    /* 0x4 */ s32 mNumPlayerInfos;
    /* 0x8 */ DemoPlayerInfo* mPlayerInfos;
    /* 0xC */ bool mIsExecutedPlayer;
};
