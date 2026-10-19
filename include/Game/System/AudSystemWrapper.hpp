#pragma once

#include "Game/AudioLib/AudSystem.hpp"
#include <revolution/types.h>

class AudSystem;
class JKRExpHeap;
class JKRHeap;
class JKRMemArchive;
class JKRSolidHeap;

class AudSystemWrapper {
public:
    AudSystemWrapper(JKRSolidHeap*, JKRHeap*, JKRHeap*);

    void loadSystemWaveData();
    void requestWaveResource();
    bool isLoadDoneRequestedWaves() const NO_INLINE;
    bool isLoadDoneSystemWaves() const;
    void clearStageWaveResources();
    void loadStageUseResources();
    void requestResourceForInitialize();
    void createAudioSystem();
    void createSoundNameConverter();
    void updateRhythm();
    void setStageResourceLoading(bool);
    void finishStageResourceLoading();
    void resetStageResources();
    void movement();
    void stopAllSound(u32);
    bool isLoadDoneWaveDataAtSystemInit() const;
    void loadStaticWaveData();
    bool isLoadDoneStaticWaveData() const;
    void loadStageWaveData(const char*, const char*, bool);
    bool isLoadDoneStageWaveData() const;
    void loadScenarioWaveData(const char*, const char*, s32);
    void loadScenarioWaveData(const char*, const char*, s32, bool);
    bool isLoadDoneScenarioWaveData() const;
    bool isPermitToReset() const;
    void prepareReset();
    void requestReset(bool);
    bool isResetDone();
    void resumeReset();
    void receiveResourceForInitialize();

    /* 0x00 */ AudSystem* mAudSystem;
    /* 0x04 */ JKRSolidHeap* mSolidHeap;
    /* 0x08 */ JKRHeap* mHeap;
    /* 0x0C */ JKRHeap* mSystemHeap;
    /* 0x10 */ AudArchiveSet mArchives;
    /* 0x28 */ JKRExpHeap* mSpkHeap;
    /* 0x2C */ JKRMemArchive* mSpkRes;
    /* 0x30 */ bool _28;
    /* 0x31 */ bool mIsResetDone;
    /* 0x32 */ bool mDisableReset;
};
