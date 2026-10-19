#include "Game/RhythmLib/AudRhythmHolder.hpp"
#include "Game/Util.hpp"
#include "Game/RhythmLib/AudRhythmSeqParser.hpp"
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include <revolution/os/OSTime.h>

AudRhythmHolder::AudRhythmHolder() : mIsPaused(), mSeqParser(), _198() {
    init();
}

void AudRhythmHolder::movement() {
    for (s32 index = 0; index < NUM_RHYTHM_INFO; index++) {
        mInfos[index].mIsActive = false;
    }
}

void AudRhythmHolder::copyInfoFromParser() {
    if (mSeqParser == nullptr) {
        return;
    }

    JASCriticalSection crit;
    mModifiedTempo = mSeqParser->getModifiedTempo();
    mTempo = mSeqParser->getTempo();
    if (!mIsPaused) {
        for (s32 i = 0; i < 6; i++) {
            mInfos[i].mPrevBeatCount = mInfos[i].mBeatCount;
        }
    }

    u32 velocity, noteOff, channel;
    s64 time;

    if (mSeqParser->checkNoteOnKey(mSeqParser->mRhythmType0, velocity, noteOff, time, channel)) {
        copyInfoFromParserOne(AudRhythmSeqParser::RhythmType_0, velocity, noteOff, time);
        mIsRhythmActive = true;
    }
    if (mSeqParser->checkNoteOnKey(mSeqParser->mRhythmType1, velocity, noteOff, time, channel)) {
        copyInfoFromParserOne(AudRhythmSeqParser::RhythmType_1, velocity, noteOff, time);
        mIsRhythmActive = true;
    }
    if (mSeqParser->checkNoteOnKey(mSeqParser->mRhythmType2, velocity, noteOff, time, channel)) {
        copyInfoFromParserOne(AudRhythmSeqParser::RhythmType_2, velocity, noteOff, time);
        mIsRhythmActive = true;
    }
    if (mSeqParser->checkNoteOnKey(mSeqParser->mRhythmType3, velocity, noteOff, time, channel)) {
        copyInfoFromParserOne(AudRhythmSeqParser::RhythmType_3, velocity, noteOff, time);
        mIsRhythmActive = true;
    }
    if (mSeqParser->checkNoteOnKey(mSeqParser->mRhythmType4, velocity, noteOff, time, channel)) {
        copyInfoFromParserOne(AudRhythmSeqParser::RhythmType_4, velocity, noteOff, time);
        mIsRhythmActive = true;
    }

    s64 currTime = OSGetTime();
    for (s32 i = 0; i < 6; i++) {
        if (mIsPaused) {
            mInfos[i].mPausedTime = (currTime - mInfos[i].mLastMeasuredTime) + mInfos[i].mPausedTime;
        } else {
            mInfos[i].mPrevPlayTime = mInfos[i].mPlayTime;
            mInfos[i].mPlayTime = (currTime - mInfos[i].mStartTime) - mInfos[i].mPausedTime;
        }
        mInfos[i].mLastMeasuredTime = currTime;
    }
}

void AudRhythmHolder::init() {
    for (s32 i = 0; i < 6; i++) {
        mInfos[i].mIsActive = false;
        mInfos[i].mHardHitCount = 0;
        mInfos[i].mSoftHitVelocity = 0;
        mInfos[i].mNoteDurationSec = 0.0f;
        mInfos[i].mStartTime = 0;
        mInfos[i].mPlayTime = 0;
        mInfos[i].mPrevPlayTime = 0;
        mInfos[i].mPrevBeatCount = 0;
        mInfos[i].mBeatCount = 0;
        mInfos[i].mLastMeasuredTime = 0;
        mInfos[i].mPausedTime = 0;
    }

    mModifiedTempo = 120.0f;
    mTempo = 120.0f;
    _8 = false;
    mIsRhythmActive = false;
}

void AudRhythmHolder::copyInfoFromParserOne(u32 type, u32 velocity, u32 noteOff, s64 time) {
    mInfos[type].mIsActive = true;
    if (velocity < 100) {
        mInfos[type].mSoftHitVelocity = velocity - 10;
        mInfos[type].mHardHitCount = 0;
    } else {
        mInfos[type].mHardHitCount++;
    }
    mInfos[type].mStartTime = time;
    mInfos[type].mNoteDurationSec = noteOff / 120.0f * (60.0f / mModifiedTempo);
    mInfos[type].mBeatCount++;
    mInfos[type].mPausedTime = 0;
}

// PLACEHOLDER(enemies): target does fdivs on the unrounded __cvt_ull_dbl result (no frsp); every u64->f32 spelling tried either calls __cvt_ull_flt or adds frsp; resolve when the original expression is recovered
f32 AudRhythmHolder::getPlayTimeSec(s32 index) {
    return (f64)(u64)mInfos[index].mPlayTime / (f32)(OS_BUS_CLOCK >> 2);
}

f32 AudRhythmHolder::getPrevPlayTimeSec(s32 index) {
    return (f64)(u64)mInfos[index].mPrevPlayTime / (f32)(OS_BUS_CLOCK >> 2);
}

f32 AudRhythmHolder::calcBeatPhase(s32 index) {
    if (0.0f == mInfos[index].mNoteDurationSec) {
        return 0.0f;
    }
    return getPlayTimeSec(index) / mInfos[index].mNoteDurationSec;
}

f32 AudRhythmHolder::calcPrevBeatPhase(s32 index) {
    if (0.0f == mInfos[index].mNoteDurationSec) {
        return 0.0f;
    }
    return getPrevPlayTimeSec(index) / mInfos[index].mNoteDurationSec;
}
