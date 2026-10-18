#pragma once

#include <revolution/types.h>

class JAUSectionHeap;

struct AudSceneSoundInfo {
    bool hasScenarioResource() const NO_INLINE;

    /* 0x00 */ const char* mSceneName;
    /* 0x04 */ const char* mStageName;
    /* 0x08 */ s8 mStageId;
    /* 0x09 */ s8 mScenarioId;
};

class AudSceneMgr {
public:
    enum PlayerMode {
        PlayerMode_Mario = 1,
        PlayerMode_Luigi = 2,
    };

    AudSceneMgr(JAUSectionHeap*);

    bool isLoadDoneSystemInit();
    void loadSystemResource();
    void requestResource();
    void loadStaticResource2nd();
    bool isLoadDoneStaticResource2nd();
    void loadStaticResource();
    bool isLoadDoneStaticResource();
    void loadStageResource(const char*, const char*);
    void loadStageResource(const AudSceneSoundInfo*);
    bool isLoadDoneStageResource();
    void loadScenarioResource(const char*, const char*, s32);
    void loadScenarioResource(const AudSceneSoundInfo*, s32);
    bool isLoadDoneScenarioResource();
    void startScene();
    void resetForScene();
    void initSceneAudioEffect();
    const AudSceneSoundInfo* findSceneSoundInfo(const char*, const char*);
    void eraseLastBgmWaveSet();
    void eraseLastSeWaveSet();
    void eraseLastSeScenarioWaveSet();
    void loadWaveSet(const s8*, s32);
    void setPlayerModeMario();
    void setPlayerModeLuigi();
    void loadPlayerResource();
    bool isPlayerResourceLoaded();

    bool isPlayerModeMario() {
        return mPlayerMode == PlayerMode_Mario;
    }

    bool isPlayerModeLuigi() {
        return mPlayerMode == PlayerMode_Luigi;
    }

    /* 0x00 */ JAUSectionHeap* mSectionHeap;
    /* 0x04 */ u32 _4;
    /* 0x08 */ s32 mPlayerMode;
    /* 0x0C */ bool mIsNewPlayerMode;
    /* 0x0D */ bool _1D;
};
