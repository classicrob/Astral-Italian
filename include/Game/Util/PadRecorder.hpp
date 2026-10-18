#pragma once

#include "Game/NameObj/NameObj.hpp"
#include <revolution/types.h>

struct KPADStatus;

/// Reads recorded pad data from memory (SMG2).
class PadDataReader {
public:
    PadDataReader();

    void setData(const char*, u32);
    virtual bool open();
    virtual u32 read(void*, u32);
    virtual bool close();

    /* 0x04 */ const u8* mData;
    /* 0x08 */ u32 mSize;
    /* 0x0C */ const u8* mPos;
};

/// Header of a pad recording (0x40 bytes).
struct PadRecordHeader {
    PadRecordHeader() {
        _4 = -1;
        _8 = -1;
    }

    /* 0x00 */ u32 mHeaderSize;
    /* 0x04 */ s32 _4;
    /* 0x08 */ s32 _8;
    /* 0x0C */ u8 _C;
    /* 0x0D */ u8 _D;
    /* 0x0E */ u8 _E;
    /* 0x0F */ u8 _F;
    /* 0x10 */ s32 _10;
    /* 0x14 */ s32 _14;
    /* 0x18 */ u8 _18[0x28];
};

/// Replays recorded pad input (SMG2: demo / "pad mode").
class WPadReadDataInfo;

class PadRecorder : public NameObj {
public:
    PadRecorder();

    void parsePacketHeader(u32*, u32*, s32*, u32);
    void prepareReplay(const char*, u32);
    bool startReplay();
    void stopReplay();
    s32 replayFrame(s32, KPADStatus*);
    const u8* decompress(KPADStatus*, const u8*, KPADStatus*);
    bool tryInitialize();
    bool readHeader();
    void readUserFile();

    /* 0x14 */ PadRecordHeader* mHeader;
    /* 0x18 */ PadDataReader* mDefaultReader;
    /* 0x1C */ PadDataReader* mReader;
    /* 0x20 */ u32 mReadPos;
    /* 0x24 */ s32 mPacketNo;
    /* 0x28 */ s32 mState;
    /* 0x2C */ u8* mPacketBufferAlloc;
    /* 0x30 */ u8* mPacketBuffer;
    /* 0x34 */ s32 mStatusNum;
    /* 0x38 */ u32 mDataSize;
    /* 0x3C */ u8* mData;
    /* 0x40 */ KPADStatus* mPrevStatus[2];
    /* 0x48 */ s16 _48;
    /* 0x4A */ s16 _4A;
};

namespace MR {
    void prepareReplay(const char*, u32);
    bool startReplayPad();
    void readPadData(int, WPadReadDataInfo*);
    bool isPadRecorderStopped();
    bool isPadRecorderReading();
    bool isRequestedStopReplayPad();
    void stopReplayPad();
}
