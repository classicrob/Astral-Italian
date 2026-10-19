#ifndef RVL_FACE_LIBRARY_INTERNAL_HIDDEN_DATABASE_H
#define RVL_FACE_LIBRARY_INTERNAL_HIDDEN_DATABASE_H
#include "RFLi_Format.h"
#include "RFLi_Types.h"
#include <revolution/types.h>
#ifdef GALAXY_NATIVE
#include <stdint.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

#define RFLi_HDB_DATA_MAX 10000
#ifdef GALAXY_NATIVE
typedef void (*RFLiHiddenDBCallback)(uintptr_t);
#endif

// Forward declarations
typedef struct RFLiCtrlBuf;

typedef struct RFLiHiddenDB {
    u32 identifier;                        // at 0x0
    s16 head;                              // at 0x4
    s16 tail;                              // at 0x6
    RFLiTableData data[RFLi_HDB_DATA_MAX]; // at 0x8
    u8 padding[22];                        // at 0x1D4C8
    u16 crc;                               // at 0x1D4DE
} RFLiHiddenDB;

typedef struct RFLiHDBList {
    RFLiHiddenCharData data[RFL_DB_CHAR_MAX]; // at 0x0
    u8 num;                                   // at 0x1900
    u8 current;                               // at 0x1901
} RFLiHDBList;

typedef struct RFLiHDBManager {
    BOOL loaded;                  // at 0x0
    BOOL readError;               // at 0x4
    void* writeTmp;               // at 0x8
    s16 writeIndex;               // at 0xC
    RFLiCallback writeCb;         // at 0x10
    void* formatTmp;              // at 0x14
    RFLiCallback formatCb;        // at 0x18
    s16 formatIndex;              // at 0x1C
    RFLiHiddenCharData* loadDst;  // at 0x20
    void* loadTmp;                // at 0x24
#ifdef GALAXY_NATIVE
    uintptr_t loadArg;
    RFLiHiddenDBCallback loadCb;
#else
    u32 loadArg;                  // at 0x28
    RFLiExCallback loadCb;        // at 0x2C
#endif
    u16 loadIndex;                // at 0x30
    RFLiHiddenCharData* cachedDB; // at 0x34
    BOOL cached;                  // at 0x38
    RFLiHDBList list;             // at 0x3C
} RFLiHDBManager;

void RFLiInitHiddenDatabase(void);
#ifdef GALAXY_NATIVE
RFLErrcode RFLiLoadHiddenDataAsync(RFLiHiddenCharData* data, u16 index,
                                   RFLiHiddenDBCallback cb, uintptr_t arg);
#else
RFLErrcode RFLiLoadHiddenDataAsync(RFLiHiddenCharData* data, u16 index,
                                   RFLiExCallback cb, u32 arg);
#endif
RFLErrcode RFLiLoadCachedHiddenData(RFLiHiddenCharData* data, u16 index);
RFLErrcode RFLiWriteCtrlToHiddenDB(const struct RFLiCtrlBuf* buf, BOOL ch);
s32 RFLiSearchHiddenData(const RFLCreateID* id);
u16 RFLiCountupHiddenDataNum(RFLSex sex);
s16 RFLiGetHiddenNext(u16 index);
s16 RFLiGetHiddenPrev(u16 index);
BOOL RFLiIsValidHiddenData(u16 index, RFLSex sex) NO_INLINE;
void RFLiClearCacheHDB(RFLiHiddenCharData* hdb);
BOOL RFLiIsCachedHDB(void);

#ifdef __cplusplus
}
#endif
#endif
