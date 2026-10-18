#include "Game/RhythmLib/AudRhythmHolder.hpp"
#include "Game/Util.hpp"
#include "Game/RhythmLib/AudRhythmSeqParser.hpp"
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include <revolution/os/OSTime.h>

AudRhythmHolder::AudRhythmHolder() : mIsPaused(), mSeqParser(), _168() {
    init();
}

void AudRhythmHolder::movement() {
    for (s32 index = 0; index < NUM_RHYTHM_INFO; index++) {
        mInfos[index].mIsActive = false;
    }
}

// NONMATCHING: Petari port differs in loop/field order (0x290 vs Petari 0x254, 88%)
void AudRhythmHolder::copyInfoFromParser() {
    if (mSeqParser == nullptr) {
        return;
    }

    JASCriticalSection crit;
    mModifiedTempo = mSeqParser->getModifiedTempo();
    mTempo = mSeqParser->getTempo();

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
            mInfos[i].mPlayTime = (currTime - mInfos[i].mStartTime) - mInfos[i].mPausedTime;
        }
        mInfos[i].mLastMeasuredTime = currTime;
    }
}

// NONMATCHING: store order inside the init loop (90%)
void AudRhythmHolder::init() {
    for (s32 i = 0; i < 6; i++) {
        mInfos[i].mIsActive = false;
        mInfos[i]._4 = 0;
        mInfos[i]._8 = 0;
        mInfos[i]._C = 0.0f;
        mInfos[i].mStartTime = 0;
        mInfos[i].mPlayTime = 0;
        mInfos[i]._30 = 0;
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
        mInfos[type]._8 = velocity - 10;
        mInfos[type]._4 = 0;
    } else {
        mInfos[type]._4++;
    }
    mInfos[type].mStartTime = time;
    mInfos[type]._C = noteOff / 120.0f * (60.0f / mModifiedTempo);
    mInfos[type]._30++;
    mInfos[type].mPausedTime = 0;
}

f32 fn_8042A170(s32 pBase, s32 index) {
    const u8* pRec = reinterpret_cast< const u8* >(pBase) + (index << 6);
    u64 time = ((u64)*(const u32*)(pRec + 0x28) << 32) | *(const u32*)(pRec + 0x2C);
    return (f64)time / (f32)(OS_BUS_CLOCK >> 2);
}

f32 fn_8042A1D0(s32 pBase, s32 index) {
    const u8* pRec = reinterpret_cast< const u8* >(pBase) + (index << 6);
    u64 time = ((u64)*(const u32*)(pRec + 0x30) << 32) | *(const u32*)(pRec + 0x34);
    return (f64)time / (f32)(OS_BUS_CLOCK >> 2);
}

f32 fn_8042A230(s32 pBase, s32 index) {
    const u8* pRec = reinterpret_cast< const u8* >(pBase) + (index << 6);
    f32 rate = *(const f32*)(pRec + 0x1C);
    if (0.0f == rate) {
        return 0.0f;
    }
    return fn_8042A170(pBase, index) / rate;
}

f32 fn_8042A280(s32 pBase, s32 index) {
    const u8* pRec = reinterpret_cast< const u8* >(pBase) + (index << 6);
    f32 rate = *(const f32*)(pRec + 0x1C);
    if (0.0f == rate) {
        return 0.0f;
    }
    return fn_8042A1D0(pBase, index) / rate;
}
