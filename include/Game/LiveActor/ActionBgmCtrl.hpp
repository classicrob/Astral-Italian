#pragma once

#include <revolution/types.h>

class ModelManager;

// One BGM row of the ActionBgmCtrl csv (0x24 bytes).
class ActionBgmInfo {
public:
    ActionBgmInfo();

    /* 0x00 */ const char* mBckName;
    /* 0x04 */ f32 mBckStartFrame;
    /* 0x08 */ const char* mBgmName;
    /* 0x0C */ s32 mBgmStateNo;
    /* 0x10 */ bool mIsBgmStop;
    /* 0x14 */ s32 mBgmFadeStep;
    /* 0x18 */ const char* mStageName;
    /* 0x1C */ s32 mStageScenarioNo;
    /* 0x20 */ bool mIsBckMatch;
    /* 0x21 */ mutable bool mIsUpdated;
};

class ActionBgmCtrl {
public:
    ActionBgmCtrl(const ModelManager*);

    static ActionBgmCtrl* tryCreate(const ModelManager*);
    void update();

private:
    void findAndUpdateBgm();
    void updateBgm(const ActionBgmInfo&);

    /* 0x00 */ const ModelManager* mModelManager;
    /* 0x04 */ const char* mPlayingBckName;
    /* 0x08 */ s32 mInfoNum;
    /* 0x0C */ ActionBgmInfo* mInfos;
};
