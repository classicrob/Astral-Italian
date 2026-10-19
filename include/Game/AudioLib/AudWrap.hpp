#pragma once

#include <revolution/types.h>

class AudStageUseResourceList;
class AudBgm;
class AudBgmMgr;
class AudBgmParamTable;
class AudInstListMgr;
class AudMeObject;
class AudMultiBgmInfo;
class AudRemixMgr;
class AudRemixSequencer;
class AudRhythmMeSystem;
class AudSceneMgr;
class AudSoundInfo;
class AudSoundNameConverter;
class AudSoundObjHolder;
class AudSoundObject;
class AudSystem;
class JAISoundHandle;
class JAISoundID;

namespace AudWrap {
    // Instrument bank to wave-bank mapping; returns -1 when the bank has no entry.
    s32 getWaveBankIdFromBankId(s32 bank);
    // Wave arcs managed separately from automatic per-sound instrument requests.
    bool isWaveArcExcludedFromSoundRequest(s32 waveBank, s32 waveArc);
    AudStageUseResourceList* getStageUseResourceList();
    AudSystem* getSystem();
    AudMultiBgmInfo* getMultiBgmInfo();
    bool hasSystem();
    AudSoundInfo* getSoundInfo();
    AudSceneMgr* getSceneMgr();
    AudInstListMgr* getInstListMgr();
    AudBgmMgr* getBgmMgr();
    AudBgm* getStageBgm();
    AudBgm* getSubBgm();
    JAISoundHandle* startStageBgm(u32, bool, s32);
    JAISoundHandle* startSubBgm(u32, bool);
    void setNextIdStageBgm(u32);
    JAISoundHandle* startLastStageBgm();
    AudBgmParamTable* getBgmParamTable();
    AudSoundObject* getSystemSeObject();
    AudSoundObject* getAtmosphereSeObject();
    AudSoundObjHolder* getSoundObjHolder();
    AudRhythmMeSystem* getRhythmMeSystem();
    AudMeObject* getSystemMeObject();
    AudRemixMgr* getRemixMgr();
    AudRemixSequencer* getRemixSequencer();
    AudSoundObject* getRemixSeqObject();
};  // namespace AudWrap

namespace AudWrap {
    void startGreenStarDemo();
}
