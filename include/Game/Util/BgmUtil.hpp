#pragma once

#include <revolution.h>

class JAISoundHandle;

namespace MR {
    u32 getCurrentStageBgmId();
    JAISoundHandle* startStageBGM(const char*, bool);
    JAISoundHandle* startStageBGM(u32, bool);
    bool startStageBGMIfNotPlaying(const char*, bool);
    JAISoundHandle* startStageBGMFromStageName(const char*, const char*, s32);

    void stopStageBGM(u32);
    void stopStageBgmForDemo();  // fn_8001BA80: fades the stage BGM (10) and sets the SceneMgr flags 0xC/0xD (used by boss demos)
    void unlockStageBGM();
    void moveVolumeStageBGM(f32, u32);
    void moveVolumeStageBGMForNoteFairy(f32, u32);
    void startCurrentStageBGM(bool);
    JAISoundHandle* startLastStageBGM();
    bool isPlayingStageBgm();
    bool isPlayingStageBgmID(u32);
    bool isPlayingStageBgmName(const char*);
    bool isPreparedStageBgm();
    void setStageBGMState(s32, u32);
    void setStageBGMStateBit(u32);
    s32 getPlayingStageBgmId();
    s32 getStageBgmState();
    void adjustPorterBGGM(s32);

    JAISoundHandle* startSubBGM(const char*, bool);
    void stopSubBGM(u32);
    void moveVolumeSubBGM(f32, u32);
    bool isPlayingSubBgmID(u32);
    JAISoundHandle* startBossBGM(s32);
    void startDepartBGM();
    void startGliderBGM();
    void startMinigameBGM();

    void startBossRushBGM();
    void startAfterBossRushBGM();

    void startStarChanceBGM(bool);
    void startStarChanceBGM2(bool);

    void startMarioFaceShipBGM();
    void startGrandStarReturnBGM();
    bool isFirstGrandStarReturnBGMStage();

    void setCubeBgmChangeInvalid();

    void clearBgmQueue();
};  // namespace MR
