#pragma once

#include <revolution/types.h>

class JKRExpHeap;
class JKRHeap;
class JKRSolidHeap;

class HeapMemoryWatcher {
public:
    HeapMemoryWatcher();

    JKRHeap* getHeapNapa(const JKRHeap*);
    JKRHeap* getHeapGDDR3(const JKRHeap*);
    void createWorldMapHeap();
    void createFileCacheHeapOnGameHeap(u32);
    void createSceneHeapOnGameHeap();
    void adjustStationedHeapNapa();
    void adjustStationedHeapGDDR3();
    void setCurrentHeapToStationedHeap();
    void setCurrentHeapToGameHeap();
    void setCurrentHeapToSceneHeap();
    void resetGameHeapNapaAndGDDR3();
    void resetGameHeapNapa();
    void resetGameHeapGDDR3() NO_INLINE;
    void destroyWorldMapHeap();
    void destroyFileCache();
    void freeAllFileCache();
    void destroySceneHeap();
    static void createRootHeap();
    void createHeaps();
    void createGameHeap();
    static void memoryErrorCallback(void*, u32, int);
    void checkRestMemory();

    /* 0x00 */ JKRExpHeap* mStationedHeapNapa;
    /* 0x04 */ JKRExpHeap* mStationedHeapGDDR;
    /* 0x08 */ JKRExpHeap* mGameHeapNapa;
    /* 0x0C */ JKRExpHeap* mGameHeapGDDR;
    /* 0x10 */ JKRSolidHeap* mWorldMapHeap;
    /* 0x14 */ JKRSolidHeap* mFileCacheHeap;
    /* 0x18 */ JKRHeap* mFileCacheDelegateHeap;
    /* 0x1C */ JKRSolidHeap* mSceneHeapNapa;
    /* 0x20 */ JKRSolidHeap* mSceneHeapGDDR;
    /* 0x24 */ JKRExpHeap* mWPadHeap;
    /* 0x28 */ JKRExpHeap* mHomeButtonLayoutHeap;
    /* 0x2C */ JKRSolidHeap* mAudSystemHeap;
#ifdef GALAXY_NATIVE
    // Host-layout copies of big-endian J3D files whose archive is in mFileCacheHeap (see
    // HeapMemoryWatcher.cpp). Created and destroyed with mFileCacheHeap.
    JKRSolidHeap* mFileCacheHostImageHeap;

    // Heap headroom diagnostics (PETARI_TRACE_BOOT): FileLoaderThread reports each mounted
    // archive; the archive bytes resident in the file cache since it was created separate the
    // Wii's share of its use (the same archive bytes) from native growth.
    void noteArchiveMounted(JKRHeap* pHeap, const char* pName, const void* pData);

    // Native room on top of the Wii file cache size for native resource-object growth (see
    // createFileCacheHeapOnGameHeap), and the free ratio the Wii placement rule sees: the file
    // cache's free bytes beyond that room over the Wii size.
    static const u32 cFileCacheNativeAllowance = 0x300000;
    f32 getFileCachePlacementFreeRatio() const;
    u32 mFileCacheArchiveBytes;
    u32 mFileCacheArchiveCount;

    // The world map heap (the Wii's 0x840000 solid heap) holds the world map and Starship resource archives
    // and the J3D objects ResourceHolder creates for them, and natively also each J3D file's host-layout copy,
    // which the Wii never allocates. Those copies go to mWorldMapHostImageHeap, a sibling of the same size
    // (see createWorldMapHeap), and native resource-object growth gets cWorldMapNativeAllowance on top of the
    // Wii size. The world map heap's archives are chosen by name, not by free space, so the allowance does not
    // change what is placed there.
    //
    // Measured (GX app, PETARI_TRACE_BOOT [heap-peak] at destroy, Starship scenario 2 and the world map stages):
    // peak use of the world map heap, with the J3D host copies on the companion, was 8,360,968 bytes for the
    // Starship (7,446,552 of it archives at mount, so 0.91 MB of resource objects after mount), 7,767,176 for
    // WorldMap01Galaxy and 7,764,168 for WorldMap04Galaxy, against the Wii's 8,650,752: every measured peak is
    // below the Wii size. The companion held 5.62 MB (65%) for the Starship and 0.57 MB for the world maps. The
    // allowance is therefore margin for native object growth the measured stages do not reach (headroom to Wii
    // size + allowance was 1.34 MB or more); re-measure with the [heap-peak] line before lowering it.
    JKRSolidHeap* mWorldMapHostImageHeap;
    static const u32 cWorldMapWiiSize = 0x840000;
    static const u32 cWorldMapNativeAllowance = 0x100000;
#endif
    static JKRExpHeap* sRootHeapGDDR3;
};
