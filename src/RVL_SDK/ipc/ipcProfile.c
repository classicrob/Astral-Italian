#include "revolution.h"
#include "private/iosrestypes.h"
#include <cstring>

#define IPC_PROFILE_REQUEST_COUNT 96
#define IPC_PROFILE_HANDLE_COUNT 128
#define IPC_PROFILE_PATH_LENGTH 48

static IOSResourceRequest IpcRequests[IPC_PROFILE_REQUEST_COUNT] = {0};
static IOSResourceRequest* IpcPending[IPC_PROFILE_REQUEST_COUNT];
static OSTime IpcQueuedAt[IPC_PROFILE_REQUEST_COUNT];
static char IpcHandlePaths[IPC_PROFILE_HANDLE_COUNT][IPC_PROFILE_PATH_LENGTH];
static char IpcPendingPaths[IPC_PROFILE_REQUEST_COUNT][IPC_PROFILE_PATH_LENGTH];

static u32 IpcNumPendingReqs = 0;
static u32 IpcNumUnIssuedReqs = 0;

static void AddReqInfo(IOSResourceRequest* req);
static void DelReqInfo(IOSResourceRequest* req);

void IPCiProfInit(void) {
    u32 i;
    IpcNumPendingReqs = 0;
    IpcNumUnIssuedReqs = 0;
    for (i = 0; i < IPC_PROFILE_REQUEST_COUNT; ++i) {
        IpcPending[i] = NULL;
        IpcQueuedAt[i] = 0;
    }
    memset(IpcHandlePaths, 0, sizeof(IpcHandlePaths));
    memset(IpcPendingPaths, 0, sizeof(IpcPendingPaths));
    memset(IpcRequests, 0, sizeof(IpcRequests));
}

void IPCiProfQueueReq(void* req, s32 fd) {
    ++IpcNumPendingReqs;
    ++IpcNumUnIssuedReqs;
    AddReqInfo((IOSResourceRequest*)req);
}

void IPCiProfAck(void) {
    --IpcNumUnIssuedReqs;
}

void IPCiProfReply(void* req, s32 fd) {
    --IpcNumPendingReqs;
    DelReqInfo((IOSResourceRequest*)req);
}

static void AddReqInfo(IOSResourceRequest* req) {
    u32 i;
    for (i = 0; i < IPC_PROFILE_REQUEST_COUNT; ++i) {
        if (IpcPending[i] == NULL) {
            BOOL enabled = OSDisableInterrupts();
            IpcPending[i] = req;
            IpcRequests[i] = *req;
            IpcQueuedAt[i] = OSGetTime();
            if (IpcRequests[i].cmd == 1) {
                strncpy(IpcPendingPaths[i], (const char*)OSPhysicalToCached(IpcRequests[i].args.open.path),
                        IPC_PROFILE_PATH_LENGTH - 1);
                IpcPendingPaths[i][IPC_PROFILE_PATH_LENGTH - 1] = '\0';
                IpcRequests[i].args.open.path = (u8*)IpcPendingPaths[i];
            }
            OSRestoreInterrupts(enabled);
            break;
        }
    }
}

static void DelReqInfo(IOSResourceRequest* req) {
    u32 i;
    for (i = 0; i < IPC_PROFILE_REQUEST_COUNT; ++i) {
        if (req == IpcPending[i] && req->handle == IpcRequests[i].cmd) {
            BOOL enabled = OSDisableInterrupts();
            if (IpcRequests[i].cmd == 1 && req->status >= 0) {
                strncpy(IpcHandlePaths[req->status], (const char*)IpcRequests[i].args.open.path,
                        IPC_PROFILE_PATH_LENGTH - 1);
                IpcHandlePaths[req->status][IPC_PROFILE_PATH_LENGTH - 1] = '\0';
                memset(IpcPendingPaths[i], 0, IPC_PROFILE_PATH_LENGTH);
            }
            if (IpcRequests[i].cmd == 2) {
                memset(IpcHandlePaths[IpcRequests[i].handle], 0, IPC_PROFILE_PATH_LENGTH);
            }
            IpcPending[i] = NULL;
            memset(&IpcRequests[i], 0, sizeof(IOSResourceRequest));
            IpcQueuedAt[i] = 0;
            OSRestoreInterrupts(enabled);
            break;
        }
    }
}
