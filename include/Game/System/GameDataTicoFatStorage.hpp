#pragma once

#include "Game/System/BinaryDataChunkHolder.hpp"
#include "Game/Util/Array.hpp"

// SMG2 TicoFat save record ("SaveDataStorageTicoFat", signature "STF1").
// Holds the star piece alms values (8 worlds x 6 entries, u16) and the list of galaxy-name hashes
// that were fed a Tico coin.
class GameDataTicoFatStorage : public EmptyNonPolymorphicBase, public BinaryDataChunkBase {
public:
    GameDataTicoFatStorage();

    virtual u32 makeHeaderHashCode() const;
    virtual u32 getSignature() const;
    virtual s32 serialize(u8*, u32) const;
    virtual s32 deserialize(const u8*, u32);
    virtual void initializeData();

    s32 getValue(int) const;
    void addValue(int, int);
    void onTicoCoinFeed(const char*);
    bool isTicoCoinFeed(const char*) const;

    /* 0x08 */ u16* mStarPieceAlms;          // 0x60 bytes
    /* 0x0C */ MR::Vector< MR::AssignableArray< u16 > >* mTicoCoinFeedList;
};
