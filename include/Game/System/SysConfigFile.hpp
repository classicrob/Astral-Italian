#pragma once

#include "Game/System/BinaryDataChunkHolder.hpp"
#include <revolution/os.h>

class BinaryDataChunkHolder;
class BinaryDataContentHeaderSerializer;

struct SysConfigCounters {
    u16 mBankedStarPiece;
    u16 mMaxBankedStarPiece;
    u8 mSharedPlayerLeft;
    u16 mPlayerLeftHash;
};

class SysConfigChunk : public BinaryDataChunkBase {
    friend class SysConfigFile;

public:
    /// @brief Creates a new `SysConfigChunk`.
    SysConfigChunk();

    virtual u32 makeHeaderHashCode() const;
    virtual u32 getSignature() const;
    virtual s32 serialize(u8*, u32) const;
    virtual s32 deserialize(const u8*, u32);
    virtual void initializeData();

    void initHeaderSerializer();

    s32 getSharedPlayerLeft() const;
    void setSharedPlayerLeft(s32, u16);
    s32 getBankedStarPieceNum() const;
    void setBankedStarPieceNum(s32);
    s32 getMaxBankedStarPieceNum() const;
    void backupCounters();
    void restoreCounters();
    void resetCounters();

    /* 0x04 */ bool mEncouragedPal16;
    union {
        /* 0x08 */ OSTime mTimeSent;
        /* 0x08 */ OSTime mTimeAnnounced; // old source alias; SMG2 stores only one time
    };
    /* 0x10 */ u32 mSentBytes;
    /* 0x14 */ SysConfigCounters* mBackupCounters;
    /* 0x18 */ SysConfigCounters* mCurrentCounters;
    /* 0x1C */ BinaryDataContentHeaderSerializer* mHeaderSerializer;
};

class SysConfigFile {
public:
    /// @brief Creates a new `SysConfigFile`.
    SysConfigFile();

    OSTime getTimeAnnounced();
    void updateTimeAnnounced();
    OSTime getTimeSent();
    void setTimeSent(OSTime);
    u32 getSentBytes();
    void setSentBytes(u32);
    void makeDataBinary(u8*, u32) const;
    void loadFromDataBinary(const u8*, u32);

    // core h10: SMG2 GameDataFunction reads the chunk pointer directly (offset 0 of SysConfigFile)
    /* 0x00 */ SysConfigChunk* mChunk;

private:
    /* 0x04 */ BinaryDataChunkHolder* mChunkHolder;
};
