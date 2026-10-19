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
#include "Game/Util/SingletonHolder.hpp"
#include <petari/host_image_heap.hpp>
#include "Game/System/NativeBootTrace.hpp"
#endif

JKRExpHeap* HeapMemoryWatcher::sRootHeapGDDR3;

#ifdef GALAXY_NATIVE
namespace {
    // Heap for the host-layout copy of a big-endian J3D file: the file cache's companion when
    // the file is inside the file cache, otherwise null (the current heap, as before). The
    // companion is a sibling of the file cache, so findFromRoot resolves a source inside the
    // file cache to the file cache itself. Both pointers are null outside their lifetime.
    void traceSceneHeap(const char* pName, JKRHeap* pHeap) {
        if (pHeap == nullptr) {
            OSReport("[heap] %s: none\n", pName);
            return;
        }

        const s32 size = static_cast< s32 >(static_cast< u8* >(pHeap->getEndAddr()) - static_cast< u8* >(pHeap->getStartAddr()));
        const s32 freeSize = pHeap->getTotalFreeSize();
        OSReport("[heap] %s: size %d, used %d, free %d (%.1f%%), max free %d\n", pName, size, size - freeSize, freeSize,
                 size != 0 ? 100.0f * freeSize / size : 0.0f, pHeap->getFreeSize());
    }

    // Every JKR heap below pHeap, one line each: depth, type, size, used, free, largest free
    // block, and the HeapMemoryWatcher role when it has one (PETARI_TRACE_BOOT, at scene ready).
    void traceHeapTree(const HeapMemoryWatcher* pWatcher, JKRHeap* pHeap, int depth) {
        const char* pRole = "";
        if (pHeap == JKRHeap::sRootHeap) pRole = "root NAPA";
        else if (pHeap == HeapMemoryWatcher::sRootHeapGDDR3) pRole = "root GDDR";
        else if (pHeap == JKRHeap::sSystemHeap) pRole = "system";
        else if (pHeap == pWatcher->mStationedHeapNapa) pRole = "stationed NAPA";
        else if (pHeap == pWatcher->mStationedHeapGDDR) pRole = "stationed GDDR";
        else if (pHeap == pWatcher->mGameHeapNapa) pRole = "game NAPA";
        else if (pHeap == pWatcher->mGameHeapGDDR) pRole = "game GDDR";
        else if (pHeap == pWatcher->mFileCacheHeap) pRole = "file cache";
        else if (pHeap == pWatcher->mFileCacheHostImageHeap) pRole = "file cache host images";
        else if (pHeap == pWatcher->mSceneHeapNapa) pRole = "scene NAPA";
        else if (pHeap == pWatcher->mSceneHeapGDDR) pRole = "scene GDDR";
        else if (pHeap == pWatcher->mWPadHeap) pRole = "WPAD";
        else if (pHeap == pWatcher->mHomeButtonLayoutHeap) pRole = "home button layout";
        else if (pHeap == pWatcher->mAudSystemHeap) pRole = "audio system";
        const u32 type = pHeap->getHeapType();
        const char typeName[5] = {static_cast< char >(type >> 24), static_cast< char >(type >> 16), static_cast< char >(type >> 8),
                                  static_cast< char >(type), '\0'};
        const s32 size = static_cast< s32 >(static_cast< u8* >(pHeap->getEndAddr()) - static_cast< u8* >(pHeap->getStartAddr()));
        const s32 freeSize = pHeap->getTotalFreeSize();
        OSReport("[heap-tree] %*s%p %s size %d used %d free %d (%.1f%%) max free %d %s\n", depth * 2, "", pHeap, typeName, size, size - freeSize,
                 freeSize, size != 0 ? 100.0f * freeSize / size : 0.0f, pHeap->getFreeSize(), pRole);
        for (JSUTree< JKRHeap >* pChild = pHeap->mChildTree.getFirstChild(); pChild != nullptr; pChild = pChild->getNextChild()) {
            traceHeapTree(pWatcher, pChild->getObject(), depth + 1);
        }
    }

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
    // The file cache holds scene archives while the Wii placement rule sees 3% free
    // (MR::getAproposHeapForSceneArchive), and then the resource objects ResourceHolder
    // creates for them, on the file cache, after that check: the Wii's 3% is their room.
    // Native resource objects and holders are larger (64-bit pointers). Measured over every
    // scene archive on the disc (native_file_cache_demand): J3D resource objects 5.8% of the
    // archive bytes; in Good Egg 1 (116 cached archives, 14.49 MB) all native non-archive use
    // was 1.42 MB, 9.8%. A stage whose archives fill the Wii rule's 97% (about 16.5 MB) would
    // need about 1.6 MB, where the Wii leaves 0.5 MB. The file cache therefore gets
    // cFileCacheNativeAllowance more room, and placement ignores that room
    // (getFileCachePlacementFreeRatio), so the same archives are cached as on the Wii (or
    // fewer, as native objects created meanwhile count as used) and the objects get 3% plus
    // the allowance.
    size += cFileCacheNativeAllowance;
    mFileCacheArchiveBytes = 0;
    mFileCacheArchiveCount = 0;
#endif
    mFileCacheHeap = ::createSolidHeap(size, mGameHeapGDDR);
    mFileCacheDelegateHeap = new (mFileCacheHeap, 0) DelegatingHeap(mFileCacheHeap);
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
}

void HeapMemoryWatcher::freeAllFileCache() {
    mFileCacheDelegateHeap->freeAll();
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
    arenaLo = reinterpret_cast< uintptr_t >(OSGetMEM2ArenaLo());
    arenaHi = reinterpret_cast< uintptr_t >(OSGetMEM2ArenaHi());
    // The start of MEM2 becomes ARAM (RVL "alternate ARAM"). The audio ARAM heap
    // occupies offsets ARGetBaseAddress() (0x4000) .. 0x4000 + 0xE00000, so reserve
    // that whole range. The Wii reserved only 0xE00000 bytes, which let the top
    // 16 KiB of audio ARAM overlap the GDDR3 root heap and made JKRAram's
    // graph-memory size wrap to 0xFFFFC000; natively it is exactly 0.
    newHi = reinterpret_cast< void* >(arenaLo + 0x4000 + 0xE00000);
    OSSetMEM2ArenaHi(newHi);
    JKRHeap::setAltAramStartAdr(arenaLo);
    pHeap = JKRExpHeap::create(newHi, arenaHi - reinterpret_cast< uintptr_t >(newHi), JKRHeap::sRootHeap, true);
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
    mAudSystemHeap = ::createSolidHeap(0x200000, JKRHeap::sRootHeap);
    systemHeap->becomeSystemHeap();

    JKRHeap* pRootHeapGDDR = HeapMemoryWatcher::sRootHeapGDDR3;
#ifdef GALAXY_NATIVE
    // Wii 0xD0 is the 0x90 heap object, one 0x10 block header and 0x30 usable bytes.
    // The native heap object is larger; keep the same usable bytes.
    u32 wpadHeapSize = OSRoundUp32B(WPADGetWorkMemorySize()) + ALIGN_NEXT(sizeof(JKRExpHeap), 0x10) + sizeof(JKRExpHeap::CMemBlock) + 0x30;
#else
    u32 wpadHeapSize = OSRoundUp32B(WPADGetWorkMemorySize()) + 0xD0;
#endif
    mWPadHeap = ::createExpHeap(wpadHeapSize, pRootHeapGDDR, false);
    mHomeButtonLayoutHeap = ::createExpHeap(0x80000, HeapMemoryWatcher::sRootHeapGDDR3, false);

    u32 stationedHeapNapaSize = JKRHeap::sRootHeap->getMaxAllocatableSize(16) - 0x500000;
    u32 stationedHeapGDDRSize = sRootHeapGDDR3->getMaxAllocatableSize(16) - 0xD80000;
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
    JKRHeap::setErrorHandler(HeapMemoryWatcher::memoryErrorCallback);
#ifdef GALAXY_NATIVE
    PetariNative::J3D::setHostImageHeapResolver(::resolveFileCacheHostImageHeap);
#endif
    createHeaps();
}

#ifdef GALAXY_NATIVE
f32 HeapMemoryWatcher::getFileCachePlacementFreeRatio() const {
    const s32 size = static_cast< s32 >(static_cast< u8* >(mFileCacheHeap->getEndAddr()) - static_cast< u8* >(mFileCacheHeap->getStartAddr()));
    const s32 wiiSize = size - static_cast< s32 >(cFileCacheNativeAllowance);
    const s32 wiiFree = mFileCacheHeap->getTotalFreeSize() - static_cast< s32 >(cFileCacheNativeAllowance);
    return wiiSize > 0 ? static_cast< f32 >(wiiFree) / wiiSize : 0.0f;
}

void HeapMemoryWatcher::noteArchiveMounted(JKRHeap* pHeap, const char* pName, const void* pData) {
    // A mounted archive's resident size: the RARC header's file size (big-endian word 1), the
    // bytes FileRipper allocated for it (before 0x40 alignment).
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

void HeapMemoryWatcher::memoryErrorCallback(void* heap, u32 bytes, int alignment) {
    JKRHeap* allocator = static_cast<JKRHeap*>(heap);
    OSPanic(__FILE__, __LINE__, "Native heap allocation failed: heap=%p, requested=%u, alignment=%d, size=%u, free=%d, max=%d",
            heap, bytes, alignment,
            static_cast< u32 >(static_cast< u8* >(allocator->getEndAddr()) - static_cast< u8* >(allocator->getStartAddr())), allocator->getTotalFreeSize(), allocator->getFreeSize());
}
#else
void HeapMemoryWatcher::memoryErrorCallback(void*, u32, int) {
    OSPanic(__FILE__, 617, "");
}
#endif

void HeapMemoryWatcher::checkRestMemory() {
#ifdef GALAXY_NATIVE
    // End of scene initialization (GameSystemSceneController::initializeScene): with
    // PETARI_TRACE_BOOT set, report the scene's heap headroom, including the file cache's
    // host-image companion, so a ready stage shows its margin rather than only the absence
    // of an allocation failure.
    if (MR::Native::isTraceBoot()) {
        if (mFileCacheHeap != nullptr) {
            const s32 size = static_cast< s32 >(static_cast< u8* >(mFileCacheHeap->getEndAddr()) - static_cast< u8* >(mFileCacheHeap->getStartAddr()));
            const s32 used = size - mFileCacheHeap->getTotalFreeSize();
            OSReport("[heap] file cache archives: %u resident, %u bytes; other use (bookkeeping, resource objects) %d bytes\n",
                     mFileCacheArchiveCount, mFileCacheArchiveBytes, used - static_cast< s32 >(mFileCacheArchiveBytes));
        }
        ::traceSceneHeap("file cache", mFileCacheHeap);
        ::traceSceneHeap("file cache host images", mFileCacheHostImageHeap);
        ::traceSceneHeap("scene NAPA", mSceneHeapNapa);
        ::traceSceneHeap("scene GDDR", mSceneHeapGDDR);
        ::traceHeapTree(this, JKRHeap::sRootHeap, 0);
        if (sRootHeapGDDR3 != nullptr && sRootHeapGDDR3->mChildTree.getParent() == nullptr) {
            ::traceHeapTree(this, sRootHeapGDDR3, 0);  // a separate root (MEM2)
        }
    }
#endif
}
