#include "Game/Speaker/SpkWave.hpp"
#ifdef GALAXY_NATIVE
#include <petari/endian.hpp>
#endif
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include <revolution/os.h>
#include <stdint.h>

SpkWave::SpkWave() : mResource(nullptr) {
}

void SpkWave::setResource(void* pResource) {
    JASCriticalSection crit;
    mResource = pResource;
}

s32 SpkWave::getWaveSize(s32 wave) const {
    if (mResource == nullptr) {
        return 0;
    }

    return getWaveData(wave)->mSize;
}

u32 SpkWave::getLoopStartPos(s32 wave) const {
    if (mResource == nullptr) {
        return 0;
    }

    return getWaveData(wave)->mLoopStartPos;
}

u32 SpkWave::getLoopEndPos(s32 wave) const {
    if (mResource == nullptr) {
        return 0;
    }

    return getWaveData(wave)->mLoopEndPos;
}

s16* SpkWave::getWave(s32 wave) const {
    if (mResource == nullptr) {
        return nullptr;
    }

    return getWaveData(wave)->mWave;
}

WaveData* SpkWave::getWaveData(s32 wave) const {
#ifdef GALAXY_NATIVE
    u8* resource = static_cast<u8*>(mResource);
    return reinterpret_cast<WaveData*>(resource + PetariNative::readU32BE(resource + wave * 4 + 8));
#else
    return reinterpret_cast< WaveData* >(reinterpret_cast< uintptr_t >(mResource) +
                                         *reinterpret_cast< u32* >(reinterpret_cast< uintptr_t >(mResource) + wave * 4 + 8));
#endif
}
