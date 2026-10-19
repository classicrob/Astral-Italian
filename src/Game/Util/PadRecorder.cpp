#include "Game/Util/PadRecorder.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/WPadHolder.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <revolution/os.h>


namespace {
    const s32 cStatusSize = 0xF0;
    bool sIsReplayPrepared;

    PadRecorder* getPadRec() NO_INLINE {
        return SingletonHolder<GameSystem>::get()->mObjHolder->mPadRecorder;
    }
}

namespace MR {
    void readPadData(int channel, WPadReadDataInfo* pInfo) {
        if (channel > 1) return;
        KPADStatus* pBuffer = pInfo->getReadBuffer();
        s32 count = getPadRec()->replayFrame(channel, pBuffer);
        if (count != -1) pInfo->mValidStatusCount = count;
    }
    bool isPadRecorderStopped() { return getPadRec()->mState == 0; }
    bool isPadRecorderReading() { return getPadRec()->mState == 2; }
    bool prepareReplay(const char* pData, u32 size) {
        sIsReplayPrepared = true;
        getPadRec()->prepareReplay(pData, size);
        return true;
    }
    bool startReplayPad() {
        sIsReplayPrepared = false;
        return getPadRec()->startReplay();
    }
    void stopReplayPad() { getPadRec()->stopReplay(); }
    bool isRequestedStopReplayPad() {
        return GameSequenceFunction::isPadMode() ? testHBMKPadTrigger(0, 0x10) : false;
    }
}

PadRecorder::PadRecorder() : NameObj("パッドレコーダー") {
    mDefaultReader = new PadDataReader();
    mReader = nullptr;
    mReadPos = 0;
    mPacketNo = 0;
    mState = 0;
    mPacketBufferAlloc = new (0x20) u8[0x20];
    mPacketBuffer = mPacketBufferAlloc;
    mStatusNum = 0;
    mDataSize = 0;
    mData = new (0x20) u8[0x1E00];
    mPrevStatus[0] = reinterpret_cast< KPADStatus* >(new (0x20) u8[cStatusSize]);
    mPrevStatus[1] = reinterpret_cast< KPADStatus* >(new (0x20) u8[cStatusSize]);
    mHeader = new PadRecordHeader();
    _48 = 0;
    _4A = 0;
}

void PadRecorder::parsePacketHeader(u32* pPacketNo, u32* pSize, s32* pNum, u32 header) {
    *pPacketNo = static_cast< u8 >(header);
    *pSize = static_cast< u16 >(header >> 8);
    *pNum = header >> 24;
}

void PadRecorder::prepareReplay(const char* pData, u32 size) {
    mDefaultReader->setData(pData, size);
    mReader = mDefaultReader;
    tryInitialize();
}

// NONMATCHING: the target materializes the state compare (subi/cntlzw/srwi.) as if through an inlined bool helper
bool PadRecorder::startReplay() {
    if (mState == 3) {
        mState = 2;
        _48 = 300;
        return true;
    }

    return false;
}

void PadRecorder::stopReplay() {
    if (mReader != nullptr && mState == 2) {
        mState = 0;
        mReader->close();
        mReader = nullptr;
    }
}

s32 PadRecorder::replayFrame(s32 channel, KPADStatus* pStatus) {
    if (mState != 2) {
        return -1;
    }

    if (mStatusNum != 0) {
        const u8* pData = mData;

        for (u32 i = 0; i < mStatusNum; i++) {
            pData = decompress(reinterpret_cast< KPADStatus* >(reinterpret_cast< u8* >(pStatus) + i * cStatusSize), pData, mPrevStatus[channel]);
        }
    }

    DCInvalidateRange(mPacketBuffer, 0x20);
    mReader->read(mPacketBuffer, 4);
    s32 num = mStatusNum;
    mReadPos += 4;

    u32 packetNo;
    u32 size;
    s32 statusNum;
    parsePacketHeader(&packetNo, &size, &statusNum, *reinterpret_cast< u32* >(mPacketBuffer));

    if (packetNo != static_cast< u8 >(mPacketNo)) {
        stopReplay();
        statusNum = 0;
    }

    if (statusNum != 0) {
        DCInvalidateRange(mData, size);
        mReader->read(mData, size);
    }

    mPacketNo++;
    mReadPos += size;
    mStatusNum = statusNum;
    mDataSize = size;
    return num;
}

const u8* PadRecorder::decompress(KPADStatus* pOut, const u8* pMask, KPADStatus* pPrev) {
    u8* pDst = reinterpret_cast< u8* >(pOut);
    u8* pLast = reinterpret_cast< u8* >(pPrev);
    const u8* pData = pMask + 0x21;
    u32 bit = 0;

    for (s32 i = cStatusSize; i != 0; i--) {
        if ((1 << bit) & *pMask) {
            u8 value = *pData++;
            *pDst = value;
            *pLast = value;
        } else {
            *pDst = *pLast;
        }

        bit++;
        pLast++;
        pDst++;

        if (bit == 8) {
            bit = 0;
            pMask++;
        }
    }

    return pData;
}

bool PadRecorder::tryInitialize() {
    MR::zeroMemory(mPrevStatus[0], cStatusSize);
    MR::zeroMemory(mPrevStatus[1], cStatusSize);

    if (mState == 0 && mReader->open()) {
        mState = 3;
        mReadPos = 0;
        mPacketNo = 0;

        if (!readHeader()) {
            _4A = 90;
            mReader->close();
            mReader = nullptr;
            return false;
        }

        readUserFile();
        mStatusNum = 0;
        mState = 2;
        replayFrame(0, nullptr);
        mState = 3;
        return true;
    }

    return false;
}

bool PadRecorder::readHeader() {
    u8 buffer[0x100] ATTRIBUTE_ALIGN(32);

    DCInvalidateRange(buffer, sizeof(buffer));
    mReader->read(buffer, sizeof(PadRecordHeader));
    *mHeader = *reinterpret_cast< PadRecordHeader* >(buffer);

    if (mHeader->mHeaderSize != sizeof(PadRecordHeader)) {
        return false;
    }

    return true;
}

void PadRecorder::readUserFile() {
    DCInvalidateRange(mPacketBuffer, 0x20);
    mReader->read(mPacketBuffer, 4);
    s32 size = *reinterpret_cast< s32* >(mPacketBuffer);

    if (GameSequenceFunction::isPadMode()) {
        mReader->mPos += size;
        return;
    }

    mReader->read(GameSequenceFunction::getClearedSaveTempBuffer(size), size);
    GameSequenceFunction::loadSaveTempBufferAsGameData();
}

PadDataReader::PadDataReader() {
    mData = nullptr;
    mSize = 0;
    mPos = nullptr;
}

void PadDataReader::setData(const char* pData, u32 size) {
    mData = reinterpret_cast< const u8* >(pData);
    mSize = size;
    mPos = reinterpret_cast< const u8* >(pData);
}

bool PadDataReader::open() {
    return true;
}

u32 PadDataReader::read(void* pDst, u32 size) {
    u8* pOut = static_cast< u8* >(pDst);
    u32 n = 0;

    while (mPos < &mData[mSize] && n < size) {
        n++;
        *pOut++ = *mPos;
        mPos++;
    }

    return n;
}

bool PadDataReader::close() {
    return true;
}

PadRecorder::~PadRecorder() {}
