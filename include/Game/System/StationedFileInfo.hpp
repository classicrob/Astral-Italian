#pragma once

#include <revolution/types.h>

namespace MR {
    struct StationedFileInfo {
        enum HeapType {
            HEAP_TYPE_NAPA = 0,
            HEAP_TYPE_GDDR = 1,
            HEAP_TYPE_NONE = 2,
        };

        // SMG2 values, read from the cStationedFileInfo table (main.dol) and the switch immediates in
        // StationedArchiveLoader / ConditionUsePlayerHeap. Value 2 has no table entry; it is only
        // handled as a mount type in loadResourcesFromTable (target switch range 1..8).
        // 2, 6 and 8 have no name in the code; those names are inferred (see HEADERS.md).
        enum LoadType {
            LOAD_TYPE_FILE = 0,
            LOAD_TYPE_MOUNT = 1,
            LOAD_TYPE_MOUNT_RESERVED = 2,
            LOAD_TYPE_MOUNT_RESOURCE_MARIO = 3,
            LOAD_TYPE_MOUNT_RESOURCE_LUIGI = 4,
            LOAD_TYPE_MOUNT_RESOURCE = 5,
            LOAD_TYPE_MOUNT_RESOURCE_STAGE = 6,         // stage/galaxy object archives (Mini*Galaxy, items, enemies)
            LOAD_TYPE_MOUNT_RESOURCE_LAYOUT = 7,
            LOAD_TYPE_MOUNT_RESOURCE_LAYOUT_SCENE = 8,  // scene layouts (map, result, pause, galaxy select)
        };

        /* 0x0 */ HeapType mHeapType;
        /* 0x4 */ LoadType mLoadType;
        /* 0x8 */ const char* mArchive;
    };

    const StationedFileInfo* getStationedFileInfoTable();
};  // namespace MR
