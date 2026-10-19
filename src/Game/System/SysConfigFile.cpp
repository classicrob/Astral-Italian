#include "Game/System/SysConfigFile.hpp"
#include "Game/System/BinaryDataContentAccessor.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MathUtil.hpp"

SysConfigChunk::SysConfigChunk() : mBackupCounters(nullptr), mCurrentCounters(nullptr), mHeaderSerializer(nullptr) {
    initializeHeaderSerializer();
    mBackupCounters = new SysConfigCounters;
    mCurrentCounters = new SysConfigCounters;
    initializeData();
}

s32 SysConfigChunk::getSharedPlayerLeft() const { return MR::clamp(static_cast<s32>(mCurrentCounters->mSharedPlayerLeft), 0L, 99L); }
void SysConfigChunk::setSharedPlayerLeft(s32 value, u16 hash) {
    mCurrentCounters->mSharedPlayerLeft = MR::clamp(value, 0L, 99L);
    mCurrentCounters->mPlayerLeftHash = hash;
}
s32 SysConfigChunk::getBankedStarPieceNum() const { return MR::clamp(static_cast<s32>(mCurrentCounters->mBankedStarPiece), 0L, 9999L); }
void SysConfigChunk::setBankedStarPieceNum(s32 value) {
    mCurrentCounters->mBankedStarPiece = MR::clamp(value, 0L, 9999L);
    if (mCurrentCounters->mMaxBankedStarPiece < mCurrentCounters->mBankedStarPiece)
        mCurrentCounters->mMaxBankedStarPiece = mCurrentCounters->mBankedStarPiece;
}
s32 SysConfigChunk::getMaxBankedStarPieceNum() const { return MR::clamp(static_cast<s32>(mCurrentCounters->mMaxBankedStarPiece), 0L, 9999L); }
void SysConfigChunk::backupCounters() { mBackupCounters->copyFrom(*mCurrentCounters); }
void SysConfigChunk::restoreCounters() { mCurrentCounters->copyFrom(*mBackupCounters); }
void SysConfigChunk::resetCounters() { mBackupCounters->reset(); mCurrentCounters->reset(); }

u32 SysConfigChunk::makeHeaderHashCode() const {
    return 0x3;
}

u32 SysConfigChunk::getSignature() const {
    return 'SYSC';
}

// NONMATCHING: regalloc and argument scheduling (FP order/grouping checked: no FP instructions).
s32 SysConfigChunk::deserialize(const u8* buffer, u32 size) {
    BinaryDataContentAccessor accessor(const_cast<u8*>(buffer));
    u32 headerSize = accessor.getHeaderSize();
    u8* data = const_cast<u8*>(buffer) + headerSize;
    {
        u8* value = static_cast<u8*>(accessor.getPointer("mIsEncouragePal60", data));
        mEncouragedPal16 = value ? *value : 0;
    }
    {
        OSTime* value = static_cast<OSTime*>(accessor.getPointer("mTimeSent", data));
        mTimeSent = value ? *value : 0;
    }
    {
        u32* value = static_cast<u32*>(accessor.getPointer("mSentBytes", data));
        mSentBytes = value ? *value : 0;
    }
    {
        u16* value = static_cast<u16*>(accessor.getPointer("mBankStarPieceNum", data));
        mBackupCounters->mBankedStarPiece = value ? *value : 0;
    }
    {
        u16* value = static_cast<u16*>(accessor.getPointer("mBankStarPieceMax", data));
        mBackupCounters->mMaxBankedStarPiece = value ? *value : 0;
    }
    {
        u8* value = static_cast<u8*>(accessor.getPointer("mGiftedPlayerLeft", data));
        mBackupCounters->mSharedPlayerLeft = value ? *value : 0;
    }
    {
        u16* value = static_cast<u16*>(accessor.getPointer("mGiftedFileNameHash", data));
        mBackupCounters->mPlayerLeftHash = value ? *value : 1;
    }
    mCurrentCounters->copyFrom(*mBackupCounters);
    u32 newSize = headerSize + mHeaderSerializer->getDataSize();
    s32 result = 3;
    if (newSize <= size) result = 0;
    return result;
}

void SysConfigChunk::initializeData() {
    mEncouragedPal16 = false;
    mTimeSent = 0;
    mSentBytes = 0;
    mBackupCounters->reset();
    mCurrentCounters->reset();
}

void SysConfigChunk::initializeHeaderSerializer() {
    mHeaderSerializer = new BinaryDataContentHeaderSerializer(new u8[64], 64);
    mHeaderSerializer->addAttribute("mIsEncouragePal60", sizeof(u8));
    mHeaderSerializer->addAttribute("mTimeSent", sizeof(OSTime));
    mHeaderSerializer->addAttribute("mSentBytes", sizeof(u32));
    mHeaderSerializer->addAttribute("mBankStarPieceNum", sizeof(u16));
    mHeaderSerializer->addAttribute("mBankStarPieceMax", sizeof(u16));
    mHeaderSerializer->addAttribute("mGiftedPlayerLeft", sizeof(u8));
    mHeaderSerializer->addAttribute("mGiftedFileNameHash", sizeof(u16));
    mHeaderSerializer->flush();
}

SysConfigFile::SysConfigFile() : mChunk(nullptr), mChunkHolder(nullptr) {
    mChunkHolder = new BinaryDataChunkHolder(0x3000, 8);
    mChunk = new SysConfigChunk();

    mChunkHolder->addChunk(mChunk);
}

OSTime SysConfigFile::getTimeAnnounced() {
    return mChunk->mTimeSent;
}

void SysConfigFile::updateTimeAnnounced() {
    SysConfigChunk* pChunk = mChunk;

    pChunk->mTimeSent = OSGetTime();
}

OSTime SysConfigFile::getTimeSent() {
    return mChunk->mTimeSent;
}

void SysConfigFile::setTimeSent(OSTime timeSent) {
    mChunk->mTimeSent = timeSent;
}

u32 SysConfigFile::getSentBytes() {
    return mChunk->mSentBytes;
}

void SysConfigFile::setSentBytes(u32 sentBytes) {
    mChunk->mSentBytes = sentBytes;
}

void SysConfigFile::makeDataBinary(u8* pBuffer, u32 size) const {
    mChunkHolder->makeFileBinary(pBuffer, size);
}

void SysConfigFile::loadFromDataBinary(const u8* pBuffer, u32 size) {
    mChunkHolder->loadFromFileBinary(pBuffer, size);
}

// NONMATCHING: regalloc and argument scheduling (FP order/grouping checked: no FP instructions).
s32 SysConfigChunk::serialize(u8* buffer, u32) const {
    void* header = mHeaderSerializer->mStream.mBuffer;
    MR::copyMemory(buffer, header, mHeaderSerializer->getHeaderSize());
    u32 headerSize = mHeaderSerializer->getHeaderSize();
    BinaryDataContentAccessor accessor(static_cast<u8*>(mHeaderSerializer->mStream.mBuffer));
    u8* data = buffer + headerSize;
    {
        u8 value = mEncouragedPal16;
        *static_cast<u8*>(accessor.getPointer("mIsEncouragePal60", data)) = value;
    }
    {
        OSTime value = mTimeSent;
        *static_cast<OSTime*>(accessor.getPointer("mTimeSent", data)) = value;
    }
    {
        u32 value = mSentBytes;
        *static_cast<u32*>(accessor.getPointer("mSentBytes", data)) = value;
    }
    {
        u16 value = mBackupCounters->mBankedStarPiece;
        *static_cast<u16*>(accessor.getPointer("mBankStarPieceNum", data)) = value;
    }
    {
        u16 value = mBackupCounters->mMaxBankedStarPiece;
        *static_cast<u16*>(accessor.getPointer("mBankStarPieceMax", data)) = value;
    }
    {
        u8 value = mBackupCounters->mSharedPlayerLeft;
        *static_cast<u8*>(accessor.getPointer("mGiftedPlayerLeft", data)) = value;
    }
    {
        u16 value = mBackupCounters->mPlayerLeftHash;
        *static_cast<u16*>(accessor.getPointer("mGiftedFileNameHash", data)) = value;
    }
    return headerSize + mHeaderSerializer->getDataSize();
}
