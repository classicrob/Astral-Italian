#include "Game/Util/PadRecorder.hpp"

PadDataReader::PadDataReader() {
    mData = nullptr;
    mSize = 0;
    mPos = nullptr;
}

void PadDataReader::setData(const char* pData, u32 size) {
    mData = reinterpret_cast< const u8* >(pData);
    mSize = size;
    mPos = reinterpret_cast< const u8* >(pData);
}

bool PadDataReader::open() {
    return true;
}

u32 PadDataReader::read(void* pDst, u32 size) {
    u8* pOut = static_cast< u8* >(pDst);
    u32 n = 0;

    while (mPos < &mData[mSize] && n < size) {
        n++;
        *pOut++ = *mPos;
        mPos++;
    }

    return n;
}

bool PadDataReader::close() {
    return true;
}
