#pragma once

#include "Game/System/BinaryDataChunkHolder.hpp"

class BinaryDataContentHeaderSerializer;

class GameDataPlayerStatus : public BinaryDataChunkBase {
public:
    /// @brief Creates a new `GameDataPlayerStatus`.
    GameDataPlayerStatus();

    /// @brief Returns the identifying hash code for this object.
    /// @return The identifying hash code.
    virtual u32 makeHeaderHashCode() const;

    /// @brief Returns the identifying magic number for this object.
    /// @return The identifying magic number.
    virtual u32 getSignature() const;

    /// @brief Writes the member variables of this object to the given data buffer.
    /// @param pBuffer The pointer to the data buffer.
    /// @param size The size of the data buffer, in bytes.
    /// @return The number of bytes written.
    virtual s32 serialize(u8* pBuffer, u32 size) const;

    /// @brief Reads the member variables of this object from the given data buffer.
    /// @param pBuffer The pointer to the data buffer.
    /// @param size The size of the data buffer, in bytes.
    /// @return `0` if successful, `3` if the buffer is too small.
    virtual s32 deserialize(const u8* pBuffer, u32 size);

    /// @brief Initializes the member variables of this object.
    virtual void initializeData();

    /// @brief Returns the number of extra lives.
    /// @return The number of extra lives, clamped to `0..99`.
    s32 getPlayerLeft() const;
    void resetPlayerLeft();

    /// @brief Adds to the number of extra lives by the given term.
    /// @param num The number of extra lives to add.
    void addPlayerLeft(int num);

    /// @brief Returns the number of universally collected Star Bits.
    /// @return The number of Star Bits, clamped to `0..9999`.
    s32 getStockedStarPieceNum() const;

    /// @brief Adds to the number of universally collected Star Bits by the given term.
    /// @param num The number of Star Bits to add.
    void addStockedStarPieceNum(int num);

    /// @brief Returns the number of universally collected coins.
    /// @return The number of coins, clamped to `0..9999`.
    s32 getStockedCoinNum() const;

    /// @brief Adds to the number of universally collected coins, awarding an extra life per 100 coins.
    /// @param num The number of coins to add.
    /// @return The change in the number of extra lives.
    s32 addStockedCoinNum(int num);

    /* 0x04 */ BinaryDataContentHeaderSerializer* mHeaderSerializer;

    /// @brief The number of extra lives.
    /* 0x08 */ u8 mPlayerLeft;

    /// @brief The number of universally collected Star Bits.
    /* 0x0A */ u16 mStockedStarPiece;

    /// @brief The number of universally collected coins.
    /* 0x0C */ u16 mStockedCoin;

    /// @brief The universally collected coin count at which the last extra life was awarded.
    /* 0x0E */ u16 mLast1upCoinNum;

    /* 0x10 */ u8 mFlag;

    /* 0x14 */ u32 mUnknown14;
};
