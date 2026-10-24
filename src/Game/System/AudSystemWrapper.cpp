#define AUD_ARCHIVE_SET_MANUAL_RESET
#include "Game/System/AudSystemWrapper.hpp"
#include "Game/Util.hpp"
#include "Game/AudioLib/AudMeNameConverter.hpp"
#include "Game/AudioLib/AudMicWrap.hpp"
#include "Game/AudioLib/AudSceneMgr.hpp"
#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/AudioLib/AudSpeakerWrap.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudInstListMgr.hpp"
#include "Game/AudioLib/AudStageUseResourceList.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/AudioLib/CSSoundNameConverter.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <JSystem/JKernel/JKRDvdRipper.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>

namespace AudRhythmWrap {
    void rhythmProc();
};  // namespace AudRhythmWrap

AudSystemWrapper::AudSystemWrapper(JKRSolidHeap* pSolidHeap, JKRHeap* pHeap, JKRHeap* pSystemHeap) {
    mIsResetDone = false;
    mAudSystem = nullptr;
    mSolidHeap = pSolidHeap;
    mHeap = pHeap;
    mSystemHeap = pSystemHeap;
    mSpkRes = nullptr;
    _28 = false;
    mDisableReset = false;
    mArchives.reset();
    mSpkHeap = JKRExpHeap::create(0x50000, pSystemHeap, false);
}

void AudSystemWrapper::requestResourceForInitialize() {
    MR::loadAsyncToMainRAM(sAudioResFile, nullptr, mHeap, JKRDvdRipper::ALLOC_DIRECTION_BACKWARD);
    MR::mountAsyncArchive(sJaiSeqArc, mSolidHeap, true, true);
    MR::mountAsyncArchive(sJaiChordArc, mSolidHeap, true, true);
    MR::mountAsyncArchive(sJaiMeArc, mSolidHeap, true, true);
    MR::mountAsyncArchive("/AudioRes/Info/JaiRemixSeq.arc", mSolidHeap, true, true);
    MR::mountAsyncArchive(AudSpeakerWrap::getResName(), mSpkHeap, true, true);
    MR::mountAsyncArchive("/AudioRes/Info/SoundIdToInstList.arc", mHeap, true, 2);
}

void AudSystemWrapper::createAudioSystem() {
    receiveResourceForInitialize();

    if (mIsResetDone) {
        OSSuspendThread(OSGetCurrentThread());
    }

    mDisableReset = true;
    mAudSystem = AudNewAudSystem(mSystemHeap, mSolidHeap, &mArchives);
    mDisableReset = false;

    MR::removeFileConsideringLanguage(sAudioResFile);
    mArchives.mSmrRes = nullptr;
    MR::removeFileConsideringLanguage("/AudioRes/Info/SoundIdToInstList.arc");
    mArchives.mInstListArchive = nullptr;

    mAudSystem->setSpeakerResource(mSpkRes);
    createSoundNameConverter();
    AudMicWrap::setMicEnv();
}

void AudSystemWrapper::createSoundNameConverter() {
    AudSingletonHolder< AudMeNameConverter >::init();
    AudSingletonHolder< CSSoundNameConverter >::init();
}

void AudSystemWrapper::updateRhythm() {
    if (mAudSystem == nullptr) {
        return;
    }

    AudRhythmWrap::rhythmProc();
}

void AudSystemWrapper::movement() {
    if (mAudSystem == nullptr) {
        return;
    }

    mAudSystem->frameWork();
}

void AudSystemWrapper::stopAllSound(u32 time) {
    if (mAudSystem) mAudSystem->stopAllSounds(time);
}

bool AudSystemWrapper::isLoadDoneWaveDataAtSystemInit() const {
    if (mAudSystem == nullptr) {
        return false;
    }

    if (mAudSystem->mSceneMgr == nullptr) {
        return false;
    }

    return mAudSystem->mSceneMgr->isLoadDoneSystemInit();
}

void AudSystemWrapper::loadStaticWaveData() {
    if (mAudSystem == nullptr) {
        return;
    }

    if (mAudSystem->mSceneMgr == nullptr) {
        return;
    }

    mAudSystem->mSceneMgr->loadStaticResource();
}

bool AudSystemWrapper::isLoadDoneStaticWaveData() const {
    if (mAudSystem == nullptr) {
        return false;
    }

    if (mAudSystem->mSceneMgr == nullptr) {
        return false;
    }

    return mAudSystem->mSceneMgr->isLoadDoneStaticResource();
}

void AudSystemWrapper::loadStageWaveData(const char* pSceneName, const char* pStageName, bool isPlayerLuigi) {
    if (mAudSystem == nullptr) {
        return;
    }

    if (mAudSystem->mSceneMgr == nullptr) {
        return;
    }

    if (isPlayerLuigi) {
        mAudSystem->mSceneMgr->setPlayerModeLuigi();
    } else {
        mAudSystem->mSceneMgr->setPlayerModeMario();
    }

    mAudSystem->mSceneMgr->loadStageResource(pSceneName, pStageName);
}

bool AudSystemWrapper::isLoadDoneStageWaveData() const {
    if (mAudSystem == nullptr) {
        return false;
    }

    if (mAudSystem->mSceneMgr == nullptr) {
        return false;
    }

    return mAudSystem->mSceneMgr->isLoadDoneStageResource();
}

void AudSystemWrapper::loadScenarioWaveData(const char* pSceneName, const char* pStageName, s32 scenarioNo) {
    if (mAudSystem == nullptr) {
        return;
    }

    if (mAudSystem->mSceneMgr == nullptr) {
        return;
    }

    mAudSystem->mSceneMgr->loadScenarioResource(pSceneName, pStageName, scenarioNo);
}

void AudSystemWrapper::setStageResourceLoading(bool flag) {
    if (mAudSystem) mAudSystem->mInstListMgr->loadRequestedWaves(flag, true);
}

void AudSystemWrapper::loadScenarioWaveData(const char* scene, const char* stage, s32 scenario, bool flag) {
    if (mAudSystem && mAudSystem->mSceneMgr) {
        mAudSystem->mSceneMgr->loadScenarioResource(scene, stage, scenario);
        AudInstListMgr* mgr = mAudSystem->mInstListMgr;
        if (GameSequenceFunction::isGreenStarEventOnly()) mgr->requestWave(0x34, 30, false);
        mgr->loadRequestedWaves(flag, true);
        mgr->resetStageWaveSlots();
        mgr->_281C = false;
    }
}

bool AudSystemWrapper::isLoadDoneScenarioWaveData() const {
    return isLoadDoneRequestedWaves();
}

void AudSystemWrapper::finishStageResourceLoading() {
    mAudSystem->mInstListMgr->clearWaveRequests();
}

// NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited (FP order/grouping checked: no FP instructions).
bool AudSystemWrapper::isLoadDoneRequestedWaves() const {
    if (mAudSystem == nullptr) {
        return false;
    } else {
        return mAudSystem->mInstListMgr->isLoadDone() != 0;
    }
}

// NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited (FP order/grouping checked: no FP instructions).
bool AudSystemWrapper::isLoadDoneSystemWaves() const {
    if (mAudSystem == nullptr) {
        return false;
    } else {
        return mAudSystem->mInstListMgr->isLoadDoneSystemWaves() != 0;
    }
}

void AudSystemWrapper::clearStageWaveResources() {
    if (mAudSystem) mAudSystem->mInstListMgr->clearStageWaveResources();
}

void AudSystemWrapper::resetStageResources() {
    if (mAudSystem) mAudSystem->mInstListMgr->resetStageResources();
}

void AudSystemWrapper::loadStageUseResources() {
    mAudSystem->mStageUseResourceList->load();
}

void AudSystemWrapper::loadSystemWaveData() {
    mAudSystem->mSceneMgr->loadSystemResource();
}

// NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited (FP order/grouping checked: no FP instructions).
void AudSystemWrapper::requestWaveResource() {
    if (mAudSystem) {
        if (mAudSystem->mSceneMgr)
            mAudSystem->mSceneMgr->requestResource();
    }
}

bool AudSystemWrapper::isPermitToReset() const {
    return !mDisableReset;
}

void AudSystemWrapper::prepareReset() {
    if (mAudSystem == nullptr) {
        mIsResetDone = true;
    } else {
        mAudSystem->preProcessToReset();
    }
}

void AudSystemWrapper::requestReset(bool stopThreads) {
    if (mAudSystem == nullptr) {
        mIsResetDone = true;
    } else {
        mAudSystem->resetAudio(10, stopThreads);
        if (mAudSystem) mAudSystem->stopAllSounds(10);
    }
}

bool AudSystemWrapper::isResetDone() {
    if (mIsResetDone) {
        return true;
    }

    if (mAudSystem == nullptr) {
        return true;
    }

    return mAudSystem->hasReset();
}

void AudSystemWrapper::resumeReset() {
    if (mIsResetDone) {
        mIsResetDone = false;
    }

    if (mAudSystem == nullptr) {
        return;
    }

    return mAudSystem->resumeReset();
}

void AudSystemWrapper::receiveResourceForInitialize() {
    mArchives.reset();
    mArchives.mSmrRes = MR::receiveFile(sAudioResFile);
    mArchives.mSeqArchive = MR::receiveArchive(sJaiSeqArc);
    mArchives.mChordArchive = MR::receiveArchive(sJaiChordArc);
    mArchives.mMeArchive = MR::receiveArchive(sJaiMeArc);
    mArchives.mRemixArchive = MR::receiveArchive("/AudioRes/Info/JaiRemixSeq.arc");
    mArchives.mInstListArchive = MR::receiveArchive("/AudioRes/Info/SoundIdToInstList.arc");
    mSpkRes = MR::receiveArchive(AudSpeakerWrap::getResName());
}
