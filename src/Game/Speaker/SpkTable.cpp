#include "Game/Speaker/SpkTable.hpp"
#include <stdint.h>

SpkTable::SpkTable() {
    mInitialized = false;
    mResourceCount = 0;
    mParameters = nullptr;
    mNames = nullptr;
}

void SpkTable::setResource(void* pRes) {
    mInitialized = false;

    u8* resource = static_cast< u8* >(pRes);
    s32* cursor = static_cast< s32* >(pRes);

    s32 resourceCount = *cursor++;
    s32 entryOff = *cursor++;
    s32 dataOffsetsStartOff = *cursor++;
    s32* pIsDataOffsetsInitialized = cursor;
    BOOL isDataOffsetsInitialized = *cursor++;

    mResourceCount = resourceCount;

    SpkParameters* entryOffset = reinterpret_cast< SpkParameters* >(resource + entryOff);
    mParameters = entryOffset;
    const char** names = reinterpret_cast< const char** >(resource + dataOffsetsStartOff);
    if (!isDataOffsetsInitialized) {
        for (s32 i = 0; i < mResourceCount; i++) {
            names[i] += reinterpret_cast< uintptr_t >(resource);
        }
    }

    mNames = names;
    *pIsDataOffsetsInitialized = TRUE;
    mInitialized = true;
}
