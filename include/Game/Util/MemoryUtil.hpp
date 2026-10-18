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
    void fillMemory(void* pDst, u8 ch, u32 size);
    u32 calcCheckSum(const void* pPtr, u32 size);
    void* allocFromWPadHeap(u32 size);
    u8 freeFromWPadHeap(void* pPtr);
};  // namespace MR
