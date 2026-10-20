#ifdef GALAXY_NATIVE
#include <petari/save_wire.hpp>
#endif
#include "Game/System/GameDataPlayerStatus.hpp"
#include "Game/System/BinaryDataContentAccessor.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"

GameDataPlayerStatus::GameDataPlayerStatus() : mHeaderSerializer(), mFlag() {
    mHeaderSerializer = new BinaryDataContentHeaderSerializer(new u8[0x40], 0x40);
    mHeaderSerializer->addAttribute("mPlayerLeft", 1);
    mHeaderSerializer->addAttribute("mStockedStarPieceNum", 2);
    mHeaderSerializer->addAttribute("mStockedCoinNum", 2);
    mHeaderSerializer->addAttribute("mLast1upCoinNum", 2);
    mHeaderSerializer->addAttribute("mFlag", 1);
    mHeaderSerializer->flush();

    initializeData();
}

u32 GameDataPlayerStatus::makeHeaderHashCode() const {
    s32 dataSize = mHeaderSerializer->getDataSize();
    return dataSize + mHeaderSerializer->getHeaderSize();
}

u32 GameDataPlayerStatus::getSignature() const {
    return 'PLAY';
}

s32 GameDataPlayerStatus::serialize(u8* pBuffer, u32 size) const {
    MR::copyMemory(pBuffer, mHeaderSerializer->getBuffer(), mHeaderSerializer->getHeaderSize());

    s32 headerSize = mHeaderSerializer->getHeaderSize();
    BinaryDataContentAccessor accessor(static_cast< u8* >(mHeaderSerializer->getBuffer()));
    u8* pData = pBuffer + headerSize;

    u8 playerLeft = mPlayerLeft;
    *static_cast< u8* >(accessor.getPointer("mPlayerLeft", pData)) = playerLeft;

    u16 starPiece = mStockedStarPiece;
#ifdef GALAXY_NATIVE
    PetariNative::SaveWire::store<u16>(accessor.getPointer("mStockedStarPieceNum", pData), starPiece);
#else
    *static_cast< u16* >(accessor.getPointer("mStockedStarPieceNum", pData)) = starPiece;
#endif

    u16 coin = mStockedCoin;
#ifdef GALAXY_NATIVE
    PetariNative::SaveWire::store<u16>(accessor.getPointer("mStockedCoinNum", pData), coin);
#else
    *static_cast< u16* >(accessor.getPointer("mStockedCoinNum", pData)) = coin;
#endif

    u16 last1up = mLast1upCoinNum;
#ifdef GALAXY_NATIVE
    PetariNative::SaveWire::store<u16>(accessor.getPointer("mLast1upCoinNum", pData), last1up);
#else
    *static_cast< u16* >(accessor.getPointer("mLast1upCoinNum", pData)) = last1up;
#endif

    u8 flag = mFlag;
    *static_cast< u8* >(accessor.getPointer("mFlag", pData)) = flag;

    return headerSize + mHeaderSerializer->getDataSize();
}

s32 GameDataPlayerStatus::deserialize(const u8* pBuffer, u32 size) {
    BinaryDataContentAccessor accessor(const_cast< u8* >(pBuffer));
    s32 headerSize = accessor.getHeaderSize();

    u8* pData = const_cast< u8* >(pBuffer) + headerSize;

    void* pPlayerLeft = accessor.getPointer("mPlayerLeft", pData);
    mPlayerLeft = pPlayerLeft != nullptr ? *static_cast< u8* >(pPlayerLeft) : 0;

    void* pStarPiece = accessor.getPointer("mStockedStarPieceNum", pData);
#ifdef GALAXY_NATIVE
    mStockedStarPiece = pStarPiece != nullptr ? PetariNative::SaveWire::load<u16>(pStarPiece) : 0;
#else
    mStockedStarPiece = pStarPiece != nullptr ? *static_cast< u16* >(pStarPiece) : 0;
#endif

    void* pCoin = accessor.getPointer("mStockedCoinNum", pData);
#ifdef GALAXY_NATIVE
    mStockedCoin = pCoin != nullptr ? PetariNative::SaveWire::load<u16>(pCoin) : 0;
#else
    mStockedCoin = pCoin != nullptr ? *static_cast< u16* >(pCoin) : 0;
#endif

    void* pLast1up = accessor.getPointer("mLast1upCoinNum", pData);
#ifdef GALAXY_NATIVE
    mLast1upCoinNum = pLast1up != nullptr ? PetariNative::SaveWire::load<u16>(pLast1up) : 0;
#else
    mLast1upCoinNum = pLast1up != nullptr ? *static_cast< u16* >(pLast1up) : 0;
#endif

    void* pFlag = accessor.getPointer("mFlag", pData);
    mFlag = pFlag != nullptr ? *static_cast< u8* >(pFlag) : 0;

    if (headerSize + mHeaderSerializer->getDataSize() > size) {
        return 3;
    }

    return 0;
}

void GameDataPlayerStatus::initializeData() {
    mPlayerLeft = 4;
    mStockedStarPiece = 0;
    mStockedCoin = 0;
    mLast1upCoinNum = 0;
    mFlag = 0;
    mUnknown14 = 0;
}

s32 GameDataPlayerStatus::getPlayerLeft() const {
    return MR::clamp(static_cast< s32 >(mPlayerLeft), 0, 99);
}

void GameDataPlayerStatus::addPlayerLeft(int num) {
    s32 playerLeft = static_cast< s32 >(mPlayerLeft) + num;
    mPlayerLeft = MR::clamp(playerLeft, 0, 99);
}

s32 GameDataPlayerStatus::getStockedStarPieceNum() const {
    return MR::clamp(static_cast< s32 >(mStockedStarPiece), 0, 9999);
}

void GameDataPlayerStatus::addStockedStarPieceNum(int num) {
    s32 starPiece = static_cast< s32 >(mStockedStarPiece) + num;
    mStockedStarPiece = MR::clamp(starPiece, 0, 9999);
}

s32 GameDataPlayerStatus::getStockedCoinNum() const {
    return MR::clamp(static_cast< s32 >(mStockedCoin), 0, 9999);
}

s32 GameDataPlayerStatus::addStockedCoinNum(int num) {
    s32 coin = static_cast< s32 >(mStockedCoin) + num;
    s32 newCoin = MR::clamp(coin, 0, 9999);
    s32 nextThreshold = MR::clamp(static_cast< s32 >(mLast1upCoinNum), 0, 9999);
    s32 oldPlayerLeft = MR::clamp(static_cast< s32 >(mPlayerLeft), 0, 99);

    mStockedCoin = newCoin;
    nextThreshold += 100;

    while (nextThreshold <= newCoin) {
        mLast1upCoinNum = static_cast< u16 >(mLast1upCoinNum + 100);
        addPlayerLeft(1);
        nextThreshold += 100;
    }

    return MR::clamp(static_cast< s32 >(mPlayerLeft), 0, 99) - oldPlayerLeft;
}

void GameDataPlayerStatus::resetPlayerLeft() {
    mUnknown14 = mPlayerLeft;
    mPlayerLeft = 4;
}
