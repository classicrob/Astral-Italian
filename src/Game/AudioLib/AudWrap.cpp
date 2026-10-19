#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Util.hpp"
#include "Game/AudioLib/AudBgmMgr.hpp"
#include "Game/AudioLib/AudRemixMgr.hpp"
#include "Game/AudioLib/AudSoundInfo.hpp"
#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/RhythmLib/AudRhythmWrap.hpp"

namespace AudWrap {
    AudSystem* getSystem() {
        return AudSystem::msBasic;
    }

    bool hasSystem() {
        return getSystem() != nullptr;
    }

    AudSoundInfo* getSoundInfo() {
        return AudSoundInfo::getInstance();
    }

    AudSceneMgr* getSceneMgr() {
        return getSystem()->mSceneMgr;
    }

    AudInstListMgr* getInstListMgr() {
        return getSystem()->mInstListMgr;
    }

    AudBgmMgr* getBgmMgr() {
        return getSystem()->mBgmMgr;
    }

    AudBgm* getStageBgm() {
        return getBgmMgr()->mBgm[AudBgmMgr::BgmType_Stage];
    }

    AudBgm* getSubBgm() {
        return getBgmMgr()->mBgm[AudBgmMgr::BgmType_Sub];
    }

    JAISoundHandle* startStageBgm(u32 soundID, bool lock, s32 fadeTime) {
        return getBgmMgr()->start(AudBgmMgr::BgmType_Stage, soundID, lock, fadeTime);
    }

    JAISoundHandle* startSubBgm(u32 soundID, bool lock) {
        return getBgmMgr()->start(AudBgmMgr::BgmType_Sub, soundID, lock, -1);
    }

    void setNextIdStageBgm(u32 soundID) {
        getBgmMgr()->setNextBGM(AudBgmMgr::BgmType_Stage, soundID);
    }

    JAISoundHandle* startLastStageBgm() {
        return getBgmMgr()->startLastBGM(AudBgmMgr::BgmType_Stage);
    }

    AudMultiBgmInfo* getMultiBgmInfo() {
        return getSystem()->mMultiBgmInfo;
    }

    AudBgmParamTable* getBgmParamTable() {
        return getSystem()->mBgmParamTable;
    }

    AudSoundObject* getSystemSeObject() {
        return getSystem()->mSystemSeObject;
    }

    AudSoundObject* getAtmosphereSeObject() {
        return getSystem()->mAtmosphereSeObject;
    }

    AudSoundObjHolder* getSoundObjHolder() {
        return getSystem()->mSoundObjHolder;
    }

    AudStageUseResourceList* getStageUseResourceList() {
        return getSystem()->mStageUseResourceList;
    }

    AudRhythmMeSystem* getRhythmMeSystem() {
        return getSystem()->mRhythmMeSystem;
    }

    AudMeObject* getSystemMeObject() {
        return getSystem()->mSystemMeObject;
    }

    AudRemixMgr* getRemixMgr() {
        return getSystem()->mRemixMgr;
    }

    AudRemixSequencer* getRemixSequencer() {
        return getRemixMgr()->mRemixSeq;
    }

    AudSoundObject* getRemixSeqObject() {
        return getRemixMgr()->mSoundObj;
    }
};  // namespace AudWrap
