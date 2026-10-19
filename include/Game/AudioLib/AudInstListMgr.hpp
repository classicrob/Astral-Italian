#pragma once
#include <revolution/types.h>
#include <JSystem/JAudio2/JAISound.hpp>
class JAUSectionHeap;
class JKRArchive;
class JMapInfo;
class ActionSoundData;
struct AudInstWaveSlot {
    AudInstWaveSlot();
    AudInstWaveSlot(s32 bank, s32 wave, bool protect) {
        reset();
        set(bank, wave, protect);
    }
    void reset() NO_INLINE;
    void set(s32, s32, bool) NO_INLINE;
    s32 advanceAge();
    s32 addRequest();
    bool load(JAUSectionHeap*, bool);
    bool isLoadDone(JAUSectionHeap*);
    bool isRequestLoadDone(JAUSectionHeap*);
    AudInstWaveSlot& operator=(const AudInstWaveSlot&);
    s32 mBankId, mWaveArc, mAge, mRequestCount;
    bool mProtected, mLoadRequested, mTailLoaded;
};
struct AudInstWaveSlotTable {
    AudInstWaveSlotTable();
    AudInstWaveSlot mSlots[256];
    s32 mNumSlots;
};
struct AudInstEntry {
    u32 mSoundId;
    u16 mBankProgNo;
};
class AudInstListMgr {
public:
    AudInstListMgr(JAUSectionHeap*, JKRArchive*);
    static void requestInstrumentRefresh();
    static void requestStageInstrumentRefresh();
    void requestSound(const char*, bool);
    void fn_802204A0(const char*, bool);
    void requestWave(s32, s32, bool);
    void requestWaveForSystem(s32, s32, bool);
    void loadRequestedWaves(bool, bool);
    bool isLoadDone();
    void loadInstList();
    void requestStageUseInstList(const char*);
    void requestInstList(const JMapInfo*);
    void resetStageWaveSlots();
    void clearStageWaveResources();
    void resetStageResources();
    void requestSoundWaves(JAISoundID, bool);
    void requestWaveBank(s32, s32, bool);
    void requestWaveArc(s32, s32, bool);
    AudInstWaveSlot* findRequestedWave(s32, s32);
    AudInstWaveSlot* findLoadedWave(s32, s32);
    void loadWaves();
    bool isLoadDoneSystemWaves();
    void clearWaveRequests();
    s32 findInstrumentWave(u16, u8, u32);
    u32 getWaveArcSize(JAUSectionHeap*, u32, s32);
    void requestActionSounds(const ActionSoundData*, bool);
    s32 findSoundIndex(JAISoundID);
    s32 findFirstSoundIndex(JAISoundID);
    JAUSectionHeap* mSectionHeap;
    JKRArchive* mArchive;
    AudInstEntry* mEntries;
    s32 mNumEntries;
    bool _10, _11;
    AudInstWaveSlotTable mRequestedWaves;
    AudInstWaveSlotTable mLoadedWaves;
    bool _281C;
};
