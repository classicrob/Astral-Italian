#include "JSystem/JAudio2/dspproc.hpp"
#ifdef GALAXY_NATIVE
#include "JSystem/JAudio2/osdsp_task.hpp"
#endif
#include <private/flipper.h>
#include <revolution/dsp.h>

extern DSPTaskInfo* __DSP_first_task;
extern DSPTaskInfo* __DSP_curr_task;
extern "C" void __DSP_exec_task(DSPTaskInfo*, DSPTaskInfo*);
extern "C" void __DSP_remove_task(DSPTaskInfo* pTask);

void Dsp_Update_Request();

#ifdef GALAXY_NATIVE
// Shared with the DSP interrupt handler, which natively runs on another host
// thread: acquire/release accesses instead of plain volatile ones.
#define DSP_SHARED_LOAD(v) __atomic_load_n(&(v), __ATOMIC_ACQUIRE)
#define DSP_SHARED_STORE(v, x) __atomic_store_n(&(v), (x), __ATOMIC_RELEASE)
#else
#define DSP_SHARED_LOAD(v) (v)
#define DSP_SHARED_STORE(v, x) ((v) = (x))
#endif

static vu8 DspRunningStatus;
static u8 sPendingFrameMode;

DSPTaskInfo* DSP_prior_task;

// Original diagnostic string at 80659C00 has no surviving reference,
// likely from code stripped from the shipped build.
extern const char sDspGeneralRequestHalt[] = "GEN-REQ HALT (Task %x)\n";

extern "C" void __DSPHandler(__OSInterrupt interrupt, OSContext* pContext) {
#ifdef GALAXY_NATIVE
    // The native DSP device acknowledges its own interrupt and calls this on
    // its interrupt thread; there is no DSP control register or exception
    // context to switch.
    (void)interrupt;
    (void)pContext;
#else
    OSContext funcContext;
    __DSPRegs[5] = ((u16)(__DSPRegs[5]) & ~0x28) | 0x80;
    OSClearContext(&funcContext);
    OSSetCurrentContext(&funcContext);
#endif

    if (DspRunningStatus == 1 || DspRunningStatus == 0) {
        __DSP_curr_task = DSP_prior_task;
    }

    while (DSPCheckMailFromDSP() == 0) {
    }

    u32 mail = DSPReadMailFromDSP();

    if ((__DSP_curr_task->flags & 2) && mail == 0xDCD10002) {
        mail = 0xDCD10003;
    }

    switch (mail) {
    case 0xDCD10000:
        __DSP_curr_task->state = 1;
        if (__DSP_curr_task == DSP_prior_task) {
            DspRunningStatus = 1;
        }

        if (__DSP_curr_task->init_cb != NULL) {
            __DSP_curr_task->init_cb(__DSP_curr_task);
        }

        break;
    case 0xDCD10001:
        __DSP_curr_task->state = 1;
        if (__DSP_curr_task == DSP_prior_task) {
            DspRunningStatus = 1;
            Dsp_Update_Request();
        }

        if (__DSP_curr_task->res_cb != NULL) {
            __DSP_curr_task->res_cb(__DSP_curr_task);
        }

        OSReport("Audio Resumed\n");
        break;
    case 0xDCD10002:
        OSReport("Yield Handler\n");
        DSPSendMailToDSP(0xCDD10001);
        while (DSPCheckMailToDSP() != 0) {
        }

        __DSP_curr_task->state = 2;
        if (__DSP_curr_task->next == NULL && sPendingFrameMode) {
            __DSP_exec_task(__DSP_curr_task, DSP_prior_task);
            sPendingFrameMode = 0;
            __DSP_curr_task = DSP_prior_task;
        } else {
            __DSP_exec_task(__DSP_curr_task, __DSP_curr_task->next);
            __DSP_curr_task = __DSP_curr_task->next;
        }

        break;
    case 0xDCD10003:
        OSReport("Done DSP Task  %x \n", __DSP_curr_task);
        if (__DSP_curr_task->done_cb != NULL) {
            __DSP_curr_task->done_cb(__DSP_curr_task);
        }

        DSPSendMailToDSP(0xCDD10001);
        while (DSPCheckMailToDSP() != 0) {
        }

        __DSP_curr_task->state = 3;
        if (__DSP_curr_task->next == NULL) {
            __DSP_exec_task(NULL, DSP_prior_task);
            __DSP_remove_task(__DSP_curr_task);
            __DSP_curr_task = DSP_prior_task;
        } else {
            __DSP_exec_task(NULL, __DSP_curr_task->next);
            __DSP_curr_task = __DSP_curr_task->next;
            __DSP_remove_task(__DSP_curr_task->prev);
        }

        break;
    case 0xDCD10004:
        if (__DSP_curr_task->req_cb != NULL) {
            __DSP_curr_task->req_cb(__DSP_curr_task);
        }

        break;
    case 0xDCD10005:
        if (__DSP_first_task == NULL || sPendingFrameMode) {
            DSPSendMailToDSP(0xCDD10003);
            while (DSPCheckMailToDSP() != 0) {
            }

            sPendingFrameMode = 0;
            __DSP_curr_task = DSP_prior_task;
            Dsp_Update_Request();
        } else {
            OSReport("Audio Yield Start\n");
            DspRunningStatus = 3;
            DSPSendMailToDSP(0xCDD10001);
            while (DSPCheckMailToDSP() != 0) {
            }

            __DSP_exec_task(DSP_prior_task, __DSP_first_task);
            __DSP_curr_task = __DSP_first_task;
            OSReport("Audio Yield Finish\n");
        }

        break;
    }

#ifndef GALAXY_NATIVE
    OSClearContext(&funcContext);
    OSSetCurrentContext(pContext);
#endif
}

static JASDspAddr sync_stack[5];

#ifdef GALAXY_NATIVE
void DsyncFrame2(u32 param_0, JASDspAddr param_1, JASDspAddr param_2) {
#else
void DsyncFrame2(u32 param_0, u32 param_1, u32 param_2) {
#endif
    if (DspRunningStatus != 1) {
        static const char yieldMessage[] = "Yield中です\n";
        OSReport(yieldMessage);
        JASDspAddr* stack;
        *(stack = sync_stack) = param_0;
        sPendingFrameMode = 1;
        stack[1] = param_1;
        stack[2] = param_2;
        return;
    }

    DsyncFrame2ch(param_0, param_1, param_2);
    sPendingFrameMode = 0;
}

#ifdef GALAXY_NATIVE
void DsyncFrame3(u32 param_0, JASDspAddr param_1, JASDspAddr param_2, JASDspAddr param_3, JASDspAddr param_4) {
#else
void DsyncFrame3(u32 param_0, u32 param_1, u32 param_2, u32 param_3, u32 param_4) {
#endif
    if (DspRunningStatus != 1) {
        JASDspAddr* stack;
        *(stack = sync_stack) = param_0;
        sPendingFrameMode = 2;
        stack[1] = param_1;
        stack[2] = param_2;
        stack[3] = param_3;
        stack[4] = param_4;
        return;
    }

    DsyncFrame4ch(param_0, param_1, param_2, param_3, param_4);
    sPendingFrameMode = 0;
}

void Dsp_Update_Request() {
    switch (sPendingFrameMode) {
    case 0:
        break;
    case 1:
        DsyncFrame2(sync_stack[0], sync_stack[1], sync_stack[2]);
        break;
    case 2:
        DsyncFrame3(sync_stack[0], sync_stack[1], sync_stack[2], sync_stack[3], sync_stack[4]);
        break;
    }
}

int Dsp_Running_Check() {
    return DspRunningStatus == 1 ? TRUE : FALSE;
}

void Dsp_Running_Start() {
    DspRunningStatus = 1;
}
