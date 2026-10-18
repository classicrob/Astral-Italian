#ifndef RVL_FACE_LIBRARY_INTERNAL_SYSTEM_H
#define RVL_FACE_LIBRARY_INTERNAL_SYSTEM_H
#include "RFL_System.h"
#include "RFLi_Controller.h"
#include "RFLi_Database.h"
#include "RFLi_HiddenDatabase.h"
#include "RFLi_NANDAccess.h"
#include "RFLi_NANDLoader.h"
#include "RFLi_Model.h"
#include "RFLi_Types.h"
#include <revolution/mem.h>
#include <revolution/types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct RFLiManager {
    void* workBuffer;                      // at 0x0
    MEMiHeapHead* rootHeap;                // at 0x4
    MEMiHeapHead* systemHeap;              // at 0x8
    MEMiHeapHead* tmpHeap;                 // at 0xC
    RFLiDBManager dbMgr;                   // at 0x10
    RFLiLoader loader;                     // at 0xC0
    RFLiHDBManager hdbMgr;                 // at 0x16C
    RFLiCtrlBufManager ctrlMgr;            // at 0x1AAC
    BOOL working;                         // at 0x1B20
    BOOL deluxeTex;                       // at 0x1B24
    u8 brokenType;                        // at 0x1B28
    RFLErrcode lastErrCode;                // at 0x1B2C
    RFLErrcode beforeCloseErr;             // at 0x1B30
    s32 lastReason;                       // at 0x1B34
    s32 beforeCloseReason;                // at 0x1B38
    RFLiAccessInfo info[RFLiFileType_Max]; // at 0x1B40
    u8 _1F00[0xC];
    RFLCallback modelDrawCb;              // at 0x1F0C
    RFLCallback iconDrawCb;               // at 0x1F10
    RFLiCoordinateData coordinateData;     // at 0x1F14
    // The target reserves a 0x1F28-byte manager before the work buffer.
} RFLiManager;

RFLErrcode RFLiBootLoadAsync(void);
void* RFLiAlloc(u32 size);
void* RFLiAlloc32(u32 size);
void RFLiFree(void* block);
RFLiDBManager* RFLiGetDBManager(void);
RFLiHDBManager* RFLiGetHDBManager(void);
RFLiLoader* RFLiGetLoader(void);
BOOL RFLiGetWorking(void);
void RFLiSetWorking(BOOL working);
RFLiManager* RFLiGetManager(void);
RFLiAccessInfo* RFLiGetAccInfo(RFLiFileType type);
RFLiCtrlBufManager* RFLiGetCtrlBufManager(void);
s32 RFLiGetLastReason(void);
void RFLiSetFileBroken(RFLiFileBrokenType type);
BOOL RFLiNotFoundError(void);
BOOL RFLiNeedRepairError(void);
BOOL RFLiCriticalError(void);
BOOL RFLiGetUseDeluxTex(void);

#ifdef __cplusplus
}
#endif
#endif
