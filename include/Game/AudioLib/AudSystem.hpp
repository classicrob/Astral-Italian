#pragma once

#include "Game/AudioLib/AudAudience.hpp"
#include "Game/AudioLib/AudBgmMgr.hpp"
#include "Game/AudioLib/AudLimitedSound.hpp"
#include <JSystem/JAudio2/JAIAudible.hpp>
#include <JSystem/JAudio2/JAIAudience.hpp>
class JAUStdSoundInfo;
#include <JSystem/JAudio2/JASAudioReseter.hpp>
#include <JSystem/JAudio2/JAUSoundMgr.hpp>
#include <JSystem/JGeometry/TMatrix.hpp>

class AudBgmParamTable;
class AudChordInfo;
class AudEffector;
class AudInstListMgr;
class AudMultiBgmInfo;
class AudSeLifeTable;
class AudStageUseResourceList;
class AudMeObject;
class AudRemixMgr;
class AudRhythmMeSystem;
class AudSceneMgr;
class AudSeStrategyMgr;
class AudSoundObjHolder;
class AudSoundObject;
class AudSystemVolumeController;
class JAISe;
class JAISoundHandle;
class JAISoundID;
class JAUSectionHeap;
class JAUStreamStaticAramMgr;
class JKRArchive;
class JKRHeap;
class JKRSolidHeap;
class SpkSystem;

extern const char sAudioResFile[];
extern const char sJaiSeqArc[];
extern const char sJaiChordArc[];
extern const char sJaiMeArc[];
extern const char sJaiRemixArc[];
extern const char sJaiAudioArc[];
extern const char sJaiSeqResArc[];
extern const char sJaiInstListArc[];

// Archives handed to AudNewAudSystem (AudSystemWrapper keeps one). SMG2 only; SMG1 passed them as separate arguments.
struct AudArchiveSet {
    AudArchiveSet() {
#ifndef AUD_ARCHIVE_SET_MANUAL_RESET
        reset();
#endif
    }

    void reset() NO_INLINE {
        mSmrRes = nullptr;
        mSeqArchive = nullptr;
        mChordArchive = nullptr;
        mMeArchive = nullptr;
        mRemixArchive = nullptr;
        mInstListArchive = nullptr;
    }

    /* 0x00 */ void* mSmrRes;
    /* 0x04 */ JKRArchive* mSeqArchive;
    /* 0x08 */ JKRArchive* mChordArchive;
    /* 0x0C */ JKRArchive* mMeArchive;
    /* 0x10 */ JKRArchive* mRemixArchive;
    /* 0x14 */ JKRArchive* mInstListArchive;
};

// SMG2: per-sound-ID polyphony limiter; up to 3 playing SEs of one sound ID, ordered by score.
struct AudSeLimitEntry {
    /* 0x00 */ JAISe* mSe;
    /* 0x04 */ f32 mScore;
};

struct AudSeLimitGroup {
    /* 0x00 */ AudSeLimitEntry mEntries[3];
};

class AudSystem : public JAUSoundMgr {
public:
    enum MenuState {
        MenuState_Off = 0,
        MenuState_Enter = 1,
        MenuState_Exit = 2,
        MenuState_Active = 3,
    };

    enum MenuType {
        MenuType_HomeButtonMenu = 0,
        MenuType_Pause = 1,
        MenuType_Error = 2,
    };

    AudSystem(JKRHeap*, JAUSectionHeap*, AudArchiveSet*);

    virtual void frameWork();
    virtual void calc();
    virtual bool startSound(JAISoundID, JAISoundHandle*, const TVec3f*);
    virtual bool startLevelSound(JAISoundID, JAISoundHandle*, const TVec3f*);

    AudChordInfo* getChordInfo();
    void setSpeakerResource(JKRArchive*);
    void switchToLuigi(JAISoundID, JAISoundHandle*);
    bool isSubmitSeByVolumeSetting(JAISoundID);
    void updateOutputMode();
    void preProcessToReset();
    void resetAudio(u32, bool);
    void resumeReset();
    bool hasReset() const;
    void initSceneVolume();
    void pause();
    void unpause();
    void enterHomeButtonMenu();
    void exitHomeButtonMenu();
    void updateHomeButtonMenu();
    bool isHomeButtonMenuActive() const;
    void enterPauseMenu();
    void exitPauseMenu();
    void updatePauseMenu();
    bool isPauseMenuActive() const;
    bool isMenuOrErrorActive() const;
    void screenSaverProcess();
    bool isEnableStartSound(JAISoundID);
    void doDvdErrorProcess();
    void exitDvdErrorProcess();
    s32 getNumOfPlaying(JAISoundID);
    u32 getPlayCountMin(JAISoundID);
    void setMicMtx(MtxPtr, s32);
    const TVec3f& getMicPos(s32);
    void setFarCamera(bool);
    void setSeVolumeSet(s32, u32);
    void recoverSeVolumeSet(u32);
    void setSeVolumeSetLevel(s32, s32 = -1, s32 = 2);
    void initVolumeSetting();
    void initCategoryArrangement();
    void setMeResource(JKRArchive*, u16, u16);
    void seFanfareProcess();
    bool isFanfareSePlaying();
    void registerLimitedSound(JAISoundID, s32);
    void clearAllLimitedSound();
    bool isRegisteredLimitedSound(JAISoundID);
    void updateLimitedSound();
    bool isAlreadyPlayingSoundNear(JAISoundID, const TVec3f*, f32);
    void setVolumeZeroForce(s32);
    void setCameraRate(f32);

    bool isRestrictedSound(JAISoundID);
    void stopAllSounds(u32 fadeTime);
    void stopAllSe(bool = false);
    void stopSeCategories(u32 fadeTime);
    bool isHomeButtonMenuEntering() const NO_INLINE;
    bool isHomeButtonMenuLeaving() const;
    void pauseSeCategories(s32 menuType, bool pause);
    void stopMenuSystemSounds();
    void clearSeLimitGroups();
    void updateSeLimitGroups();
    s32 registerSeLimit(JAISe*, s32);
    void setField9A0(s32 value);
    void setField9A4(bool value);
    static void setField834(s32 value);

    void set830(u32 var) {
        mSystemSoundOnlyFrames = var;
    }

    AudEffector* getAudEffector() const {
        return mAudEffector;
    }

    static AudSystem* msBasic;

    static AudSystem* get() {
        return msBasic;
    }

    /* 0x0814 */ bool mIsResetReady;
    /* 0x0815 */ bool mIsReset;
    /* 0x0816 */ bool mStopThreads;
    /* 0x0818 */ JASAudioReseter mReseter;
    /* 0x0828 */ bool mIsSoundMuted;
    /* 0x0829 */ bool mIsPaused;
    /* 0x082A */ bool mIsSceneSoundMuted;
    /* 0x082B */ bool mIsStartSoundBlocked;  // blocks startSound
    /* 0x082C */ bool mIsStartLevelSoundBlocked;  // blocks startLevelSound
    /* 0x0830 */ s32 mSystemSoundOnlyFrames;
    /* 0x0834 */ s32 mRestrictedSoundFrames;  // SMG2; >= 0 enables isRestrictedSound filtering, counts down per frame
    /* 0x0838 */ JAUStdSoundInfo* mSoundInfo;
    /* 0x083C */ JKRHeap* mHeap;  // SMG2; heap the system-lifetime objects are created in (ctor arg)
    /* 0x0840 */ JAUSectionHeap* mSectionHeap;
    /* 0x0844 */ JAUStreamStaticAramMgr* mStreamAramMgr;
    /* 0x0848 */ AudAudience< 2 > mAudience;
    /* 0x094C */ s32 mSoundMode;  // SCGetSoundMode()
    /* 0x0950 */ AudSoundObject* mSystemSeObject;
    /* 0x0954 */ AudSoundObject* mAtmosphereSeObject;
    /* 0x0958 */ AudMeObject* mSystemMeObject;
    /* 0x095C */ AudSceneMgr* mSceneMgr;
    /* 0x0960 */ AudBgmMgr* mBgmMgr;  // SMG2: heap object (SMG1 embedded it)
    /* 0x0964 */ AudSoundObjHolder* mSoundObjHolder;
    /* 0x0968 */ AudRhythmMeSystem* mRhythmMeSystem;
    /* 0x096C */ AudRemixMgr* mRemixMgr;
    /* 0x0970 */ AudEffector* mAudEffector;
    /* 0x0974 */ AudSeStrategyMgr* mSeStrategyMgr;
    /* 0x0978 */ AudSystemVolumeController* mVolumeController;
    /* 0x097C */ AudInstListMgr* mInstListMgr;
    /* 0x0980 */ AudMultiBgmInfo* mMultiBgmInfo;
    /* 0x0984 */ AudBgmParamTable* mBgmParamTable;
    /* 0x0988 */ s32 mHomeButtonMenuState;
    /* 0x098C */ s32 mHomeButtonToggleTime;
    /* 0x0990 */ s32 mPauseMenuState;
    /* 0x0994 */ s32 mPauseMenuToggleTime;
    /* 0x0998 */ bool mIsDvdError;
    /* 0x099C */ AudLimitedSoundInfo* mLimitedSoundInfo;
    /* 0x09A0 */ s32 mFanfareHoldFrames;
    /* 0x09A4 */ bool mIsFanfareProcessEnabled;
    /* 0x09A5 */ bool mIsDestroySceneKeepAllSound;
    /* 0x09A8 */ SpkSystem* mSpkSystem;
    /* 0x09AC */ AudSeLifeTable* mSeLifeTable;
    /* 0x09B0 */ AudStageUseResourceList* mStageUseResourceList;
    /* 0x09B4 */ s32 mNumSeLimitGroups;
    /* 0x09B8 */ AudSeLimitGroup mSeLimitGroups[32];  // SMG2; 0x300 bytes
};

AudSystem* AudNewAudSystem(JKRHeap*, JKRSolidHeap*, AudArchiveSet*);
AudSystem* AudNewAudSystem_(JKRHeap*, JAUSectionHeap*, int, AudArchiveSet*) NO_INLINE;

#include <JSystem/JAudio2/JAUStdSoundInfo.hpp>
