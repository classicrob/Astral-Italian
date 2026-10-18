#pragma once

#include "Game/System/BinaryDataChunkHolder.hpp"

// SMG2 world map save record. Only the parts read by the GameDataFunction unit are named so far.
class GameDataWorldMapStorage : public BinaryDataChunkBase {
public:
    virtual u32 makeHeaderHashCode() const;
    virtual u32 getSignature() const;
    virtual s32 serialize(u8*, u32) const;
    virtual s32 deserialize(const u8*, u32);
    virtual void initializeData();

    void setCurrentWorldNo(u8);
    bool isOnPointFlag(s32, s32) const;      // SMG2 fn_804DE650 (provisional name)
    void setPointFlag(s32, s32, bool);       // SMG2 fn_804DE680 (provisional name)

    // PLACEHOLDER(core): 0x04..0x0F and the flag bytes at 0x07+ are not laid out yet; resolve when
    // Game/System/GameDataWorldMapStorage is ported (ctor at 0x804DE5E0).
    u8 _04[0xC];
    /* 0x10 */ u8 mCurrentWorldNo;
};
