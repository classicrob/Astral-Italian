#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudBgmParamTable.hpp"
#include "Game/AudioLib/AudEffector.hpp"
#include "Game/AudioLib/AudInstListMgr.hpp"
#include "Game/AudioLib/AudMultiBgmInfo.hpp"
#include "Game/AudioLib/AudSeLifeTable.hpp"
#include "Game/AudioLib/AudStageUseResourceList.hpp"
#include "Game/AudioLib/AudLimitedSound.hpp"
#include "Game/AudioLib/AudParams.hpp"
#include "Game/AudioLib/AudRemixMgr.hpp"
#include "Game/AudioLib/AudSceneMgr.hpp"
#include "Game/AudioLib/AudSeStrategy.hpp"
#include "Game/AudioLib/AudSoundId.hpp"
#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/AudioLib/AudSoundInfo.hpp"
#include "Game/AudioLib/AudSoundObjHolder.hpp"
#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/AudioLib/AudSystemVolumeController.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/RhythmLib/AudChordInfo.hpp"
#include "Game/RhythmLib/AudMeChannelMgr.hpp"
#include "Game/RhythmLib/AudMeObject.hpp"
#include "Game/RhythmLib/AudMePlayer.hpp"
#include "Game/RhythmLib/AudMeTrack.hpp"
#include "Game/RhythmLib/AudRhythmMeSystem.hpp"
#include "Game/Speaker/SpkSystem.hpp"
#include <JSystem/JAudio2/JAIAudible.hpp>
#include <JSystem/JAudio2/JAISeMgr.hpp>
#include <JSystem/JAudio2/JAISeqDataMgr.hpp>
#include <JSystem/JAudio2/JAISoundStarter.hpp>
#include <JSystem/JAudio2/JAIStreamDataMgr.hpp>
#include <JSystem/JAudio2/JASAiCtrl.hpp>
#include <JSystem/JAudio2/JASAramStream.hpp>
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include <JSystem/JAudio2/JASDriverIF.hpp>
#include <JSystem/JAudio2/JASSoundParams.hpp>
#include <JSystem/JAudio2/JAUAudioArcLoader.hpp>
#include <JSystem/JAudio2/JAUInitializer.hpp>
#include <JSystem/JAudio2/JAUSectionHeap.hpp>
#include <JSystem/JAudio2/JAUSoundMgr.hpp>
#include <JSystem/JAudio2/JAUStdSoundInfo.hpp>
#include <JSystem/JAudio2/JAUStreamDataMgr.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JKernel/JKRArchive.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <revolution/sc.h>
#include <revolution/vi/vifuncs.h>

// Match the original pool singleton initialization order.
template JASMemPool< AudMeTrack > JASPoolAllocObject< AudMeTrack >::memPool_;
template JASMemPool< AudMe > JASPoolAllocObject< AudMe >::memPool_;
template JASMemPool< AudSeStrategy > JASPoolAllocObject< AudSeStrategy >::memPool_;
template JASMemPool< AudDopplerAudible< 2 > > JASPoolAllocObject< AudDopplerAudible< 2 > >::memPool_;


#ifdef GALAXY_NATIVE
const char sAudioResFile[] = "/AudioRes/SMR.szs";
const char sJaiSeqArc[] = "/AudioRes/Seqs/JaiSeq.arc";
const char sJaiChordArc[] = "/AudioRes/Info/JaiChord.arc";
const char sJaiMeArc[] = "/AudioRes/Info/JaiMe.arc";
const char sJaiRemixArc[] = "/AudioRes/Info/JaiRemixSeq.arc";
const char sJaiInstListArc[] = "/AudioRes/Info/SoundIdToInstList.arc";
#else
__declspec(section ".data") const char sAudioResFile[] = "/AudioRes/SMR.szs";
__declspec(section ".data") const char sJaiSeqArc[] = "/AudioRes/Seqs/JaiSeq.arc";
__declspec(section ".data") const char sJaiChordArc[] = "/AudioRes/Info/JaiChord.arc";
__declspec(section ".data") const char sJaiMeArc[] = "/AudioRes/Info/JaiMe.arc";
__declspec(section ".data") const char sJaiRemixArc[] = "/AudioRes/Info/JaiRemixSeq.arc";
__declspec(section ".data") const char sJaiInstListArc[] = "/AudioRes/Info/SoundIdToInstList.arc";
#endif

AudSystem* AudSystem::msBasic;

AudSystem* AudNewAudSystem_(JKRHeap* pHeap, JAUSectionHeap* pSectionHeap, int maxChannels, AudArchiveSet* pArchives) {
    AudSystem* system = new AudSystem(pHeap, pSectionHeap, pArchives);
    system->mAudience.mMaxChannels = maxChannels;
    return system;
}

namespace {
    int init_JAS_JAI_(JKRSolidHeap* pHeap) {
        JAU_JASInitializer initJAS;
        initJAS.audioMemory_ = AudParams::aramAddr;
        initJAS.audioMemSize_ = AudParams::aramSize;
        initJAS.heapSize_ = AudParams::workAreaSize;
        initJAS.dvdThreadPriority_ = AudParams::dvdThreadPriority;
        initJAS.audioThreadPriority_ = AudParams::audioThreadPriority;
        initJAS.dvdThreadId_ = AudParams::dvdThreadSwitchID;
        initJAS.audioThreadId_ = AudParams::audioThreadSwitchID;
        initJAS.field_0x1c = AudParams::numFreeTracks;
        initJAS.field_0x20 = AudParams::numFreeTrackChannels;
        initJAS.aramBlockSize_ = AudParams::streamBlockSize;
        initJAS.aramChannelNum_ = AudParams::numStreamChannels;
        initJAS.dspLevel_ = AudParams::outputGain;
        initJAS.initJASystem(pHeap);

        JAU_JAIInitializer initJAI;
        initJAI.mMaxNumSe = AudParams::numFreeSe;
        initJAI.mMaxNumSeq = AudParams::numFreeSeq;
        initJAI.mMaxNumStream = AudParams::numFreeStream;
        initJAI.mMaxNumChildren = AudParams::numFreeSoundChild;
        initJAI.initJAInterface();

        AudDopplerAudible< 2 >::newMemPool(initJAI.mMaxNumSe);
        AudSeStrategy::newMemPool(initJAI.mMaxNumSe);
        SpkSystem::newSoundMemPool(64);
        AudMe::newMemPool(32);
        AudMeTrack::newMemPool(96);

        return initJAI.mMaxNumSeq;
    }
};  // namespace

AudSystem* AudNewAudSystem(JKRHeap* pHeap, JKRSolidHeap* pSolidHeap, AudArchiveSet* pArchives) {
    int maxNumSeq = ::init_JAS_JAI_(pSolidHeap);
    JAUSectionHeap* newHeap = JAUNewSectionHeap(true);
    JKRHeap* prevHeap = MR::becomeCurrentHeap(newHeap->getHeap());
    u32 maxSeqDataSize = 0;

    if (pArchives->mSmrRes == nullptr) {
        if (DVDConvertPathToEntrynum(sAudioResFile) >= 0) {
            JAUAudioArcLoader loader(newHeap);
            loader.load(sAudioResFile);
            newHeap->getHeap()->freeTail();
        }
    } else {
        JAUAudioArcLoader loader(newHeap);
        loader.load(pArchives->mSmrRes);
        newHeap->getHeap()->freeTail();
    }

    if (pArchives->mSeqArchive == nullptr) {
        if (DVDConvertPathToEntrynum(sJaiSeqArc) >= 0) {
            JKRArchive* seqArc = JKRArchive::mount(sJaiSeqArc, JKRArchive::MOUNT_MODE_DVD, newHeap->getHeap(), JKRArchive::MOUNT_DIRECTION_1);
            newHeap->setSeqDataArchive(seqArc);
            maxSeqDataSize = AudParams::maxSeqDataSize;
        } else {
            maxNumSeq = 0;
        }
    } else {
        newHeap->setSeqDataArchive(pArchives->mSeqArchive);
        maxSeqDataSize = AudParams::maxSeqDataSize;
    }

    if (pArchives->mChordArchive == nullptr) {
        pArchives->mChordArchive = JKRArchive::mount(sJaiChordArc, JKRArchive::MOUNT_MODE_MEM, newHeap->getHeap(), JKRArchive::MOUNT_DIRECTION_1);
    }

    if (pArchives->mMeArchive == nullptr) {
        pArchives->mMeArchive = JKRArchive::mount(sJaiMeArc, JKRArchive::MOUNT_MODE_MEM, newHeap->getHeap(), JKRArchive::MOUNT_DIRECTION_1);
    }

    AudSystem* system = AudNewAudSystem_(pHeap, newHeap, 1, pArchives);
    if (maxSeqDataSize != 0) {
        newHeap->newDynamicSeqBlocks(maxSeqDataSize, maxNumSeq);
    }

    MR::becomeCurrentHeap(prevHeap);
    newHeap->finishBuild();

    return system;
}

static JAUStdSoundInfo* newSoundInfo(JAUSection* pSection) {
    JAUSection* heap = pSection;
    JKRHeap* prevHeap = heap->getHeap_()->becomeCurrentHeap();

    JAUDisposerObject_< JAUStdSoundInfo >* soundInfo = new JAUDisposerObject_< JAUStdSoundInfo >(true);

    heap->get_C4().append(soundInfo);
    prevHeap->becomeCurrentHeap();
    return soundInfo;
}

namespace {
    // position of the sound's audible (if it has one)
// NONMATCHING: instruction scheduling
    bool getSoundPos(const JAISound& rSound, TVec3f* pPos) NO_INLINE {
        JAIAudible* audible = rSound.mAudible;
        if (audible != nullptr) {
            pPos->set(audible->getPos());
            return true;
        }

        return false;
    }
};  // namespace

AudSystem::AudSystem(JKRHeap* pHeap, JAUSectionHeap* pSectionHeap, AudArchiveSet* pArchives)
    : mIsResetReady(), mIsReset(), mStopThreads(), mIsSoundMuted(), mIsPaused(), mIsSceneSoundMuted(), mIsStartSoundBlocked(), mIsStartLevelSoundBlocked(), mSystemSoundOnlyFrames(-1), mRestrictedSoundFrames(-1), mSoundInfo(),
      mHeap(pHeap), mSectionHeap(pSectionHeap), mStreamAramMgr(), mSoundMode(), mSystemSeObject(), mSystemMeObject(), mSceneMgr(), mBgmMgr(),
      mRhythmMeSystem(), mRemixMgr(), mAudEffector(), mSeStrategyMgr(), mVolumeController(), mInstListMgr(), mMultiBgmInfo(), mBgmParamTable(),
      mHomeButtonMenuState(), mHomeButtonToggleTime(-1), mPauseMenuState(), mPauseMenuToggleTime(-1), mIsDvdError(), mLimitedSoundInfo(), mFanfareHoldFrames(),
      mIsFanfareProcessEnabled(true), mIsDestroySceneKeepAllSound(), mSpkSystem(), mSeLifeTable(), mStageUseResourceList(), mNumSeLimitGroups() {
    msBasic = this;
    mSoundMode = SCGetSoundMode();
    mLimitedSoundInfo = new (mSectionHeap->getHeap(), 0) AudLimitedSoundInfo[2];
    clearAllLimitedSound();
    clearSeLimitGroups();

    JAISeqDataMgr* seSeqDataMgr = pSectionHeap->getSeSeqDataMgr();
    if (seSeqDataMgr != nullptr) {
        getSeMgr().setSeqDataMgr(seSeqDataMgr);
    }

    JAISeqDataMgr* seqSeqDataMgr = pSectionHeap->getSeqSeqDataMgr();
    if (seqSeqDataMgr != nullptr) {
        getSeqMgr().setSeqDataMgr(seqSeqDataMgr);
    }

    setAudience(&mAudience);

    JAUStreamStaticAramMgr* streamAramMgr = new JAUStreamStaticAramMgr();
    streamAramMgr->alloc(AudParams::streamAramBlocks_perChunk, JASKernel::getAramHeap());
    mStreamAramMgr = streamAramMgr;
    mStreamMgr.setStreamAramMgr(mStreamAramMgr);

    mSoundInfo = nullptr;
    JAUSoundTable* soundTable = mSectionHeap->getSoundTable();
    if (soundTable != nullptr) {
        mSoundInfo = newSoundInfo(mSectionHeap);
    }

    JAIStreamDataMgr* streamDataMgr = pSectionHeap->getStreamDataMgr();
    if (streamDataMgr != nullptr) {
        mStreamMgr.setStreamDataMgr(streamDataMgr);
    } else if (mSoundInfo != nullptr) {
        mStreamMgr.setStreamDataMgr(mSoundInfo);
    }

    initCategoryArrangement();
    initVolumeSetting();

    mSystemSeObject = new (mSectionHeap->getHeap(), 0) AudSoundObject(nullptr, 10, mSectionHeap->getHeap());
    mSceneMgr = new (mSectionHeap->getHeap(), 0) AudSceneMgr(mSectionHeap);
    mBgmMgr = new AudBgmMgr();
    mRhythmMeSystem = new (mSectionHeap->getHeap(), 0) AudRhythmMeSystem(mSectionHeap->getHeap(), 3, true);
    mRemixMgr = new (mSectionHeap->getHeap(), 0) AudRemixMgr(mSectionHeap->getHeap());
    mRemixMgr->init();
    mSeStrategyMgr = new (mSectionHeap->getHeap(), 0) AudSeStrategyMgr();
    mSeMgr.setStrategyMgr(mSeStrategyMgr);

    if (pArchives->mChordArchive != nullptr) {
        mRhythmMeSystem->mChordInfo.init(pArchives->mChordArchive);
    }

    if (pArchives->mMeArchive != nullptr) {
        setMeResource(pArchives->mMeArchive, 1, 0);
    }

    if (pArchives->mRemixArchive != nullptr) {
        mRemixMgr->setRemixSeqResource(pArchives->mRemixArchive->getResource(static_cast< u16 >(0)));
    }

    mSystemMeObject = new (mSectionHeap->getHeap(), 0) AudMeObject(nullptr, 8, mSectionHeap->getHeap());
    mVolumeController = new (mSectionHeap->getHeap(), 0) AudSystemVolumeController(this);
    mAudEffector = new (mSectionHeap->getHeap(), 0) AudEffector();
    mAudEffector->initFxLine(mSectionHeap->getHeap());

    JKRHeap* prevHeap = MR::becomeCurrentHeap(mHeap);

    AudSingletonHolder< AudSoundNameConverter >::init();

    mSpkSystem = new SpkSystem(mHeap);
    mInstListMgr = new AudInstListMgr(mSectionHeap, pArchives->mInstListArchive);
    mInstListMgr->loadInstList();
    mSoundObjHolder = new AudSoundObjHolder(AudParams::numInspectableSoundObj);
    mAtmosphereSeObject = new AudSoundObject(nullptr, 8, nullptr);
    mMultiBgmInfo = new AudMultiBgmInfo();
    mBgmParamTable = new AudBgmParamTable();
    mSeLifeTable = new AudSeLifeTable();
    mStageUseResourceList = new AudStageUseResourceList();
    MR::becomeCurrentHeap(prevHeap);
}

void AudSystem::clearSeLimitGroups() {
    mNumSeLimitGroups = 0;

    for (s32 i = 0; i < 32; i++) {
        for (s32 j = 0; j < 3; j++) {
            mSeLimitGroups[i].mEntries[j].mSe = nullptr;
            mSeLimitGroups[i].mEntries[j].mScore = 0.0f;
        }
    }
}

void AudSystem::updateSeLimitGroups() {
    AudSoundInfo* pSoundInfo = AudWrap::getSoundInfo();
    clearSeLimitGroups();

    for (s32 category = 0; category < JAISeMgr::NUM_CATEGORIES; category++) {
        for (JSULink< JAISe >* it = getSeMgr().getCategory(category)->getSeList()->getFirst(); it != nullptr; it = it->getNext()) {
            JAISe* pSe = it->getObject();

            if (pSe->isStopping()) {
                continue;
            }

            s32 limit = pSoundInfo->getLimitNum(pSe->mSoundID);
            if (limit > 0) {
                s32 index = registerSeLimit(pSe, limit);
                if (index < 0 || index >= limit) {
                    pSe->stop();
                }
            }
        }
    }
}

// NONMATCHING: register allocation / loop counters only (instruction sequence identical)
inline void shiftSeLimitEntries(AudSeLimitEntry* entries, s32 index, s32 limit) {
    for (s32 k = index; k < limit - 1; k++) {
        entries[k + 1] = entries[k];
    }
}

s32 AudSystem::registerSeLimit(JAISe* pSe, s32 limit) {
    f32 score = 0.0f;
    TVec3f pos;
    if (getSoundPos(*pSe, &pos)) {
        score = PSVECDistance(reinterpret_cast< const Vec* >(&pos), reinterpret_cast< const Vec* >(&mAudience.mStates[0].getPos()));
    }

    for (s32 i = 0; i < mNumSeLimitGroups; i++) {
        AudSeLimitEntry* pEntries = mSeLimitGroups[i].mEntries;
        if (pEntries[0].mSe->mSoundID != pSe->mSoundID) {
            continue;
        }

        for (s32 j = 0; j < limit; j++) {
            if (pEntries[j].mSe == nullptr) {
                pEntries[j].mSe = pSe;
                pEntries[j].mScore = score;
                return j;
            }

            if (pEntries[j].mScore > score) {
                AudSeLimitEntry newEntry;
                newEntry.mSe = pSe;
                newEntry.mScore = score;
                AudSeLimitEntry dropped;
                dropped = pEntries[limit - 1];

                shiftSeLimitEntries(pEntries, j, limit);

                pEntries[j] = newEntry;
                if (dropped.mSe != nullptr) {
                    dropped.mSe->stop();
                }

                return j;
            }
        }

        return -1;
    }

    if (mNumSeLimitGroups < 32) {
        mSeLimitGroups[mNumSeLimitGroups].mEntries[0].mSe = pSe;
        mSeLimitGroups[mNumSeLimitGroups].mEntries[0].mScore = score;
        mNumSeLimitGroups++;
        return 0;
    }

    return -1;
}

AudChordInfo* AudSystem::getChordInfo() {
    return &mRhythmMeSystem->mChordInfo;
}

void AudSystem::setSpeakerResource(JKRArchive* pArchive) {
    mSpkSystem->setResource(pArchive, 0, 1);
}

// NONMATCHING: equivalent boolean normalization
void AudSystem::frameWork() {
    screenSaverProcess();
    updateHomeButtonMenu();
    updatePauseMenu();
    seFanfareProcess();

    if (mVolumeController != nullptr) {
        mVolumeController->update();
    }

    mBgmMgr->movement();
    mRhythmMeSystem->update();
    mRemixMgr->update();
    updateSeLimitGroups();
    JAUSoundMgr::calc();
    mixOut();
    mSoundObjHolder->update();

    if (mSpkSystem != nullptr) {
        mSpkSystem->framework();
    }

    if (mSystemSoundOnlyFrames >= 0) {
        mSystemSoundOnlyFrames--;
        if (mSystemSoundOnlyFrames <= 0) {
            mSystemSoundOnlyFrames = -1;
        }
    }

    if (mRestrictedSoundFrames >= 0) {
        mRestrictedSoundFrames--;
        if (mRestrictedSoundFrames <= 0) {
            mRestrictedSoundFrames = -1;
        }
    }

    updateLimitedSound();
    mAudEffector->update();
    mAudience.updateSetting();

    if (!(mHomeButtonMenuState != MenuState_Off) == true && !(mPauseMenuState != MenuState_Off) == true) {
        mSeLifeTable->update();
    }
}

void AudSystem::calc() {
    if (mIsReset && mReseter.checkDone()) {
        if (!mStopThreads) {
            stopSync();
        }

        return;
    }

    JAUSoundMgr::calc();
}

bool AudSystem::startSound(JAISoundID soundID, JAISoundHandle* pHandle, const TVec3f* pPos) {
    if (mIsResetReady && soundID.getGroupID() != 0xD) {  // HOMESE
        return false;
    }

    if (mIsReset) {
        return false;
    }

    if (mIsSoundMuted) {
        return false;
    }

    if (mIsSceneSoundMuted) {
        return false;
    }

    if (mSystemSoundOnlyFrames >= 0 && soundID.getGroupID() != 0xD && soundID.getGroupID() != 0x0) {  // HOMESE, SE_SY
        return false;
    }

    if (mRestrictedSoundFrames >= 0 && isRestrictedSound(soundID)) {
        return false;
    }

    if (mIsStartSoundBlocked && soundID.getGroupID() != 0xD && soundID.getGroupID() != 0x0) {  // HOMESE, SE_SY
        return false;
    }

    if (isSubmitSeByVolumeSetting(soundID)) {
        return false;
    }

    if (soundID.getGroupID() != 0xD && mHomeButtonMenuState == MenuState_Active) {  // disable non-home-menu sounds while the menu is active
        return false;
    }

    if (!JAUSoundMgr::startSound(soundID, pHandle, pPos)) {
        return false;
    }

    switchToLuigi(soundID, pHandle);
    return true;
}

bool AudSystem::startLevelSound(JAISoundID soundID, JAISoundHandle* pHandle, const TVec3f* pPos) {
    if (mIsResetReady && soundID.getGroupID() != 0xD) {  // HOMESE
        return false;
    }

    if (mIsReset) {
        return false;
    }

    if (mIsSoundMuted) {
        return false;
    }

    if (mIsSceneSoundMuted) {
        return false;
    }

    if (mSystemSoundOnlyFrames >= 0 && soundID.getGroupID() != 0xD && soundID.getGroupID() != 0x0) {  // HOMESE, SE_SY
        return false;
    }

    if (mRestrictedSoundFrames >= 0 && isRestrictedSound(soundID)) {
        return false;
    }

    if (mIsStartLevelSoundBlocked && soundID.getGroupID() != 0xD && soundID.getGroupID() != 0x0) {  // HOMESE, SE_SY
        return false;
    }

    if (isSubmitSeByVolumeSetting(soundID)) {
        return false;
    }

    if (soundID.getGroupID() != 0xD && mHomeButtonMenuState != 0) {  // disable non-home-menu sounds if menu is not cleared
        return false;
    }

    if (!JAISoundStarter::startLevelSound(soundID, pHandle, pPos)) {
        return false;
    }

    return true;
}

void AudSystem::switchToLuigi(JAISoundID soundID, JAISoundHandle* pHandle) {
    if (!mSceneMgr->isPlayerModeLuigi()) {
        return;
    }

    if (pHandle == nullptr) {
        return;
    }

    if (!pHandle->isSoundAttached()) {
        return;
    }

    if (soundID.getGroupID() != 0x1) {  // SE_PV
        return;
    }

    JASCriticalSection crit;

    JASTrack* track = pHandle->mSound->getTrack();
    if (track != nullptr) {
        track->writePort(12, 20);
    }
}

bool AudSystem::isSubmitSeByVolumeSetting(JAISoundID soundID) {
    if (mVolumeController == nullptr) {
        return false;
    }

    if (mVolumeController->mCurrVolumeSet >= 1 && mVolumeController->mCurrVolumeSet < 3) {
        return u32(soundID) - 2 <= 2;
    }
    return false;
}

void AudSystem::updateOutputMode() {
    switch (SCGetSoundMode()) {
    case 0:
        mSoundMode = 0;
        JASDriver::setOutputMode(0);  // neither? (TODO)
        break;
    case 1:
        mSoundMode = 1;
        JASDriver::setOutputMode(1);  // only pan
        break;
    case 2:
        mSoundMode = 2;
        JASDriver::setOutputMode(2);  // pan and effect
        break;
    }
}

void AudSystem::preProcessToReset() {
    mSeqMgr.stop(0);
    mStreamMgr.stop(0);
    mBgmMgr->clearLastBGM(AudBgmMgr::BgmType_Stage);
    mBgmMgr->clearLastBGM(AudBgmMgr::BgmType_Sub);
    mBgmMgr->clearNextBGM(AudBgmMgr::BgmType_Stage);
    mBgmMgr->clearNextBGM(AudBgmMgr::BgmType_Sub);

    if (mRhythmMeSystem != nullptr) {
        mRhythmMeSystem->mMeMgr->stopAll(-2);
    }

    mIsResetReady = true;
}

void AudSystem::resetAudio(u32 time, bool stopThreads) {
    mReseter.start(time * JASDriver::getSubFrames(), stopThreads);
    mStopThreads = stopThreads;
    mIsReset = true;
}

void AudSystem::resumeReset() {
    mReseter.resume();
    initParams();
    initVolumeSetting();
    if (mVolumeController != nullptr) {
        mVolumeController->init();
    }

    mIsSceneSoundMuted = false;
    mIsStartSoundBlocked = false;
    mIsStartLevelSoundBlocked = false;
    mHomeButtonMenuState = MenuState_Off;
    mHomeButtonToggleTime = -1;
    mPauseMenuState = MenuState_Off;
    mPauseMenuToggleTime = -1;
    mIsReset = false;
    mIsResetReady = false;
}

bool AudSystem::hasReset() const {
    return mIsReset && mReseter.checkDone() && (mStopThreads || getNumActiveSounds() == 0);
}

void AudSystem::initSceneVolume() {
    unpause();
    if (mVolumeController != nullptr) {
        mVolumeController->init();
    }

    setSeVolumeSet(AudSystemVolumeController::VolumeSet_0, 0);
    mSeqMgr.getParams()->moveVolume(AudParams::masterVolBgm, 0);
    mStreamMgr.getParams()->moveVolume(AudParams::masterVolStream, 0);
    mPauseMenuState = MenuState_Off;
    mPauseMenuToggleTime = -1;
}

void AudSystem::pause() {
    if (mIsPaused) {
        return;
    }

    mBgmMgr->pause();
    mRhythmMeSystem->setIdle(true);
    mIsPaused = true;
}

void AudSystem::unpause() {
    mBgmMgr->unpause();
    mRhythmMeSystem->setIdle(false);
    mIsPaused = false;
}



void AudSystem::stopAllSounds(u32 fadeTime) {
    JAUSoundMgr::stop(fadeTime);
}

void AudSystem::stopAllSe() {
    mSeMgr.stop();
}

void AudSystem::stopSeCategories(u32 fadeTime) {
    JAISeMgr* pSeMgr = &mSeMgr;

    for (int i = 0; i < 16; i++) {
        JAISeCategoryMgr* pCategory = pSeMgr->getCategory(i);

        if (i != 0 && i != 13) {
            pCategory->stop(fadeTime);
        }
    }
}

bool AudSystem::isHomeButtonMenuEntering() const {
    return mHomeButtonMenuState == 1;
}

bool AudSystem::isHomeButtonMenuLeaving() const {
    return static_cast< u32 >(mHomeButtonMenuState - 2) <= 1;
}

void AudSystem::pauseSeCategories(s32 menuType, bool pause) {
    for (int i = 0; i < 16; i++) {
        if (i != 13 && (menuType != 1 || i != 0)) {
            mSeMgr.getCategory(i)->pause(pause);
        }
    }
}

void AudSystem::stopMenuSystemSounds() {
    AudSoundObject* pObject = mSystemSeObject;

    if (pObject != nullptr) {
        pObject->stopSound(JAISoundID(0x11), 0);
        mSystemSeObject->stopSound(JAISoundID(3), 0);
    }
}

void AudSystem::enterHomeButtonMenu() {
    stopMenuSystemSounds();
    mHomeButtonMenuState = MenuState_Enter;
    mHomeButtonToggleTime = 15 + 1;
    mSeqMgr.getParams()->moveVolume(0.0f, 15);
    mStreamMgr.getParams()->moveVolume(0.0f, 15);

    AudSystemVolumeController* pController = mVolumeController;
    if (pController != nullptr) {
        pController->setSeVolumeSetTrig(5, 15);
    }

    mSpkSystem->stopSound(-1, 100);
}

void AudSystem::exitHomeButtonMenu() {
    updateOutputMode();
    mHomeButtonMenuState = MenuState_Exit;
    mHomeButtonToggleTime = 20 + 1;

    if (!isPauseMenuActive()) {
        mSeqMgr.getParams()->moveVolume(AudParams::masterVolBgm, 20);
        mStreamMgr.getParams()->moveVolume(AudParams::masterVolStream, 15);
    }

    AudSystemVolumeController* pController = mVolumeController;
    if (pController != nullptr) {
        pController->recoverSeVolumeSet(20);
    }

    if (!isPauseMenuActive() && !mIsDvdError) {
        pauseSeCategories(0, false);
        unpause();
    }
}

void AudSystem::updateHomeButtonMenu() {
    switch (mHomeButtonMenuState) {
    case MenuState_Off:
        break;
    case MenuState_Enter:
        if (mHomeButtonToggleTime > 0 && --mHomeButtonToggleTime <= 0) {
            mHomeButtonToggleTime = -1;
            mHomeButtonMenuState = MenuState_Active;
            if (!isPauseMenuActive()) {
                pause();
                pauseSeCategories(0, true);
            }
        }

        break;
    case MenuState_Exit:
        if (mHomeButtonToggleTime > 0 && --mHomeButtonToggleTime <= 0) {
            mHomeButtonToggleTime = -1;
            mHomeButtonMenuState = MenuState_Off;
        }

        break;
    case MenuState_Active:
        break;
    }
}

bool AudSystem::isHomeButtonMenuActive() const {
    if (mHomeButtonMenuState == MenuState_Off || mHomeButtonMenuState == MenuState_Exit) {
        return false;
    }

    return true;
}

void AudSystem::enterPauseMenu() {
    stopMenuSystemSounds();
    mPauseMenuState = MenuState_Enter;
    mPauseMenuToggleTime = 1;
    mSeqMgr.getParams()->moveVolume(0.0f, 0);
    mStreamMgr.getParams()->moveVolume(0.0f, 0);

    AudSystemVolumeController* pController = mVolumeController;
    if (pController != nullptr) {
        pController->setSeVolumeSetTrig(6, 0);
    }

    mSpkSystem->stopSound(-1, 100);
}

void AudSystem::exitPauseMenu() {
    mPauseMenuState = MenuState_Exit;
    mPauseMenuToggleTime = 20 + 1;
    mSeqMgr.getParams()->moveVolume(AudParams::masterVolBgm, 20);
    mStreamMgr.getParams()->moveVolume(AudParams::masterVolStream, 20);

    AudSystemVolumeController* pController = mVolumeController;
    if (pController != nullptr) {
        pController->recoverSeVolumeSet(20);
    }

    unpause();
    pauseSeCategories(1, false);
}

void AudSystem::updatePauseMenu() {
    switch (mPauseMenuState) {
    case MenuState_Off:
        break;
    case MenuState_Enter:
        if (mPauseMenuToggleTime > 0 && --mPauseMenuToggleTime <= 0) {
            mPauseMenuToggleTime = -1;
            mPauseMenuState = MenuState_Active;
            pause();
            pauseSeCategories(1, true);
        }

        break;
    case MenuState_Exit:
        if (mPauseMenuToggleTime > 0 && --mPauseMenuToggleTime <= 0) {
            mPauseMenuToggleTime = -1;
            mPauseMenuState = MenuState_Off;
        }

        break;
    case MenuState_Active:
        break;
    }
}

bool AudSystem::isPauseMenuActive() const {
    if (mPauseMenuState == MenuState_Off || mPauseMenuState == MenuState_Exit) {
        return false;
    }

    return true;
}

void AudSystem::setField9A0(s32 value) {
    mFanfareHoldFrames = value;
}

void AudSystem::setField9A4(bool value) {
    mIsFanfareProcessEnabled = value;
}

void AudSystem::setField834(s32 value) {
#ifdef GALAXY_NATIVE
    msBasic->mRestrictedSoundFrames = value;
#else
    *reinterpret_cast< s32* >(reinterpret_cast< u8* >(msBasic) + 0x834) = value;
#endif
}

// Used by AudBgmVolumeController::update and AudSystemVolumeController::updateSeVolumeSetLevel.
// The byte at +0x998 is the DVD error flag (set in doDvdErrorProcess, cleared in exitDvdErrorProcess).
bool AudSystem::isMenuOrErrorActive() const {
    if (isPauseMenuActive()) {
        return true;
    }

    if (isHomeButtonMenuActive()) {
        return true;
    }

    return mIsDvdError;
}

void AudSystem::screenSaverProcess() {
    if (VIGetDVDStopMotorCount() > 0) {
        return;
    }

    setSeVolumeSetLevel(AudSystemVolumeController::VolumeSet_ScreenSaver);
    mBgmMgr->volDownLevel(false);
}

bool AudSystem::isEnableStartSound(JAISoundID soundID) {
    if (mIsReset) {
        return false;
    }

    if (mIsSoundMuted) {
        return false;
    }

    if (mIsResetReady && soundID.getGroupID() != 0xD) {  // HOMESE
        return false;
    }

    if (mSystemSoundOnlyFrames >= 0 && soundID.getGroupID() != 0xD && soundID.getGroupID() != 0x0) {  // HOMESE, SE_SY
        return false;
    }

    if (mRestrictedSoundFrames >= 0 && isRestrictedSound(soundID)) {
        return false;
    }

    return true;
}

bool AudSystem::isRestrictedSound(JAISoundID soundID) {
    u8 groupID = soundID.getGroupID();

    if (groupID == 13) {
        return false;
    }

    if (groupID == 1) {
        return false;
    }

    if (groupID == 2) {
        return false;
    }

    switch (static_cast< u32 >(soundID)) {
    case 0xA0135:
        return false;
    case 0xA0136:
        return false;
    case 0xE8:
        return false;
    case 0x111:
        return false;
    case 0x70076:
        return false;
    case 0x70077:
        return false;
    case 0x70078:
        return false;
    case 0x501FC:
        return false;
    case 0x50359:
        return false;
    case 0x5035A:
        return false;
    case 0x5035B:
        return false;
    default:
        return true;
    }
}

void AudSystem::doDvdErrorProcess() {
    mIsDvdError = true;

    if (!isPauseMenuActive() && !isHomeButtonMenuActive() && !isHomeButtonMenuEntering()) {
        pauseSeCategories(2, true);
        pause();
        mSeqMgr.getParams()->moveVolume(0.0f, 0);
        mStreamMgr.getParams()->moveVolume(0.0f, 0);
    }
}

void AudSystem::exitDvdErrorProcess() {
    if (!isPauseMenuActive() && !isHomeButtonMenuActive()) {
        mSeqMgr.getParams()->moveVolume(AudParams::masterVolBgm, 20);
        mStreamMgr.getParams()->moveVolume(AudParams::masterVolStream, 20);
        unpause();
        pauseSeCategories(2, false);
    }

    mIsDvdError = false;
}

s32 AudSystem::getNumOfPlaying(JAISoundID soundID) {
    s32 count = 0;
    for (s32 category = 0; category < JAISeMgr::NUM_CATEGORIES; category++) {
        for (JSULink< JAISe >* it = getSeMgr().getCategory(category)->getSeList()->getFirst(); it != getSeMgr().getCategory(0)->getSeList()->getEnd();
             it = it->getNext()) {
            if (it->getObject()->mSoundID == soundID) {
                count++;
            }
        }
    }

    return count;
}

u32 AudSystem::getPlayCountMin(JAISoundID soundID) {
    u32 min = -1;
    JAISeMgr& mgr = getSeMgr();
    for (s32 category = 0; category < JAISeMgr::NUM_CATEGORIES; category++) {
        for (JSULink< JAISe >* it = mgr.getCategory(category)->getSeList()->getFirst(); it != mgr.getCategory(0)->getSeList()->getEnd();
             it = it->getNext()) {
            if (it->getObject()->mSoundID == soundID) {
                if (it->getObject()->getCount() < min) {
                    min = it->getObject()->getCount();
                }
            }
        }
    }

    return min;
}

void AudSystem::setMicMtx(MtxPtr pMtx, s32 mic) {
    mAudience.mStates[mic].setMtx(pMtx);
}

const TVec3f& AudSystem::getMicPos(s32 mic) {
    return mAudience.mStates[mic].getPos();
}

void AudSystem::setCameraRate(f32 rate) {
    mAudience.mCameraRate = MR::fastSqrtf(rate);
}

void AudSystem::setFarCamera(bool isFar) {
    mAudience.mFarCamera = isFar;
}

void AudSystem::setSeVolumeSet(s32 volumeSet, u32 maxSteps) {
    if (mVolumeController != nullptr) {
        mVolumeController->setSeVolumeSetTrig(volumeSet, maxSteps);
    }
}

void AudSystem::recoverSeVolumeSet(u32 maxSteps) {
    if (mVolumeController != nullptr) {
        mVolumeController->recoverSeVolumeSet(maxSteps);
    }
}

void AudSystem::setSeVolumeSetLevel(s32 volumeSet, s32 fadeTime, s32 holdFrames) {
    if (mVolumeController != nullptr) {
        mVolumeController->setSeVolumeSetLevel(volumeSet, fadeTime, holdFrames);
    }
}

void AudSystem::initVolumeSetting() {
    setSeVolumeSet(AudSystemVolumeController::VolumeSet_0, 0);
    mSeMgr.getParams()->moveVolume(AudParams::masterVolSe, 0);
    mSeqMgr.getParams()->moveVolume(AudParams::masterVolBgm, 0);
    mStreamMgr.getParams()->moveVolume(AudParams::masterVolStream, 0);
    if (mRhythmMeSystem != nullptr) {
        mRhythmMeSystem->mMeMgr->mMasterVolume = 1.0f;
    }
}

// NONMATCHING: in-TU constant records fold to immediates; target loads their bytes.
void AudSystem::initCategoryArrangement() {
    JAISeCategoryArrangement arrangement;

    arrangement.mItems[0].mMaxActiveSe = AudParams::arrangeSE_SY.mMaxActiveSe;
    arrangement.mItems[0].mMaxInactiveSe = AudParams::arrangeSE_SY.mMaxInactiveSe;
    arrangement.mItems[1].mMaxActiveSe = AudParams::arrangeSE_PV.mMaxActiveSe;
    arrangement.mItems[1].mMaxInactiveSe = AudParams::arrangeSE_PV.mMaxInactiveSe;
    arrangement.mItems[2].mMaxActiveSe = AudParams::arrangeSE_PM.mMaxActiveSe;
    arrangement.mItems[2].mMaxInactiveSe = AudParams::arrangeSE_PM.mMaxInactiveSe;
    arrangement.mItems[3].mMaxActiveSe = AudParams::arrangeSE_BV.mMaxActiveSe;
    arrangement.mItems[3].mMaxInactiveSe = AudParams::arrangeSE_BV.mMaxInactiveSe;
    arrangement.mItems[4].mMaxActiveSe = AudParams::arrangeSE_BM.mMaxActiveSe;
    arrangement.mItems[4].mMaxInactiveSe = AudParams::arrangeSE_BM.mMaxInactiveSe;
    arrangement.mItems[5].mMaxActiveSe = AudParams::arrangeSE_EV.mMaxActiveSe;
    arrangement.mItems[5].mMaxInactiveSe = AudParams::arrangeSE_EV.mMaxInactiveSe;
    arrangement.mItems[6].mMaxActiveSe = AudParams::arrangeSE_EM.mMaxActiveSe;
    arrangement.mItems[6].mMaxInactiveSe = AudParams::arrangeSE_EM.mMaxInactiveSe;
    arrangement.mItems[7].mMaxActiveSe = AudParams::arrangeSE_SV.mMaxActiveSe;
    arrangement.mItems[7].mMaxInactiveSe = AudParams::arrangeSE_SV.mMaxInactiveSe;
    arrangement.mItems[8].mMaxActiveSe = AudParams::arrangeSE_OJ.mMaxActiveSe;
    arrangement.mItems[8].mMaxInactiveSe = AudParams::arrangeSE_OJ.mMaxInactiveSe;
    arrangement.mItems[9].mMaxActiveSe = AudParams::arrangeSE_AT.mMaxActiveSe;
    arrangement.mItems[9].mMaxInactiveSe = AudParams::arrangeSE_AT.mMaxInactiveSe;
    arrangement.mItems[10].mMaxActiveSe = AudParams::arrangeSE_DM.mMaxActiveSe;
    arrangement.mItems[10].mMaxInactiveSe = AudParams::arrangeSE_DM.mMaxInactiveSe;
    arrangement.mItems[11].mMaxActiveSe = AudParams::arrangeSE_SM.mMaxActiveSe;
    arrangement.mItems[11].mMaxInactiveSe = AudParams::arrangeSE_SM.mMaxInactiveSe;
    arrangement.mItems[12].mMaxActiveSe = AudParams::arrangeSE_RS.mMaxActiveSe;
    arrangement.mItems[12].mMaxInactiveSe = AudParams::arrangeSE_RS.mMaxInactiveSe;
    arrangement.mItems[13].mMaxActiveSe = AudParams::arrangeHOMESE.mMaxActiveSe;
    arrangement.mItems[13].mMaxInactiveSe = AudParams::arrangeHOMESE.mMaxInactiveSe;

    mSeMgr.setCategoryArrangement(arrangement);
}

void AudSystem::setMeResource(JKRArchive* pArchive, u16 a1, u16 a2) {
    if (pArchive == nullptr) {
        return;
    }

    void* meTable = pArchive->getResource(a1);
    void* meSeq = pArchive->getResource(a2);
    mRhythmMeSystem->mMeMgr->setMeTable(meTable);
    mRhythmMeSystem->mMeMgr->setMeSeq(meSeq);
}

// PLACEHOLDER(core): SMG2 BGM sound-id enum is not recovered (AudSoundId.hpp is the SMG1 table); resolve when the SMG2 ids are named
void AudSystem::seFanfareProcess() {
    if (mFanfareHoldFrames > 0) {
        mFanfareHoldFrames--;
        return;
    }

    if (mIsFanfareProcessEnabled) {
        if (mBgmMgr->mBgm[AudBgmMgr::BgmType_Sub] != nullptr) {
            u32 id = mBgmMgr->mBgm[AudBgmMgr::BgmType_Sub]->getSoundID();
            if (id - 0x1000000 <= 1 || id == 0x100002C) {
                mBgmMgr->volDownLevel(true);
                return;
            }
        }

        bool firstMorph = false;
        if (getNumOfPlaying(SE_SY_MORPH) != 0) {
            if (mBgmMgr->mBgm[AudBgmMgr::BgmType_Sub] != nullptr) {
                if (mBgmMgr->mBgm[AudBgmMgr::BgmType_Sub]->getSoundID() == 0x1000005) {
                    firstMorph = true;
                }
            }
        }

        if (isFanfareSePlaying()) {
            mBgmMgr->volDownLevel(firstMorph);
        }
    }
}

bool AudSystem::isFanfareSePlaying() {
    AudSoundInfo* info = AudSoundInfo::getInstance();
    for (s32 category = 0; category < JAISeMgr::NUM_CATEGORIES; category++) {
        for (JSULink< JAISe >* it = getSeMgr().getCategory(category)->getSeList()->getFirst(); it != getSeMgr().getCategory(0)->getSeList()->getEnd();
             it = it->getNext()) {
            if (info->isFanfareSe(it->getObject()->mSoundID)) {
                return true;
            }
        }
    }

    return false;
}

void AudSystem::registerLimitedSound(JAISoundID soundID, s32 delay) {
    stopSoundID(soundID);

    for (s32 i = 0; i < 2; i++) {
        if (mLimitedSoundInfo[i].mSoundID == soundID) {
            return;
        }
    }

    for (s32 i = 0; i < 2; i++) {
        if (mLimitedSoundInfo[i].isFree()) {
            mLimitedSoundInfo[i].set(soundID, delay);
            break;
        }
    }
}

void AudSystem::clearAllLimitedSound() {
    for (s32 i = 0; i < 2; i++) {
        mLimitedSoundInfo[i].init();
    }
}

bool AudSystem::isRegisteredLimitedSound(JAISoundID soundID) {
    for (s32 i = 0; i < 2; i++) {
        if (mLimitedSoundInfo[i].mSoundID == soundID) {
            return true;
        }
    }

    return false;
}

void AudSystem::updateLimitedSound() {
    for (s32 i = 0; i < 2; i++) {
        mLimitedSoundInfo[i].update();
    }
}

bool AudSystem::isAlreadyPlayingSoundNear(JAISoundID soundID, const TVec3f* pPos, f32 range) {
    if (pPos == nullptr) {
        return false;
    }

    TVec3f pos;
    for (s32 category = 0; category < JAISeMgr::NUM_CATEGORIES; category++) {
        for (JSULink< JAISe >* it = getSeMgr().getCategory(category)->getSeList()->getFirst(); it != getSeMgr().getCategory(0)->getSeList()->getEnd();
             it = it->getNext()) {
            JAISound& rSound = *it->getObject();
            if (rSound.mSoundID != soundID) {
                continue;
            }

            if (!getSoundPos(rSound, &pos)) {
                continue;
            }

            TVec3f diff(0.0f, 0.0f, 0.0f);
            diff.sub(pos, *pPos);
            if (diff.length() <= range && diff.length() != 0.0f) {
                return true;
            }
        }
    }

    return false;
}

void AudSystem::setVolumeZeroForce(s32 menuType) {
    for (s32 category = 0; category < JAISeMgr::NUM_CATEGORIES; category++) {
        if (category == 0xD) {  // HOMESE
            continue;
        }

        if (menuType == MenuType_Pause && category == 0x0) {  // pause and SE_SY
            continue;
        }

        mSeMgr.getCategory(category)->stop(18);
    }
}

#ifndef GALAXY_NATIVE
JAIStreamAramMgr::~JAIStreamAramMgr() {}
#endif

namespace AudParams {
    const JAISeCategoryArrangementItem arrangeSE_SY = {8, 16};
    const JAISeCategoryArrangementItem arrangeSE_PV = {2, 4};
    const JAISeCategoryArrangementItem arrangeSE_PM = {10, 18};
    const JAISeCategoryArrangementItem arrangeSE_BV = {8, 12};
    const JAISeCategoryArrangementItem arrangeSE_BM = {20, 24};
    const JAISeCategoryArrangementItem arrangeSE_OJ = {10, 18};
    const JAISeCategoryArrangementItem arrangeSE_AT = {16, 28};
    const JAISeCategoryArrangementItem arrangeSE_DM = {10, 20};
    const JAISeCategoryArrangementItem arrangeSE_SM = {24, 32};
    const JAISeCategoryArrangementItem arrangeSE_EV = {16, 36};
    const JAISeCategoryArrangementItem arrangeSE_EM = {16, 20};
    const JAISeCategoryArrangementItem arrangeSE_SV = {8, 12};
    const JAISeCategoryArrangementItem arrangeSE_RS = {16, 20};
    const JAISeCategoryArrangementItem arrangeHOMESE = {8, 16};
}

#ifdef GALAXY_NATIVE
namespace AudParams {
    // JAS/JAI initialization (Wii: unnamed .sdata2 constants 0x807D73C0-0x807D7400 loaded by init_JAS_JAI_ and
    // AudNewAudSystem; names from the initializer fields they are stored into).
    const int numFreeSe = 120;
    const int numFreeSeq = 3;
    const int numFreeStream = 3;
    const int numFreeSoundChild = 160;
    const int aramAddr = 0;
    const int aramSize = 0xDB0000;
    const int workAreaSize = 0x1000;
    const int audioThreadPriority = 2;
    const int dvdThreadPriority = 5;
    const int dvdThreadSwitchID = -1;
    const int audioThreadSwitchID = -1;
    const int numFreeTracks = 200;
    const int numFreeTrackChannels = 1;
    const int streamBlockSize = 0x2760;
    const int numStreamChannels = 4;
    const f32 outputGain = 0.9f;
    const int maxSeqDataSize = 0xDAC0;
    const s32 numInspectableSoundObj = 8;
    const f32 masterVolBgm = 1.2f;
    const f32 masterVolSe = 1.2f;
    const f32 masterVolStream = 2.6f;
    const s32 streamAramBlocks_perChunk = 36;
    const f32 streamSampleRate = 0x1.f400000000000p+14f;
    const f32 faderVolZeroVolume = 0x0.0p+0f;
    const f32 faderVolDownVolume = 0x1.554c980000000p-2f;
    const f32 faderInterruptedVolume = 0x0.0p+0f;
    const s32 faderVolZeroTime = 5;
    const s32 faderVolDownTime = 3;
    const s32 faderInterruptedTime = 10;
    const s32 faderVolZeroRecoverTime = 5;
    const s32 faderVolDownRecoverTime = 20;
    const s32 faderInterruptedRecoverTime = 60;
    const f32 micFarDistance = 0x1.b580000000000p+12f;
    const f32 micNearDistance = 0x1.2c00000000000p+8f;
    const f32 micMinVolume = 0x0.0p+0f;
    const f32 micFrontDolbyZ = -0x1.f400000000000p+9f;
    const f32 micBehindDolbyZ = 0x1.f400000000000p+11f;
    const f32 micMaxDolby = 0x1.0000000000000p+0f;
    const f32 micMinDolby = 0x0.0p+0f;
    const f32 micCenterDolby = 0x1.b333340000000p-1f;
    const f32 micPanAmplitude = 0x1.0000000000000p-1f;
    const f32 micSonicSpeed = 0x1.9000000000000p+9f;
    const f32 micPitchDeltaRatioRange = 0x1.8000000000000p+0f;
    const s32 micPriorityDownRange = 255;
}
#endif
