#include "revolution/wpad.h"

void WPADiClearMemoryControl(s32 chan) {
    WPADMEMControlBlock* memory = &_wpdcb[chan]->memoryControl;
    memory->used = FALSE;
    memory->p_buf = NULL;
    memory->len = 0;
    memory->addr = 0;
    memory->callback = NULL;
}

// Descriptive name: the historical SDK spelling is unavailable.
// NONMATCHING: address assembly register choice and schedule only.
s32 WPADWriteExtensionRegister(s32 chan, void* data, u16 length, u8 extension, u16 address, WPADCallback callback) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL setup;
    s32 result;
    BOOL enable = OSDisableInterrupts();

    result = p_wpd->status;
    setup = p_wpd->setup;
    OSRestoreInterrupts(enable);

    if (result != WPAD_ERR_NO_CONTROLLER) {
        if (!setup) {
            result = WPAD_ERR_BUSY;
        } else {
            result = WPADiSendWriteData(&p_wpd->cmdq, data, length,
                                       ((extension << 16) | address) | 0x04000000, callback)
                         ? WPAD_ERR_NONE : WPAD_ERR_BUSY;
        }
    }
    if (result != WPAD_ERR_NONE && callback != NULL) {
        callback(chan, result);
    }
    return result;
}
