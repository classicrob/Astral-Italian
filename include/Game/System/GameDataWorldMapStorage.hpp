#pragma once

#include "Game/System/BinaryDataChunkHolder.hpp"

// SMG2 world map save record ("SaveDataStorageWorldMap", 0x11 bytes).
class GameDataWorldMapStorage : public EmptyNonPolymorphicBase, public BinaryDataChunkBase {
public:
    GameDataWorldMapStorage();

    virtual u32 makeHeaderHashCode() const;
    virtual u32 getSignature() const;
    virtual s32 serialize(u8*, u32) const;
    virtual s32 deserialize(const u8*, u32);
    virtual void initializeData();

    void setCurrentWorldNo(s32);
    bool isOnPointFlag(s32, s32) const;      // SMG2 fn_804DE650 (provisional name)
    void setPointFlag(s32, s32, bool);       // SMG2 fn_804DE680 (provisional name)

    /* 0x08 */ u8 mPointFlag[8];             // point flag bytes, 1-based index (byte at index-1)
    /* 0x10 */ u8 mCurrentWorldNo;
};
