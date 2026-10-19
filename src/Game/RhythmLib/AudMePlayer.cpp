#include "Game/RhythmLib/AudMeTrack.hpp"
#include "Game/Util.hpp"

#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/RhythmLib/AudChordInfo.hpp"
#include "Game/RhythmLib/AudMeHandles.hpp"
#include "Game/RhythmLib/AudMePlayer.hpp"
#include "Game/Util/MathUtil.hpp"
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include <stdint.h>

void AudMeHandle::releaseMe() {
    if (isMeAttached()) {
        mMe->mHandle = nullptr;
        mMe = nullptr;
    }
}

AudMe::AudMe() : JSULink< AudMe >(this), mHandle(), mTrack(), _18(), mPos(), mMeId(-1), mStopTimer(-1), mTimer(), mStartRequested(), mData(), mSeqStartPos(), mMasterVolume(1.0f) {
    initTimed();
}

AudMe::~AudMe() {
    release();
}

void AudMe::requestStart(void* pData, u32 a1) {
    mData = pData;
    mSeqStartPos = a1;
    mStartRequested = 1;
}

bool AudMe::start(void* pData, u32 a1) {
    mTrack = new AudMeTrack();
    if (mTrack == nullptr) {
        return false;
    }

    JASCriticalSection crit;
    mTrack->setSeqData(pData, a1);
    updateTimedParams(true);
    mTrack->startSeq();

    return true;
}

void AudMe::stop(s32 time) {
    if (time == -1) {
        if (mTrack != nullptr) {
            mTrack->stopAtCurrentNoteEnd();
        }

        return;
    }

    if (time == -2) {
        doneStop();
        return;
    }

    if (time == 0) {
        time = mDefaultStopTime;
    }

    if (time != 0) {
        setTimedParam(MoveParam_DistFilter, 0.0f, time);
        return;
    }

    doneStop();
}

void AudMe::release() {
    if (mTrack != nullptr) {
        JASCriticalSection crit;
        if (mTrack->mStatus != AudMeTrack::STATUS_FREE) {
            stop(0);
        }

        delete mTrack;
        mTrack = nullptr;
        mMeId = -1;
    }

    releaseHandle();
}

void AudMe::attachHandle(AudMeHandle* pHandle) {
    if (mHandle != nullptr) {
        releaseHandle();
    }

    mHandle = pHandle;
    mHandle->mMe = this;
}

void AudMe::releaseHandle() {
    if (mHandle == nullptr) {
        return;
    }

    mHandle->mMe = nullptr;
    mHandle = nullptr;
}

void AudMe::update() {
    if (mStartRequested == 1) {
        start(mData, mSeqStartPos);
        mStartRequested = 0;
    }

    if (mStopTimer > 0) {
        mStopTimer--;
        if (mStopTimer <= 0) {
            stop(0);
            mStopTimer = -1;
        }
    }

    if (mMoveParams[MoveParam_DistFilter].mValue <= 0.0f) {
        doneStop();
    }

    if (mTrack != nullptr) {
        updateTimedParams(false);
        {
            JASCriticalSection crit;
            mTrack->gframeProc();
        }

        if (mTrack->mStatus == AudMeTrack::STATUS_FREE) {
            delete mTrack;
            mTrack = nullptr;
        }
    }

    mTimer++;
}

void AudMe::setTimedParam(s32 param, f32 value, u32 time) {
    if (time == 0) {
        mMoveParams[param].mValue = value;
        mMoveParams[param].mDelta = 0.0f;
    } else {
        mMoveParams[param].mDelta = (value - mMoveParams[param].mValue) / time;
    }

    mMoveParams[param].mTarget = value;
}

void AudMe::initTimed() {
    mMoveParams[MoveParam_Volume].mValue = 1.0f;
    mMoveParams[MoveParam_Pitch].mValue = 1.0f;
    mMoveParams[MoveParam_Pan].mValue = 0.5f;
    mMoveParams[MoveParam_FxMix].mValue = 0.0f;
    mMoveParams[MoveParam_Dolby].mValue = 0.0f;
    mMoveParams[MoveParam_DistFilter].mValue = 1.0f;

    for (s32 i = 0; i < TIMED_PARAMS; i++) {
        mMoveParams[i].mDelta = 0.0f;
        mMoveParams[i].mTarget = mMoveParams[i].mValue;
    }
}

void AudMe::updateTimedParams(bool immediate) {
    if (!immediate) {
        for (s32 i = 0; i < TIMED_PARAMS; i++) {
            if (mMoveParams[i].mDelta != 0.0f) {
                mMoveParams[i].mValue += mMoveParams[i].mDelta;
                if (mMoveParams[i].mDelta > 0.0f) {
                    if (mMoveParams[i].mValue >= mMoveParams[i].mTarget) {
                        mMoveParams[i].mValue = mMoveParams[i].mTarget;
                        mMoveParams[i].mDelta = 0.0f;
                    }
                } else {
                    if (mMoveParams[i].mValue <= mMoveParams[i].mTarget) {
                        mMoveParams[i].mValue = mMoveParams[i].mTarget;
                        mMoveParams[i].mDelta = 0.0f;
                    }
                }
            }
        }
    }

    if (mTrack == nullptr) {
        return;
    }

    f32 volume = mMasterVolume * (mVolume * (mMoveParams[MoveParam_Volume].mValue * mMoveParams[MoveParam_DistFilter].mValue));
    u32 numChannelMgrs = mTrack->mNumChannelMgrs;
    f32 pan = (mMoveParams[MoveParam_Pan].mValue - 0.5f) + (mOuterPan - 0.5f);
    pan += 0.5f;
    f32 dolby = mMoveParams[MoveParam_Dolby].mValue + mOuterDolby;

    for (u32 i = 0; i < numChannelMgrs; i++) {
        JASCriticalSection crit;
        AudMeChannelMgr* pChannelMgr = mTrack->mChannelMgrs[i];
        if (pChannelMgr == nullptr) {
            continue;
        }

        pChannelMgr->mParams.mVolume = volume;
        pChannelMgr->mParams.mFxMix = mMoveParams[MoveParam_FxMix].mValue;
        pChannelMgr->mParams.mPitch = mMoveParams[MoveParam_Pitch].mValue;
        pChannelMgr->mParams.mPan = pan;
        pChannelMgr->mParams.mDolby = dolby;
    }
}

void AudMe::setOuterPlayingParams(f32 volume, f32 f2, f32 f3, s32 priority, s32 a5) {
    mVolume = volume;
    mOuterPan = f2;
    mOuterDolby = f3;
    mPriority = priority;
    mDefaultStopTime = a5;
}

void AudMe::doneStop() {
    if (mTrack != nullptr) {
        mTrack->stopSeq();
        release();
    }

    releaseHandle();
    mStopTimer = -1;
}

void AudMe::setPauseFlag(bool pause) {
    if (mTrack != nullptr) {
        JASCriticalSection crit;
        mTrack->setPauseFlag(pause);
    }
}

AudMeMgr::AudMeMgr() : JASGlobalInstance< AudMeMgr >(true), mMeSeq(), mMasterVolume(1.0f) {
    for (s32 i = 0; i < 64; i++) {
        _10[i] = 60;
    }
}

void AudMeMgr::setMeSeq(void* pData) {
    mMeSeq = (AudMeTable*)pData;
}

void AudMeMgr::setMeTable(void* pData) {
    mParams.setResource(pData);
}

bool AudMeMgr::startMe(u32 meId, AudMeHandle* pHandle, const TVec3f* pPos) {
    if (meId == -1) {
        return false;
    }

    if (mMeSeq == nullptr) {
        return false;
    }

    if (!AudWrap::getSystem()->getChordInfo()->isAvailable()) {
        return false;
    }

    if (pHandle != nullptr && pHandle->isMeAttached()) {
        if (pHandle->getMe()->mStopTimer > -1) {
            pHandle->getMe()->stop(0);
        }
    }

    AudMe* me = new AudMe();
    if (me == nullptr) {
        return false;
    }

    if (pHandle != nullptr) {
        me->attachHandle(pHandle);
    }

    if (pPos != nullptr) {
        me->mPos = pPos;
    }

    s32 startPos = getSeqStartPos(meId);
    if (startPos < 0) {
        pHandle->releaseMe();
        delete me;
        return false;
    }

    if (isRequestedMe(startPos)) {
        pHandle->releaseMe();
        delete me;
        return false;
    }

    me->mMeId = meId;
    setOuterPlayingParam(meId, me);
    me->requestStart(mMeSeq, startPos);
    mMeList.append(me);
    me->mMasterVolume = mMasterVolume;
    return true;
}

void AudMeMgr::freeDeadMe() {
    for (JSULink< AudMe >* it = mMeList.getFirst(); it != mMeList.getEnd();) {
        JSULink< AudMe >* next = it->getNext();
        if (it->getObject()->mTrack == nullptr) {
            mMeList.remove(it);
            delete it->getObject();
        }

        it = next;
    }
}

void AudMeMgr::updateEachMe() {
    for (JSULink< AudMe >* it = mMeList.getFirst(); it != mMeList.getEnd(); it = it->getNext()) {
        if (!AudWrap::getSystem()->getChordInfo()->isAvailable()) {
            it->getObject()->stop(0);
        }

        it->getObject()->update();
    }
}

void AudMeMgr::stopAll(s32 time) {
    for (JSULink< AudMe >* it = mMeList.getFirst(); it != mMeList.getEnd(); it = it->getNext()) {
        it->getObject()->stop(time);
    }
}

void AudMeMgr::setPauseFlagAll(bool pause) {
    for (JSULink< AudMe >* it = mMeList.getFirst(); it != mMeList.getEnd(); it = it->getNext()) {
        it->getObject()->setPauseFlag(pause);
    }
}

void AudMeMgr::update() {
    updateEachMe();
    freeDeadMe();
}

s32 AudMeMgr::getSeqStartPos(u32 meId) {
    if (mMeSeq == nullptr) {
        return -1;
    }

    s32* pStartPos = mMeSeq->mSeqStartPos;
    if (meId >= mMeSeq->mNumEntries) {
        return -1;
    }

    return pStartPos[meId];
}

bool AudMeMgr::isRequestedMe(u32 meId) {
    AudMeTrack* track;
    for (JSULink< AudMe >* it = mMeList.getFirst(); it != mMeList.getEnd(); it = it->getNext()) {
        track = it->getObject()->mTrack;
        if (track != nullptr) {
            JASCriticalSection crit;
            if (meId == track->mMeId) {
                bool requested = track->mStatus == AudMeTrack::STATUS_RUN || track->mStatus == AudMeTrack::STATUS_READY;
                if (requested) {
                    return true;
                }
            }
        }
    }

    return false;
}

void AudMeMgr::setOuterPlayingParam(s32 meId, AudMe* pMe) {
    AudMePlayingParams* params = &mParams.mParams[meId];
    f32 vol = params->mChordVolume / 127.0f;
    vol *= vol;
    f32 f2 = params->mPan / 127.0f;
    f32 f3 = params->mDolby / 127.0f;
    pMe->setOuterPlayingParams(vol, f2, f3, params->mPriority, params->mStopTime);
}

void AudMePlayingParamsHolder::setResource(void* pRes) {
    s32* cursor = (s32*)pRes;

    s32 numEntries = *cursor++;
    s32 entryOff = *cursor++;
    s32 namesOff = *cursor++;

    mNumEntries = numEntries;

    u8* resource = static_cast< u8* >(pRes);
    const char** offsets = reinterpret_cast< const char** >(resource + namesOff);
    mParams = reinterpret_cast< AudMePlayingParams* >(resource + entryOff);

    for (u32 i = 0; i < numEntries; i++) {
        uintptr_t address = reinterpret_cast< uintptr_t >(offsets[i]);
        address += reinterpret_cast< uintptr_t >(pRes);
        offsets[i] = reinterpret_cast< const char* >(address);
    }

    mNames = (const char**)offsets;
}
