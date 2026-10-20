#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/DelegatingHeap.hpp"
#include <JSystem/JKernel/JKRHeap.hpp>
#include <revolution/aralt.h>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <revolution/os.h>
#include <revolution/wpad.h>
#ifdef GALAXY_NATIVE
#include <cstdint>
#include "Game/System/NativeBootTrace.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <petari/host_image_heap.hpp>

namespace {
    // Heap for the host-layout copy of a big-endian J3D file (MacioGalaxy): the file cache's companion when the
    // file is inside the file cache, otherwise null (the current heap). The companion is a sibling of the file
    // cache, so findFromRoot resolves a source inside the file cache to the file cache itself.
    JKRHeap* resolveFileCacheHostImageHeap(const void* pSource) {
        HeapMemoryWatcher* pWatcher = SingletonHolder< HeapMemoryWatcher >::get();
        if (pWatcher == nullptr || pWatcher->mFileCacheHeap == nullptr || pWatcher->mFileCacheHostImageHeap == nullptr) {
            return nullptr;
        }
        if (JKRHeap::findFromRoot(const_cast< void* >(pSource)) != pWatcher->mFileCacheHeap) {
            return nullptr;
        }
        return pWatcher->mFileCacheHostImageHeap;
    }
}  // namespace
#endif

JKRExpHeap* HeapMemoryWatcher::sRootHeapGDDR3;

namespace {
    JKRExpHeap* createExpHeap(u32 size, JKRHeap* pHeap, bool a3) NO_INLINE {
        JKRExpHeap* heap;

        if (a3) {
            void* data = new (pHeap, -4) u8[size];
            heap = JKRExpHeap::create(data, size, pHeap, true);
        } else {
            heap = JKRExpHeap::create(size, pHeap, true);
        }

        if (MR::isEqualCurrentHeap(heap)) {
            JKRHeap::sRootHeap->becomeCurrentHeap();
        }

        return heap;
    }

    JKRSolidHeap* createSolidHeap(u32 size, JKRHeap* pHeap) {
        JKRSolidHeap* heap = JKRSolidHeap::create(size, pHeap, true);

        if (MR::isEqualCurrentHeap(heap)) {
            JKRHeap::sRootHeap->becomeCurrentHeap();
        }

        return heap;
    }

    void destroyHeapAndSetNULL(JKRHeap** pHeap) {
        if (*pHeap != nullptr) {
            JKRHeap::destroy(*pHeap);
            *pHeap = nullptr;
        }
    }
};  // namespace

JKRHeap* HeapMemoryWatcher::getHeapNapa(const JKRHeap* pHeap) {
    if (pHeap == mStationedHeapNapa || pHeap == mStationedHeapGDDR) {
        return mStationedHeapNapa;
    }

    if (pHeap == mSceneHeapNapa || pHeap == mSceneHeapGDDR) {
        return mSceneHeapNapa;
    }

    return nullptr;
}

JKRHeap* HeapMemoryWatcher::getHeapGDDR3(const JKRHeap* pHeap) {
    if (pHeap == mStationedHeapNapa || pHeap == mStationedHeapGDDR) {
        return mStationedHeapGDDR;
    }

    if (pHeap == mSceneHeapNapa || pHeap == mSceneHeapGDDR) {
        return mSceneHeapGDDR;
    }

    return nullptr;
}

void HeapMemoryWatcher::createWorldMapHeap() {
    mWorldMapHeap = ::createSolidHeap(0x840000, mGameHeapGDDR);
}

void HeapMemoryWatcher::createFileCacheHeapOnGameHeap(u32 size) {
#ifdef GALAXY_NATIVE
    // Native resource objects for cached archives are larger (64-bit pointers): extra room on the file cache.
    size += cFileCacheNativeAllowance;
    mFileCacheArchiveBytes = 0;
    mFileCacheArchiveCount = 0;
#endif
    mFileCacheHeap = ::createSolidHeap(size, mGameHeapGDDR);
    mFileCacheDelegateHeap = new (mFileCacheHeap, 0) DelegatingHeap(mFileCacheHeap);
#ifdef GALAXY_NATIVE
    // Each J3D file in a cached archive also gets a same-size host-layout copy (J3DModelLoader/J3DAnmLoader), on a
    // sibling heap of the same size (32-aligned copies of 32-multiple files fit in size + 32).
    mFileCacheHostImageHeap = ::createSolidHeap(size + 0x20, mGameHeapGDDR);
#endif
}

void HeapMemoryWatcher::createSceneHeapOnGameHeap() {
    mSceneHeapNapa = ::createSolidHeap(-1, mGameHeapNapa);
    mSceneHeapGDDR = ::createSolidHeap(-1, mGameHeapGDDR);
}
void HeapMemoryWatcher::adjustStationedHeapNapa() {
    MR::adjustHeapSize(mStationedHeapNapa);
}

void HeapMemoryWatcher::adjustStationedHeapGDDR3() {
    MR::adjustHeapSize(mStationedHeapGDDR);
}

void HeapMemoryWatcher::setCurrentHeapToStationedHeap() {
    MR::becomeCurrentHeap(mStationedHeapNapa);
}

void HeapMemoryWatcher::setCurrentHeapToGameHeap() {
    MR::becomeCurrentHeap(JKRHeap::sSystemHeap);
}

void HeapMemoryWatcher::setCurrentHeapToSceneHeap() {
    MR::becomeCurrentHeap(mSceneHeapNapa);
}

void HeapMemoryWatcher::resetGameHeapNapaAndGDDR3() {
    resetGameHeapNapa();
    resetGameHeapGDDR3();
}

void HeapMemoryWatcher::resetGameHeapNapa() {
    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mGameHeapNapa));
    mGameHeapNapa = ::createExpHeap(-1, JKRHeap::sRootHeap, false);
}

void HeapMemoryWatcher::resetGameHeapGDDR3() {
    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mGameHeapGDDR));
    mGameHeapGDDR = ::createExpHeap(-1, sRootHeapGDDR3, false);
}

void HeapMemoryWatcher::destroyWorldMapHeap() {
    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mWorldMapHeap));
}

void HeapMemoryWatcher::destroyFileCache() {
    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mFileCacheHeap));
#ifdef GALAXY_NATIVE
    // The file cache's J3D objects point into these copies; both go away together.
    if (mFileCacheHostImageHeap != nullptr) {
        ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mFileCacheHostImageHeap));
    }
#endif
}

void HeapMemoryWatcher::freeAllFileCache() {
    mFileCacheDelegateHeap->freeAll();
#ifdef GALAXY_NATIVE
    if (mFileCacheHostImageHeap != nullptr) {
        mFileCacheHostImageHeap->freeAll();
    }
#endif
}

void HeapMemoryWatcher::destroySceneHeap() {
    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mSceneHeapNapa));
    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mSceneHeapGDDR));
}

void HeapMemoryWatcher::createRootHeap() {
    JKRExpHeap* pHeap;
    void* newHi;
#ifdef GALAXY_NATIVE
    uintptr_t arenaHi, arenaLo;

    JKRExpHeap::createRoot(1, true);
    arenaLo = reinterpret_cast<uintptr_t>(OSGetMEM2ArenaLo());
    arenaHi = reinterpret_cast<uintptr_t>(OSGetMEM2ArenaHi());
    newHi = reinterpret_cast<void*>(arenaLo + ARGetBaseAddress() + 0xDB0000);
    OSSetMEM2ArenaHi(newHi);
    JKRHeap::setAltAramStartAdr(arenaLo);
    pHeap = JKRExpHeap::create(newHi, arenaHi - reinterpret_cast<uintptr_t>(newHi), JKRHeap::sRootHeap, true);
#else
    u32 arenaHi, arenaLo;

    JKRExpHeap::createRoot(1, true);
    arenaLo = reinterpret_cast< u32 >(OSGetMEM2ArenaLo());
    arenaHi = reinterpret_cast< u32 >(OSGetMEM2ArenaHi());
    newHi = reinterpret_cast< void* >(arenaLo + ARGetBaseAddress() + 0xDB0000);
    OSSetMEM2ArenaHi(newHi);
    JKRHeap::setAltAramStartAdr(arenaLo);
    pHeap = JKRExpHeap::create(newHi, arenaHi - reinterpret_cast< u32 >(newHi), JKRHeap::sRootHeap, true);
#endif

    if (MR::isEqualCurrentHeap(pHeap)) {
        JKRHeap::sRootHeap->becomeCurrentHeap();
    }

    HeapMemoryWatcher::sRootHeapGDDR3 = pHeap;
}

void HeapMemoryWatcher::createHeaps() {
    MR::CurrentHeapRestorer heapRestorer = MR::CurrentHeapRestorer(JKRHeap::sRootHeap);

    JKRExpHeap* systemHeap = ::createExpHeap(0x40000, JKRHeap::sRootHeap, false);
#ifdef GALAXY_NATIVE
    // Native audio objects are larger (64-bit pointers): the SMG2 bank set overflows the Wii's 0x200000 at
    // bank 45 of the boot audio archive (MacioGalaxy measured about +280 KB over SMG1's 0x1E0000).
    mAudSystemHeap = ::createSolidHeap(0x400000, JKRHeap::sRootHeap);
#else
    mAudSystemHeap = ::createSolidHeap(0x200000, JKRHeap::sRootHeap);
#endif
    systemHeap->becomeSystemHeap();

    JKRHeap* pRootHeapGDDR = HeapMemoryWatcher::sRootHeapGDDR3;
    u32 wpadHeapSize = OSRoundUp32B(WPADGetWorkMemorySize()) + 0xD0;
    mWPadHeap = ::createExpHeap(wpadHeapSize, pRootHeapGDDR, false);
    mHomeButtonLayoutHeap = ::createExpHeap(0x80000, HeapMemoryWatcher::sRootHeapGDDR3, false);

#ifdef GALAXY_NATIVE
    // The stationed heaps take everything but the game heaps' share. Natively the game heaps also hold
    // host-layout J3D/particle copies and larger objects, and the native arenas are 128/256 MiB, so the
    // game heaps keep 64/96 MiB more than the Wii's 0x500000/0xD80000 (the stationed heaps still get far
    // more than on the Wii).
    u32 stationedHeapNapaSize = JKRHeap::sRootHeap->getMaxAllocatableSize(16) - (0x500000 + 0x4000000);
    u32 stationedHeapGDDRSize = sRootHeapGDDR3->getMaxAllocatableSize(16) - (0xD80000 + 0x6000000);
#else
    u32 stationedHeapNapaSize = JKRHeap::sRootHeap->getMaxAllocatableSize(16) - 0x500000;
    u32 stationedHeapGDDRSize = sRootHeapGDDR3->getMaxAllocatableSize(16) - 0xD80000;
#endif
    mStationedHeapNapa = ::createExpHeap(stationedHeapNapaSize, JKRHeap::sRootHeap, false);
    mStationedHeapGDDR = ::createExpHeap(stationedHeapGDDRSize, HeapMemoryWatcher::sRootHeapGDDR3, false);
    createGameHeap();
}

void HeapMemoryWatcher::createGameHeap() {
    mGameHeapNapa = ::createExpHeap(-1, JKRHeap::sRootHeap, false);
    mGameHeapGDDR = ::createExpHeap(-1, HeapMemoryWatcher::sRootHeapGDDR3, false);
}

HeapMemoryWatcher::HeapMemoryWatcher()
    : mStationedHeapNapa(), mStationedHeapGDDR(), mGameHeapNapa(), mGameHeapGDDR(), mWorldMapHeap(), mFileCacheHeap(), mFileCacheDelegateHeap(), mSceneHeapNapa(),
      mSceneHeapGDDR(), mWPadHeap(), mHomeButtonLayoutHeap(), mAudSystemHeap() {
#ifdef GALAXY_NATIVE
    mFileCacheHostImageHeap = nullptr;
    mFileCacheArchiveBytes = 0;
    mFileCacheArchiveCount = 0;
#endif
    JKRHeap::setErrorHandler(HeapMemoryWatcher::memoryErrorCallback);
#ifdef GALAXY_NATIVE
    PetariNative::J3D::setHostImageHeapResolver(::resolveFileCacheHostImageHeap);
#endif
    createHeaps();
}

void HeapMemoryWatcher::memoryErrorCallback(void*, u32, int) {
    OSPanic(__FILE__, 617, "");
}

void HeapMemoryWatcher::checkRestMemory() {
}

#ifdef GALAXY_NATIVE
void HeapMemoryWatcher::noteArchiveMounted(JKRHeap* pHeap, const char* pName, const void* pData) {
    const u8* pBytes = static_cast< const u8* >(pData);
    const u32 bytes = pBytes == nullptr ? 0 : (u32(pBytes[4]) << 24) | (u32(pBytes[5]) << 16) | (u32(pBytes[6]) << 8) | pBytes[7];
    const char* pHeapName = "other";
    if (pHeap == mFileCacheHeap) {
        pHeapName = "file cache";
        mFileCacheArchiveBytes += bytes;
        mFileCacheArchiveCount++;
    } else if (pHeap == mSceneHeapGDDR) {
        pHeapName = "scene GDDR";
    } else if (pHeap == mSceneHeapNapa) {
        pHeapName = "scene NAPA";
    } else if (pHeap == mStationedHeapNapa || pHeap == mStationedHeapGDDR) {
        pHeapName = "stationed";
    }
    if (MR::Native::isTraceBoot()) {
        OSReport("[heap-arc] %s -> %s, %u bytes; file cache free %d\n", pName, pHeapName, bytes,
                 mFileCacheHeap != nullptr ? mFileCacheHeap->getTotalFreeSize() : -1);
    }
}
#endif
