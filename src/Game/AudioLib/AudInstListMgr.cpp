#include "Game/AudioLib/AudInstListMgr.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/Enemy/ActionSoundData.hpp"
#include "Game/GameAudio/ActionSoundInfo.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Scene/StageDataArchiveHolder.hpp"
#include "Game/Util.hpp"
#include "JSystem/JAudio2/JASBasicBank.hpp"
#include "JSystem/JAudio2/JASBasicInst.hpp"
#include "JSystem/JAudio2/JASDrumSet.hpp"
#include "JSystem/JAudio2/JASBankTable.hpp"
#include "JSystem/JAudio2/JASHeapCtrl.hpp"
#include "JSystem/JAudio2/JASBasicWaveBank.hpp"
#include "JSystem/JAudio2/JAUSectionHeap.hpp"
#include "JSystem/JKernel/JKRArchive.hpp"

namespace AudInstListMgrData {
    const char* sInstListFileName = "SoundIdToInstList.bcsv";
}

AudInstWaveSlot::AudInstWaveSlot()
    : mBankId(-1), mWaveArc(0), mAge(0), mRequestCount(0), mProtected(false), mLoadRequested(false), mTailLoaded(false) {
    reset();
}
void AudInstWaveSlot::reset() {
    mBankId = -1;
    mWaveArc = 0;
    mAge = 0;
    mRequestCount = 0;
    mProtected = false;
    mLoadRequested = false;
    mTailLoaded = false;
}
void AudInstWaveSlot::set(s32 bank, s32 wave, bool protect) {
    mBankId = bank;
    mWaveArc = wave;
    mProtected = protect;
}
s32 AudInstWaveSlot::advanceAge() {
    if (mAge < 0x7FFFFFFF)
        ++mAge;
    return mAge;
}
s32 AudInstWaveSlot::addRequest() {
    if (mRequestCount < 0x7FFFFFFF)
        ++mRequestCount;
    return mRequestCount;
}
bool AudInstWaveSlot::load(JAUSectionHeap* section, bool tail) {
    addRequest();
    mLoadRequested = true;
    mTailLoaded = tail;
    if (tail) {
        u32 bank = mBankId;
        const JAUSection::TSectionData& data = section->getSectionData();
        u32 mask = 1 << mWaveArc;
        if (data.registeredWaveBankTables.test(bank)) {
            JASWaveBank* waveBank = section->getWaveBankTable().getWaveBank(bank);
            if (waveBank) {
                for (u32 i = 0; i < waveBank->getArcCount(); ++i)
                    if (mask & (1 << i))
                        waveBank->getWaveArc(i)->loadTail(0);
                return true;
            }
        }
        return false;
    }
    return section->loadWaveArc(mBankId, 1 << mWaveArc);
}
// NONMATCHING: equivalent early-return blocks are ordered differently.
bool AudInstWaveSlot::isLoadDone(JAUSectionHeap* section) {
    if (mBankId < 0 || !mLoadRequested)
        return true;
    return section->isWaveLoaded(mBankId, mWaveArc);
}
// NONMATCHING: equivalent early-return blocks are ordered differently.
bool AudInstWaveSlot::isRequestLoadDone(JAUSectionHeap* section) {
    if (mBankId < 0 || !mLoadRequested)
        return true;
    return section->isWaveLoaded(mBankId, mWaveArc);
}
AudInstListMgr::AudInstListMgr(JAUSectionHeap* section, JKRArchive* archive)
    : mSectionHeap(section), mArchive(archive), mEntries(0), mNumEntries(0), _10(false), _11(false), _281C(false) {
}
AudInstWaveSlotTable::AudInstWaveSlotTable() : mNumSlots(0) {
}
void AudInstListMgr::loadInstList() {
    JMapInfo info;
    info.attach(mArchive->getResource(AudInstListMgrData::sInstListFileName));
    mNumEntries = 0;
    for (JMapInfoIter i = info.begin(); i != info.end(); ++i.mIndex)
        ++mNumEntries;
    mEntries = new AudInstEntry[mNumEntries];
    s32 index = 0;
    for (JMapInfoIter i = info.begin(); i != info.end(); ++i.mIndex) {
        u16 bp = i.getValueU16("BankProgNo");
        u8 bank = bp >> 8;
        findInstrumentWave(bank, (u8)bp, AudWrap::getWaveBankIdFromBankId(bank));
        mEntries[index].mSoundId = i.getValueU32("SoundId");
        mEntries[index].mBankProgNo = bp;
        ++index;
    }
}
void AudInstListMgr::requestStageUseInstList(const char* stage) {
    JMapInfo* info = StageDataArchive::createUseResourceJMapInfo(stage);
    if (info)
        requestInstList(info);
}
void AudInstListMgr::requestInstList(const JMapInfo* info) {
    s32 count = MR::getCsvDataElementNum(info);
    const char* name = 0;
    for (s32 i = 0; i < count; ++i) {
        MR::getCsvDataStr(&name, info, "SoundName", i);
        if (!MR::isEqualString(name, "<Anonymous>")) {
            JAISoundID id = AudSoundNameConverter::getSoundIDFromName(name);
            requestSoundWaves(id, false);
        }
    }
}
void AudInstListMgr::resetStageWaveSlots() {
    for (AudInstWaveSlot* i = mLoadedWaves.mSlots; i != mLoadedWaves.mSlots + mLoadedWaves.mNumSlots; ++i)
        i->advanceAge();
}
void AudInstListMgr::clearStageWaveResources() {
    mRequestedWaves.mNumSlots = 0;
    resetStageWaveSlots();
}
void AudInstListMgr::resetStageResources() {
    mRequestedWaves.mNumSlots = 0;
    resetStageWaveSlots();
}
void AudInstListMgr::requestWaveForSystem(s32 bank, s32 wave, bool protect) {
    AudInstWaveSlot slot;
    slot.set(bank, wave, protect);
    slot.mLoadRequested = true;
    mLoadedWaves.mSlots[mLoadedWaves.mNumSlots++] = slot;
}
void AudInstListMgr::requestSound(const char* name, bool protect) {
    JAISoundID id = AudSoundNameConverter::getSoundIDFromName(name);
    requestSoundWaves(id, protect);
}
void AudInstListMgr::requestSoundWaves(JAISoundID id, bool protect) {
    u8 bank;
    s32 first = findFirstSoundIndex(id);
    for (s32 i = first; i < mNumEntries; ++i) {
        if (mEntries[i].mSoundId > id)
            break;
        if (mEntries[i].mSoundId == id) {
            u32 bp = mEntries[i].mBankProgNo;
            bank = bp >> 8;
            s32 waveBank = AudWrap::getWaveBankIdFromBankId(bank);
            s32 wave = findInstrumentWave(bank, (u8)bp, waveBank);
            if (!AudWrap::isWaveArcExcludedFromSoundRequest(waveBank, wave))
                requestWaveBank(waveBank, wave, protect);
        }
    }
}
void AudInstListMgr::requestWave(s32 bank, s32 wave, bool protect) {
    requestWaveBank(bank, wave, protect);
}
void AudInstListMgr::requestWaveBank(s32 bank, s32 wave, bool protect) {
    if (wave <= 0) {
        JASWaveBank* waveBank = mSectionHeap->getWaveBankTable().getWaveBank(bank);
        if (waveBank->getArcCount() > 1) {
            for (u32 i = 1; i < waveBank->getArcCount(); ++i)
                requestWaveArc(bank, i, protect);
        } else
            requestWaveArc(bank, 0, protect);
    } else
        requestWaveArc(bank, wave, protect);
}
void AudInstListMgr::requestWaveArc(s32 bank, s32 wave, bool protect) {
    if ((wave != 0 || mSectionHeap->getWaveBankTable().getWaveBank(bank)->getArcCount() <= 1) && !findRequestedWave(bank, wave)) {
        AudInstWaveSlot slot(bank, wave, protect);
        mRequestedWaves.mSlots[mRequestedWaves.mNumSlots++] = slot;
    }
}
AudInstWaveSlot* AudInstListMgr::findRequestedWave(s32 bank, s32 wave) {
    for (s32 i = 0; i < mRequestedWaves.mNumSlots; ++i) {
        AudInstWaveSlot* slot = &mRequestedWaves.mSlots[i];
        if (bank == slot->mBankId && wave == slot->mWaveArc)
            return slot;
    }
    return 0;
}
AudInstWaveSlot* AudInstListMgr::findLoadedWave(s32 bank, s32 wave) {
    for (s32 i = 0; i < mLoadedWaves.mNumSlots; ++i) {
        AudInstWaveSlot* slot = &mLoadedWaves.mSlots[i];
        if (bank == slot->mBankId && wave == slot->mWaveArc)
            return slot;
    }
    return 0;
}
void AudInstListMgr::loadRequestedWaves(bool eraseOld, bool eraseTail) {
    if (GameSequenceFunction::hasWorldMapHeap() && !_10 && !_11)
        eraseOld = false;
    _10 = false;
    _11 = false;
    if (eraseOld) {
        bool erasedNormal = false;
        AudInstWaveSlot* i = mLoadedWaves.mSlots;
        while (i != mLoadedWaves.mSlots + mLoadedWaves.mNumSlots) {
            if (i->mProtected) {
                ++i;
                continue;
            }
            if (i->mAge == 0) {
                ++i;
                continue;
            }
            if (!eraseTail && i->mTailLoaded == 1) {
                ++i;
                continue;
            }
            if (!i->mTailLoaded && !erasedNormal && findRequestedWave(i->mBankId, i->mWaveArc)) {
                ++i;
                continue;
            }
            mSectionHeap->eraseWaveArc(i->mBankId, 1 << i->mWaveArc);
            if (!i->mTailLoaded)
                erasedNormal = true;
            if ((mLoadedWaves.mSlots + mLoadedWaves.mNumSlots) - i - 1 > 0) {
                for (AudInstWaveSlot* j = i; j + 1 != mLoadedWaves.mSlots + mLoadedWaves.mNumSlots; ++j)
                    *j = *(j + 1);
            }
            --mLoadedWaves.mNumSlots;
        }
    }
    loadWaves();
}
static inline bool isTailLoadWave(const AudInstWaveSlot* slot) {
    s32 b = slot->mBankId;
    s32 w = slot->mWaveArc;
    if (b == 2 && w == 15)
        return true;
    if (b == 6 && w == 27)
        return true;
    if (b == 6 && w == 26)
        return true;
    if (b == 49 && w == 8)
        return true;
    if (b == 34 && w == 3)
        return true;
    if (b == 34 && w == 4)
        return true;
    if (b == 25 && w == 5)
        return true;
    if (b == 25 && w == 8)
        return true;
    return false;
}
void AudInstListMgr::loadWaves() {
    for (AudInstWaveSlot* i = mRequestedWaves.mSlots; i != mRequestedWaves.mSlots + mRequestedWaves.mNumSlots; ++i) {
        if (i->mBankId < 0)
            continue;
        AudInstWaveSlot* loaded = findLoadedWave(i->mBankId, i->mWaveArc);
        if (loaded) {
            loaded->mAge = 0;
            loaded->addRequest();
            i->reset();
            continue;
        }
        u32 size = getWaveArcSize(mSectionHeap, i->mBankId, i->mWaveArc);
        if (size <= JASKernel::getAramFreeSize()) {
            bool tail = false;
            if (!i->mProtected) {
                if (_281C)
                    tail = true;
                if (isTailLoadWave(i))
                    tail = true;
            }
            i->load(mSectionHeap, tail);
            mLoadedWaves.mSlots[mLoadedWaves.mNumSlots++] = *i;
        } else
            i->reset();
    }
}
bool AudInstListMgr::isLoadDone() {
    for (AudInstWaveSlot* i = mRequestedWaves.mSlots; i != mRequestedWaves.mSlots + mRequestedWaves.mNumSlots; ++i)
        if (!i->isLoadDone(mSectionHeap))
            return false;
    return true;
}
bool AudInstListMgr::isLoadDoneSystemWaves() {
    for (AudInstWaveSlot* i = mRequestedWaves.mSlots; i != mRequestedWaves.mSlots + mRequestedWaves.mNumSlots; ++i)
        if (!i->isRequestLoadDone(mSectionHeap))
            return false;
    return true;
}
void AudInstListMgr::clearWaveRequests() {
    mRequestedWaves.mNumSlots = 0;
}
static inline s32 findWaveIndex(JASBasicWaveBank::TWaveGroup* wg, u32 id) {
    for (u32 w = 0; w < wg->mWaveCount; ++w)
        if (id == wg->getWaveID(w))
            return w;
    return -1;
}
static inline s32 findWaveGroup(JASWaveBank* wb, const u32* waves, s32 count) {
    if (wb->getArcCount() == 1)
        return 0;
    for (s32 group = wb->getArcCount() - 1; group >= 0; --group) {
        JASBasicWaveBank::TWaveGroup* wg = static_cast< JASBasicWaveBank* >(wb)->getWaveGroup(group);
        s32 matched = 0;
        for (; matched < count; ++matched) {
            if (findWaveIndex(wg, waves[matched]) < 0)
                break;
        }
        if (matched == count)
            return group;
    }
    return -1;
}
s32 AudInstListMgr::findInstrumentWave(u16 bank, u8 program, u32 waveBankId) {
    JASBasicBank* basicBank = static_cast< JASBasicBank* >(JASDefaultBankTable::getInstance()->JASBankTable< 256 >::getBank(bank));
    JASInst* inst = basicBank->getInst(program);
    u32 waves[128];
    s32 count = 0;
    switch (inst->getType()) {
    case 'BSIC': {
        const JASBasicInst* basic = static_cast< const JASBasicInst* >(inst);
        s32 n = basic->getKeyRegionCount();
        for (s32 i = 0; i < n; ++i) {
            const JASBasicInst::TKeymap* km = basic->getKeyRegion(i);
            const JASBasicInst::TVelomap* velo = km->getVeloRegion(0);
            if (velo)
                waves[count++] = velo->_4;
        }
        break;
    }
    case 'PERC': {
        JASDrumSet* drums = static_cast< JASDrumSet* >(inst);
        for (u32 i = 0; i < 128; ++i) {
            const JASDrumSet::TPerc* perc = drums->getPerc(i);
            if (perc)
                for (u32 j = 0; j < perc->getVeloRegionCount(); ++j) {
                    const JASDrumSet::TVelomap* velo = perc->getVeloRegion(j);
                    if (velo)
                        waves[count++] = velo->_4;
                }
        }
        break;
    }
    }
    JASWaveBank* wb = mSectionHeap->getWaveBankTable().getWaveBank(waveBankId);
    return findWaveGroup(wb, waves, count);
}
u32 AudInstListMgr::getWaveArcSize(JAUSectionHeap* section, u32 bank, s32 wave) {
    u32 size = 0;
    JASWaveBank* wb = section->getWaveBankTable().getWaveBank(bank);
    if (wb) {
        JASWaveArc* arc = wb->getWaveArc(wave);
        if (arc)
            size = arc->mFileLength;
    }
    return size;
}
void AudInstListMgr::requestActionSounds(const ActionSoundData* data, bool protect) {
    s32 count = data->mCount;
    for (s32 i = 0; i < count; ++i) {
        ActionSoundInfo* info = data->getInfo(i);
        if (info->fn_80220890()) {
            JAISoundID id = AudSoundNameConverter::getSoundIDFromName(info->getSoundLabel());
            requestSoundWaves(id, protect);
        }
    }
}
s32 AudInstListMgr::findSoundIndex(JAISoundID id) {
    s32 low = 0, high = mNumEntries - 1;
    while (low <= high) {
        s32 mid = low + (high - low) / 2;
        u32 v = mEntries[mid].mSoundId;
        if (v == id)
            return mid;
        if (v > id)
            high = mid - 1;
        else if (v < id)
            low = mid + 1;
    }
    return -1;
}
s32 AudInstListMgr::findFirstSoundIndex(JAISoundID id) {
    s32 found = findSoundIndex(id);
    if (found < 0)
        return 0;
    for (s32 i = found - 1; i >= 0; --i)
        if (mEntries[i].mSoundId != id)
            return i + 1;
    return 0;
}
namespace MR {
    void requestActionSoundWaves(const ActionSoundData* data, bool protect) {
        AudWrap::getInstListMgr()->requestActionSounds(data, protect);
    }
}  // namespace MR
void AudInstListMgr::requestStageInstrumentRefresh() {
    AudInstListMgr* mgr = AudWrap::getInstListMgr();
    mgr->_10 = true;
}
void AudInstListMgr::requestInstrumentRefresh() {
    AudInstListMgr* mgr = AudWrap::getInstListMgr();
    mgr->_11 = true;
}
AudInstWaveSlot& AudInstWaveSlot::operator=(const AudInstWaveSlot& s) {
    mBankId = s.mBankId;
    mWaveArc = s.mWaveArc;
    mAge = s.mAge;
    mRequestCount = s.mRequestCount;
    mProtected = s.mProtected;
    mLoadRequested = s.mLoadRequested;
    mTailLoaded = s.mTailLoaded;
    return *this;
}
