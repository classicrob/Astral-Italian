#pragma once

#include <revolution/types.h>

class JAUSectionHeap;
class JKRArchive;

// One bank-program wave slot; 0x14 bytes, set up by the (still unnamed) slot ctor fn_800871A0.
struct AudInstWaveSlot {
    /* 0x00 */ u8 _0[0x14];
};

struct AudInstWaveSlotTable {
    AudInstWaveSlotTable();  // fn_80087500

    /* 0x0000 */ AudInstWaveSlot mSlots[256];
    /* 0x1400 */ s32 mNumSlots;
};

struct AudInstEntry {
    /* 0x00 */ void* mData;
    /* 0x04 */ u16 mBankProgNo;
};

// Loads SoundIdToInstList.bcsv (BankProgNo/SoundId columns) from /AudioRes/Info/SoundIdToInstList.arc and tracks
// which instrument waves are loaded.
class AudInstListMgr {
public:
    AudInstListMgr(JAUSectionHeap*, JKRArchive*);  // fn_80087490
    static void requestInstrumentRefresh();
    void fn_80087940(const char*, bool);
    void fn_802204A0(const char*, bool);
    void requestWave(s32, s32, bool);
    void requestWaveForSystem(s32, s32, bool);
    void loadRequestedWaves(bool, bool);
    bool isLoadDone();
    void loadInstList();                           // fn_80087560
    void requestStageUseInstList(const char*);     // fn_80087740; PLACEHOLDER(core) name

    /* 0x0000 */ JAUSectionHeap* mSectionHeap;
    /* 0x0004 */ JKRArchive* mArchive;
    /* 0x0008 */ AudInstEntry* mEntries;
    /* 0x000C */ s32 mNumEntries;
    /* 0x0010 */ bool _10;
    /* 0x0011 */ bool _11;
    /* 0x0014 */ AudInstWaveSlotTable mTables[2];
    /* 0x281C */ bool _281C;
};
