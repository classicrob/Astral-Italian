#pragma once

#include <JSystem/JKernel/JKRHeap.hpp>
#include <revolution/mem.h>

class JKRExpHeap;
class JKRSolidHeap;

namespace MR {
    class CurrentHeapRestorer {
    public:
        CurrentHeapRestorer(JKRHeap* pHeap);

        ~CurrentHeapRestorer();

    private:
        /* 0x00 */ JKRHeap* _0;
    };

    class NewDeleteAllocator {
    public:
        static void* alloc(MEMAllocator* pAllocator, u32 size);
        static void free(MEMAllocator* pAllocator, void* pPtr);

        static MEMAllocatorFunc sAllocatorFunc;
        static MEMAllocator sAllocator;
    };

    JKRHeap* getCurrentHeap();
    JKRExpHeap* getStationedHeapNapa();
    JKRExpHeap* getStationedHeapGDDR3();
    JKRHeap* getHeapNapa(const JKRHeap* pHeap);
    JKRHeap* getHeapGDDR3(const JKRHeap* pHeap);
    JKRHeap* becomeCurrentHeap(JKRHeap* pHeap) NO_INLINE;  // returns the previous current heap
    bool isEqualCurrentHeap(JKRHeap* pHeap);
    void adjustHeapSize(JKRExpHeap* pHeap);
    bool tryGetHeapSizeExceptEntry(u32*, const char*, const char*);
    bool existsWorldMapHeapGalaxy(const char*);
    bool tryGetHeapSizeExceptForGalaxy(u32*, const char*, const char*, const char*, s32);
    bool isUseWorldMapHeapForGalaxy(const char*, const char*, s32);
    // SMG2 fn_80040EA0 (address-named; symbol fn_80040EA0__2MRFUl; u32 >> 31, used by KCollisionServer::isBinaryInitialized)
    u32 fn_80040EA0(u32);

    template < int N >
    class JKRHeapAllocator;

    template <>
    class JKRHeapAllocator< 0 > {
    public:
        static void* alloc(MEMAllocator* pAllocator, u32 size);
        static void free(MEMAllocator* pAllocator, void* pPtr);

        static MEMAllocator sAllocator;
        static MEMAllocatorFunc sAllocatorFunc;
        static JKRHeap* sHeap;
    };

    /// @brief Copies `size` bytes from `pSrc` to `pDst`.
    /// @param pDst The pointer to the memory to copy to.
    /// @param pSrc The pointer to the memory to copy from.
    /// @param size The number of bytes to copy.
    void copyMemory(void* pDst, const void* pSrc, u32 size);

    /// @brief Fills `size` bytes at `pDst` with `0`.
    /// @param pDst The pointer to the memory to fill.
    /// @param size The number of bytes to fill.
    void zeroMemory(void* pDst, u32 size);
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
class JKRHeap;

namespace MR {
    MEMAllocator* getHomeButtonLayoutAllocator();
    f32 getHeapFreeRatio(JKRHeap* pHeap);
    JKRHeap* getAproposHeapForSceneArchive(f32 maxFreeSizeRate);
    JKRSolidHeap* getSceneHeapNapa();
    JKRSolidHeap* getSceneHeapGDDR3();
    JKRHeap* getWorldMapHeap();
    JKRHeap* getFileCacheHeap();
    JKRHeap* getUseResourceHeap();   // fn_80040DE0: HeapMemoryWatcher+0x18
    void fillMemory(void* pDst, u8 ch, u32 size);
    u32 calcCheckSum(const void* pPtr, u32 size);
    void* allocFromWPadHeap(u32 size);
    u8 freeFromWPadHeap(void* pPtr);
};  // namespace MR
