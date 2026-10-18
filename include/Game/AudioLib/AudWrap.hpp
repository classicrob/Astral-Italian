#pragma once

#include <revolution/types.h>

class AudStageUseResourceList;
class AudBgm;
class AudBgmMgr;
class AudInstListMgr;
class AudMeObject;
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

namespace AudWrap {
    AudStageUseResourceList* getStageUseResourceList();
    AudSystem* getSystem();
    bool hasSystem();
    AudSoundInfo* getSoundInfo();
    AudSceneMgr* getSceneMgr();
    AudInstListMgr* getInstListMgr();   // fn_8007C890
    AudBgmMgr* getBgmMgr();
    AudBgm* getStageBgm();
    AudBgm* getSubBgm();
    JAISoundHandle* startStageBgm(u32, bool);
    JAISoundHandle* startStageBgm(u32, bool, s32);  // target symbol __7AudWrapFUlbl; AudWrap.cpp still defines the 2-arg form
    JAISoundHandle* startSubBgm(u32, bool);
    void setNextIdStageBgm(u32);
    JAISoundHandle* startLastStageBgm();
    AudSoundObject* getSystemSeObject();
    AudSoundObject* getAtmosphereSeObject();
    AudSoundObjHolder* getSoundObjHolder();
    AudRhythmMeSystem* getRhythmMeSystem();
    AudMeObject* getSystemMeObject();
    AudRemixMgr* getRemixMgr();
    AudRemixSequencer* getRemixSequencer();
    AudSoundObject* getRemixSeqObject();
    AudSoundNameConverter* getSoundNameConverter();
};  // namespace AudWrap

namespace AudWrap {
    void startGreenStarDemo();
}
