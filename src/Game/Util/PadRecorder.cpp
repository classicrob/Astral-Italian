#include "Game/Util/PadRecorder.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/WPadHolder.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <revolution/os.h>
#ifdef GALAXY_NATIVE
#include <petari/endian.hpp>
#include <cstdint>
#include <cstring>
#endif


namespace {
    const s32 cStatusSize = 0xF0;
    bool sIsReplayPrepared;

#ifdef GALAXY_NATIVE
    // .pad recordings contain Wii big-endian scalar representations. Keep the
    // delta history in wire order; only the returned KPAD sample is host order.
    template <typename T>
    void readRecordedScalar(T& value, const u8* wire) {
        static_assert(sizeof(T) == 4 || sizeof(T) == 8, "recorded scalar width");
        if constexpr (sizeof(T) == 4) {
            const std::uint32_t bits = PetariNative::readU32BE(wire);
            std::memcpy(&value, &bits, sizeof(value));
        } else {
            const std::uint64_t bits = (std::uint64_t(PetariNative::readU32BE(wire)) << 32) |
                                       PetariNative::readU32BE(wire + 4);
            std::memcpy(&value, &bits, sizeof(value));
        }
    }

    void readRecordedVec(Vec& value, const u8* wire) {
        readRecordedScalar(value.x, wire);
        readRecordedScalar(value.y, wire + 4);
        readRecordedScalar(value.z, wire + 8);
    }

    void readRecordedVec2(Vec2& value, const u8* wire) {
        readRecordedScalar(value.x, wire);
        readRecordedScalar(value.y, wire + 4);
    }

    void decodeRecordedStatus(KPADStatus* status) {
        static_assert(sizeof(KPADStatus) == cStatusSize, "Wii pad sample width");
        u8 wire[cStatusSize];
        std::memcpy(wire, status, sizeof(wire));
        readRecordedScalar(status->hold, wire);
        readRecordedScalar(status->trig, wire + 4);
        readRecordedScalar(status->release, wire + 8);
        readRecordedVec(status->acc, wire + 0x0C);
        readRecordedScalar(status->acc_value, wire + 0x18);
        readRecordedScalar(status->acc_speed, wire + 0x1C);
        readRecordedVec2(status->pos, wire + 0x20);
        readRecordedVec2(status->vec, wire + 0x28);
        readRecordedScalar(status->speed, wire + 0x30);
        readRecordedVec2(status->horizon, wire + 0x34);
        readRecordedVec2(status->hori_vec, wire + 0x3C);
        readRecordedScalar(status->hori_speed, wire + 0x44);
        readRecordedScalar(status->dist, wire + 0x48);
        readRecordedScalar(status->dist_vec, wire + 0x4C);
        readRecordedScalar(status->dist_speed, wire + 0x50);
        readRecordedVec2(status->acc_vertical, wire + 0x54);
        // The four byte-sized status fields at 0x5C remain unchanged.
        const u8 device = status->dev_type;
        if (device == WPAD_DEV_FREESTYLE || device == WPAD_DEV_MPLS_FREESTYLE) {
            readRecordedVec2(status->ex_status.fs.stick, wire + 0x60);
            readRecordedVec(status->ex_status.fs.acc, wire + 0x68);
            readRecordedScalar(status->ex_status.fs.acc_value, wire + 0x74);
            readRecordedScalar(status->ex_status.fs.acc_speed, wire + 0x78);
        } else if (device == WPAD_DEV_CLASSIC || device == WPAD_DEV_MPLS_CLASSIC ||
                   (device >= 16 && device <= 19)) {
            readRecordedScalar(status->ex_status.cl.hold, wire + 0x60);
            readRecordedScalar(status->ex_status.cl.trig, wire + 0x64);
            readRecordedScalar(status->ex_status.cl.release, wire + 0x68);
            readRecordedVec2(status->ex_status.cl.lstick, wire + 0x6C);
            readRecordedVec2(status->ex_status.cl.rstick, wire + 0x74);
            readRecordedScalar(status->ex_status.cl.ltrigger, wire + 0x7C);
            readRecordedScalar(status->ex_status.cl.rtrigger, wire + 0x80);
        } else if (device == 3) { // Balance board, as in read_kpad_ext.
            readRecordedScalar(status->ex_status.bl.tgc_weight, wire + 0x60);
            for (u32 i = 0; i < 4; ++i) {
                readRecordedScalar(status->ex_status.bl.weight[i], wire + 0x68 + i * 8);
                readRecordedScalar(status->ex_status.bl.weight_ave[i], wire + 0x88 + i * 8);
            }
            readRecordedScalar(status->ex_status.bl.weight_err, wire + 0xA8);
            readRecordedScalar(status->ex_status.bl.tgc_weight_err, wire + 0xAC);
        }
        // Inactive extension bytes stay exactly as reconstructed from the file.
        readRecordedVec(status->mpls.angularVelocity, wire + 0xB0);
        readRecordedVec(status->mpls.angle, wire + 0xBC);
        readRecordedVec(status->mpls.direction.X, wire + 0xC8);
        readRecordedVec(status->mpls.direction.Y, wire + 0xD4);
        readRecordedVec(status->mpls.direction.Z, wire + 0xE0);
        readRecordedScalar(status->unk_EC, wire + 0xEC);
    }
#endif


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
#ifdef GALAXY_NATIVE
    parsePacketHeader(&packetNo, &size, &statusNum, PetariNative::readU32BE(mPacketBuffer));
#else
    parsePacketHeader(&packetNo, &size, &statusNum, *reinterpret_cast< u32* >(mPacketBuffer));
#endif

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

#ifdef GALAXY_NATIVE
    decodeRecordedStatus(pOut);
#endif
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
#ifdef GALAXY_NATIVE
    static_assert(sizeof(PadRecordHeader) == 0x40, "Wii pad header width");
    std::memcpy(mHeader, buffer, sizeof(PadRecordHeader));
    readRecordedScalar(mHeader->mHeaderSize, buffer);
    readRecordedScalar(mHeader->_4, buffer + 4);
    readRecordedScalar(mHeader->_8, buffer + 8);
    readRecordedScalar(mHeader->_10, buffer + 0x10);
    readRecordedScalar(mHeader->_14, buffer + 0x14);
#else
    *mHeader = *reinterpret_cast< PadRecordHeader* >(buffer);
#endif

    if (mHeader->mHeaderSize != sizeof(PadRecordHeader)) {
        return false;
    }

    return true;
}

void PadRecorder::readUserFile() {
    DCInvalidateRange(mPacketBuffer, 0x20);
    mReader->read(mPacketBuffer, 4);
#ifdef GALAXY_NATIVE
    s32 size;
    readRecordedScalar(size, mPacketBuffer);
#else
    s32 size = *reinterpret_cast< s32* >(mPacketBuffer);
#endif

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
