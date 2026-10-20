#ifdef GALAXY_NATIVE
#include <petari/save_wire.hpp>
#endif
#include "Game/System/BinaryDataContentAccessor.hpp"
#include "Game/Util/HashUtil.hpp"
#include <JSystem/JSupport/JSUInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

BinaryDataContentHeaderSerializer::BinaryDataContentHeaderSerializer(u8* pData, u32 dataSize)
    : mStream(pData, dataSize), mAttributeNum(0), mDataSize(0) {
    u16 numAttributes = 0;
#ifdef GALAXY_NATIVE
    PetariNative::SaveWire::write<u16>(mStream, numAttributes);
#else
    mStream.write(&numAttributes, sizeof(u16));
#endif
    u16 streamDataSize = 0;
#ifdef GALAXY_NATIVE
    PetariNative::SaveWire::write<u16>(mStream, streamDataSize);
#else
    mStream.write(&streamDataSize, sizeof(u16));
#endif
}

void BinaryDataContentHeaderSerializer::addAttribute(const char* pName, u32 attributeSize) {
    u16 hash = MR::getHashCode(pName);
#ifdef GALAXY_NATIVE
    PetariNative::SaveWire::write<u16>(mStream, hash);
#else
    mStream.write(&hash, sizeof(u16));
#endif
    u16 offset = mDataSize;
#ifdef GALAXY_NATIVE
    PetariNative::SaveWire::write<u16>(mStream, offset);
#else
    mStream.write(&offset, sizeof(u16));
#endif
    mDataSize += attributeSize;
    mAttributeNum++;
}

void BinaryDataContentHeaderSerializer::flush() {
    s32 restorePos = mStream.getPosition();
    mStream.seek(0, SEEK_FROM_START);
    u16 attributeNum = mAttributeNum;
#ifdef GALAXY_NATIVE
    PetariNative::SaveWire::write<u16>(mStream, attributeNum);
#else
    mStream.write(&attributeNum, sizeof(u16));
#endif
    u16 dataSize = mDataSize;
#ifdef GALAXY_NATIVE
    PetariNative::SaveWire::write<u16>(mStream, dataSize);
#else
    mStream.write(&dataSize, sizeof(u16));
#endif
    mStream.seek(restorePos, SEEK_FROM_START);
}

u32 BinaryDataContentHeaderSerializer::getHeaderSize() const {
    return mStream.getPosition();
}

u32 BinaryDataContentHeaderSerializer::getDataSize() const {
    return mDataSize;
}

BinaryDataContentAccessor::BinaryDataContentAccessor(u8* pData) : mData(pData) {
}

s32 BinaryDataContentAccessor::getHeaderSize() const {
#ifdef GALAXY_NATIVE
    return PetariNative::SaveWire::load<u16>(mData) * 4 + 4;
#else
    return *reinterpret_cast< u16* >(mData + 0) * 4 + 4;
#endif
}

s32 BinaryDataContentAccessor::getDataSize() const {
#ifdef GALAXY_NATIVE
    return PetariNative::SaveWire::load<u16>(mData + 2);
#else
    return *reinterpret_cast< u16* >(mData + 2);
#endif
}

// stripped
s32 BinaryDataContentAccessor::getAttributeNum() const {
#ifdef GALAXY_NATIVE
    return PetariNative::SaveWire::load<u16>(mData);
#else
    return *reinterpret_cast< u16* >(mData + 0);
#endif
}

void* BinaryDataContentAccessor::getPointer(const char* pAttributeName, u8* pData) const {
    JSUMemoryInputStream inStream = JSUMemoryInputStream(mData, getHeaderSize());
    inStream.skip(4);

    for (s32 idx = 0; idx < getAttributeNum(); idx++) {
#ifdef GALAXY_NATIVE
        u16 hash = PetariNative::SaveWire::read<u16>(inStream);
        u16 position = PetariNative::SaveWire::read<u16>(inStream);
#else
        u16 hash = inStream.readU16();
        u16 position = inStream.readU16();
#endif
        if (hash == static_cast< u16 >(MR::getHashCode(pAttributeName))) {
            return pData + position;
        }
    }

    return nullptr;
}
