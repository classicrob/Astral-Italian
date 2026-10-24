#include "revolution/wpad.h"
#include "revolution/wpad/lint.h"
#include "revolution/os/OSReset.h"
#include "revolution/wbc.h"
#include "private/flipper.h"
#include "revolution/dvd.h"
#include "revolution/sc.h"
#include "revolution/vi.h"
#include "revolution/wud.h"
#include "revolution/wud/wud.h"
#include <cstdio>
#include <mem.h>

static const char* __WPADVersion = "<< RVL_SDK - WPAD \trelease build: Dec 11 2009 15:59:48 (0x4302_145) >>";

WPADControlBlock _wpd[WPAD_MAX_CONTROLLERS];
WPADControlBlock* _wpdcb[WPAD_MAX_CONTROLLERS];

u8 _sleepTime;
u8 _dpdSensitivity;
u8 _sensorBarPos;
BOOL _rumble;
u8 _speakerVolume;
u8 _gametype;
u16 _gameTitle[17];
const char* _gamecode;

// Receive accepts handle 16 as well; InitSub initializes only entries 0..15.
s8 _dev_handle_index[17];
u8 _chan_active_state[WPAD_MAX_CONTROLLERS];
OSAlarm _managerAlarm;

DVDDiskID _diskId;

static u8 _scSetting;
static vu8 _shutdown;
static s8 _afhChannel;
static u8 _regShutdown = 0;

static u16 _senseCnt = 0;
static u8 _checkCnt = 0;
static u16 _afhCnt = 0;

static u8 _extCnt[WPAD_MAX_CONTROLLERS] = {0, 0, 0, 0};
static u8 _rumbleCnt[WPAD_MAX_CONTROLLERS] = {0, 0, 0, 0};

static s8 _infRes[4];

static BOOL _initialized;
static BOOL _startup = FALSE;
static int _recFlag = -1;
static int _recCnt = 0;

// Optional no-argument initialization hooks, called in this order when installed.
void (*__wpadOptionalInitCallback0)(void);
void (*__wpadOptionalInitCallback1)(void);
void (*__wpadOptionalInitCallback2)(void);
void (*__wpadOptionalInitCallback3)(void);
void (*__wpadOptionalInitCallback4)(void);
void (*__wpadOptionalInitCallback5)(void);
void (*__wpadOptionalInitCallback6)(void);


enum { WPAD_CHANGE_ACC, WPAD_CHANGE_DPD, WPAD_CHANGE_EXTENSION, WPAD_CHANGE_MPLS };
static u16 __wpadChangeDiffThreshold[4] = {6, 4, 6, 12};
static u16 __wpadChangeHystThreshold[4] = {30, 30, 30, 30};

extern void DEBUGPrint(const char*, ...);

static void __ClearControlBlock(s32 chan);
void __wpadCertWork(s32 chan);
static void setupCallback(s32 chan, s32 result);
static void __wpadAbortConnectionCallback(s32 chan, s32 result);
static void firmwareCheckCallback(s32 chan, s32 result);
s32 __wpadRetrieveChannel(WUDDevInfo* info);
static void __wpadConnectionCallback(WUDDevInfo* info, u8 open);
static void __wpadReceiveCallback(u8 dev_handle, u8* p_rpt, u16);
void WPADGetAccGravityUnit(s32 chan, u32 type, WPADAcc* acc);
static s32 __wpadSendDataSub(s32 chan, WPADCommand cmd);
BOOL WPADiSendWriteDataCmd(WPADCmdQueue* queue, u8 cmd, u32 addr, WPADCallback callback);
BOOL WPADiSendWriteData(WPADCmdQueue* queue, void* p_buf, u16 len, u32 addr, WPADCallback callback);
BOOL WPADiSendReadData(WPADCmdQueue* queue, void* p_buf, u16 len, u32 addr, WPADCallback callback);
void WPADiClearQueue(WPADCmdQueue* queue);

void* __wpadNoAlloc(u32 size) {
    return NULL;
}

u8 __wpadNoFree(void* ptr) {
    return 0;
}

// Retail capability flags retain real storage loads; historical names for the last three are unverified.
static BOOL _callbackByKPAD;

void WPADSetCallbackByKPAD(BOOL enabled) { _callbackByKPAD = enabled; }

BOOL _trnEnabled;
BOOL WPADIsEnabledTRN(void) { return _trnEnabled; }
BOOL _gtrEnabled;
BOOL WPADIsEnabledGTR(void) { return _gtrEnabled; }
BOOL _drmEnabled;
BOOL WPADIsEnabledDRM(void) { return _drmEnabled; }
BOOL _tkoEnabled;
BOOL WPADIsEnabledTKO(void) { return _tkoEnabled; }
BOOL _device19Enabled;
BOOL WPADIsEnabledDevice19(void) { return _device19Enabled; }
BOOL _device20Enabled;
BOOL WPADIsEnabledDevice20(void) { return _device20Enabled; }
BOOL _rawReportsEnabled;
BOOL WPADIsEnabledRawReports(void) { return _rawReportsEnabled; }

BOOL WBCSetupCalibration(void) { return FALSE; }
BOOL WBCGetCalibrationStatus(void) { return FALSE; }
u32 WBCGetBatteryLevel(u8 battery) {
#pragma unused(battery)
    return 0;
}
s32 WBCRead(WPADBLStatus* report, f64* weight, u32 count) {
#pragma unused(report, weight, count)
    return -1;
}
s32 WBCSetZEROPoint(f64* averagePressure, u32 count) {
#pragma unused(averagePressure, count)
    return -1;
}
s32 WBCGetTGCWeight(f64 averageWeight, f64* result, WPADBLStatus* report) {
#pragma unused(averageWeight, result, report)
    return -1;
}

// NONMATCHING: equivalent default-case lowering adds one instruction.
BOOL OnShutdown(BOOL final, u32 event) {
    BOOL shutdown;
    s32 status = WUDGetStatus();
    if (final || status == 0) return TRUE;
    if (status == 4 || status == 1 || status == 2) return FALSE;
    if (WUDIsBusy()) {
        WUDCancelSyncDevice();
        return FALSE;
    }
    shutdown = TRUE;
    switch (event) {
    case 0:
        WPADRegisterAllocator(__wpadNoAlloc, __wpadNoFree);
        // Fall through: fatal errors use immediate shutdown too.
    case 2:
    case 3:
        shutdown = TRUE;
        break;
    case 1:
    case 4:
    case 6:
        shutdown = FALSE;
        break;
    case 5:
        shutdown = __OSIsReturnToIdle != FALSE;
        break;
    }
    if (shutdown) __WPADShutdown();
    else __WPADReconnect(TRUE);
    return FALSE;
}

static s32 WPADiGetStatus(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable;
    s32 status;

    enable = OSDisableInterrupts();
    status = p_wpd->status;
    OSRestoreInterrupts(enable);

    return status;
}

s32 WPADiSendData(s32 chan, WPADCommand cmd) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    s32 status;
    BOOL enabled = OSDisableInterrupts();
    if (p_wpd->getStatFlag) status = WPAD_ERR_BUSY;
    else status = WPADiGetStatus(chan);
    OSRestoreInterrupts(enabled);
    if (status == WPAD_ERR_NONE) status = __wpadSendDataSub(chan, cmd);
    else if (status == WPAD_ERR_BUSY) {
        if (OSTicksToSeconds(__OSGetSystemTime() - p_wpd->cmdTimer) > 1) {
            p_wpd->cmdTimer = __OSGetSystemTime();
            WPADiDisconnect(chan, FALSE);
        }
    }
    return status;
}

void __wpadCalcRadioQuality(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    u32 packet = 2;
    u16 rs;

    if (_senseCnt != 10) return;

    rs = (u16)(p_wpd->radioSense * (10 - 1));
    rs += (u16)((p_wpd->packetCnt * 100) / packet);
    rs /= 10;
    rs = (u16)((rs > 100) ? 100 : rs);

    p_wpd->radioSense = (u8)rs;
    p_wpd->packetCnt = 0;

    if (p_wpd->audioStop) {
        if (rs > 85) {
            p_wpd->audioStop = 0;
            p_wpd->audioStopCnt = 0;
        } else if (rs > 80) {
            p_wpd->audioStopCnt++;
            if (p_wpd->audioStopCnt >= 20) {
                p_wpd->audioStop = 0;
                p_wpd->audioStopCnt = 0;
            }
        }
    } else {
        if (rs < 75) {
            p_wpd->audioStop = 1;
            p_wpd->audioStopCnt = 0;
        } else if (rs < 80) {
            p_wpd->audioStopCnt++;
            if (p_wpd->audioStopCnt >= 1) {
                p_wpd->audioStop = 1;
                p_wpd->audioStopCnt = 0;
            }
        }
    }
}

BOOL WPADiIsCoreFormat(u32 fmt) {
    if (fmt == WPAD_FMT_CORE || fmt == WPAD_FMT_CORE_ACC || fmt == WPAD_FMT_CORE_ACC_DPD) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL WPADiIsFsFormat(u32 fmt) {
    if (fmt == WPAD_FMT_FREESTYLE || fmt == WPAD_FMT_FREESTYLE_ACC || fmt == WPAD_FMT_FREESTYLE_ACC_DPD) {
        return TRUE;
    } else {
        return FALSE;
    }
}
BOOL WPADiIsClFormat(u32 fmt) {
    if (fmt == WPAD_FMT_CLASSIC || fmt == WPAD_FMT_CLASSIC_ACC || fmt == WPAD_FMT_CLASSIC_ACC_DPD) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static u8 IsButtonChanged(u16 curr, u16 prev) {
    return (u8)((curr != prev) ? 1 : 0);
}

static u8 IsAnalogChanged(s32 curr, s32 prev, s32 threshold) {
    s32 diff = ((curr - prev) < 0) ? prev - curr : curr - prev;
    return (u8)((diff > threshold) ? 1 : 0);
}

static u32 __wpadCheckChangeCount(BOOL changed, u16* count, u16* hysteresis, u32 type) {
    if (changed) {
        ++*count;
        if (*count > __wpadChangeDiffThreshold[type]) {
            *count = 0;
            *hysteresis = 0;
            return 1;
        }
    } else {
        *hysteresis = (u16)((*hysteresis + 1) % __wpadChangeHystThreshold[type]);
        if (*hysteresis == __wpadChangeHystThreshold[type] - 1 && *count > 0) --*count;
    }
    return 0;
}

// NONMATCHING: compiler boolean/loop lowering, register allocation and scheduling differ.
BOOL __wpadIsControllerDataChanged(WPADControlBlock* p_wpd, const void* p_curr, const void* p_prev) {
    const WPADStatus* curr = (const WPADStatus*)p_curr;
    const WPADStatus* prev = (const WPADStatus*)p_prev;
    u8 result = 0;
    if ((curr->err == WPAD_ERR_NONE || curr->err == -7) &&
        (prev->err == WPAD_ERR_NONE || prev->err == -7)) {
        BOOL accChanged;
        BOOL dpdChanged;
        result = IsButtonChanged(curr->button, prev->button);
        accChanged = IsAnalogChanged(curr->accX, prev->accX, 12) |
                     IsAnalogChanged(curr->accY, prev->accY, 12) |
                     IsAnalogChanged(curr->accZ, prev->accZ, 12);
        result |= __wpadCheckChangeCount(accChanged, &p_wpd->diffCountAcc, &p_wpd->hystCountAcc, WPAD_CHANGE_ACC);
        dpdChanged = IsAnalogChanged(curr->obj[0].x, prev->obj[0].x, 2) |
                     IsAnalogChanged(curr->obj[0].y, prev->obj[0].y, 2) |
                     IsAnalogChanged(curr->obj[1].x, prev->obj[1].x, 2) |
                     IsAnalogChanged(curr->obj[1].y, prev->obj[1].y, 2) |
                     IsAnalogChanged(curr->obj[2].x, prev->obj[2].x, 2) |
                     IsAnalogChanged(curr->obj[2].y, prev->obj[2].y, 2) |
                     IsAnalogChanged(curr->obj[3].x, prev->obj[3].x, 2) |
                     IsAnalogChanged(curr->obj[3].y, prev->obj[3].y, 2);
        result |= __wpadCheckChangeCount(dpdChanged, &p_wpd->diffCountDpd, &p_wpd->hystCountDpd, WPAD_CHANGE_DPD);
    }
    if (curr->err == WPAD_ERR_NONE && prev->err == WPAD_ERR_NONE) {
        switch (p_wpd->dataFormat) {
        case 3: case 4: case 5: {
            const WPADFSStatus* fsCurr = (const WPADFSStatus*)p_curr;
            const WPADFSStatus* fsPrev = (const WPADFSStatus*)p_prev;
            result |= __wpadCheckChangeCount(
                IsAnalogChanged(fsCurr->fsAccX, fsPrev->fsAccX, 12) |
                IsAnalogChanged(fsCurr->fsAccY, fsPrev->fsAccY, 12) |
                IsAnalogChanged(fsCurr->fsAccZ, fsPrev->fsAccZ, 12),
                &p_wpd->diffCountfsAcc, &p_wpd->hystCountfsAcc, WPAD_CHANGE_EXTENSION);
            result |= IsAnalogChanged(fsCurr->fsStickX, fsPrev->fsStickX, 1);
            result |= IsAnalogChanged(fsCurr->fsStickY, fsPrev->fsStickY, 1);
            break;
        }
        case 6: case 7: case 8: case 11: case 15: case 17: case 18: {
            const WPADCLStatus* clCurr = (const WPADCLStatus*)p_curr;
            const WPADCLStatus* clPrev = (const WPADCLStatus*)p_prev;
            s32 left, right, trigger;
            switch (p_wpd->devMode) {
            case 1: left = 16; right = 32; trigger = 8; break;
            case 3: left = 4; right = 4; trigger = 1; break;
            default: left = 1; right = 1; trigger = 1; break;
            }
            result |= IsButtonChanged(clCurr->clButton, clPrev->clButton);
            result |= IsAnalogChanged(clCurr->clLStickX / left, clPrev->clLStickX / left, 1);
            result |= IsAnalogChanged(clCurr->clLStickY / left, clPrev->clLStickY / left, 1);
            result |= IsAnalogChanged(clCurr->clRStickX / right, clPrev->clRStickX / right, 1);
            result |= IsAnalogChanged(clCurr->clRStickY / right, clPrev->clRStickY / right, 1);
            result |= IsAnalogChanged(clCurr->clTriggerL / trigger, clPrev->clTriggerL / trigger, 1);
            result |= IsAnalogChanged(clCurr->clTriggerR / trigger, clPrev->clTriggerR / trigger, 1);
            break;
        }
        case 10: {
            const WPADPercussionStatus* pc = (const WPADPercussionStatus*)p_curr;
            const WPADPercussionStatus* pp = (const WPADPercussionStatus*)p_prev;
            result |= IsButtonChanged(pc->button, pp->button);
            result |= IsAnalogChanged(pc->force[0], pp->force[0], 1);
            result |= IsAnalogChanged(pc->force[1], pp->force[1], 1);
            break;
        }
        case 12: {
            const WPADBLStatus* bc = (const WPADBLStatus*)p_curr;
            const WPADBLStatus* bp = (const WPADBLStatus*)p_prev;
            result |= __wpadCheckChangeCount(
                IsAnalogChanged(bc->pressure[0], bp->pressure[0], 50) |
                IsAnalogChanged(bc->pressure[1], bp->pressure[1], 50) |
                IsAnalogChanged(bc->pressure[2], bp->pressure[2], 50) |
                IsAnalogChanged(bc->pressure[3], bp->pressure[3], 50),
                &p_wpd->diffCountfsAcc, &p_wpd->hystCountfsAcc, WPAD_CHANGE_EXTENSION);
            break;
        }
        case 13:
            if (p_wpd->trnFormatSelector) result |= ((const WPADVSStatus*)p_curr)->button > 200;
            break;
        case 16: {
            const WPADMPStatus* mc = (const WPADMPStatus*)p_curr;
            const WPADMPStatus* mp = (const WPADMPStatus*)p_prev;
            result |= __wpadCheckChangeCount(
                IsAnalogChanged(mc->rotation[0], mp->rotation[0], 64) |
                IsAnalogChanged(mc->rotation[1], mp->rotation[1], 64) |
                IsAnalogChanged(mc->rotation[2], mp->rotation[2], 64),
                &p_wpd->diffCountMpls, &p_wpd->hystCountMpls, WPAD_CHANGE_MPLS);
            if ((mc->status & 1) && (mp->status & 1)) {
                if (p_wpd->devMode == 5) {
                    const WPADFSStatus* fc = (const WPADFSStatus*)p_curr;
                    const WPADFSStatus* fp = (const WPADFSStatus*)p_prev;
                    result |= __wpadCheckChangeCount(
                        IsAnalogChanged(fc->fsAccX, fp->fsAccX, 12) |
                        IsAnalogChanged(fc->fsAccY, fp->fsAccY, 12) |
                        IsAnalogChanged(fc->fsAccZ, fp->fsAccZ, 12),
                        &p_wpd->diffCountfsAcc, &p_wpd->hystCountfsAcc, WPAD_CHANGE_EXTENSION);
                    result |= IsAnalogChanged(fc->fsStickX, fp->fsStickX, 1);
                    result |= IsAnalogChanged(fc->fsStickY, fp->fsStickY, 1);
                } else if (p_wpd->devMode == 7) {
                    const WPADCLStatus* cc = (const WPADCLStatus*)p_curr;
                    const WPADCLStatus* cp = (const WPADCLStatus*)p_prev;
                    result |= IsButtonChanged(cc->clButton, cp->clButton);
                    result |= IsAnalogChanged(cc->clLStickX / 16, cp->clLStickX / 16, 1);
                    result |= IsAnalogChanged(cc->clLStickY / 16, cp->clLStickY / 16, 1);
                    result |= IsAnalogChanged(cc->clRStickX / 32, cp->clRStickX / 32, 1);
                    result |= IsAnalogChanged(cc->clRStickY / 32, cp->clRStickY / 32, 1);
                    result |= IsAnalogChanged(cc->clTriggerL / 8, cp->clTriggerL / 8, 1);
                    result |= IsAnalogChanged(cc->clTriggerR / 8, cp->clTriggerR / 8, 1);
                }
            }
            break;
        }
        }
    }
    return result;
}

void WPADRecalibrate(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable = OSDisableInterrupts();
    p_wpd->isSetStickOrigin = 0;
    OSRestoreInterrupts(enable);
}

// NONMATCHING: compiler register allocation and scheduling differ.
void __wpadCalcRecalibration(s32 chan, const WPADStatus* current) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    u32 count = 0;
    u16 classicButtons = 0;
    if (current->err == WPAD_ERR_NONE || current->err == -7) {
        if (current->button == 0x1C10 &&
            ((p_wpd->dataFormat >= 3 && p_wpd->dataFormat <= 8) || p_wpd->dataFormat == 16)) count = 1;
        if (p_wpd->devType == 2) classicButtons = ((const WPADCLStatus*)current)->clButton;
        if (p_wpd->devType == 7) classicButtons = ((const WPADCLStatus*)current)->clButton;
        if (classicButtons == 0x1450 &&
            ((p_wpd->dataFormat >= 6 && p_wpd->dataFormat <= 8) || p_wpd->dataFormat == 16)) count = 1;
        p_wpd->recalibrateCount += count;
        if (p_wpd->recalibrateCount > 600) {
            WPADControlBlock* controller = _wpdcb[chan];
            BOOL enabled = OSDisableInterrupts();
            controller->isSetStickOrigin = 0;
            controller->recalibrateCount = 0;
            OSRestoreInterrupts(enabled);
        }
    }
}

static void __SetScreenSaverFlag(BOOL flag) {
    if (flag) {
        __VIResetRFIdle();
    }
}

void __wpadCalcControllerData(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL screen = FALSE;
    if (_checkCnt == 5) {
        BOOL changed;
        const WPADStatus* previous;
        BOOL enabled = OSDisableInterrupts();
        u8 index = (u8)(p_wpd->rxBufIndex ? 0 : 1);
        const WPADStatus* current = (const WPADStatus*)(p_wpd->rxBuf + index * WPAD_RX_DATASIZE);
        OSRestoreInterrupts(enabled);
        changed = __wpadIsControllerDataChanged(p_wpd, current, p_wpd->prBuf);
        __wpadCalcRecalibration(chan, current);
        if (changed) {
            screen = TRUE;
            p_wpd->lastUpdateTime = __OSGetSystemTime();
            memcpy(p_wpd->prBuf, current, WPAD_RX_DATASIZE);
        } else if (_sleepTime > 0) {
            int seconds = (int)OSTicksToSeconds(__OSGetSystemTime() - p_wpd->lastUpdateTime);
            if (seconds > 60 * _sleepTime) WPADiDisconnect(chan, FALSE);
        }
        previous = (const WPADStatus*)p_wpd->prBuf;
        if (previous->err != WPAD_ERR_NONE && previous->err != -7) memcpy(p_wpd->prBuf, current, WPAD_RX_DATASIZE);
        __SetScreenSaverFlag(screen);
    }
}

s8 __GetCmdNumber(WPADCmdQueue* queue) {
    BOOL enable;
    s8 num;

    enable = OSDisableInterrupts();

    num = (s8)(queue->tail - queue->head);
    if (num < 0) {
        num += queue->cmdlen;
    }

    OSRestoreInterrupts(enable);
    return num;
}

BOOL WPADiIsAvailableCmdQueue(WPADCmdQueue* queue, s8 cmd_num) {
    s8 curr = __GetCmdNumber(queue);

    if (curr + cmd_num <= queue->cmdlen - 1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL WPADiPushCommand(WPADCmdQueue* queue, WPADCommand cmd) {
    BOOL enable;

    enable = OSDisableInterrupts();

    if (__GetCmdNumber(queue) == queue->cmdlen - 1) {
        OSRestoreInterrupts(enable);
        return FALSE;
    }

    memset(queue->cmd + queue->tail, 0, sizeof(WPADCommand));
    memcpy(queue->cmd + queue->tail, &cmd, sizeof(WPADCommand));
    queue->tail = (s8)((queue->tail == queue->cmdlen - 1) ? 0 : queue->tail + 1);
    OSRestoreInterrupts(enable);
    return TRUE;
}

BOOL WPADiSendDPDCSB(WPADCmdQueue* queue, BOOL enable, WPADCallback callback) {
    WPADCommand cmd;
    BOOL result;

    cmd.command = WPAD_HIDREP_DPDCSB;
    cmd.len = 1;
    cmd.data[0] = (u8)((enable) ? (u8)4 : (u8)0);
    cmd.callback = callback;

    result = WPADiPushCommand(queue, cmd);
    return result;
}

static void WPADiRumbleMotor(s32 chan, BOOL sendCmd) {
    WPADCommand cmd;

    if (sendCmd == TRUE || __GetCmdNumber(&_wpdcb[chan]->cmdq) > 0) {
        _wpdcb[chan]->rumble = FALSE;
    } else if (_rumbleCnt[chan] == 5) {
        _wpdcb[chan]->rumble = FALSE;

        cmd.command = WPAD_HIDREP_VIBRATOR;
        cmd.len = 1;
        cmd.data[0] = 0;
        cmd.callback = NULL;

        __wpadSendDataSub(chan, cmd);
    }
    _rumbleCnt[chan] = (u8)((_wpdcb[chan]->rumble) ? _rumbleCnt[chan] + 1 : 0);
}

static u8 __GetDpdSensitivity() {
    u8 level;

    level = (u8)SCGetBtDpdSensibility();
    if (level < 1) {
        level = 1;
    }
    if (level > 5) {
        level = 5;
    }

    return level;
}

static u8 __GetSensorBarPosition() {
    u8 pos;

    if (1 == SCGetWpadSensorBarPosition()) {
        pos = WPAD_SENSOR_BAR_POS_TOP;
    } else {
        pos = WPAD_SENSOR_BAR_POS_BOTTOM;
    }
    return pos;
}

static BOOL __GetMotorMode() {
    BOOL rumble;

    if (1 == SCGetWpadMotorMode()) {
        rumble = TRUE;
    } else {
        rumble = FALSE;
    }
    return rumble;
}

static u8 __ClampSpeakerVolume(u8 vol) {
    u8 v = vol;

    if (vol <= 0) {
        v = 0;
    }
    if (vol >= 127) {
        v = 127;
    }

    return v;
}

u16 _WPADGetStackBufferStatus(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable;
    s32 status;
    s8 handle;

    enable = OSDisableInterrupts();
    status = p_wpd->status;
    handle = p_wpd->dev_handle;
    OSRestoreInterrupts(enable);

    if (status == WPAD_ERR_NO_CONTROLLER) {
        return 0;
    } else {
        return _WUDGetQueuedSize(handle);
    }
}

u16 _WPADGetModuleBufferStatus(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable;
    s32 status;
    s8 handle;

    enable = OSDisableInterrupts();
    status = p_wpd->status;
    handle = p_wpd->dev_handle;
    OSRestoreInterrupts(enable);

    if (status == WPAD_ERR_NO_CONTROLLER) {
        return 0;
    } else {
        return _WUDGetNotAckedSize(handle);
    }
}

static u8 __GetSpeakerVolume() {
    u8 vol;
    vol = SCGetWpadSpeakerVolume();
    vol = __ClampSpeakerVolume(vol);
    return vol;
}

static void WPADiGetScSettings() {
    if (_scSetting) {
        if (SCCheckStatus() == 0) {
            _dpdSensitivity = __GetDpdSensitivity();
            _sensorBarPos = __GetSensorBarPosition();
            _rumble = __GetMotorMode();
            _speakerVolume = __GetSpeakerVolume();
            _scSetting = 0;
        }
    }
}

static OSShutdownFunctionInfo ShutdownFunctionInfo = {OnShutdown, 127};

static void WPADiAfh() {
    BOOL enable;
    u8* channel = (u8*)OSPhysicalToCached(0x31A2);

    DCInvalidateRange(channel, 1);

    if (_afhChannel != *channel) {
        enable = OSDisableInterrupts();
        _afhChannel = (s8)*channel;
        OSRestoreInterrupts(enable);
        WUDSetDisableChannel(_afhChannel);
    }
}


static void __reconnect(BOOL exec) {
    DEBUGPrint("Reconnect Start!!\n");
    BTA_DmSendHciReset();
    WPADiShutdown(exec);
}

static BOOL WPADiProcessExtCommand(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    WPADCommand cmd;
    s32 err;
    if (WPADiGetCommand(&p_wpd->extq, &cmd)) {
        if (cmd.command == WPAD_HIDREP_REPTYPE || p_wpd->infos.attach) {
            err = WPADiSendData(chan, cmd);
            if (err == WPAD_ERR_NONE) {
                WPADiPopCommand(&p_wpd->extq);
                _extCnt[chan] = 0;
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL WPADiSendGetContStat(WPADCmdQueue* queue, WPADInfo* info, WPADCallback callback) {
    WPADCommand cmd;
    BOOL result;

    cmd.command = WPAD_HIDREP_GETSTAT;
    cmd.len = 1;
    cmd.data[0] = 0;
    cmd.callback = callback;
    cmd.info = info;

    result = WPADiPushCommand(queue, cmd);
    return result;
}

BOOL WPADiSendSetPort(WPADCmdQueue* queue, u8 pattern, WPADCallback callback) {
    WPADCommand cmd;
    BOOL result;

    cmd.command = WPAD_HIDREP_PORT;
    cmd.len = 1;
    cmd.data[0] = (u8)(pattern << 4);
    cmd.callback = callback;

    result = WPADiPushCommand(queue, cmd);
    return result;
}

BOOL WPADiGetCommand(WPADCmdQueue* queue, WPADCommand* cmd) {
    BOOL enabled;
    if (__GetCmdNumber(queue) == 0) return FALSE;
    enabled = OSDisableInterrupts();
    memcpy(cmd, queue->cmd + queue->head, sizeof(WPADCommand));
    OSRestoreInterrupts(enabled);
    return TRUE;
}

BOOL WPADiPopCommand(WPADCmdQueue* queue) {
    BOOL enable = OSDisableInterrupts();

    if (__GetCmdNumber(queue) == 0) {
        OSRestoreInterrupts(enable);
        return FALSE;
    }

    memset(queue->cmd + queue->head, 0, sizeof(WPADCommand));
    queue->head = (s8)((queue->head == queue->cmdlen - 1) ? 0 : queue->head + 1);

    OSRestoreInterrupts(enable);
    return TRUE;
}

static BOOL WPADiProcessCommand(s32 chan, BOOL sent) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    WPADCommand cmd;
    s32 err;

    if (sent) return TRUE;
    if (WPADiGetCommand(&p_wpd->cmdq, &cmd)) {
        err = WPADiSendData(chan, cmd);
        if (err == WPAD_ERR_NONE) {
            WPADiPopCommand(&p_wpd->cmdq);
            return TRUE;
        }
    }
    return FALSE;
}

static inline BOOL WPADiCheckReconnect(void) {
    BOOL reconnect = _recFlag;
    if (reconnect >= 0) {
        if (--_recCnt <= 0) {
            BTA_DmSendHciReset();
            OSCancelAlarm(&_managerAlarm);
            WUDSetHidRecvCallback(NULL);
            WUDShutdown(reconnect);
        }
        return TRUE;
    }
    return FALSE;
}

// NONMATCHING: compiler inlining, save-gpr range and stack scheduling differ.
void WPADiManageHandler(OSAlarm*, OSContext*) {
    s32 chan;
    s32 status = WUDGetStatus();
    if (status != WPAD_STATE_SETUP) {
        if (status == WPAD_STATE_ENABLED && !_initialized) {
            _initialized = TRUE;
            _recCnt = 50;
            WUDSetHidConnCallback(__wpadConnectionCallback);
            WUDSetHidRecvCallback(__wpadReceiveCallback);
        }
        return;
    }
    if (WPADiCheckReconnect()) return;
    if (_afhCnt == 60000) WPADiAfh();
    if (SCCheckStatus() == 0) {
        WUDUpdateSCSetting();
        if (_scSetting) {
            _dpdSensitivity = __GetDpdSensitivity();
            _sensorBarPos = __GetSensorBarPosition();
            _rumble = __GetMotorMode();
            _speakerVolume = __GetSpeakerVolume();
            _scSetting = 0;
        }
    }
    for (chan = 0; chan < WPAD_MAX_CONTROLLERS; ++chan) {
        u8 device;
        WPADControlBlock* controller = _wpdcb[chan];
        if (controller->used) {
            BOOL sent = FALSE;
            if (_extCnt[chan] == 5) {
                sent = WPADiProcessExtCommand(chan);
            }
            _extCnt[chan] = (u8)(_extCnt[chan] == 5 ? _extCnt[chan] : _extCnt[chan] + 1);
            sent |= WPADiProcessCommand(chan, sent);
            WPADiRumbleMotor(chan, sent);
            __wpadCalcControllerData(chan);
            __wpadCalcRadioQuality(chan);
        }
        controller = _wpdcb[chan];
        if (controller->extensionReconnectPending) {
            if (controller->extensionReconnectTimer-- < 0) _wpdcb[chan]->extensionReconnectPending = 0;
        }
        __wpadCertWork(chan);
        device = _wpdcb[chan]->devType;
        if ((u8)(device - 5) <= 2 || device == 250) {
            controller = _wpdcb[chan];
            if (controller->mplsInitTicks < 200) ++controller->mplsInitTicks;
        }
    }
    _senseCnt = (u16)(_senseCnt == 10 ? 0 : _senseCnt + 1);
    _checkCnt = (u8)(_checkCnt == 5 ? 0 : _checkCnt + 1);
    _afhCnt = (u16)(_afhCnt == 60000 ? 0 : _afhCnt + 1);
    BTA_HhGetAclQueueInfo();
}

u8 __WPADiManageHandlerStack[4096] ATTRIBUTE_ALIGN(32);

void WPADiManageHandler0(OSAlarm* alarm, OSContext* context) {
    OSSwitchFiberEx((u32)alarm, (u32)context, 0, 0, (u32)WPADiManageHandler, (u32)(__WPADiManageHandlerStack + sizeof(__WPADiManageHandlerStack)));
}

static void __ClearControlBlock(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];

    p_wpd->infoBuf = NULL;
    p_wpd->motor = FALSE;

    p_wpd->resultCallback = NULL;
    p_wpd->extensionCallback = NULL;
    p_wpd->sampleCallback = NULL;
    p_wpd->samplingBufs_ptr = NULL;
    p_wpd->bufLatestIndex = 0;
    p_wpd->bufLength = 0;
    p_wpd->dataFormat = WPAD_FMT_CORE;
    p_wpd->status = WPAD_ERR_NO_CONTROLLER;
    p_wpd->devType = WPAD_DEV_NOT_FOUND;
    p_wpd->devMode = 0;
    p_wpd->isSetStickOrigin = 0;
    p_wpd->recalibrateCount = 0;
    p_wpd->getStatFlag = 0;
    p_wpd->dpdDummyObjSize = 12;
    p_wpd->dpdFormat = 0;
    p_wpd->dpdCommand = 0;

    p_wpd->diffCountAcc = 0;
    p_wpd->hystCountAcc = 0;
    p_wpd->diffCountDpd = 0;
    p_wpd->hystCountDpd = 0;
    p_wpd->diffCountfsAcc = 0;
    p_wpd->hystCountfsAcc = 0;
    p_wpd->diffCountMpls = 0;
    p_wpd->hystCountMpls = 0;

    p_wpd->lastUpdateTime = __OSGetSystemTime();
    p_wpd->cmdTimer = __OSGetSystemTime();
    p_wpd->cmdTimeoutAction = 0;
    p_wpd->reqVolume = 0;
    p_wpd->reqVolCb = NULL;

    p_wpd->readBufPtr = NULL;
    p_wpd->readBaseAddr = 0;
    p_wpd->readLength = 0;
    p_wpd->readError = 0;

    p_wpd->dev_handle = -1;
    p_wpd->used = FALSE;
    p_wpd->setup = FALSE;
    p_wpd->oldFw = FALSE;
    p_wpd->audioStop = 1;
    p_wpd->audioStopCnt = 0;
    p_wpd->audioFrames = 0;

    p_wpd->keyIdx = 0;
    p_wpd->radioSense = 0;
    p_wpd->packetCnt = 0;
    p_wpd->disconnect = 1;

    p_wpd->infoLocked = 0;
    p_wpd->infoCallback = NULL;

    p_wpd->pwrSave = 0;
    p_wpd->boardBatteryLevel = 4;
    p_wpd->extensionPreviousType = 253;
    p_wpd->extensionReconnectPending = 0;
    p_wpd->extensionReconnectTimer = 0;
    p_wpd->dpdExtendedReportMask = 0;

    memset(&p_wpd->infos, 0, sizeof(WPADInfo));
    memset(p_wpd->readBuf, 0, sizeof(p_wpd->readBuf));
    memset(&p_wpd->conf, 0, sizeof(WPADConfig));
    memset(&p_wpd->extConf, 0, sizeof(WPADExtConfig));
    memset(&p_wpd->key, 0, sizeof(p_wpd->key));
    memset(&p_wpd->ft, 0, sizeof(p_wpd->ft));
    memset(&p_wpd->sb, 0, sizeof(p_wpd->sb));
    memset(&p_wpd->gameInfo, 0, sizeof(WPADMEMGameInfo));
    p_wpd->rxBufIndex = 0;
    memset(p_wpd->rxBuf, 0, WPAD_RX_DATASIZE * 2);
    ((WPADStatus*)p_wpd->rxBuf)->err = WPAD_ERR_NO_CONTROLLER;
    ((WPADStatus*)(p_wpd->rxBuf + WPAD_RX_DATASIZE))->err = WPAD_ERR_NO_CONTROLLER;
    memcpy(p_wpd->prBuf, p_wpd->rxBuf, WPAD_RX_DATASIZE);

    p_wpd->gameInfoErr[0] = WPAD_ERR_NO_CONTROLLER;
    p_wpd->gameInfoErr[1] = WPAD_ERR_NO_CONTROLLER;
    p_wpd->cmdq.cmd = p_wpd->cmds;
    p_wpd->cmdq.cmdlen = (u32)(sizeof(p_wpd->cmds) / sizeof(WPADCommand));
    p_wpd->extq.cmd = p_wpd->exts;
    p_wpd->extq.cmdlen = (u32)(sizeof(p_wpd->exts) / sizeof(WPADCommand));
    WPADiClearQueue(&p_wpd->cmdq);
    WPADiClearQueue(&p_wpd->extq);

    p_wpd->mplsReinitPending = 0;
    p_wpd->mplsControlBusy = 0;
    p_wpd->mplsModeReadback[0] = 0;
    p_wpd->mplsModeReadback[1] = 0;
    p_wpd->mplsRequestedMode = 0;
    p_wpd->mplsControlRetries = 3;
    p_wpd->mplsDiscardReports = 5;
    p_wpd->mplsControlCallback = NULL;
    p_wpd->mplsInitTicks = 0;

    p_wpd->certChallengeBit = -1;
    p_wpd->certBusy = 0;
    p_wpd->certState = -1;
    p_wpd->certValidity = 0;
    p_wpd->certWorkDelay = 3000;
    p_wpd->certTransferRetries = 2;
    p_wpd->certParamBytes = 0;
    p_wpd->certParam = NULL;
    memset(p_wpd->certParamX, 0, sizeof(p_wpd->certParamX));
    memset(p_wpd->certParamY, 0, sizeof(p_wpd->certParamY));
    memset(p_wpd->certWork, 0, sizeof(p_wpd->certWork));
    p_wpd->certWork[0] = 1;
    p_wpd->certParamY[0] = 1;
    p_wpd->certParamX[0] = 1;

    p_wpd->extensionErrorCountdown = 0;
    p_wpd->extensionDataError = -1;
    p_wpd->extensionDataDeviceType = 253;
    p_wpd->extensionDataLength = 0;
    memset(p_wpd->extensionData, 0, sizeof(p_wpd->extensionData));
    WPADiClearMemoryControl(chan);
    // Meaning remains unknown; the target explicitly clears AE4 here.
    p_wpd->reservedControllerWord = 0;

    _extCnt[chan] = 0;
    _rumbleCnt[chan] = 0;
}

BOOL __SetSensorBarPower(BOOL flag) {
    u32 reg;
    u32 regBak;
    BOOL enable;
    BOOL result;

    enable = OSDisableInterrupts();

    regBak = ACRReadReg(0xC0);

    if (flag) {
        reg = regBak | 0x100;
    } else {
        reg = regBak & ~0x100;
    }

    ACRWriteReg(0xC0, reg);
    result = (regBak & 0x100) ? TRUE : FALSE;
    OSRestoreInterrupts(enable);
    return result;
}

// NONMATCHING: BSS/global relocation identities; all nonrelocated instruction words are exact.
void WPADiInitSub() {
    s32 chan;
    int i;

    __SetSensorBarPower(TRUE);

    for (i = 0; i < 16; i++) {
        _dev_handle_index[i] = -1;
    }

    for (chan = 0; chan < WPAD_MAX_CONTROLLERS; ++chan) {
        _wpdcb[chan] = &_wpd[chan];
        _chan_active_state[chan] = 0;
        _wpdcb[chan]->connectCallback = NULL;
        __ClearControlBlock(chan);
        OSInitThreadQueue(&_wpd[chan].threadq);
        _wpdcb[chan]->certCachedValidity = 0;

        _extCnt[chan] = 0;
        _rumbleCnt[chan] = 0;
    }

    _sleepTime = 5;
    _gamecode = OSGetAppGamename();
    _gametype = OSGetAppType();
    _dpdSensitivity = __GetDpdSensitivity();
    _sensorBarPos = __GetSensorBarPosition();
    _rumble = __GetMotorMode();
    _speakerVolume = __GetSpeakerVolume();
    _senseCnt = 0;
    _checkCnt = 0;
    _afhCnt = 0;
    _shutdown = 0;
    _scSetting = 1;
    _afhChannel = -1;

    _callbackByKPAD = FALSE;

    OSRegisterVersion(__WPADVersion);
    if (__wpadOptionalInitCallback0) __wpadOptionalInitCallback0();
    if (__wpadOptionalInitCallback1) __wpadOptionalInitCallback1();
    if (__wpadOptionalInitCallback2) __wpadOptionalInitCallback2();
    if (__wpadOptionalInitCallback3) __wpadOptionalInitCallback3();
    if (__wpadOptionalInitCallback4) __wpadOptionalInitCallback4();
    if (__wpadOptionalInitCallback5) __wpadOptionalInitCallback5();
    if (__wpadOptionalInitCallback6) __wpadOptionalInitCallback6();

    OSCreateAlarm(&_managerAlarm);
    OSSetPeriodicAlarm(&_managerAlarm, OSGetTime(), OSMillisecondsToTicks(1), WPADiManageHandler0);
}

void WPADInit(void) {
    BOOL result;

    _startup = TRUE;

    if (_regShutdown == 0) {
        OSRegisterShutdownFunction(&ShutdownFunctionInfo);
        _regShutdown = 1;
    }

    result = WUDInit();

    if (result) {
        _initialized = FALSE;
        _recFlag = -1;
        _recCnt = 50;
        WPADiInitSub();
    }
}

// These SDK entry points are retained in the retail binary for the product.sel export symbol table.
#pragma push
#pragma force_active on
BOOL WPADStartFastSimpleSync() {
    return WUDStartFastSyncSimple();
}

BOOL WPADStopSimpleSync() {
    return WUDStopSyncSimple();
}

WPADSyncDeviceCallback WPADSetSimpleSyncCallback(WPADSyncDeviceCallback callback) {
    return WUDSetSyncSimpleCallback(callback);
}
#pragma pop

void WPADRegisterAllocator(WPADAlloc alloc, WPADFree free) {
    WUDRegisterAllocator(alloc, free);
}

u32 WPADGetWorkMemorySize(void) {
    return WUDGetAllocatedMemSize();
}

s32 WPADGetStatus() {
    return WUDGetStatus();
}

u8 WPADGetSensorBarPosition() {
    BOOL enable;
    u8 pos;

    enable = OSDisableInterrupts();
    pos = _sensorBarPos;
    OSRestoreInterrupts(enable);
    return pos;
}

static void setupCallback(s32 chan, s32 result) {
    WPADControlBlock* p_wpd = _wpdcb[chan];

    if (result == WPAD_ERR_NONE) {
        p_wpd->setup = TRUE;

        if (p_wpd->connectCallback) {
            p_wpd->connectCallback(chan, result);
        }
    } else {
        WPADiDisconnect(chan, FALSE);
    }
}

static void __wpadAbortConnectionCallback(s32 chan, s32 result) {
    WPADControlBlock* controller = _wpdcb[chan];
    if (result != WPAD_ERR_NONE) {
        WPADiClearQueue(&controller->cmdq);
        WPADiDisconnect(chan, FALSE);
    }
}

static void firmwareCheckCallback(s32 chan, s32 result) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    u16 size;
    u32 addr;
    u8 port;
    BOOL enable;

    if (result == WPAD_ERR_NO_CONTROLLER) {
        return;
    }

    enable = OSDisableInterrupts();
    p_wpd->oldFw = (result == WPAD_ERR_NONE) ? TRUE : FALSE;
    p_wpd->status = WPAD_ERR_NONE;
    OSRestoreInterrupts(enable);
    size = (u16)((result == WPAD_ERR_NONE) ? 20 : 42);
    addr = (u32)((result == WPAD_ERR_NONE) ? 0x176c : 0);
    port = (u8)(p_wpd->devType == 3 ? 1 : 0x01 << chan);
    WPADiSendSetReportType(&p_wpd->cmdq, WPAD_FMT_CORE, p_wpd->pwrSave, __wpadAbortConnectionCallback);
    WPADiSendDPDCSB(&p_wpd->cmdq, FALSE, __wpadAbortConnectionCallback);
    WPADiSendSetPort(&p_wpd->cmdq, port, __wpadAbortConnectionCallback);
    WPADiSendReadData(&p_wpd->cmdq, p_wpd->readBuf, sizeof(WPADMEMGameInfo), 0x2A, __wpadAbortConnectionCallback);
    WPADiSendReadData(&p_wpd->cmdq, p_wpd->readBuf, sizeof(WPADMEMGameInfo), 0x2A + sizeof(WPADMEMGameInfo), __wpadAbortConnectionCallback);
    WPADiSendReadData(&p_wpd->cmdq, p_wpd->readBuf, size, addr, setupCallback);
    WPADiSendGetContStat(&p_wpd->cmdq, NULL, NULL);
}

// NONMATCHING: compiler register allocation and scheduling differ.
s32 __wpadRetrieveChannel(WUDDevInfo* info) {
    u8* address;
    s32 chan;
    s32 selected = -1;
    address = _WUDGetDevAddr(info->devHandle);
    if (!memcmp(info->conf.devName, "Nintendo RVL-WBC", 16)) {
        selected = 3;
        if (_wpdcb[3]->used) {
            btm_remove_acl(address);
            return -1;
        }
    } else {
        for (chan = 0; chan < WPAD_MAX_CONTROLLERS; ++chan) {
            if (WUDIsLatestDevice(chan, address) && !_chan_active_state[chan]) {
                selected = chan;
                break;
            }
            if (!_chan_active_state[chan] && selected < 0) selected = chan;
        }
    }
    if (!WUDIsLatestDevice(selected, address)) WUDSetDeviceHistory(selected, address);
    // ORIGINAL BUG (SDK): writes active[-1] when no slot is free after a dropped HID-close notification; see PORT-PLAN hazards.
    _chan_active_state[selected] = 1;
    return selected;
}

// NONMATCHING: compiler register allocation and struct-copy scheduling differ.
static void __wpadConnectionCallback(WUDDevInfo* info, u8 open) {
    u8 dev_handle = info->devHandle;
    s32 chan = -1;
    WPADControlBlock* p_wpd;
    WPADCommand cmd;

    if (open) {
        chan = __wpadRetrieveChannel(info);
        if (chan < 0) return;
        _dev_handle_index[dev_handle] = (s8)(chan & 0xff);
        __ClearControlBlock(chan);
        p_wpd = _wpdcb[chan];

        if (!memcmp(info->conf.devName, "Nintendo RVL-CNT", 16)) {
            p_wpd->devType = WPAD_DEV_CORE;
            p_wpd->dataFormat = WPAD_FMT_CORE;
        } else if (!memcmp(info->conf.devName, "Nintendo RVL-WBC", 16) && WUDIsLinkedWBC()) {
            p_wpd->devType = 3;
            p_wpd->dataFormat = 12;
        } else {
            p_wpd->devType = WPAD_DEV_FUTURE;
            p_wpd->dataFormat = WPAD_FMT_CORE;
        }
        p_wpd->dev_handle = (s8)dev_handle;
        p_wpd->used = TRUE;
        p_wpd->status = WPAD_ERR_NONE;
        p_wpd->radioSense = 100;
        p_wpd->disconnect = 0;
        p_wpd->extEnc = 0;
        p_wpd->extensionPreviousType = p_wpd->devType;
        WPADiSendReadData(&p_wpd->cmdq, p_wpd->readBuf, 1, 0x1770, firmwareCheckCallback);
        __SetScreenSaverFlag(TRUE);
    } else {
        chan = _dev_handle_index[dev_handle];
        _dev_handle_index[dev_handle] = -1;

        if (chan != -1) {
            p_wpd = _wpdcb[chan];
            p_wpd->status = WPAD_ERR_NO_CONTROLLER;

            if (p_wpd->resultCallback) {
                p_wpd->resultCallback(chan, WPAD_ERR_NO_CONTROLLER);
            } else if (p_wpd->memoryControl.callback) {
                p_wpd->memoryControl.callback(chan, WPAD_ERR_NO_CONTROLLER);
            }
            while (WPADiGetCommand(&p_wpd->cmdq, &cmd)) {
                if (cmd.callback) {
                    cmd.callback(chan, WPAD_ERR_NO_CONTROLLER);
                }
                WPADiPopCommand(&p_wpd->cmdq);
            }

            if (p_wpd->samplingBufs_ptr) {
                WPADSetAutoSamplingBuf(chan, p_wpd->samplingBufs_ptr, p_wpd->bufLength);
            }

            __ClearControlBlock(chan);
            _chan_active_state[chan] = 0;

            if (p_wpd->connectCallback) {
                p_wpd->connectCallback(chan, WPAD_ERR_NO_CONTROLLER);
            }
        }
    }
}

static void __wpadReceiveCallback(u8 dev_handle, u8* report, u16 length) {
#pragma unused(length)
    if (dev_handle <= 16) {
        u8 chan = (u8)_dev_handle_index[dev_handle];
        if (chan < WPAD_MAX_CONTROLLERS) WPADiHIDParser(chan, report);
    }
}

void WPADGetAccGravityUnit(s32 chan, u32 type, WPADAcc* acc) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable;

    enable = OSDisableInterrupts();
    if (acc) {
        switch (type) {
        case WPAD_DEV_CORE:
            acc->x = (s16)(p_wpd->conf.acc_1g.x - p_wpd->conf.acc_0g.x);
            acc->y = (s16)(p_wpd->conf.acc_1g.y - p_wpd->conf.acc_0g.y);
            acc->z = (s16)(p_wpd->conf.acc_1g.z - p_wpd->conf.acc_0g.z);
            break;

        case WPAD_DEV_FREESTYLE:
            acc->x = (s16)(p_wpd->extConf.fs.acc_1g.x - p_wpd->extConf.fs.acc_0g.x);
            acc->y = (s16)(p_wpd->extConf.fs.acc_1g.y - p_wpd->extConf.fs.acc_0g.y);
            acc->z = (s16)(p_wpd->extConf.fs.acc_1g.z - p_wpd->extConf.fs.acc_0g.z);
            break;
        }
    }
    OSRestoreInterrupts(enable);
}

void WPADiGetMplsCalibration(s32 chan, WPADMplsCalibration* low, WPADMplsCalibration* high) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable = OSDisableInterrupts();

    if (low != NULL && high != NULL) {
        *low = p_wpd->extConf.mplsCalibration[0];
        *high = p_wpd->extConf.mplsCalibration[1];
    }
    OSRestoreInterrupts(enable);
}

static void CloseCallback(s32 chan, s32 result) {
    WPADControlBlock* p_wpd = _wpdcb[chan];

    if (result != WPAD_ERR_NO_CONTROLLER) {
        BTA_HhClose((u8)p_wpd->dev_handle);
    }
}

void WPADDisconnect(s32 chan) {
    if (WPADiGetStatus(chan) != WPAD_ERR_NO_CONTROLLER) {
        WUDSetDeviceHistory(chan, NULL);
        WPADiDisconnect(chan, TRUE);
    }
}

void WPADSetAutoSleepTime(u8 minute) {
    BOOL enable = OSDisableInterrupts();
    _sleepTime = minute;
    OSRestoreInterrupts(enable);
}

s32 WPADProbe(s32 chan, u32* type) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    s32 status;
    BOOL enable = OSDisableInterrupts();
    if (type != NULL) {
        *type = p_wpd->devType;
    }
    status = p_wpd->status;
    if (status != WPAD_ERR_NO_CONTROLLER) {
        if (p_wpd->devType == WPAD_DEV_NOT_FOUND) {
            status = WPAD_ERR_NO_CONTROLLER;
        } else if (!p_wpd->setup) {
            status = WPAD_ERR_BUSY;
        }
    }
    OSRestoreInterrupts(enable);
    return status;
}

// NONMATCHING: warning-string relocation identities only; instruction words otherwise exact.
WPADSamplingCallback WPADSetSamplingCallback(s32 chan, WPADSamplingCallback callback) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    WPADSamplingCallback prev;
    BOOL enable;
    if (_callbackByKPAD) {
        OSReport("WARNING: Overwritten the callback needed by KPAD.\n");
        OSReport("         Please call KPADSetSamplingCallback instead of WPADSetSamplingCallback.\n");
    }
    enable = OSDisableInterrupts();
    prev = p_wpd->sampleCallback;
    p_wpd->sampleCallback = callback;
    OSRestoreInterrupts(enable);
    return prev;
}

// NONMATCHING: warning-string relocation identities only; instruction words otherwise exact.
WPADConnectCallback WPADSetConnectCallback(s32 chan, WPADConnectCallback callback) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    WPADConnectCallback prev;
    BOOL enable;
    if (_callbackByKPAD) {
        OSReport("WARNING: Overwritten the callback needed by KPAD.\n");
        OSReport("         Please call KPADSetConnectCallback instead of WPADSetConnectCallback.\n");
    }
    enable = OSDisableInterrupts();
    prev = p_wpd->connectCallback;
    p_wpd->connectCallback = callback;
    OSRestoreInterrupts(enable);
    return prev;
}

WPADExtensionCallback WPADSetExtensionCallback(s32 chan, WPADExtensionCallback callback) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    WPADExtensionCallback prev;
    BOOL enable;

    enable = OSDisableInterrupts();
    prev = p_wpd->extensionCallback;
    p_wpd->extensionCallback = callback;
    OSRestoreInterrupts(enable);

    return prev;
}

u32 WPADGetDataFormat(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    u32 fmt;
    BOOL enable = OSDisableInterrupts();
    fmt = p_wpd->dataFormat;
    OSRestoreInterrupts(enable);
    return fmt;
}

s32 WPADSetDataFormat(s32 chan, u32 fmt) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    s32 ret;
    BOOL busy;
    BOOL enable;
    BOOL setup;
    u32 prevFmt;

    enable = OSDisableInterrupts();
    setup = p_wpd->setup;
    ret = p_wpd->status;
    prevFmt = p_wpd->dataFormat;
    OSRestoreInterrupts(enable);

    if (ret == WPAD_ERR_NO_CONTROLLER) {
        return WPAD_ERR_NO_CONTROLLER;
    }
    if (!setup) {
        return WPAD_ERR_BUSY;
    }
    if (prevFmt == fmt) {
        return WPAD_ERR_NONE;
    }
    busy = WPADiSendSetReportType(&p_wpd->cmdq, fmt, p_wpd->pwrSave, NULL);
    if (busy) {
        enable = OSDisableInterrupts();
        p_wpd->dataFormat = fmt;
        p_wpd->dpdExtendedReportMask = 0;
        OSRestoreInterrupts(enable);
        return WPAD_ERR_NONE;
    }
    return WPAD_ERR_BUSY;
}

void __infoCallback(s32 chan, s32 result) {
    WPADControlBlock* p_wpd = _wpdcb[chan];

    if (p_wpd->infoCallback) {
        p_wpd->infoCallback(chan, result);
    }
    p_wpd->infoCallback = NULL;
    p_wpd->infoLocked = 0;
}

void WPADiDisconnect(s32 chan, BOOL polite) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable;
    s32 status;

    status = WPADiGetStatus(chan);

    if (status != WPAD_ERR_NO_CONTROLLER) {
        if (polite) {
            enable = OSDisableInterrupts();
            if (p_wpd->disconnect) {
                OSRestoreInterrupts(enable);
                return;
            }
            p_wpd->disconnect = 1;
            OSRestoreInterrupts(enable);
            WPADControlLed(chan, 0, CloseCallback);
        } else {
            u8 addr[6];
            WPADGetAddress(chan, addr);
            btm_remove_acl(addr);
        }
    }
}

s32 WPADGetInfoAsync(s32 chan, WPADInfo* info, WPADCallback callback) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL busy;
    BOOL enable;
    BOOL setup;
    s32 result;
    u8 lock;

    enable = OSDisableInterrupts();
    setup = p_wpd->setup;
    result = p_wpd->status;
    lock = p_wpd->infoLocked;
    OSRestoreInterrupts(enable);

    if (result != WPAD_ERR_NO_CONTROLLER) {
        if (!setup || lock) {
            result = WPAD_ERR_BUSY;
        } else {
            enable = OSDisableInterrupts();
            p_wpd->infoLocked = 1;
            p_wpd->infoCallback = callback;
            OSRestoreInterrupts(enable);
            busy = WPADiSendGetContStat(&p_wpd->cmdq, info, __infoCallback);

            if (busy) {
                result = WPAD_ERR_NONE;
            } else {
                result = WPAD_ERR_BUSY;
                enable = OSDisableInterrupts();
                p_wpd->infoLocked = 0;
                p_wpd->infoCallback = NULL;
                OSRestoreInterrupts(enable);
            }
        }
    }

    if (result != WPAD_ERR_NONE) {
        if (callback) {
            callback(chan, result);
        }
    }

    return result;
}

void WPADControlMotor(s32 chan, u32 command) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable;
    BOOL needed = FALSE;
    s32 status;

    enable = OSDisableInterrupts();
    status = p_wpd->status;

    if (status == WPAD_ERR_NO_CONTROLLER) {
        OSRestoreInterrupts(enable);
        return;
    }

    if (_rumble == FALSE) {
        if (command != WPAD_MOTOR_STOP || p_wpd->motor != TRUE) {
            OSRestoreInterrupts(enable);
            return;
        }
    }

    if ((command == WPAD_MOTOR_STOP && p_wpd->motor == FALSE) || (command == WPAD_MOTOR_RUMBLE && p_wpd->motor == TRUE)) {
        OSRestoreInterrupts(enable);
        return;
    }

    p_wpd->motor = (command == WPAD_MOTOR_STOP) ? FALSE : TRUE;
    p_wpd->rumble = TRUE;
    OSRestoreInterrupts(enable);
}

// These SDK entry points are retained in the retail binary.
#pragma push
#pragma force_active on
void WPADEnableMotor(BOOL enable) {
    BOOL intr = OSDisableInterrupts();
    _rumble = enable;
    OSRestoreInterrupts(intr);
}

BOOL WPADIsMotorEnabled() {
    BOOL result;
    BOOL enable = OSDisableInterrupts();
    result = _rumble;
    OSRestoreInterrupts(enable);
    return result;
}
#pragma pop

s32 WPADControlLed(s32 chan, u8 pattern, WPADCallback callback) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL busy;
    BOOL enable;
    BOOL setup;
    s32 result = WPAD_ERR_NONE;

    enable = OSDisableInterrupts();
    result = p_wpd->status;
    setup = p_wpd->setup;
    OSRestoreInterrupts(enable);

    if (result != WPAD_ERR_NO_CONTROLLER) {
        if (!setup) {
            result = WPAD_ERR_BUSY;
        } else {
            busy = WPADiSendSetPort(&p_wpd->cmdq, pattern, callback);
            result = (busy) ? WPAD_ERR_NONE : WPAD_ERR_BUSY;
        }
    }

    if (result != WPAD_ERR_NONE) {
        if (callback) {
            callback(chan, result);
        }
    }
    return result;
}

// These SDK entry points are retained in the retail binary.
#pragma push
#pragma force_active on
BOOL WPADSaveConfig(WPADFlushCallback callback) {
    BOOL result = TRUE;
    BOOL enable;
    u8 volume;
    u8 mode;

    if (SCCheckStatus() != 0) {
        return FALSE;
    }

    enable = OSDisableInterrupts();
    volume = _speakerVolume;
    mode = (u8)((_rumble) ? 1 : 0);
    OSRestoreInterrupts(enable);

    result &= SCSetWpadSpeakerVolume(volume);
    result &= SCSetWpadMotorMode(mode);

    if (result) {
        SCFlushAsync(callback);
    } else if (callback) {
        callback(2);
    }
    return result;
}
#pragma pop

// Descriptive name; historical SDK spelling is unverified.
// NONMATCHING: generated switch-table relocation identity differs; all format sizes agree.
NO_INLINE u32 __wpadGetReportSize(u32 format) {
    switch (format) {
    case 3: case 4: case 5: return sizeof(WPADFSStatus);
    case 6: case 7: case 8: case 11: case 15: case 17: case 18: return sizeof(WPADCLStatus);
    case 19: return sizeof(WPADRawStatus);
    case 10: return 0x2E; // Core, button and the two percussion bytes.
    case 12: return 0x34; // Core, four pressures, temperature and battery.
    case 13: return sizeof(WPADVSStatus);
    case 9: return sizeof(WPADStatusEx);
    case 16: return 0x3E;
    default: return sizeof(WPADStatus);
    }
}

void WPADRead(s32 chan, void* status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADStatus* data;
    BOOL enabled = OSDisableInterrupts();
    u32 size;
    data = (WPADStatus*)(p_cb->rxBuf + (u8)(p_cb->rxBufIndex == 0) * WPAD_RX_DATASIZE);
    size = __wpadGetReportSize(p_cb->dataFormat);
    if (data->err != WPAD_ERR_NONE) size = sizeof(WPADStatus);
    memcpy(status, data, size);
    OSRestoreInterrupts(enabled);
}

// NONMATCHING: compiler loop lowering and register allocation differ.
void WPADSetAutoSamplingBuf(s32 chan, void* buf, u32 length) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    BOOL enabled = OSDisableInterrupts();
    s8 err = p_cb->status == WPAD_ERR_NO_CONTROLLER ? WPAD_ERR_NO_CONTROLLER : WPAD_ERR_INVALID;
    int size = __wpadGetReportSize(p_cb->dataFormat);
    int i;
    if (buf) {
        memset(buf, 0, size * length);
        for (i = 0; i < length; i++) ((WPADStatus*)((u8*)buf + i * size))->err = err;
        p_cb->bufLatestIndex = 0xFFFFFFFF;
        p_cb->bufLength = length;
    }
    p_cb->samplingBufs_ptr = buf;
    OSRestoreInterrupts(enabled);
}

void WPADiExcludeButton(s32 chan) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable;
    void* p_data;
    u8 index;
    WPADStatus* p_wp;
    WPADCLStatus* p_cp;

    enable = OSDisableInterrupts();
    index = (u8)((p_wpd->rxBufIndex) ? 0 : 1);
    p_data = (void*)(p_wpd->rxBuf + index * WPAD_RX_DATASIZE);

    p_wp = (WPADStatus*)p_data;
    if ((p_wp->button & WPAD_BTN_HCURSOR_MASK) == WPAD_BTN_HCURSOR_MASK) {
        p_wp->button &= ~WPAD_BUTTON_RIGHT;
    }
    if ((p_wp->button & WPAD_BTN_VCURSOR_MASK) == WPAD_BTN_VCURSOR_MASK) {
        p_wp->button &= ~WPAD_BUTTON_DOWN;
    }

    if (p_wpd->dataFormat - 6 <= 2 || p_wpd->dataFormat == 11 || p_wpd->dataFormat == 15 || p_wpd->dataFormat == 17) {
        p_cp = (WPADCLStatus*)p_data;
        if ((p_cp->clButton & WPAD_CLB_HCURSOR_MASK) == WPAD_CLB_HCURSOR_MASK) {
            p_cp->clButton &= ~WPAD_CL_BUTTON_RIGHT;
        }
        if ((p_cp->clButton & WPAD_CLB_VCURSOR_MASK) == WPAD_CLB_VCURSOR_MASK) {
            p_cp->clButton &= ~WPAD_CL_BUTTON_DOWN;
        }
    }

    if (p_wpd->dataFormat == 10) {
        WPADPercussionStatus* p_percussion = (WPADPercussionStatus*)p_data;
        if ((p_percussion->button & WPAD_CLB_HCURSOR_MASK) == WPAD_CLB_HCURSOR_MASK) p_percussion->button &= ~WPAD_CL_BUTTON_RIGHT;
        if ((p_percussion->button & WPAD_CLB_VCURSOR_MASK) == WPAD_CLB_VCURSOR_MASK) p_percussion->button &= ~WPAD_CL_BUTTON_DOWN;
    }
    OSRestoreInterrupts(enable);
}

// NONMATCHING: compiler duplicates the copy path instead of merging its length selection.
void WPADiCopyOut(s32 chan) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADStatus* data;
    void* output;
    u32 size;
    BOOL enabled = OSDisableInterrupts();
    data = (WPADStatus*)(p_cb->rxBuf + (u8)(p_cb->rxBufIndex == 0) * WPAD_RX_DATASIZE);
    size = __wpadGetReportSize(p_cb->dataFormat);
    if (p_cb->samplingBufs_ptr) {
        p_cb->bufLatestIndex++;
        if (p_cb->bufLatestIndex >= p_cb->bufLength) p_cb->bufLatestIndex = 0;
        output = (u8*)p_cb->samplingBufs_ptr + p_cb->bufLatestIndex * size;
        if (data->err != WPAD_ERR_NONE) size = sizeof(WPADStatus);
        memcpy(output, data, size);
    }
    if (p_cb->sampleCallback) p_cb->sampleCallback(chan);
    p_cb->packetCnt++;
    OSRestoreInterrupts(enabled);
}

BOOL WPADIsSpeakerEnabled(s32 chan) {
    BOOL state;
    BOOL enable;
    WPADControlBlock* p_wpd = _wpdcb[chan];
    enable = OSDisableInterrupts();
    state = p_wpd->infos.speaker;
    OSRestoreInterrupts(enable);
    return state;
}

BOOL WPADiSendMuteSpeaker(WPADCmdQueue* queue, BOOL enable, WPADCallback callback) {
    WPADCommand cmd;
    BOOL result;
    cmd.command = WPAD_HIDREP_MUTE;
    cmd.len = 1;
    cmd.data[0] = (u8)((enable) ? (u8)4 : (u8)0);
    cmd.callback = callback;
    result = WPADiPushCommand(queue, cmd);
    return result;
}

BOOL WPADiSendEnableSpeaker(WPADCmdQueue* queue, BOOL enable, WPADCallback callback) {
    WPADCommand cmd;
    BOOL result;
    cmd.command = WPAD_HIDREP_ENSPK;
    cmd.len = 1;
    cmd.data[0] = (u8)((enable) ? (u8)4 : (u8)0);
    cmd.callback = callback;
    result = WPADiPushCommand(queue, cmd);
    return result;
}

s32 WPADControlSpeaker(s32 chan, u32 command, WPADCallback callback) {
    BOOL enable;
    BOOL spk;
    BOOL setup;
    s32 result;
    u8 initCmd[] = {0x00, 0x00, 0xd0, 0x07, 0x40, 0x0c, 0x0e};
    WPADControlBlock* p_wpd = _wpdcb[chan];

    enable = OSDisableInterrupts();
    spk = p_wpd->infos.speaker;
    result = p_wpd->status;
    setup = p_wpd->setup;
    OSRestoreInterrupts(enable);

    if (result != WPAD_ERR_NO_CONTROLLER) {
        if (!setup) {
            result = WPAD_ERR_BUSY;
        } else if (command == 0) {
            if (!spk) {
                result = WPAD_ERR_NONE;
            } else {
                enable = OSDisableInterrupts();
                if (WPADiIsAvailableCmdQueue(&p_wpd->cmdq, 5)) {
                    WPADiSendMuteSpeaker(&p_wpd->cmdq, TRUE, NULL);
                    WPADiSendWriteDataCmd(&p_wpd->cmdq, SPK_DERESET, SPK_RST_REG, NULL);
                    WPADiSendWriteDataCmd(&p_wpd->cmdq, SPK_CLK_OFF, SPK_CLK_REG, NULL);
                    WPADiSendEnableSpeaker(&p_wpd->cmdq, FALSE, NULL);
                    WPADiSendGetContStat(&p_wpd->cmdq, NULL, callback);
                    OSRestoreInterrupts(enable);
                    return WPAD_ERR_NONE;
                } else {
                    result = WPAD_ERR_BUSY;
                }
                OSRestoreInterrupts(enable);
            }
        } else {
            switch (command) {
            case 1:
            case 5:
                enable = OSDisableInterrupts();
                if (WPADiIsAvailableCmdQueue(&p_wpd->cmdq, 7)) {
                    WPADiSendEnableSpeaker(&p_wpd->cmdq, TRUE, NULL);
                    WPADiSendMuteSpeaker(&p_wpd->cmdq, TRUE, NULL);
                    WPADiSendWriteDataCmd(&p_wpd->cmdq, SPK_CLK_ON, SPK_CLK_REG, NULL);
                    WPADiSendWriteDataCmd(&p_wpd->cmdq, SPK_RESET, SPK_RST_REG, NULL);
                    initCmd[4] = _speakerVolume;
                    WPADiSendWriteData(&p_wpd->cmdq, initCmd, sizeof(initCmd), SPK_RST_REG, NULL);
                    WPADiSendMuteSpeaker(&p_wpd->cmdq, FALSE, NULL);
                    WPADiSendGetContStat(&p_wpd->cmdq, NULL, callback);
                    OSRestoreInterrupts(enable);
                    return WPAD_ERR_NONE;
                } else {
                    result = WPAD_ERR_BUSY;
                }
                OSRestoreInterrupts(enable);
                break;

            case 2:
                if (!WPADiSendMuteSpeaker(&p_wpd->cmdq, TRUE, callback)) {
                    result = WPAD_ERR_BUSY;
                } else {
                    return WPAD_ERR_NONE;
                }
                break;

            case 3:
                if (!WPADiSendMuteSpeaker(&p_wpd->cmdq, FALSE, callback)) {
                    result = WPAD_ERR_BUSY;
                } else {
                    return WPAD_ERR_NONE;
                }
                break;

            case 4:
                if (!WPADiSendWriteDataCmd(&p_wpd->cmdq, SPK_CTRL_PLAY, SPK_CTRL_REG, callback)) {
                    result = WPAD_ERR_BUSY;
                } else {
                    return WPAD_ERR_NONE;
                }
                break;
            }
        }
    }

    if (callback) {
        callback(chan, result);
    }

    return result;
}

u8 WPADGetSpeakerVolume() {
    BOOL enable;
    u8 vol;

    enable = OSDisableInterrupts();
    vol = _speakerVolume;
    OSRestoreInterrupts(enable);

    return vol;
}

// These SDK entry points are retained in the retail binary.
#pragma push
#pragma force_active on
void WPADSetSpeakerVolume(u8 volume) {
    BOOL enable;

    enable = OSDisableInterrupts();
    _speakerVolume = __ClampSpeakerVolume(volume);
    OSRestoreInterrupts(enable);
}
#pragma pop

BOOL IsBusyStream(s32 chan) {
    BOOL enable;
    WPADControlBlock* p_wpd = _wpdcb[chan];
    u8 stop;
    u32 devType;
    u8 bufNum;
    u16 qSize;
    u16 notAck;
    u8 frames;
    s8 cmdNum;
    u8 linkNum;

    enable = OSDisableInterrupts();
    stop = p_wpd->audioStop;
    devType = p_wpd->devType;
    bufNum = WUDGetBufferStatus();
    cmdNum = __GetCmdNumber(&p_wpd->cmdq);
    qSize = _WPADGetStackBufferStatus(chan);
    notAck = _WPADGetModuleBufferStatus(chan);
    frames = p_wpd->audioFrames;
    linkNum = _WUDGetLinkNumber();
    OSRestoreInterrupts(enable);

    if ((stop) || (notAck > 3) || (bufNum == 10) || (bufNum >= linkNum * 2 + 2) || (devType == WPAD_DEV_UNKNOWN) ||
        (cmdNum >= WPAD_COMMAND_CMD_MAX_LEN - 3) || (frames >= 1)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL WPADCanSendStreamData(s32 chan) {
    BOOL enable;
    BOOL setup;
    s32 status;
    WPADControlBlock* p_wpd = _wpdcb[chan];

    enable = OSDisableInterrupts();
    status = p_wpd->status;
    setup = p_wpd->setup;
    OSRestoreInterrupts(enable);

    if ((status == WPAD_ERR_NO_CONTROLLER) || (setup == FALSE) || (IsBusyStream(chan))) {
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL WPADiSendStreamData(WPADCmdQueue* queue, void* p_buf, u16 len) {
    WPADCommand cmd;
    u8 length = (u8)(len << 3);

    cmd.command = WPAD_HIDREP_STRM;
    cmd.len = 21;
    cmd.data[0] = length;
    cmd.callback = NULL;
    memcpy(cmd.data + 1, p_buf, len);
    return WPADiPushCommand(queue, cmd);
}

s32 WPADSendStreamData(s32 chan, void* p_buf, u16 len) {
    BOOL busy;
    BOOL enable;
    BOOL setup;
    s32 status;
    WPADControlBlock* p_wpd = _wpdcb[chan];

    enable = OSDisableInterrupts();
    status = p_wpd->status;
    setup = p_wpd->setup;
    OSRestoreInterrupts(enable);

    if (status == WPAD_ERR_NO_CONTROLLER) {
        return WPAD_ERR_NO_CONTROLLER;
    }

    if (!setup) {
        return WPAD_ERR_BUSY;
    }

    if (IsBusyStream(chan)) {
        return WPAD_ERR_BUSY;
    }

    busy = WPADiSendStreamData(&p_wpd->cmdq, p_buf, len);
    if (!busy) {
        return WPAD_ERR_BUSY;
    }
    enable = OSDisableInterrupts();
    p_wpd->audioFrames++;
    OSRestoreInterrupts(enable);
    return WPAD_ERR_NONE;
}

// NONMATCHING: target uses an address label for the same sensitivity byte.
u8 WPADGetDpdSensitivity() {
    BOOL enable;
    u8 sensitivity;

    enable = OSDisableInterrupts();
    sensitivity = _dpdSensitivity;
    OSRestoreInterrupts(enable);

    return sensitivity;
}

BOOL WPADIsDpdEnabled(s32 chan) {
    BOOL state;
    BOOL enable;
    WPADControlBlock* p_wpd = _wpdcb[chan];

    enable = OSDisableInterrupts();
    state = p_wpd->infos.dpd;
    OSRestoreInterrupts(enable);
    return state;
}

static void __dpdCb(s32 chan, s32 result) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    p_wpd->dpdFormat = p_wpd->dpdCommand;
    p_wpd->dpdChangePending = 0;
    p_wpd->infos.dpd = (p_wpd->dpdCommand == 0) ? FALSE : TRUE;
}

BOOL WPADiSendEnableDPD(WPADCmdQueue* queue, BOOL enable, WPADCallback callback) {
    WPADCommand cmd;
    BOOL result;

    cmd.command = WPAD_HIDREP_ENDPD;
    cmd.len = 1;
    cmd.data[0] = (u8)((enable) ? (u8)4 : (u8)0);
    cmd.callback = callback;

    return WPADiPushCommand(queue, cmd);
}

// NONMATCHING: sensitivity/config-table symbol identities differ; instruction words agree.
s32 WPADControlDpd(s32 chan, u32 command, WPADCallback callback) {
    BOOL enable;
    BOOL dpd;
    BOOL setup;
    u8 format;
    u8 lastCmd;
    s32 result;
    WPADControlBlock* p_wpd = _wpdcb[chan];

    static const u8 cfg1[5][9] = {
        0x02, 0x00, 0x00, 0x71, 0x01, 0x00, 0x64, 0x00, 0xfe, 0x02, 0x00, 0x00, 0x71, 0x01, 0x00, 0x96, 0x00, 0xb4, 0x02, 0x00, 0x00, 0x71, 0x01,
        0x00, 0xaa, 0x00, 0x64, 0x02, 0x00, 0x00, 0x71, 0x01, 0x00, 0xc8, 0x00, 0x36, 0x07, 0x00, 0x00, 0x71, 0x01, 0x00, 0x72, 0x00, 0x20,
    };
    static const u8 cfg2[5][2] = {
        0xfd, 0x05, 0xb3, 0x04, 0x63, 0x03, 0x35, 0x03, 0x1f, 0x03,
    };

    enable = OSDisableInterrupts();
    dpd = p_wpd->infos.dpd;
    format = p_wpd->dpdFormat;
    lastCmd = p_wpd->dpdCommand;
    result = p_wpd->status;
    setup = p_wpd->setup;
    OSRestoreInterrupts(enable);

    if (result != WPAD_ERR_NO_CONTROLLER) {
        if (!setup) {
            result = WPAD_ERR_BUSY;
        } else if (command == 0) {
            if (!dpd) {
                result = WPAD_ERR_NONE;
            } else {
                enable = OSDisableInterrupts();

                if (WPADiIsAvailableCmdQueue(&p_wpd->cmdq, 3)) {
                    p_wpd->dpdCommand = (u8)command;

                    WPADiSendEnableDPD(&p_wpd->cmdq, FALSE, NULL);
                    WPADiSendDPDCSB(&p_wpd->cmdq, FALSE, __dpdCb);
                    WPADiSendGetContStat(&p_wpd->cmdq, NULL, callback);

                    OSRestoreInterrupts(enable);
                    return WPAD_ERR_NONE;
                } else {
                    result = WPAD_ERR_BUSY;
                }

                OSRestoreInterrupts(enable);
            }
        } else if (command != lastCmd) {
            enable = OSDisableInterrupts();

            if (WPADiIsAvailableCmdQueue(&p_wpd->cmdq, 8)) {
                p_wpd->dpdCommand = (u8)command;
                p_wpd->dpdChangePending = 1;

                WPADiSendEnableDPD(&p_wpd->cmdq, TRUE, NULL);
                WPADiSendDPDCSB(&p_wpd->cmdq, TRUE, NULL);
                WPADiSendWriteDataCmd(&p_wpd->cmdq, 1, 0x04b00030, NULL);
                WPADiSendWriteData(&p_wpd->cmdq, (void*)cfg1[_dpdSensitivity - 1], 9, 0x04b00000, NULL);
                WPADiSendWriteData(&p_wpd->cmdq, (void*)cfg2[_dpdSensitivity - 1], 2, 0x04b0001a, NULL);
                WPADiSendWriteDataCmd(&p_wpd->cmdq, (u8)command, 0x04b00033, NULL);
                WPADiSendWriteDataCmd(&p_wpd->cmdq, 8, 0x04b00030, __dpdCb);
                WPADiSendGetContStat(&p_wpd->cmdq, NULL, callback);

                OSRestoreInterrupts(enable);
                return WPAD_ERR_NONE;
            } else {
                result = WPAD_ERR_BUSY;
            }
            OSRestoreInterrupts(enable);
        }
    }

    if (callback) {
        callback(chan, result);
    }

    return result;
}

s32 WPADControlBLC(s32 chan, u8 command, WPADCallback callback) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    s32 result;
    BOOL setup;
    BOOL enable;
    u8 data[7];

    enable = OSDisableInterrupts();
    result = p_wpd->status;
    setup = p_wpd->setup;
    OSRestoreInterrupts(enable);

    if (result == WPAD_ERR_NO_CONTROLLER) {
        goto _exit;
    }
    if (!setup || !WUDIsLinkedWBC()) {
        result = WPAD_ERR_BUSY;
        goto _exit;
    }
    {
        data[0] = data[1] = data[2] = 0xAA;
        data[3] = 0x55;
        data[4] = data[5] = data[6] = command;
        enable = OSDisableInterrupts();
        switch (command) {
        case 0xAA:
            if (WPADiIsAvailableCmdQueue(&p_wpd->cmdq, 4)) {
                WPADWriteExtensionRegister(chan, data, 7, 0xA4, 0xF1, NULL);
                WPADWriteExtensionRegister(chan, data, 1, 0xA4, 0xF1, NULL);
                WPADWriteExtensionRegister(chan, data, 1, 0xA4, 0xF1, NULL);
                WPADWriteExtensionRegister(chan, data, 1, 0xA4, 0xF1, callback);
                OSRestoreInterrupts(enable);
                return WPAD_ERR_NONE;
            }
            result = WPAD_ERR_BUSY;
            break;
        case 0x55:
            result = WPADWriteExtensionRegister(chan, data, 7, 0xA4, 0xF1, callback);
            if (result == WPAD_ERR_NONE) {
                OSRestoreInterrupts(enable);
                return WPAD_ERR_NONE;
            }
            break;
        case 0:
            result = WPADWriteExtensionRegister(chan, data, 1, 0xA4, 0xF1, callback);
            if (result == WPAD_ERR_NONE) {
                OSRestoreInterrupts(enable);
                return WPAD_ERR_NONE;
            }
            break;
        default:
            result = WPAD_ERR_BUSY;
            break;
        }
        OSRestoreInterrupts(enable);
    }
_exit:
    if (callback) {
        callback(chan, result);
    }
    return result;
}

// NONMATCHING: incoming report/index GPR allocation and canonical data labels differ; target452B control flow and typed fields retained.
static s32 __wpadSendDataSub(s32 chan, WPADCommand cmd) {
    BOOL enable;
    BOOL motor;
    BT_HDR* p_buf = NULL;
    u8* ptr;
    s8 handle;
    s32 status;
    WPADControlBlock* p_wpd = _wpdcb[chan];
    u8 rep_id = (u8)cmd.command;
    u8* p_data = cmd.data;
    u16 len = cmd.len;

    enable = OSDisableInterrupts();
    status = p_wpd->status;
    handle = p_wpd->dev_handle;
    if (handle < 0) {
        OSRestoreInterrupts(enable);
        return WPAD_ERR_INVALID;
    }
    p_wpd->status = WPAD_ERR_BUSY;
    motor = p_wpd->motor & _rumble;

    if (rep_id == WPAD_HIDREP_VIBRATOR) {
        p_wpd->status = status;
    } else if (rep_id == WPAD_HIDREP_STRM) {
        p_wpd->status = status;
        p_wpd->audioFrames--;
    } else {
        switch (rep_id) {
        case WPAD_HIDREP_WRDATA:
            break;

        case WPAD_HIDREP_RDDATA:
            p_wpd->readError = 0;
            p_wpd->readBaseAddr = cmd.readAddr;
            p_wpd->readLength = cmd.readLen;
            p_wpd->readBufPtr = cmd.readBuf;
            break;

        case WPAD_HIDREP_GETSTAT:
            p_wpd->status = status;
            p_wpd->infoBuf = cmd.info;
            p_wpd->getStatFlag = 1;
            break;

        default:
            p_data[0] |= 2;
            break;
        }

        p_wpd->resultCallback = cmd.callback;
        p_wpd->cmdId = rep_id;
        p_wpd->cmdTimer = OSSecondsToTicks(2) + __OSGetSystemTime();
        p_wpd->cmdTimeoutAction = 0;
    }

    OSRestoreInterrupts(enable);

    p_buf = GKI_getbuf((u8)(10 + len + sizeof(BT_HDR)));
    p_buf->len = (u8)(len + 1);
    p_buf->offset = 10;
    ptr = (u8*)(p_buf + 1) + p_buf->offset;

    ptr[0] = rep_id;

    memcpy(ptr + 1, p_data, len);

    if (motor) {
        ptr[1] |= 1;
    } else {
        ptr[1] &= ~1;
    }

    BTA_HhSendData((u8)handle, p_buf);
    return WPAD_ERR_NONE;
}

// NONMATCHING: switch-table/block ordering differs; target report IDs, command fields and queue behavior retained.
BOOL WPADiSendSetReportType(WPADCmdQueue* queue, u32 fmt, BOOL powerSave, WPADCallback callback) {
    WPADCommand cmd;
    BOOL result;

    cmd.command = WPAD_HIDREP_REPTYPE;
    cmd.len = 2;
    cmd.data[0] = (u8)((powerSave) ? 0 : 4);
    cmd.callback = callback;

    switch (fmt) {
    case WPAD_FMT_CORE:
        cmd.data[1] = 0x30;
        break;
    case WPAD_FMT_CORE_ACC:
        cmd.data[1] = 0x31;
        break;
    case WPAD_FMT_CORE_ACC_DPD:
        cmd.data[1] = 0x33;
        break;
    case WPAD_FMT_FREESTYLE:
        cmd.data[1] = 0x32;
        break;
    case WPAD_FMT_FREESTYLE_ACC:
        cmd.data[1] = 0x35;
        break;
    case WPAD_FMT_FREESTYLE_ACC_DPD:
        cmd.data[1] = 0x37;
        break;
    case WPAD_FMT_CLASSIC:
        cmd.data[1] = 0x32;
        break;
    case WPAD_FMT_CLASSIC_ACC:
        cmd.data[1] = 0x35;
        break;
    case WPAD_FMT_CLASSIC_ACC_DPD:
        cmd.data[1] = 0x37;
        break;
    case WPAD_FMT_CORE_ACC_DPD_FULL:
        cmd.data[1] = 0x3e;
        break;
    case 10: cmd.data[1] = 0x32; break;
    case 11: cmd.data[1] = 0x37; break;
    case 15: cmd.data[1] = 0x37; break;
    case 18: cmd.data[1] = 0x37; break;
    case 19: cmd.data[1] = 0x35; break;
    case 12: cmd.data[1] = 0x34; break;
    case 13: cmd.data[1] = 0x35; break;
    case 17: cmd.data[1] = 0x37; break;
    case 16: cmd.data[1] = 0x37; break;
    }

    return WPADiPushCommand(queue, cmd);
}

void WPADGetAddress(s32 chan, u8* addr) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    BOOL enable;
    s8 handle;
    u8* devAddr;

    enable = OSDisableInterrupts();
    handle = p_wpd->dev_handle;
    OSRestoreInterrupts(enable);

    devAddr = _WUDGetDevAddr((u8)handle);

    if (devAddr) {
        memcpy(addr, devAddr, 6);
    } else {
        memset(addr, 0, 6);
    }
}

void WPADShutdown() {
    return;
}

void __WPADShutdown() {
    BOOL enabled = OSDisableInterrupts();
    int chan;
    if (_shutdown) {
        OSRestoreInterrupts(enabled);
        return;
    }
    _shutdown = 1;
    WUDSetVisibility(0, 0);
    for (chan = 0; chan < WPAD_MAX_CONTROLLERS; ++chan) WUDSetDeviceHistory(chan, NULL);
    WPADiShutdown(FALSE);
    OSRestoreInterrupts(enabled);
}

void __WPADReconnect(BOOL exec) {
    BOOL enable = OSDisableInterrupts();

    if (!_startup) {
        OSRestoreInterrupts(enable);
        return;
    }

    if (_shutdown) {
        OSRestoreInterrupts(enable);
        return;
    }

    _shutdown = 1;
    _recFlag = (exec) ? 1 : 0;
    OSRestoreInterrupts(enable);
}

static void WPADiShutdown(BOOL exec) {
    OSCancelAlarm(&_managerAlarm);
    WUDSetHidRecvCallback(NULL);
    WUDShutdown(exec);
}

BOOL WPADiSendWriteDataCmd(WPADCmdQueue* queue, u8 cmd, u32 addr, WPADCallback callback) {
    return WPADiSendWriteData(queue, &cmd, 1, addr, callback);
}

BOOL WPADiSendWriteData(WPADCmdQueue* queue, void* p_buf, u16 len, u32 addr, WPADCallback callback) {
    WPADCommand cmd;
    BOOL result;
    u8 length = (u8)(len & WPAD_WRITE_LEN_MASK);
    cmd.command = WPAD_HIDREP_WRDATA;
    cmd.len = 21;
    cmd.callback = callback;
    memcpy(cmd.data, &addr, sizeof(addr));
    memcpy(cmd.data + 4, &length, sizeof(length));
    memcpy(cmd.data + 5, p_buf, len);

    result = WPADiPushCommand(queue, cmd);
    return result;
}

BOOL WPADiSendReadData(WPADCmdQueue* queue, void* p_buf, u16 len, u32 addr, WPADCallback callback) {
    WPADCommand cmd;
    BOOL result;
    ASSERT(p_buf != NULL);

    cmd.command = WPAD_HIDREP_RDDATA;
    cmd.len = 6;
    cmd.callback = callback;

    memcpy(cmd.data, &addr, sizeof(addr));
    memcpy(cmd.data + 4, &len, sizeof(len));

    cmd.readBuf = p_buf;
    cmd.readLen = len;
    cmd.readAddr = addr;

    result = WPADiPushCommand(queue, cmd);
    return result;
}

void WPADiClearQueue(WPADCmdQueue* queue) {
    BOOL enable;

    enable = OSDisableInterrupts();

    queue->head = 0;
    queue->tail = 0;
    memset(queue->cmd, 0, queue->cmdlen * sizeof(WPADCommand));

    OSRestoreInterrupts(enable);
}

u8 WPADiGetMplsStatus(s32 chan) {
    u8 mode = 0;
    WPADControlBlock* p_cb = _wpdcb[chan];
    BOOL enabled = OSDisableInterrupts();
    if ((u8)(p_cb->devType + 251) <= 2 || p_cb->devType == 250) mode = p_cb->devMode;
    OSRestoreInterrupts(enabled);
    return mode;
}

// NONMATCHING: equivalent byte casts/result materialization and register/stack allocation differ.
void __wpadMplsControlCallback(s32 chan, s32 result) {
    WPADControlBlock* p_cb = _wpdcb[chan];

    switch (p_cb->mplsControlState) {
    case 1:
        if (result == WPAD_ERR_NONE) {
            if ((u8)(p_cb->mplsModeReadback[1] + 251) > 2 && p_cb->mplsModeReadback[1] != 250) {
                if (--p_cb->mplsControlRetries == 0) {
                    result = WPAD_ERR_TRANSFER;
                    break;
                }
                if (p_cb->mplsControlRetries > 1) {
                    p_cb->mplsRegisterAddress = 0x04a600fe;
                } else {
                    p_cb->mplsRegisterAddress = 0x04a400fe;
                }
                result = WPADiSendReadData(&p_cb->cmdq, p_cb->mplsModeReadback, 2,
                    p_cb->mplsRegisterAddress, __wpadMplsControlCallback) == FALSE ? WPAD_ERR_BUSY : WPAD_ERR_NONE;
            } else {
                u32 address = (p_cb->mplsRegisterAddress & ~0xffU) | 0xf0;
                if (address == 0x04a600f0) {
                    p_cb->mplsControlState = 3;
                    result = WPADiSendWriteDataCmd(&p_cb->cmdq, 0x55, address,
                        __wpadMplsControlCallback) == FALSE ? WPAD_ERR_BUSY : WPAD_ERR_NONE;
                } else if (p_cb->mplsModeReadback[0] == p_cb->mplsRequestedMode) {
                    p_cb->mplsControlState = 0;
                } else if (--p_cb->mplsControlRetries != 0) {
                    p_cb->mplsRegisterAddress = 0x04a400fe;
                    result = WPADiSendReadData(&p_cb->cmdq, p_cb->mplsModeReadback, 2,
                        p_cb->mplsRegisterAddress, __wpadMplsControlCallback) == FALSE ? WPAD_ERR_BUSY : WPAD_ERR_NONE;
                } else {
                    p_cb->mplsControlState = 6;
                    result = WPADiSendWriteDataCmd(&p_cb->cmdq, 0, 0x04a400fe,
                        __wpadMplsControlCallback) == FALSE ? WPAD_ERR_BUSY : WPAD_ERR_NONE;
                }
            }
        } else if (result == WPAD_ERR_TRANSFER && p_cb->mplsControlRetries != 0) {
            --p_cb->mplsControlRetries;
            if (p_cb->mplsControlRetries > 1) {
                p_cb->mplsRegisterAddress = 0x04a600fe;
            } else {
                p_cb->mplsRegisterAddress = 0x04a400fe;
            }
            result = WPADiSendReadData(&p_cb->cmdq, p_cb->mplsModeReadback, 2,
                p_cb->mplsRegisterAddress, __wpadMplsControlCallback) == FALSE ? WPAD_ERR_BUSY : WPAD_ERR_NONE;
        }
        break;
    case 2:
        if (result == WPAD_ERR_NONE) {
            if ((u8)(p_cb->mplsModeReadback[1] + 251) <= 2 || p_cb->mplsModeReadback[1] == 250) {
                p_cb->mplsControlState = 7;
                result = WPADiSendWriteDataCmd(&p_cb->cmdq, 0, 0x04a400fe,
                    __wpadMplsControlCallback) == FALSE ? WPAD_ERR_BUSY : WPAD_ERR_NONE;
            } else {
                result = WPAD_ERR_TRANSFER;
            }
        }
        break;
    case 3:
        if (result == WPAD_ERR_NONE) {
            p_cb->mplsControlState = 4;
            result = WPADiSendWriteDataCmd(&p_cb->cmdq, p_cb->mplsRequestedMode, 0x04a600fe,
                __wpadMplsControlCallback) == FALSE ? WPAD_ERR_BUSY : WPAD_ERR_NONE;
        }
        break;
    case 4:
        if (result == WPAD_ERR_NONE) {
            p_cb->mplsReinitPending = 1;
            p_cb->mplsControlState = 0;
        }
        break;
    case 5:
        if (result == WPAD_ERR_NONE) {
            if ((u8)(p_cb->mplsModeReadback[1] + 251) <= 2 || p_cb->mplsModeReadback[1] == 250) {
                p_cb->mplsControlState = 8;
                result = WPADiSendWriteDataCmd(&p_cb->cmdq, 0, 0x04a400f2,
                    __wpadMplsControlCallback) == FALSE ? WPAD_ERR_BUSY : WPAD_ERR_NONE;
            } else {
                result = WPAD_ERR_TRANSFER;
            }
        }
        break;
    case 6:
        if (result == WPAD_ERR_NONE) {
            p_cb->mplsControlRetries = 4;
            p_cb->mplsControlState = 1;
            result = WPADiSendReadData(&p_cb->cmdq, p_cb->mplsModeReadback, 1, 0x04a600ff,
                __wpadMplsControlCallback) == FALSE ? WPAD_ERR_BUSY : WPAD_ERR_NONE;
        }
        break;
    case 7:
        result = WPAD_ERR_NONE;
        // fall through
    case 8:
        p_cb->mplsControlState = 0;
        break;
    }

    if (p_cb->mplsControlState == 0 || result != WPAD_ERR_NONE) {
        WPADCallback callback = p_cb->mplsControlCallback;
        p_cb->mplsControlBusy = 0;
        p_cb->mplsControlCallback = NULL;
        if (callback != NULL) callback(chan, result);
    }
}

// NONMATCHING: equivalent device-range lowering and register allocation differ.
s32 WPADiControlMpls(s32 chan, u8 mode, WPADCallback callback) {
    BOOL enabled;
    WPADControlBlock* p_cb = _wpdcb[chan];
    BOOL setup;
    u8 device;
    u8 currentMode;
    s32 result;
    u8 busy;

    enabled = OSDisableInterrupts();
    device = p_cb->devType;
    currentMode = p_cb->devMode;
    result = p_cb->status;
    setup = p_cb->setup;
    busy = p_cb->mplsControlCallback != NULL ? 1 : p_cb->mplsControlBusy;

    if (result == WPAD_ERR_NO_CONTROLLER) {
        // Preserve disconnected status.
    } else if (!setup || busy) {
        result = WPAD_ERR_BUSY;
    } else if (p_cb->certValidity < 0) {
        result = WPAD_ERR_INVALID;
    } else if (mode == currentMode) {
        result = WPAD_ERR_NONE;
    } else if (((u32)(device - 5) <= 2 || device == 250) && (mode == 0 || mode == 0x80) ||
               (((u32)device - 5) > 2 && device != 250) && (mode == 4 || mode == 5 || mode == 7)) {
        if (mode == 0x80 && p_cb->mplsInitTicks < 200) {
            result = WPAD_ERR_BUSY;
        } else {
            p_cb->mplsRequestedMode = mode;
            p_cb->mplsControlRetries = 4;
            switch (mode) {
            case 0:
                p_cb->mplsControlState = 2;
                p_cb->mplsRegisterAddress = 0x04a400fe;
                break;
            case 0x80:
                p_cb->mplsControlState = 5;
                p_cb->mplsRegisterAddress = 0x04a400fe;
                break;
            default:
                p_cb->mplsControlState = 1;
                p_cb->mplsRegisterAddress = 0x04a600fe;
                break;
            }
            if (!WPADiSendReadData(&p_cb->cmdq, p_cb->mplsModeReadback, 2,
                p_cb->mplsRegisterAddress, __wpadMplsControlCallback)) {
                result = WPAD_ERR_BUSY;
            } else {
                p_cb->mplsControlBusy = 1;
                p_cb->mplsControlCallback = callback;
                OSRestoreInterrupts(enabled);
                return WPAD_ERR_NONE;
            }
        }
    } else {
        result = WPAD_ERR_INVALID;
    }
    OSRestoreInterrupts(enabled);
    if (callback != NULL) callback(chan, result);
    return result;
}

// Actual 512-bit certificate modulus and challenge multiplier; word zero is the limb count.
#define WPAD_CERT_MODULUS_INITIALIZER { \
    16, 0xa3dc5f11, 0x1b2ec797, 0xac9657b8, 0xcbeb788d, 0x991ef8b8, \
    0x70caa54e, 0x071ce896, 0x63252cd0, 0xfd8b6316, 0x9e835020, 0xbf9ed1ef, \
    0x0f870932, 0xbc88b819, 0xaf2c02e5, 0x657bbfb9, 0x81194f1c, 0 \
}
static const u32 __wpadCertModulus[18] = WPAD_CERT_MODULUS_INITIALIZER;
static const u32 __wpadCertMultiplier[18] = {
    16, 0x99d2071a, 0x8d823a01, 0x4c50a978, 0xd8a5bfff, 0x4caf5ce8,
    0x13a71cba, 0x6fcff21b, 0x8c5399bf, 0x8896982d, 0xcb6ec891, 0x6ba136da,
    0x25b8f224, 0x8f0f42c0, 0x15ea9941, 0xfdd1903e, 0x1af308b6, 0
};

// NONMATCHING: two independent header stores are scheduled in a different order.
void __wpadCertFailed(s32 chan) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_cb->certValidity = -1;
    p_cb->certBusy = 0;
    p_cb->certChallengeBit = -1;
    p_cb->certState = -1;
    p_cb->certWorkDelay = 0;
    p_cb->certTransferRetries = 0;
    p_cb->certWorkCount = 0;
    memset(p_cb->certParamX, 0, sizeof(p_cb->certParamX));
    memset(p_cb->certParamY, 0, sizeof(p_cb->certParamY));
    memset(p_cb->certWork, 0, sizeof(p_cb->certWork));
    p_cb->certParamX[0] = p_cb->certParamY[0] = p_cb->certWork[0] = 1;
    p_cb->certParamBytes = 0;
    p_cb->certCachedValidity = -1;
    p_cb->devType = 252;
    if (p_cb->extensionCallback != NULL) p_cb->extensionCallback(chan, 252);
}

void __wpadCertCalcModX(s32 chan) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    u32 shifted[66];
    u32 start = (u32)OSGetTime();
    s32 modulusBits = LINTMsb(__wpadCertModulus);
    do {
        s32 workBits = LINTMsb(p_cb->certWork);
        if (workBits > modulusBits) {
            LINTLshift(shifted, __wpadCertModulus, workBits - modulusBits - 1);
            if (LINTCmp(p_cb->certWork, shifted) >= 0) {
                LINTSub(p_cb->certWork, p_cb->certWork, shifted);
            }
        } else {
            if (LINTCmp(p_cb->certWork, __wpadCertModulus) >= 0) {
                LINTSub(p_cb->certWork, p_cb->certWork, __wpadCertModulus);
            }
            memcpy(p_cb->certParamX, p_cb->certWork, sizeof(p_cb->certParamX));
            p_cb->certState = 9;
        }
    } while (OSTicksToMicroseconds((u32)OSGetTime() - start) < 80);
    p_cb->certBusy = 0;
    p_cb->certWorkDelay = 0;
}

void __wpadCertCalcModY(s32 chan) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    u32 shifted[66];
    u32 start = (u32)OSGetTime();
    s32 modulusBits = LINTMsb(__wpadCertModulus);
    do {
        s32 workBits = LINTMsb(p_cb->certWork);
        if (workBits > modulusBits) {
            LINTLshift(shifted, __wpadCertModulus, workBits - modulusBits - 1);
            if (LINTCmp(p_cb->certWork, shifted) >= 0) {
                LINTSub(p_cb->certWork, p_cb->certWork, shifted);
            }
        } else {
            if (LINTCmp(p_cb->certWork, __wpadCertModulus) >= 0) {
                LINTSub(p_cb->certWork, p_cb->certWork, __wpadCertModulus);
            }
            memcpy(p_cb->certParamY, p_cb->certWork, sizeof(p_cb->certParamY));
            p_cb->certState = 11;
        }
    } while (OSTicksToMicroseconds((u32)OSGetTime() - start) < 80);
    p_cb->certBusy = 0;
    p_cb->certWorkDelay = 0;
}

void __wpadCertChallengeCallback(s32 chan, s32 result) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_cb->certBusy = 0;
    if (result == WPAD_ERR_NONE) {
        p_cb->certWorkDelay = p_cb->certChallengeBit == 0 ? 1000 : 9000;
        p_cb->certState = 4;
    } else if (p_cb->certTransferRetries++ != 0) {
        p_cb->certWorkDelay = 0;
        __wpadCertFailed(chan);
    }
}

void __wpadCertProbeReadyCallback(s32 chan, s32 result) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_cb->certBusy = 0;
    if (result == WPAD_ERR_NONE) {
        p_cb->certWorkDelay = 0;
        ++p_cb->certTransferRetries;
        if (p_cb->certState == 0) {
            if (p_cb->certProbeResponse <= 4) p_cb->certTransferRetries = 0;
            if (p_cb->certProbeResponse <= 12) {
                p_cb->certWorkDelay = 5500;
                p_cb->certTransferRetries = 0;
            }
            if (p_cb->certProbeResponse == 14) {
                p_cb->certState = 1;
                p_cb->certTransferRetries = 0;
            }
        }
        if (p_cb->certState == 4 && p_cb->certProbeResponse == 26) {
            p_cb->certState = 5;
            p_cb->certTransferRetries = 0;
        }
    } else {
        p_cb->certWorkDelay = 1000;
        p_cb->certTransferRetries += 4;
    }
    if (p_cb->certTransferRetries > 100) __wpadCertFailed(chan);
}

void __wpadCertGetParamCallback(s32 chan, s32 result) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_cb->certBusy = 0;
    if (result == WPAD_ERR_NONE) {
        u32 i;
        for (i = 0; i < p_cb->readLength / 4; ++i) {
            p_cb->certParam[i + p_cb->certParamBytes / 4 + 1] =
                (u32)p_cb->readBuf[i * 4] | ((u32)p_cb->readBuf[i * 4 + 1] << 8) |
                ((u32)p_cb->readBuf[i * 4 + 2] << 16) | ((u32)p_cb->readBuf[i * 4 + 3] << 24);
        }
        p_cb->certTransferRetries = 0;
        p_cb->certParamBytes += p_cb->readLength;
        if (p_cb->certParamBytes < 64) {
            p_cb->certWorkDelay = 0;
        } else {
            for (i = 0; i < 16; ++i) {
                if (p_cb->certParam[i + 1] != 0) p_cb->certParam[0] = i + 1;
            }
        }
    } else if (p_cb->certTransferRetries++ < 1) {
        p_cb->certWorkDelay = 0;
    } else {
        __wpadCertFailed(chan);
    }
}

// NONMATCHING: compiler removes a transient completion boolean; register/stack allocation differs.
static inline void __wpadCertProbeReady(s32 chan) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADiSendReadData(&p_cb->extq, &p_cb->certProbeResponse, 1, 0x04a400f7,
        __wpadCertProbeReadyCallback);
}

enum { WPAD_CERT_PARAM_X, WPAD_CERT_PARAM_Y, WPAD_CERT_WORK };

static inline BOOL __wpadCertReadParam(s32 chan, u8 param) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    if (p_cb->certParamBytes < 64) {
        switch (param) {
        case WPAD_CERT_PARAM_X: p_cb->certParam = p_cb->certParamX; break;
        case WPAD_CERT_PARAM_Y: p_cb->certParam = p_cb->certParamY; break;
        default: p_cb->certParam = p_cb->certWork; break;
        }
        WPADiSendReadData(&p_cb->extq, p_cb->readBuf, 64, 0x04a40050 + p_cb->certParamBytes,
            __wpadCertGetParamCallback);
        return FALSE;
    }
    p_cb->certBusy = 0;
    p_cb->certParamBytes = 0;
    return TRUE;
}

void __wpadCertVerifyParamY(s32 chan) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    if (__wpadCertReadParam(chan, WPAD_CERT_WORK)) {
        u32 modulusMinusOne[18] = WPAD_CERT_MODULUS_INITIALIZER;
        --modulusMinusOne[1];
        if ((p_cb->certWork[0] > 1 || p_cb->certWork[1] > 1) &&
            ((p_cb->certChallengeBit != 0 && LINTCmp(p_cb->certWork, __wpadCertModulus) == -1) ||
             (p_cb->certChallengeBit == 0 && LINTCmp(p_cb->certWork, modulusMinusOne) == -1)) &&
            LINTCmp(p_cb->certWork, p_cb->certParamY) == 0) {
            memset(p_cb->certWork, 0, sizeof(p_cb->certWork));
            p_cb->certWork[0] = 1;
            p_cb->certState = 7;
            p_cb->certWorkDelay = 0;
        } else {
            __wpadCertFailed(chan);
        }
    }
}

// NONMATCHING: compiler removes transient completion booleans and changes register/stack allocation.
void __wpadCertWork(s32 chan) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    if (p_cb->certValidity == 0 && p_cb->certState >= 0 && p_cb->infos.attach) {
        if (!p_cb->certBusy && p_cb->certWorkDelay-- < 0) {
            p_cb->certBusy = 1;
            switch (p_cb->certState) {
            case 0:
                __wpadCertProbeReady(chan);
                break;
            case 1:
                if (__wpadCertReadParam(chan, WPAD_CERT_PARAM_X)) {
                    if ((p_cb->certParamX[0] > 1 || p_cb->certParamX[1] > 1) &&
                        LINTCmp(p_cb->certParamX, __wpadCertModulus) == -1) {
                        p_cb->certState = 2;
                        p_cb->certWorkDelay = 500;
                    } else {
                        __wpadCertFailed(chan);
                    }
                }
                break;
            case 2:
                if (__wpadCertReadParam(chan, WPAD_CERT_WORK)) {
                    if ((p_cb->certWork[0] > 1 || p_cb->certWork[1] > 1) &&
                        LINTCmp(p_cb->certWork, __wpadCertModulus) == -1 &&
                        LINTCmp(p_cb->certWork, p_cb->certParamX) == 0) {
                        memset(p_cb->certWork, 0, sizeof(p_cb->certWork));
                        p_cb->certWork[0] = 1;
                        p_cb->certChallengeBit = OSGetTick() & 1;
                        p_cb->certState = 3;
                        p_cb->certWorkDelay = 0;
                    } else {
                        __wpadCertFailed(chan);
                    }
                }
                break;
            case 3:
                WPADiSendWriteDataCmd(&p_cb->extq, (u8)p_cb->certChallengeBit, 0x04a400f1,
                    __wpadCertChallengeCallback);
                break;
            case 4:
                __wpadCertProbeReady(chan);
                break;
            case 5:
                if (__wpadCertReadParam(chan, WPAD_CERT_PARAM_Y)) {
                    u32 modulusMinusOne[18] = WPAD_CERT_MODULUS_INITIALIZER;
                    --modulusMinusOne[1];
                    if ((p_cb->certParamY[0] > 1 || p_cb->certParamY[1] > 1) &&
                        ((p_cb->certChallengeBit != 0 && LINTCmp(p_cb->certParamY, __wpadCertModulus) == -1) ||
                         (p_cb->certChallengeBit == 0 && LINTCmp(p_cb->certParamY, modulusMinusOne) == -1))) {
                        p_cb->certWorkDelay = 500;
                        p_cb->certState = 6;
                    } else {
                        __wpadCertFailed(chan);
                    }
                }
                break;
            case 6:
                __wpadCertVerifyParamY(chan);
                break;
            case 7:
                if (p_cb->certChallengeBit != 0) {
                    LINTMul(p_cb->certWork, p_cb->certParamX, __wpadCertMultiplier);
                } else {
                    memcpy(p_cb->certWork, p_cb->certParamX, sizeof(p_cb->certParamX));
                }
                p_cb->certBusy = 0;
                p_cb->certWorkDelay = 0;
                p_cb->certState = 8;
                break;
            case 8:
                __wpadCertCalcModX(chan);
                break;
            case 9:
                LINTMul(p_cb->certWork, p_cb->certParamY, p_cb->certParamY);
                p_cb->certBusy = 0;
                p_cb->certWorkDelay = 0;
                p_cb->certState = 10;
                break;
            case 10:
                __wpadCertCalcModY(chan);
                break;
            case 11:
                {
                    s32 comparison = LINTCmp(p_cb->certParamX, p_cb->certParamY);
                    p_cb->certBusy = 0;
                    p_cb->certWorkDelay = 0;
                    p_cb->certState = 12;
                    if (comparison == 0) {
                        p_cb->certValidity = 1;
                        p_cb->certCachedValidity = 1;
                    } else {
                        __wpadCertFailed(chan);
                    }
                }
                break;
            }
        }
        if (p_cb->certWorkCount++ > 60000) {
            p_cb->certState = 120;
            __wpadCertFailed(chan);
        }
    }
}
