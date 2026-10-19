#pragma once

#include "Game/System/BinaryDataChunkHolder.hpp"

struct GameEventValue {
    /* 0x00 */ const char* mName;
    /* 0x04 */ u16 mDefaultValue;
};

class GameEventValueChecker : public BinaryDataChunkBase {
public:
    GameEventValueChecker();

    virtual u32 makeHeaderHashCode() const;
    virtual u32 getSignature() const;
    virtual s32 serialize(u8*, u32) const;
    virtual s32 deserialize(const u8*, u32);
    virtual void initializeData();

    u16 getValue(const char*) const;
    void setValue(const char*, u16);

    /// @brief Reads a 32-bit value stored as the "<name>/hi" and "<name>/lo" 16-bit entries.
    u32 getU32Value(const char*) const;
    /// @brief Writes a 32-bit value as the "<name>/hi" and "<name>/lo" 16-bit entries.
    void setU32Value(const char*, u32);
    /// @brief Reads the 16-bit entry "<name>/<index>".
    u16 getIndexedValue(const char*, s32) const;
    /// @brief Writes the 16-bit entry "<name>/<index>".
    void setIndexedValue(const char*, s32, u16);
    /// @brief Tests bit (bit % 16) of the 16-bit entry "<name>/<bit / 16>".
    bool isOnIndexedBit(const char*, s32) const;
    /// @brief Sets bit (bit % 16) of the 16-bit entry "<name>/<bit / 16>".
    void onIndexedBit(const char*, s32);

    s32 findIndex(const char*) const;
    s32 findIndexFromHashCode(u16) const;

    /* 0x04 */ u16* mValues;
    /* 0x08 */ s32 mNumValues;
};
