#pragma once

#include <revolution.h>

class JAISoundHandle;
class JMapInfoIter;

namespace MR {
    u32 getCurrentStageBgmId();
    JAISoundHandle* startStageBGM(const char*, bool);
    JAISoundHandle* startStageBGM(u32, bool);
    JAISoundHandle* startStageBGMIfNotPlaying(const char*, bool);
    JAISoundHandle* startStageBGMFromStageName(const char*, const char*, s32, bool = false, bool = false);

    void stopStageBGM(u32);
    void stopStageBgmForDemo();  // fn_8001BA80: fades the stage BGM (10) and sets the SceneMgr flags 0xC/0xD (used by boss demos)
    s32 unlockStageBGM();
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
    JAISoundHandle* startDepartBGM();
    JAISoundHandle* startGliderBGM();
    JAISoundHandle* startMinigameBGM();

    void startBossRushBGM();
    void startAfterBossRushBGM();

    JAISoundHandle* startStarChanceBGM(bool);
    JAISoundHandle* startStarChanceBGM2(bool);

    JAISoundHandle* startMarioFaceShipBGM(bool);  // SMG2: the target takes a bool (symbols.txt mangles it as Fv)
    void startGrandStarReturnBGM();
    bool isFirstGrandStarReturnBGMStage();
    const char* getGrandStarReturnBGMName();
    bool isUseScenarioSelectMusicZeroStars();
    bool isUseScenarioSelectMusicOneStar();
    bool isGravityBGMPlaying();

    void setCubeBgmChangeInvalid();
    bool isCubeBgmChangeInvalid();

    void clearBgmQueue();

    // PLACEHOLDER(core): fn_* names from the BgmUtil asm; semantic names not yet known.
    void setBgmMgr954(s32);
    void resetStageBGMInterrupted();
    void setStageBGMVolumeState(s32, u32, f32);
    void getStageBgmJMapArgs(s32*, s32*, const JMapInfoIter&);
    void fadeOutStageBGMForIndex(s32, s32);
    void startStageBGMForIndex(s32, s32);
    void stopStageBGMImmediately();
    void startGrandStarReturnBGMIfNotUnlocked();
    void startFirstAstroStageBgm();
    void startEmptyNameBGMIfNotUnlocked();
    void stopStageAndSubBGM(u32);
};  // namespace MR

namespace MR { void requestStageBGMVolumeDown(s32, bool); }
