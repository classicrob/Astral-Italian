#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util.hpp"
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/Util/MutexHolder.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <JSystem/JAudio2/JASMutex.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <mem.h>
#include <cstdio>

namespace MR {
    CurrentHeapRestorer::CurrentHeapRestorer(JKRHeap* pHeap) {
        _0 = JKRHeap::sCurrentHeap;
        OSLockMutex(&MR::MutexHolder< 1 >::sMutex);
        MR::becomeCurrentHeap(pHeap);
    }

    CurrentHeapRestorer::~CurrentHeapRestorer() {
        MR::becomeCurrentHeap(_0);
        OSUnlockMutex(&MR::MutexHolder< 1 >::sMutex);
    }



    void* NewDeleteAllocator::alloc(MEMAllocator* pAllocator, u32 size) {
        return new u8[size];
    }

    void NewDeleteAllocator::free(MEMAllocator* pAllocator, void* pPtr) {
        delete static_cast< u8* >(pPtr);
    }

    MEMAllocatorFunc NewDeleteAllocator::sAllocatorFunc = {
        NewDeleteAllocator::alloc,
        NewDeleteAllocator::free,
    };
    MEMAllocator NewDeleteAllocator::sAllocator = {&sAllocatorFunc};


    MEMAllocator* getHomeButtonLayoutAllocator() {
        JKRHeapAllocator< 0 >::sHeap = SingletonHolder< HeapMemoryWatcher >::get()->mHomeButtonLayoutHeap;

        return &JKRHeapAllocator< 0 >::sAllocator;
    }

    JKRHeap* getCurrentHeap() {
        return JKRHeap::sCurrentHeap;
    }

    inline f32 getHeapFreeRatio(JKRHeap* pHeap) {
        u32 size = pHeap->mSize;
        return static_cast< f32 >(pHeap->getTotalFreeSize()) / size;
    }


    JKRExpHeap* getStationedHeapNapa() {
        return SingletonHolder< HeapMemoryWatcher >::get()->mStationedHeapNapa;
    }

    JKRExpHeap* getStationedHeapGDDR3() {
        return SingletonHolder< HeapMemoryWatcher >::get()->mStationedHeapGDDR;
    }

    JKRSolidHeap* getSceneHeapNapa() {
        return SingletonHolder< HeapMemoryWatcher >::get()->mSceneHeapNapa;
    }

    JKRSolidHeap* getSceneHeapGDDR3() {
        return SingletonHolder< HeapMemoryWatcher >::get()->mSceneHeapGDDR;
    }

    JKRHeap* getWorldMapHeap() {
        return SingletonHolder< HeapMemoryWatcher >::get()->mWorldMapHeap;
    }

    JKRHeap* getFileCacheHeap() {
        return SingletonHolder< HeapMemoryWatcher >::get()->mFileCacheHeap;
    }

    JKRHeap* getUseResourceHeap() {
        return SingletonHolder< HeapMemoryWatcher >::get()->mFileCacheDelegateHeap;
    }

    JKRHeap* getHeapNapa(const JKRHeap* pHeap) {
        return SingletonHolder< HeapMemoryWatcher >::get()->getHeapNapa(pHeap);
    }

    JKRHeap* getHeapGDDR3(const JKRHeap* pHeap) {
        return SingletonHolder< HeapMemoryWatcher >::get()->getHeapGDDR3(pHeap);
    }

    JKRHeap* becomeCurrentHeap(JKRHeap* pHeap) {
        JASMutexLock lock(&MR::MutexHolder< 1 >::sMutex);
        return pHeap->becomeCurrentHeap();
    }

    bool isEqualCurrentHeap(JKRHeap* pHeap) {
        return JKRHeap::sCurrentHeap == pHeap;
    }

    void adjustHeapSize(JKRExpHeap* pHeap) {
        pHeap->adjustSize();
    }

    u32 fn_80040EA0(u32 value) {
        return value >> 31;
    }

    u32 getFileCacheSizeForGalaxy(const char* pScene, const char* pStage, s32 scenario) {
        u32 size = 0xB80000;
        tryGetHeapSizeExceptForGalaxy(&size, "FileCacheSize", pScene, pStage, scenario);
        if (isUseWorldMapHeapForGalaxy(pScene, pStage, scenario)) {
            size -= 0x840000;
        }
        return size;
    }

    bool tryGetHeapSizeExceptForGalaxy(u32* pSize, const char* pHeapName, const char* pScene, const char* pStage,
                                       s32 scenario) {
        if (!isEqualString(pScene, "Game")) {
            return false;
        }

        if (scenario > 0) {
            char buf[0x40];
            snprintf(buf, 0x40, "%s%d", pStage, scenario);
            if (tryGetHeapSizeExceptEntry(pSize, pHeapName, buf)) {
                return true;
            }
        }

        return tryGetHeapSizeExceptEntry(pSize, pHeapName, pStage);
    }

    bool isUseWorldMapHeapForGalaxy(const char* pScene, const char* pStage, s32 scenario) {
        if (!isEqualString(pScene, "Game")) {
            return false;
        }

        if (scenario > 0) {
            char buf[0x40];
            snprintf(buf, 0x40, "%s%d", pStage, scenario);
            if (existsWorldMapHeapGalaxy(buf)) {
                return true;
            }
        }

        return existsWorldMapHeapGalaxy(pStage);
    }

    void copyMemory(void* pDst, const void* pSrc, u32 size) {
        u8* pDstBytes = static_cast< u8* >(pDst);
        const u8* pSrcBytes = static_cast< const u8* >(pSrc);
        u32 srcAlignment = reinterpret_cast< u32 >(pSrc) & 3;
        u32 dstAlignment = reinterpret_cast< u32 >(pDst) & 3;

        if (srcAlignment == dstAlignment && (size & 0xF) == 0) {
            const u32* pSrcWords = reinterpret_cast< const u32* >(pSrcBytes);
            u32* pDstWords = reinterpret_cast< u32* >(pDstBytes);

            for (u32 count = size / 16; count != 0; count--) {
                u32 word3, word2, word1, word0;
                word0 = *pSrcWords++;
                word1 = *pSrcWords++;
                word2 = *pSrcWords++;
                word3 = *pSrcWords++;
                *pDstWords++ = word0;
                *pDstWords++ = word1;
                *pDstWords++ = word2;
                *pDstWords++ = word3;
            }
        } else if (srcAlignment == dstAlignment && size >= 16) {
            if (srcAlignment != 0) {
                for (u8 count = 4 - srcAlignment; count != 0; count--) {
                    *pDstBytes++ = *pSrcBytes++;
                    size--;
                }
            }

            while (size >= 4) {
                *reinterpret_cast< u32* >(pDstBytes) = *reinterpret_cast< const u32* >(pSrcBytes);
                size -= 4;
                pSrcBytes += 4;
                pDstBytes += 4;
            }

            if (size != 0) {
                for (; size != 0; size--) {
                    *pDstBytes++ = *pSrcBytes++;
                }
            }
        } else {
            for (; size != 0; size--) {
                *pDstBytes++ = *pSrcBytes++;
            }
        }
    }

    void fillMemory(void* pDst, u8 ch, u32 size) {
        if (ch == 0) {
            zeroMemory(pDst, size);
        } else {
            memset(pDst, ch, size);
        }
    }

    void zeroMemory(void* pDst, u32 size) {
        u8* pDstBytes = static_cast< u8* >(pDst);
        u32 alignment = reinterpret_cast< u32 >(pDst) & 3;

        if ((reinterpret_cast< u32 >(pDst) & 0x1F) == 0 && (size & 0x1F) == 0) {
            DCZeroRange(pDst, size);
        } else if (alignment == 0 && (size & 0xF) == 0) {
            for (u32 count = size / 16; count != 0; count--) {
                u32* pWords = reinterpret_cast< u32* >(pDstBytes);
                pWords[0] = 0;
                pWords[1] = 0;
                pWords[2] = 0;
                pWords[3] = 0;
                pDstBytes += 16;
            }
        } else if (size >= 16) {
            if (alignment != 0) {
                for (u8 count = 4 - alignment; count != 0; count--) {
                    *pDstBytes++ = 0;
                    size--;
                }
            }

            while (size >= 4) {
                *reinterpret_cast< u32* >(pDstBytes) = 0;
                pDstBytes += 4;
                size -= 4;
            }

            if (size != 0) {
                for (; size != 0; size--) {
                    *pDstBytes++ = 0;
                }
            }
        } else {
            for (; size != 0; size--) {
                *pDstBytes++ = 0;
            }
        }
    }

    u32 calcCheckSum(const void* pPtr, u32 size) {
        u16 sum;
        u32 invSum;

        invSum = 0;
        sum = 0;

        const u16* p = static_cast< const u16* >(pPtr);
        u32 checkSize = size / sizeof(u16);

        for (int i = 0; i < checkSize; i++, p++) {
            sum += *p;
            invSum += static_cast< u16 >(~(*p));
        }

        return (sum << 16) | static_cast< u16 >(invSum);
    }

    void* allocFromWPadHeap(u32 size) {
        return SingletonHolder< HeapMemoryWatcher >::get()->mWPadHeap->alloc(size, 0);
    }

    u8 freeFromWPadHeap(void* pPtr) {
        SingletonHolder< HeapMemoryWatcher >::get()->mWPadHeap->free(pPtr);

        return 1;
    }

    void* JKRHeapAllocator< 0 >::alloc(MEMAllocator* pAllocator, u32 size) {
        return sHeap->alloc(size, 0);
    }

    void JKRHeapAllocator< 0 >::free(MEMAllocator* pAllocator, void* pPtr) {
        sHeap->free(pPtr);
    }

    MEMAllocatorFunc JKRHeapAllocator< 0 >::sAllocatorFunc = {
        JKRHeapAllocator< 0 >::alloc,
        JKRHeapAllocator< 0 >::free,
    };

    MEMAllocator JKRHeapAllocator< 0 >::sAllocator = {&sAllocatorFunc, nullptr, 4, 0};

    JKRHeap* JKRHeapAllocator< 0 >::sHeap;
};  // namespace MR
