#pragma once

#include "Game/System/BinaryDataChunkHolder.hpp"
#include <revolution/os.h>

class BinaryDataChunkHolder;
class BinaryDataContentHeaderSerializer;

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

    // core h10: SMG2 flag byte at chunk+4 (read/written inline by GameDataFunction)
    /* 0x04 */ u8 mEncouragedPal16;

private:
    /* 0x08 */ OSTime mTimeAnnounced;
    /* 0x10 */ OSTime mTimeSent;
    /* 0x18 */ u32 mSentBytes;
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
