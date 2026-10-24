#include "revolution/wpad.h"
#include "revolution/wud.h"
#include "revolution/os/OSCrc.h"
#include <cmath>


static u8 _mplsExtensionType[WPAD_MAX_CONTROLLERS];
static u8 _mplsExtensionSubtype[WPAD_MAX_CONTROLLERS];
static u8 _devType[WPAD_MAX_CONTROLLERS];
static u8 _devMode[WPAD_MAX_CONTROLLERS];
static u8 _devCmpt[WPAD_MAX_CONTROLLERS];
static u8 _retryCnt[WPAD_MAX_CONTROLLERS];
static f32 rolag[WPAD_MAX_CONTROLLERS];
static f32 calibX[WPAD_MAX_CONTROLLERS];
static f32 calibY[WPAD_MAX_CONTROLLERS];
static f32 centerX[WPAD_MAX_CONTROLLERS];
static f32 centerY[WPAD_MAX_CONTROLLERS];

void __a1_20_status_report(u8 chan, u8* data, WPADStatus* p_status);
void __a1_21_user_data(u8 chan, u8* data, WPADStatus* p_status);
void __a1_22_ack(u8 chan, u8* data, WPADStatus* p_status);
void __a1_30_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_31_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_32_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_33_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_34_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_35_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_36_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_37_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_3d_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_3e_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_3f_data_type(u8 chan, u8* data, WPADStatus* p_status);
void __a1_unused_report(u8 chan, u8* data, WPADStatus* p_status);

#define HID_WPAD_BUTTON_MASK (u16)(0x9F1F)
#define HID_WPAD_BUTTON_CLEAR (u16)(0x60E0)
#define HID_NEAREMPTY_MASK (u8)(0x80)
#define HID_DPD_MASK (u8)(0x8)
#define HID_SPEAKER_MASK (u8)(0x4)
#define HID_ATTACH_MASK (u8)(0x2)
#define HID_LOWBAT_MASK (u8)(0x1)
#define HID_LED_MASK (u8)(0xF0)
#define HID_FW_MASK (u8)(0xF0)
#define HID_PRT_MASK (u8)(0xF)

#define HID_IDX_BYTE7 1
#define HID_IDX_BYTE8 2
#define HID_IDX_BYTE9 3
#define HID_IDX_BYTE10 4
#define HID_IDX_BYTE11 5
#define HID_IDX_BYTE12 6
#define HID_IDX_BYTE13 7
#define HID_IDX_BYTE14 8
#define HID_IDX_BYTE15 9
#define HID_IDX_BYTE16 10
#define HID_IDX_BYTE17 11
#define HID_IDX_BYTE18 12
#define HID_IDX_BYTE19 13
#define HID_IDX_BYTE20 14
#define HID_IDX_BYTE21 15
#define HID_IDX_BYTE22 16
#define HID_IDX_BYTE23 17
#define HID_IDX_BYTE24 18
#define HID_IDX_BYTE25 19
#define HID_IDX_BYTE26 20
#define HID_IDX_BYTE27 21

void (*__a1_input_reports_array[])(u8 chan, u8* data, WPADStatus* p_status) = {
    __a1_20_status_report, __a1_21_user_data,  __a1_22_ack,        __a1_unused_report, __a1_unused_report, __a1_unused_report, __a1_unused_report,
    __a1_unused_report,    __a1_unused_report, __a1_unused_report, __a1_unused_report, __a1_unused_report, __a1_unused_report, __a1_unused_report,
    __a1_unused_report,    __a1_unused_report, __a1_30_data_type,  __a1_31_data_type,  __a1_32_data_type,  __a1_33_data_type,  __a1_34_data_type,
    __a1_35_data_type,     __a1_36_data_type,  __a1_37_data_type,  __a1_unused_report, __a1_unused_report, __a1_unused_report, __a1_unused_report,
    __a1_unused_report,    __a1_3d_data_type,  __a1_3e_data_type,  __a1_3f_data_type};

static const u8 checkInvalidData[21] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

static void resetExtensionCertificate(s32 chan) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_cb->certValidity = 0;
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
}

NO_INLINE BOOL __wpadCheckCalibration(u8* data, u8 length, u8 checksumType);
static inline void initExtension(s32);
void __parse_ext_data(s32 chan, WPADStatus* p_status, u8 mode, u8* data, u8 len);

// NONMATCHING: relocation identities differ; instruction words and callback arguments agree.
void __wpadAbortInitExtension(s32 chan, s32 result) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    u32 type;
    if (result != WPAD_ERR_NONE) {
        WPADiClearQueue(&p_cb->extq);
        p_cb->extEnc = 0;
        if (result == WPAD_ERR_NO_CONTROLLER) type = 253;
        else if (p_cb->infos.attach) {
            if (_retryCnt[chan]++ < 32) {
                initExtension(chan);
                return;
            }
            type = 252;
        } else {
            WPADiSendSetReportType(&p_cb->extq, p_cb->dataFormat, p_cb->pwrSave, NULL);
            return;
        }
        p_cb->devType = type;
        p_cb->extensionPreviousType = type;
        p_cb->extensionReconnectPending = 0;
        if (p_cb->extensionCallback) p_cb->extensionCallback(chan, type);
    }
}


// NONMATCHING: identical instruction words and scalar pool; generated BSS/literal relocation identities differ.
void __wpadGetDevConfig(s32 chan, s32 result) {
    WPADControlBlock* p_wpd = _wpdcb[chan];
    u8* p_buf = (u8*)p_wpd->readBufPtr;
    u8 checkSum;
    u8 i;
    int j;
    int index;
    DPDObject obj[WPAD_DPD_MAX_OBJECTS];
    const s16 dummyObjX[] = {127, 896, 896, 127};
    const s16 dummyObjY[] = {93, 93, 674, 674};
    f32 difaveX;
    f32 difaveY;
    f32 deltaX;
    f32 deltaY;
    f32 x[WPAD_DPD_MAX_OBJECTS];
    f32 y[WPAD_DPD_MAX_OBJECTS];
    f32 ag[WPAD_DPD_MAX_OBJECTS];
    f32 ag_org[WPAD_DPD_MAX_OBJECTS];

    for (i = 0; i < WPAD_DPD_MAX_OBJECTS; i++) {
        p_wpd->conf.obj[i].x = dummyObjX[i];
        p_wpd->conf.obj[i].y = dummyObjY[i];
        p_wpd->conf.obj[i].size = (u16)p_wpd->dpdDummyObjSize;
        p_wpd->conf.obj[i].traceId = i;
    }

    p_wpd->conf.acc_0g.x = 530;
    p_wpd->conf.acc_0g.y = 530;
    p_wpd->conf.acc_0g.z = 530;
    p_wpd->conf.acc_1g.x = 636;
    p_wpd->conf.acc_1g.y = 636;
    p_wpd->conf.acc_1g.z = 636;
    p_wpd->conf.volume = 0;
    p_wpd->conf.motor = 0;

    if (result != WPAD_ERR_NONE) {
        return;
    }

    if (!p_wpd->oldFw) {
        for (i = 0; i < 2; i++) {
            checkSum = 0;
            index = i * (11);
            for (j = index; j < index + 10; j++) {
                checkSum += p_buf[j];
            }
            checkSum = (u8)(checkSum + 0x55);

            if (checkSum == p_buf[index + 10]) {

                obj[0].x = (s16)((s16)((s16)(((u16)(p_buf[index])) & 0xFF) | (u16)(((u16)(p_buf[index + 2] & 0x30)) << 4)));
                obj[0].y =
                    (s16)(WPAD_DPD_IMG_RESO_WY - 1 - (s16)((s16)(((u16)(p_buf[index + 1])) & 0xFF) | (u16)(((u16)(p_buf[index + 2] & 0xC0)) << 2)));
                obj[0].size = (u16)p_wpd->dpdDummyObjSize;
                obj[0].traceId = 0;

                obj[1].x = (s16)((s16)((s16)(((u16)(p_buf[index + 3])) & 0xFF) | (u16)(((u16)(p_buf[index + 2] & (u8)(0x30 >> 4))) << 8)));
                obj[1].y = (s16)(WPAD_DPD_IMG_RESO_WY - 1 -
                                 (s16)((s16)(((u16)(p_buf[index + 4])) & 0xFF) | (u16)(((u16)(p_buf[index + 2] & (u8)(0xC0 >> 4))) << 6)));
                obj[1].size = (u16)p_wpd->dpdDummyObjSize;
                obj[1].traceId = 1;

                obj[2].x = (s16)((s16)((s16)(((u16)(p_buf[index + 5])) & 0xFF) | (u16)(((u16)(p_buf[index + 7] & 0x30)) << 4)));
                obj[2].y =
                    (s16)(WPAD_DPD_IMG_RESO_WY - 1 - (s16)((s16)(((u16)(p_buf[index + 6])) & 0xFF) | (u16)(((u16)(p_buf[index + 7] & 0xC0)) << 2)));
                obj[2].size = (u16)p_wpd->dpdDummyObjSize;
                obj[2].traceId = 2;

                obj[3].x = (s16)((s16)((s16)(((u16)(p_buf[index + 8])) & 0xFF) | (u16)(((u16)(p_buf[index + 7] & (u8)(0x30 >> 4))) << 8)));
                obj[3].y = (s16)(WPAD_DPD_IMG_RESO_WY - 1 -
                                 (s16)((s16)(((u16)(p_buf[index + 9])) & 0xFF) | (u16)(((u16)(p_buf[index + 7] & (u8)(0xC0 >> 4))) << 6)));
                obj[3].size = (u16)p_wpd->dpdDummyObjSize;
                obj[3].traceId = 3;

                for (j = 0; j < WPAD_DPD_MAX_OBJECTS; j++) {
                    if ((obj[j].x < (s16)(WPAD_DPD_IMG_RESO_WX / 2)) && (obj[j].y < (s16)(WPAD_DPD_IMG_RESO_WY / 2))) {
                        p_wpd->conf.obj[0].x = obj[j].x;
                        p_wpd->conf.obj[0].y = obj[j].y;
                        p_wpd->conf.obj[0].size = obj[j].size;
                        p_wpd->conf.obj[0].traceId = obj[j].traceId;
                    }
                    if ((obj[j].x > (s16)(WPAD_DPD_IMG_RESO_WX / 2)) && (obj[j].y < (s16)(WPAD_DPD_IMG_RESO_WY / 2))) {
                        p_wpd->conf.obj[1].x = obj[j].x;
                        p_wpd->conf.obj[1].y = obj[j].y;
                        p_wpd->conf.obj[1].size = obj[j].size;
                        p_wpd->conf.obj[1].traceId = obj[j].traceId;
                    }
                    if ((obj[j].x > (s16)(WPAD_DPD_IMG_RESO_WX / 2)) && (obj[j].y > (s16)(WPAD_DPD_IMG_RESO_WY / 2))) {
                        p_wpd->conf.obj[2].x = obj[j].x;
                        p_wpd->conf.obj[2].y = obj[j].y;
                        p_wpd->conf.obj[2].size = obj[j].size;
                        p_wpd->conf.obj[2].traceId = obj[j].traceId;
                    }
                    if ((obj[j].x < (s16)(WPAD_DPD_IMG_RESO_WX / 2)) && (obj[j].y > (s16)(WPAD_DPD_IMG_RESO_WY / 2))) {
                        p_wpd->conf.obj[3].x = obj[j].x;
                        p_wpd->conf.obj[3].y = obj[j].y;
                        p_wpd->conf.obj[3].size = obj[j].size;
                        p_wpd->conf.obj[3].traceId = obj[j].traceId;
                    }
                }

                break;
            } else {
            }
        }
    } else {
    }

    difaveX = difaveY = 0.0f;
    for (i = 0; i < WPAD_DPD_MAX_OBJECTS; i++) {
        x[i] = (f32)(p_wpd->conf.obj[i].x);
        y[i] = (f32)(p_wpd->conf.obj[i].y);
        difaveX += p_wpd->conf.obj[i].x - dummyObjX[i];
        difaveY += p_wpd->conf.obj[i].y - dummyObjY[i];

    }

    difaveX = (f32)((f32)difaveX / (f32)4.0f);
    difaveY = (f32)((f32)difaveY / (f32)4.0f);
    deltaX = (f32)atan((f32)((f32)((f32)126.5f + difaveX) / (f32)(1332.5f))) - (f32)atan((f32)((f32)126.5f / 1332.5f));
    deltaY = (f32)atan((f32)((f32)((f32)93.0f + difaveY) / (f32)(1337.4f))) - (f32)atan((f32)((f32)93.0f / 1337.4f));
    calibX[chan] = (f32)(-1.0f * (f32)tan(deltaX)) * 1332.5f;
    calibY[chan] = (f32)(-1.0f * (f32)tan(deltaY)) * 1337.4f;


    centerX[chan] = centerY[chan] = 0.0f;
    for (i = 0; i < WPAD_DPD_MAX_OBJECTS; i++) {
        x[i] += calibX[chan];
        y[i] += calibY[chan];
        centerX[chan] += x[i];
        centerY[chan] += y[i];
    }

    centerX[chan] = (f32)((f32)centerX[chan] / (f32)4.0f);
    centerY[chan] = (f32)((f32)centerY[chan] / (f32)4.0f);

    rolag[chan] = 0.0f;
    for (i = 0; i < WPAD_DPD_MAX_OBJECTS; i++) {
        ag[i] = (f32)atan((f32)(y[i] - centerY[chan]) / (f32)(x[i] - centerX[chan]));
        ag_org[i] = (f32)atan((f32)((f32)(dummyObjY[i]) - 383.5f) / (f32)((f32)(dummyObjX[i]) - 511.5f));

        rolag[chan] += ag[i] - ag_org[i];
    }

    rolag[chan] = (f32)((f32)rolag[chan] / (f32)4.0f);

    for (i = 0; i < 2; i++) {
        checkSum = 0;

        index = i * (10);
        if (!p_wpd->oldFw) {
            index += 22;
        }

        for (j = index; j < index + 9; j++) {
            checkSum += p_buf[j];
        }
        checkSum = (u8)(checkSum + 0x55);
        if (checkSum == p_buf[index + 9]) {
            p_wpd->conf.acc_0g.x = (s16)((u16)(((u16)(p_buf[index + 0]) << 2) & 0xFFFC) | (u16)(((u16)(p_buf[index + 3]) >> 4) & 3));
            p_wpd->conf.acc_0g.y = (s16)((u16)(((u16)(p_buf[index + 1]) << 2) & 0xFFFC) | (u16)(((u16)(p_buf[index + 3]) >> 2) & 3));
            p_wpd->conf.acc_0g.z = (s16)((u16)(((u16)(p_buf[index + 2]) << 2) & 0xFFFC) | (u16)(((u16)(p_buf[index + 3])) & 3));
            p_wpd->conf.acc_1g.x = (s16)((u16)(((u16)(p_buf[index + 4]) << 2) & 0xFFFC) | (u16)(((u16)(p_buf[index + 7]) >> 4) & 3));
            p_wpd->conf.acc_1g.y = (s16)((u16)(((u16)(p_buf[index + 5]) << 2) & 0xFFFC) | (u16)(((u16)(p_buf[index + 7]) >> 2) & 3));
            p_wpd->conf.acc_1g.z = (s16)((u16)(((u16)(p_buf[index + 6]) << 2) & 0xFFFC) | (u16)(((u16)(p_buf[index + 7])) & 3));
            p_wpd->conf.volume = (u8)(p_buf[index + 8] & (u8)0x7F);
            p_wpd->conf.motor = (u8)(p_buf[index + 8] & (u8)0x80);
            break;
        } else {
        }
    }
}

NO_INLINE void __wpadGetClConfig(s32 chan, u8* data) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_cb->extConf.cl.lstk.x_max = (s8)data[0];
    p_cb->extConf.cl.lstk.x_min = (s8)data[1];
    p_cb->extConf.cl.lstk.y_max = (s8)data[3];
    p_cb->extConf.cl.lstk.y_min = (s8)data[4];
    p_cb->extConf.cl.rstk.x_max = (s8)data[6];
    p_cb->extConf.cl.rstk.x_min = (s8)data[7];
    p_cb->extConf.cl.rstk.y_max = (s8)data[9];
    p_cb->extConf.cl.rstk.y_min = (s8)data[10];
    if (!p_cb->isSetStickOrigin) {
        p_cb->extConf.cl.lstk.x = (s8)data[2];
        p_cb->extConf.cl.lstk.y = (s8)data[5];
        p_cb->extConf.cl.rstk.x = (s8)data[8];
        p_cb->extConf.cl.rstk.y = (s8)data[11];
        p_cb->extConf.cl.triggerL = data[12];
        p_cb->extConf.cl.triggerR = data[13];
    }
}

static inline void __wpadGetMplsConfig(s32 chan, u8* data, u8 slot) {
    WPADControlBlock* p_cal = _wpdcb[chan];
    p_cal->extConf.mplsCalibration[slot].axis[0].zero = 5000.0f + (f32)((data[4] << 8) | data[5]) / 10.0f;
    p_cal->extConf.mplsCalibration[slot].axis[1].zero = 5000.0f + (f32)((data[0] << 8) | data[1]) / 10.0f;
    p_cal->extConf.mplsCalibration[slot].axis[2].zero = 5000.0f + (f32)((data[2] << 8) | data[3]) / 10.0f;
    p_cal->extConf.mplsCalibration[slot].referenceRate = data[12] * 6;
    if (!p_cal->extConf.mplsCalibration[slot].referenceRate) p_cal->extConf.mplsCalibration[slot].referenceRate = 91;
    p_cal->extConf.mplsCalibration[slot].axis[0].scale = (f32)(s16)((s16)(data[10] << 8) | (s16)data[11]) / 10.0f;
    p_cal->extConf.mplsCalibration[slot].axis[1].scale = (f32)(s16)((s16)(data[6] << 8) | (s16)data[7]) / 10.0f;
    p_cal->extConf.mplsCalibration[slot].axis[2].scale = (f32)(s16)((s16)(data[8] << 8) | (s16)data[9]) / 10.0f;
    {
        f32 scale = p_cal->extConf.mplsCalibration[slot].axis[0].scale;
        if (scale < 0.0f) scale -= 4000.0f;
        else scale = 4000.0f + scale;
        p_cal->extConf.mplsCalibration[slot].axis[0].scale = scale;
    }
    {
        f32 scale = p_cal->extConf.mplsCalibration[slot].axis[1].scale;
        if (scale < 0.0f) scale -= 4000.0f;
        else scale = 4000.0f + scale;
        p_cal->extConf.mplsCalibration[slot].axis[1].scale = scale;
    }
    {
        f32 scale = p_cal->extConf.mplsCalibration[slot].axis[2].scale;
        if (scale < 0.0f) scale -= 4000.0f;
        else scale = 4000.0f + scale;
        p_cal->extConf.mplsCalibration[slot].axis[2].scale = scale;
    }
}

static inline void __wpadGetFsConfig(s32 chan, u8* data) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_cb->extConf.fs.acc_0g.x = (data[0] << 2) | ((data[3] >> 4) & 3);
    p_cb->extConf.fs.acc_0g.y = (data[1] << 2) | ((data[3] >> 2) & 3);
    p_cb->extConf.fs.acc_0g.z = (data[2] << 2) | (data[3] & 3);
    p_cb->extConf.fs.acc_1g.x = (data[4] << 2) | ((data[7] >> 4) & 3);
    p_cb->extConf.fs.acc_1g.y = (data[5] << 2) | ((data[7] >> 2) & 3);
    p_cb->extConf.fs.acc_1g.z = (data[6] << 2) | (data[7] & 3);
    p_cb->extConf.fs.stick.x_max = (s8)data[8];
    p_cb->extConf.fs.stick.x_min = (s8)data[9];
    p_cb->extConf.fs.stick.y_max = (s8)data[11];
    p_cb->extConf.fs.stick.y_min = (s8)data[12];
    if (!p_cb->isSetStickOrigin) {
        p_cb->extConf.fs.stick.x = (s8)data[10];
        p_cb->extConf.fs.stick.y = (s8)data[13];
    }
}

// NONMATCHING: compiler shares repeated calibration paths; target byte decoding, calls and FP operation order retained.
void __wpadGetExtConfig(s32 chan, s32 result) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    u8* data = p_cb->readBufPtr;
    u8 tagHigh, tagLow;
    if (!p_cb->infos.attach) {
        _devType[chan] = 0;
        p_cb->devType = 0;
        _devMode[chan] = 0;
        p_cb->devMode = 0;
        return;
    }
    switch (result) {
    case WPAD_ERR_NO_CONTROLLER:
        p_cb->devType = 253;
        p_cb->extensionPreviousType = 253;
        p_cb->extensionReconnectPending = 0;
        break;
    case WPAD_ERR_NONE:
        p_cb->devType = _devType[chan];
        break;
    default:
        p_cb->devType = 252;
        p_cb->extensionPreviousType = 252;
        p_cb->extensionReconnectPending = 0;
        break;
    }
    p_cb->devMode = _devMode[chan];
    switch ((u8)p_cb->devType) {
    case 1:
        p_cb->extConf.fs.acc_0g.x = p_cb->extConf.fs.acc_0g.y = p_cb->extConf.fs.acc_0g.z = 512;
        p_cb->extConf.fs.acc_1g.x = p_cb->extConf.fs.acc_1g.y = p_cb->extConf.fs.acc_1g.z = 716;
        if (__wpadCheckCalibration(data, 14, 0)) __wpadGetFsConfig(chan, data);
        else if (__wpadCheckCalibration(data + 16, 14, 0)) __wpadGetFsConfig(chan, data + 16);
        break;
    case 2:
        if (__wpadCheckCalibration(data, 14, 0)) __wpadGetClConfig(chan, data);
        else __wpadGetClConfig(chan, data + 16);
        break;
    case 5: case 6: case 7: case 250:
        p_cb->mplsDiscardReports = 5;
        p_cb->extConf.mplsCalibration[0].axis[0].zero = p_cb->extConf.mplsCalibration[0].axis[1].zero = p_cb->extConf.mplsCalibration[0].axis[2].zero = 8192.0f;
        p_cb->extConf.mplsCalibration[0].axis[0].scale = p_cb->extConf.mplsCalibration[0].axis[1].scale = p_cb->extConf.mplsCalibration[0].axis[2].scale = 5333.0f;
        p_cb->extConf.mplsCalibration[0].referenceRate = 1200;
        p_cb->extConf.mplsCalibration[1].axis[0].zero = p_cb->extConf.mplsCalibration[1].axis[1].zero = p_cb->extConf.mplsCalibration[1].axis[2].zero = 8192.0f;
        p_cb->extConf.mplsCalibration[1].axis[0].scale = p_cb->extConf.mplsCalibration[1].axis[1].scale = p_cb->extConf.mplsCalibration[1].axis[2].scale = 5400.0f;
        p_cb->extConf.mplsCalibration[1].referenceRate = 270;
        p_cb->extConf.mplsCalibrationTag = 0xFFFF;
        p_cb->extConf.mplsCalibrationCRC32 = 0xFFFFFFFF;
        tagHigh = data[14];
        tagLow = data[15];
        memcpy(data + 14, data + 16, 14);
        data[28] = tagHigh;
        data[29] = tagLow;
        if (__wpadCheckCalibration(data, 28, 1)) {
            __wpadGetMplsConfig(chan, data, 0);
            __wpadGetMplsConfig(chan, data + 14, 1);
            p_cb->extConf.mplsCalibrationTag = (data[13] << 8) | data[27];
            p_cb->extConf.mplsCalibrationCRC32 = __OSCalcCRC32(data, 28);
        }
        p_cb->certState = 0;
        if (p_cb->certCachedCalibrationTag == p_cb->extConf.mplsCalibrationTag && p_cb->certCachedCalibrationCRC32 == p_cb->extConf.mplsCalibrationCRC32) {
            if (p_cb->certCachedValidity == 1) {
                p_cb->certValidity = 1;
                p_cb->certState = 12;
            }
        } else p_cb->certCachedValidity = 0;
        if (p_cb->certCachedValidity != 1 && p_cb->extensionIdByte7 >= 14) _devType[chan] = p_cb->devType = 252;
        p_cb->certCachedCalibrationTag = p_cb->extConf.mplsCalibrationTag;
        p_cb->certCachedCalibrationCRC32 = p_cb->extConf.mplsCalibrationCRC32;
        break;
    case 3:
        if (p_cb->readLength == 1) return;
        break;
    }
    if (p_cb->extensionCallback) p_cb->extensionCallback(chan, p_cb->devType);
    p_cb->resultCallback = NULL;
}

// NONMATCHING: compiler loop unrolling and register allocation differ; both checksum modes and return values verified.
NO_INLINE BOOL __wpadCheckCalibration(u8* data, u8 length, u8 checksumType) {
    u32 sum = 0;
    BOOL valid = FALSE;
    u8 i;
    if (checksumType == 0) {
        for (i = 0; i < length; i++) sum += data[i];
        if ((u8)(sum + 0x55) == data[length] && (u8)(sum + 0xAA) == data[length + 1]) valid = TRUE;
    } else {
        u32 expected;
        u32 actual = __OSCalcCRC32(data, length);
        expected = *(u32*)(data + length);
        if (actual == expected) valid = TRUE;
    }
    return valid;
}

// NONMATCHING: initial default-store scheduling and relocation identities differ; target calibration fields and state transitions retained.
void __wpadGetExtConfig2(s32 chan, s32 result) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    u8* data = p_cb->readBufPtr;
    WPADControlBlock* p_cal;
    if (p_cb->mplsPassthroughConfigBusy) {
        if (result == WPAD_ERR_NONE) {
            if (_mplsExtensionType[chan] == 0) {
                p_cb->extConf.fs.acc_0g.x = p_cb->extConf.fs.acc_0g.y = p_cb->extConf.fs.acc_0g.z = 512;
                p_cb->extConf.fs.acc_1g.x = p_cb->extConf.fs.acc_1g.y = p_cb->extConf.fs.acc_1g.z = 716;
                if (__wpadCheckCalibration(data, 14, 0)) {
                    p_cal = _wpdcb[chan];
                    p_cal->extConf.fs.acc_0g.x = (data[0] << 2) | ((data[3] >> 4) & 3);
                    p_cal->extConf.fs.acc_0g.y = (data[1] << 2) | ((data[3] >> 2) & 3);
                    p_cal->extConf.fs.acc_0g.z = (data[2] << 2) | (data[3] & 3);
                    p_cal->extConf.fs.acc_1g.x = (data[4] << 2) | ((data[7] >> 4) & 3);
                    p_cal->extConf.fs.acc_1g.y = (data[5] << 2) | ((data[7] >> 2) & 3);
                    p_cal->extConf.fs.acc_1g.z = (data[6] << 2) | (data[7] & 3);
                    p_cal->extConf.fs.stick.x_max = (s8)data[8];
                    p_cal->extConf.fs.stick.x_min = (s8)data[9];
                    p_cal->extConf.fs.stick.y_max = (s8)data[11];
                    p_cal->extConf.fs.stick.y_min = (s8)data[12];
                    if (!p_cal->isSetStickOrigin) {
                        p_cal->extConf.fs.stick.x = (s8)data[10];
                        p_cal->extConf.fs.stick.y = (s8)data[13];
                    }
                    p_cb->mplsPassthroughConfigState = 4;
                } else if (p_cb->mplsPassthroughConfigState == 3) {
                    p_cb->mplsPassthroughConfigState += 3;
                } else p_cb->mplsPassthroughConfigState = 4;
            } else p_cb->mplsPassthroughConfigState = 4;
        } else if (result == WPAD_ERR_TRANSFER) p_cb->mplsPassthroughConfigState = 2;
    }
}

// NONMATCHING: compiler comparison materialization differs.
NO_INLINE BOOL __wpadIsExtEncryptMain(u8 type) {
    if ((u8)(type + 255) <= 3 || (u8)(type + 239) <= 1 || type == 20 || (u8)(type + 235) <= 7) {
        return TRUE;
    }
    return FALSE;
}

extern BOOL __OSInIPL;

// NONMATCHING: compiler switch lowering, scheduling and register allocation differ.
void __wpadGetExtType(s32 chan, s32 result) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADCmdQueue* cmdq = &p_cb->extq;
    u8* data = p_cb->readBuf;
    if (result == WPAD_ERR_NONE) {
        if (!p_cb->infos.attach) {
            _devType[chan] = 0;
            p_cb->devType = 0;
            _devMode[chan] = 0;
            p_cb->devMode = 0;
            return;
        }
        if (!memcmp(data, checkInvalidData, 16) || (data[12] == 0 && data[13] == 0)) {
            _devType[chan] = 252;
            _devMode[chan] = 0;
        } else {
            _devCmpt[chan] = 0;
            _devMode[chan] = data[14];
            switch (data[15]) {
            case 0: _devType[chan] = 1; break;
            case 1: _devType[chan] = 2; break;
            case 2: _devType[chan] = WUDIsLinkedWBC() ? 3 : 251; break;
            case 16: _devType[chan] = WPADIsEnabledGTR() ? 16 : 251; break;
            case 17: _devType[chan] = WPADIsEnabledDevice19() ? 19 : 251; break;
            case 3:
                if (__OSInIPL) {
                    _devType[chan] = 2;
                    _devMode[chan] = 1;
                    _devCmpt[chan] = 1;
                } else {
                    switch (data[10]) {
                    case 0: _devType[chan] = WPADIsEnabledDRM() ? 17 : 251; break;
                    case 1: _devType[chan] = WPADIsEnabledTKO() ? 18 : 251; break;
                    case 3: _devType[chan] = WPADIsEnabledDevice20() ? 20 : 251; break;
                    case 4: _devType[chan] = WPADIsEnabledRawReports() ? 21 : 251; break;
                    case 5: _devType[chan] = WPADIsEnabledRawReports() ? 22 : 251; break;
                    case 6: _devType[chan] = WPADIsEnabledRawReports() ? 23 : 251; break;
                    case 7: _devType[chan] = WPADIsEnabledRawReports() ? 24 : 251; break;
                    case 8: _devType[chan] = WPADIsEnabledRawReports() ? 25 : 251; break;
                    case 9: _devType[chan] = WPADIsEnabledRawReports() ? 26 : 251; break;
                    case 10: _devType[chan] = WPADIsEnabledRawReports() ? 27 : 251; break;
                    case 11: _devType[chan] = WPADIsEnabledRawReports() ? 28 : 251; break;
                    default: _devType[chan] = 251; break;
                    }
                }
                break;
            case 4:
                if (WPADIsEnabledTRN()) {
                    _devType[chan] = 4;
                    p_cb->trnFormatSelector = 0;
                } else _devType[chan] = 251;
                break;
            case 5: {
                s32 mode = data[14];
                if ((4 <= mode && mode <= 5) || mode == 7) {
                    _devType[chan] = 5;
                } else {
                    _devType[chan] = 251;
                }
                p_cb->mplsPassthroughConfigBusy = 0;
                p_cb->mplsPassthroughConfigState = 0;
                p_cb->reservedMplsState = 253;
                _mplsExtensionType[chan] = data[9];
                _mplsExtensionSubtype[chan] = data[8];
                p_cb->mplsInitTicks = 0;
                break;
            }
            default: _devType[chan] = 251; break;
            }
        }
        if (_devType[chan] == 2 && (u8)(_devMode[chan] + 255) > 2) _devType[chan] = 252;
        if (p_cb->extensionReconnectPending && p_cb->extensionPreviousType == _devType[chan]) {
            if (p_cb->isSetStickOrigin) p_cb->isSetStickOrigin = 1;
        } else p_cb->isSetStickOrigin = 0;
        p_cb->extensionPreviousType = _devType[chan];
        p_cb->extensionReconnectPending = 0;
        switch ((u32)_devType[chan]) {
        case 251:
        case 252:
            p_cb->devType = _devType[chan];
            p_cb->devMode = _devMode[chan];
            if (p_cb->extensionCallback) p_cb->extensionCallback(chan, p_cb->devType);
            return;
        case 5:
        case 6:
        case 7:
        case 250:
            resetExtensionCertificate(chan);
            p_cb->extensionIdByte7 = data[7];
            break;
        default:
            if (__wpadIsExtEncryptMain(_devType[chan])) {
                p_cb->extEnc = 2;
                WPADiCreateKey(chan);
            } else {
                p_cb->extEnc = 3;
                WPADiCreateKeyFor3rd(chan);
            }
            WPADiSendWriteDataCmd(cmdq, 0xAA, 0x04A400F0, __wpadAbortInitExtension);
            WPADiSendWriteData(cmdq, p_cb->key, 6, 0x04A40040, __wpadAbortInitExtension);
            WPADiSendWriteData(cmdq, p_cb->key + 6, 6, 0x04A40046, __wpadAbortInitExtension);
            WPADiSendWriteData(cmdq, p_cb->key + 12, 4, 0x04A4004C, __wpadAbortInitExtension);
            if (_devType[chan] == 3) {
                WPADiSendWriteDataCmd(cmdq, 0xAA, 0x04A400F1, __wpadAbortInitExtension);
                WPADiSendWriteDataCmd(cmdq, 0xAA, 0x04A400F1, __wpadAbortInitExtension);
                WPADiSendWriteDataCmd(cmdq, 0xAA, 0x04A400F1, __wpadAbortInitExtension);
            }
            break;
        }
        WPADiSendReadData(cmdq, p_cb->readBuf, 32, 0x04A40020, p_cb->extensionCallback);
    } else {
        p_cb->devType = 252;
        p_cb->devMode = 0;
        p_cb->extensionPreviousType = 252;
        p_cb->extensionReconnectPending = 0;
    }
}

void __wpadGetGameInfo(s32 chan, s32 result, u8 index) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    u8* data = p_cb->readBufPtr;
    int i;
    u32 sum = 0;
    if (result == WPAD_ERR_NONE) {
        for (i = 0; i < 47; i++) sum += data[i];
        if (((WPADMEMGameInfo*)data)->checkSum == (u8)(sum + 0x55)) {
            memcpy(&p_cb->gameInfo, data, sizeof(WPADMEMGameInfo));
            p_cb->gameInfoErr[index] = WPAD_ERR_NONE;
        } else p_cb->gameInfoErr[index] = WPAD_ERR_INVALID;
    } else p_cb->gameInfoErr[index] = WPAD_ERR_INVALID;
}

static inline void initExtension(s32 chan) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADiClearQueue(&p_cb->extq);
    WPADiSendSetReportType(&p_cb->extq, p_cb->dataFormat, p_cb->pwrSave, __wpadAbortInitExtension);
    p_cb->extEnc = 1;
    if (p_cb->mplsReinitPending) {
        WPADiSendWriteDataCmd(&p_cb->extq, 0, 0x04A400FB, __wpadAbortInitExtension);
        WPADiSendReadData(&p_cb->extq, p_cb->readBuf, 16, 0x04A400F0, __wpadAbortInitExtension);
    } else {
        WPADiSendWriteDataCmd(&p_cb->extq, 0x55, 0x04A400F0, __wpadAbortInitExtension);
        WPADiSendWriteDataCmd(&p_cb->extq, 0, 0x04A400FB, __wpadAbortInitExtension);
        WPADiSendReadData(&p_cb->extq, p_cb->readBuf + 10, 6, 0x04A400FA, __wpadAbortInitExtension);
    }
    p_cb->mplsReinitPending = 0;
}

// Descriptive name: validate the requested data format against the actual HID report ID.
void __wpadValidateReportFormat(u8 chan, u8 report, WPADStatus* status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    if ((report == 0x30 && p_cb->dataFormat == 0) ||
        (report == 0x31 && p_cb->dataFormat == 1) ||
        (report == 0x32 && (p_cb->dataFormat == 3 || p_cb->dataFormat == 6 || p_cb->dataFormat == 10)) ||
        (report == 0x33 && p_cb->dataFormat == 2) ||
        (report == 0x34 && p_cb->dataFormat == 12) ||
        (report == 0x35 && (p_cb->dataFormat == 4 || p_cb->dataFormat == 7 || p_cb->dataFormat == 13 || p_cb->dataFormat == 19)) ||
        (report == 0x37 && ((p_cb->dataFormat - 15 <= 3) || p_cb->dataFormat == 5 || p_cb->dataFormat == 8 || p_cb->dataFormat == 11)) ||
        (report == 0x3E && p_cb->dataFormat == 9) || (report == 0x3F && p_cb->dataFormat == 9)) {
        status->err = WPAD_ERR_NONE;
    } else {
        status->err = WPAD_ERR_INVALID;
    }
}

// NONMATCHING: compiler register allocation and scheduling differ.
s32 WPADiHIDParser(u8 chan, u8* data) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADControlBlock* p_buffer;
    WPADStatus* status;
    BOOL enabled;
    s32 result = 0;
    if ((u8)(data[0] + 0xE0) <= 0x1F) {
        enabled = OSDisableInterrupts();
        p_buffer = _wpdcb[chan];
        status = (WPADStatus*)(p_buffer->rxBuf + p_buffer->rxBufIndex * WPAD_RX_DATASIZE);
        if (data[0] != 0x3E && data[0] != 0x3F) memset(status, 0, WPAD_RX_DATASIZE);
        __wpadValidateReportFormat(chan, data[0], status);
        __a1_input_reports_array[data[0] - 0x20](chan, data, status);
        if (!p_cb->setup) status->err = WPAD_ERR_INVALID;
        status->dev = p_cb->devType;
        if (data[0] != 0x3E && data[0] != 0x3F) {
            p_buffer = _wpdcb[chan];
            p_buffer->rxBufIndex = !p_buffer->rxBufIndex;
        }
        OSRestoreInterrupts(enabled);
        WPADiExcludeButton(chan);
        WPADiCopyOut(chan);
    } else result = -1;
    return result;
}

// NONMATCHING: compiler register allocation and short-mask lowering differ.
void __a1_20_status_report(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADControlBlock* p_copy;
    BOOL attach = p_cb->infos.attach;
    BOOL enabled;
    WPADExtensionCallback extcb;
    enabled = OSDisableInterrupts();
    if (!p_cb->setup) {
        OSRestoreInterrupts(enabled);
        return;
    }
    p_cb->infos.attach = (s8)(data[3] & 2) >> 1;
    p_cb->infos.lowBat = data[3] & 1;
    p_cb->infos.led = (data[3] >> 4) & 0xF;
    p_cb->infos.protocol = 0;
    p_cb->infos.firmware = data[5] & 0xF0;
    p_cb->infos.nearempty = (data[1] >> 7) & 1;
    p_cb->infos.dpd = (data[3] >> 3) & 1;
    p_cb->infos.speaker = (data[3] >> 2) & 1;
    if (p_cb->devType == 3) p_cb->infos.battery = p_cb->boardBatteryLevel;
    else if (data[6] >= 0x55) p_cb->infos.battery = 4;
    else if (data[6] >= 0x44) p_cb->infos.battery = 3;
    else if (data[6] >= 0x33) p_cb->infos.battery = 2;
    else if (data[6] >= 3) p_cb->infos.battery = 1;
    else p_cb->infos.battery = 0;
    if (p_cb->infos.attach) {
        if (!attach) {
            _retryCnt[chan] = 0;
            extcb = p_cb->extensionCallback;
            initExtension(chan);
            p_cb->devType = 255;
            p_cb->devMode = 0;
            resetExtensionCertificate(chan);
            if (extcb) extcb(chan, 255);
        }
    } else {
        p_cb->devType = 0;
        p_cb->devMode = 0;
        WPADiClearQueue(&p_cb->extq);
        WPADiSendSetReportType(&p_cb->extq, p_cb->dataFormat, p_cb->pwrSave, NULL);
        if (attach) {
            p_cb->extensionReconnectPending = 1;
            p_cb->extensionReconnectTimer = 300;
            resetExtensionCertificate(chan);
            if (p_cb->extensionCallback) p_cb->extensionCallback(chan, 0);
        }
    }
    if (p_cb->infoBuf) {
        memcpy(p_cb->infoBuf, &p_cb->infos, sizeof(WPADInfo));
        p_cb->infoBuf = NULL;
    }
    p_copy = _wpdcb[chan];
    memcpy(p_status, p_copy->rxBuf + (u32)(p_copy->rxBufIndex == 0) * WPAD_RX_DATASIZE, WPAD_RX_DATASIZE);
    p_status->button = ((data[1] | (u16)((u16)(data[2] << 8) & 0xFF00)) & HID_WPAD_BUTTON_MASK) | (p_status->button & 0x6000);
    if (p_status->dev != p_cb->devType) {
        p_status->dev = p_cb->devType;
        p_status->err = WPAD_ERR_INVALID;
    }
    if (p_cb->resultCallback && p_cb->getStatFlag) {
        p_cb->resultCallback(chan, WPAD_ERR_NONE);
        p_cb->resultCallback = NULL;
    }
    p_cb->getStatFlag = 0;
    OSRestoreInterrupts(enabled);
}

// NONMATCHING: compiler register allocation and comparison lowering differ.
void __a1_21_user_data(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADControlBlock* p_copy;
    WPADControlBlock* p_mpls;
    u8* p_buf;
    u8 err, len;
    u16 base, i2c, addr;
    s16 offset;
    s32 result;
    BOOL enabled = OSDisableInterrupts();
    err = data[3] & 0xF;
    if (err) {
        p_cb->readError = -1;
        if (p_cb->resultCallback) {
            if (!p_cb->extensionCallback || p_cb->extensionCallback != p_cb->resultCallback) p_cb->resultCallback(chan, WPAD_ERR_TRANSFER);
            p_cb->resultCallback = NULL;
        }
        p_cb->status = WPAD_ERR_NONE;
    }
    base = p_cb->readBaseAddr;
    i2c = p_cb->readBaseAddr >> 16;
    len = (data[3] >> 4) + 1;
    addr = (data[4] << 8) | data[5];
    offset = addr - base;
    if (addr >= base && addr <= base + p_cb->readLength) {
        if (!err) memcpy((u8*)p_cb->readBufPtr + offset, data + 6, len);
        if (base + p_cb->readLength == addr + len) {
            result = p_cb->readError < 0 ? WPAD_ERR_TRANSFER : WPAD_ERR_NONE;
            if (i2c == 0x04A4 && (p_cb->extEnc == 2 || p_cb->extEnc == 3)) WPADiDecode(chan, p_cb->readBufPtr, p_cb->readLength, base);
            if ((p_cb->readBaseAddr == 0 && !p_cb->oldFw) || (p_cb->readBaseAddr == 0x176C && p_cb->oldFw == 1)) __wpadGetDevConfig(chan, result);
            if (p_cb->readBaseAddr == 0x04A40020) __wpadGetExtConfig(chan, result);
            if (p_cb->readBaseAddr == 0x04A400F0 || p_cb->readBaseAddr == 0x04A400FA) __wpadGetExtType(chan, result);
            if (p_cb->readBaseAddr == 0x04A400F6) {
                p_mpls = _wpdcb[chan];
                p_buf = p_mpls->readBufPtr;
                if (p_mpls->mplsPassthroughConfigBusy && p_mpls->mplsPassthroughConfigState == 1) {
                    if (result != WPAD_ERR_NONE) p_mpls->mplsPassthroughConfigState = 0;
                    else {
                        p_mpls->mplsPassthroughConfigState = 2;
                        p_mpls->isSetStickOrigin = 0;
                        _mplsExtensionType[chan] = p_buf[3];
                        _mplsExtensionSubtype[chan] = p_buf[2];
                    }
                }
            }
            if (p_cb->readBaseAddr == 0x04A40040) __wpadGetExtConfig2(chan, result);
            if (p_cb->readBaseAddr == 0x04A400F3) {
                if (p_cb->mplsRegisterF3 == 3) p_cb->mplsPassthroughConfigState = 9;
                else if (p_cb->mplsRegisterF3 == 4) p_cb->mplsPassthroughConfigState = 6;
            }
            if (p_cb->readBaseAddr == 0x2A) __wpadGetGameInfo(chan, result, 0);
            if (p_cb->readBaseAddr == 0x62) __wpadGetGameInfo(chan, result, 1);
            if (p_cb->resultCallback) {
                p_cb->resultCallback(chan, result);
                p_cb->resultCallback = NULL;
            }
            p_cb->status = WPAD_ERR_NONE;
        }
    }
    p_copy = _wpdcb[chan];
    memcpy(p_status, p_copy->rxBuf + (u32)(p_copy->rxBufIndex == 0) * WPAD_RX_DATASIZE, WPAD_RX_DATASIZE);
    p_status->button = ((data[1] | (u16)((u16)(data[2] << 8) & 0xFF00)) & HID_WPAD_BUTTON_MASK) | (p_status->button & 0x6000);
    if (p_status->dev != p_cb->devType) {
        p_status->dev = p_cb->devType;
        p_status->err = WPAD_ERR_INVALID;
    }
    OSRestoreInterrupts(enabled);
}

// NONMATCHING: compiler register allocation and comparison lowering differ.
void __a1_22_ack(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADControlBlock* p_copy;
    BOOL enabled = OSDisableInterrupts();
    u8 response;
    s32 result;
    u16 buttons;
    p_copy = _wpdcb[chan];
    memcpy(p_status, p_copy->rxBuf + (u32)(p_copy->rxBufIndex == 0) * WPAD_RX_DATASIZE, WPAD_RX_DATASIZE);
    buttons = (u16)(data[2] << 8);
    buttons |= data[1];
    p_status->button = (buttons & HID_WPAD_BUTTON_MASK) | (p_status->button & 0x6000);
    if (p_status->dev != p_cb->devType) {
        p_status->dev = p_cb->devType;
        p_status->err = WPAD_ERR_INVALID;
    }
    response = data[3];
    result = data[4] == 0 ? WPAD_ERR_NONE : WPAD_ERR_TRANSFER;
    if (p_cb->cmdId == response) {
        if (p_cb->resultCallback) {
            p_cb->resultCallback(chan, result);
            p_cb->resultCallback = NULL;
        }
        p_cb->status = WPAD_ERR_NONE;
    }
    OSRestoreInterrupts(enabled);
}

static const f32 PI = 3.141592f;

// NONMATCHING: identical instruction words and scalar pool; generated BSS/literal relocation identities differ.
void __parse_dpd_data(s32 chan, WPADStatus** p_status, u8 fmt, u8* p_data, u8 len) {
    u8 i;
    u8 x;
    u8 y;
    u8 xy;
    f32 a;
    f32 b;

    if (_wpdcb[chan]->dpdChangePending) {
        (*p_status)->err = WPAD_ERR_INVALID;
        return;
    }
    if (fmt == 3) {
        for (i = 0; i < WPAD_DPD_MAX_OBJECTS; i++) {
            if (i * 3 + 2 < len) {
                x = *(p_data + i * 3);
                y = *(p_data + i * 3 + 1);
                xy = *(p_data + i * 3 + 2);

                (*p_status)->obj[i].x = (s16)((s16)((s16)(((u16)x) & 0xFF) | (u16)(((u16)xy & 0x30) << 4)));
                (*p_status)->obj[i].y = (s16)(WPAD_DPD_IMG_RESO_WY - 1 - (s16)((s16)(((u16)y) & 0xFF) | (u16)(((u16)xy & 0xC0) << 2)));
                (*p_status)->obj[i].size = (u16)(((xy & 0xF) & 0xFF));
                (*p_status)->obj[i].size = (u8)((f32)((*p_status)->obj[i].size) * (f32)((*p_status)->obj[i].size) * PI);
                if ((*p_status)->obj[i].size == 0 || (*p_status)->obj[i].x == (s16)(WPAD_DPD_IMG_RESO_WX - 1) ||
                    (*p_status)->obj[i].y == (s16)(WPAD_DPD_IMG_RESO_WY - 1)) {
                    (*p_status)->obj[i].x = (s16)0;
                    (*p_status)->obj[i].y = (s16)(WPAD_DPD_IMG_RESO_WY - 1);
                    (*p_status)->obj[i].size = 0;
                }
                (*p_status)->obj[i].traceId = i;
            } else {
                (*p_status)->obj[i].x = (s16)0;
                (*p_status)->obj[i].y = (s16)(WPAD_DPD_IMG_RESO_WY - 1);
                (*p_status)->obj[i].size = 0;
                (*p_status)->obj[i].traceId = i;
            }
        }
    } else if (fmt == 1) {
        for (i = 0; i < WPAD_DPD_MAX_OBJECTS; i++) {
            x = *(p_data + i * 3 - (i / 2));
            y = *(p_data + i * 3 - (i / 2) + 1);
            xy = *(p_data + (i / 2) * 5 + 2);

            if ((i % 2) == 0) {
                (*p_status)->obj[i].x = (s16)((s16)((s16)(((u16)x) & 0xFF) | (u16)(((u16)(xy & 0x30)) << 4)));
                (*p_status)->obj[i].y = (s16)(WPAD_DPD_IMG_RESO_WY - 1 - (s16)((s16)(((u16)y) & 0xFF) | (u16)(((u16)(xy & 0xC0)) << 2)));
            } else {
                (*p_status)->obj[i].x = (s16)((s16)((s16)(((u16)x) & 0xFF) | (u16)(((u16)(xy & (u8)(0x30 >> 4))) << 8)));
                (*p_status)->obj[i].y = (s16)(WPAD_DPD_IMG_RESO_WY - 1 - (s16)((s16)(((u16)y) & 0xFF) | (u16)(((u16)(xy & (u8)(0xC0 >> 4))) << 6)));
            }
            if ((*p_status)->obj[i].x == (s16)(WPAD_DPD_IMG_RESO_WX - 1) || (*p_status)->obj[i].y == (s16)(WPAD_DPD_IMG_RESO_WY - 1)) {
                (*p_status)->obj[i].x = (s16)0;
                (*p_status)->obj[i].y = (s16)(WPAD_DPD_IMG_RESO_WY - 1);
                (*p_status)->obj[i].size = 0;
            } else {
                (*p_status)->obj[i].size = 12;
            }
            (*p_status)->obj[i].traceId = i;
        }
    }

    for (i = 0; i < WPAD_DPD_MAX_OBJECTS; i++) {
        if ((*p_status)->obj[i].x != 0 || (*p_status)->obj[i].y != (WPAD_DPD_IMG_RESO_WY - 1)) {
            WPADStatus* p_cur = *p_status;
            a = (((f32)(*p_status)->obj[i].x) + calibX[chan] - centerX[chan]);
            b = (((f32)p_cur->obj[i].y) + calibY[chan] - centerY[chan]);
            p_cur->obj[i].x =
                (s16)((f32)((f32)a * (f32)cos(-1.0f * rolag[chan])) - (f32)((f32)b * (f32)sin(-1.0f * rolag[chan])) + centerX[chan]);
            (*p_status)->obj[i].y =
                (s16)((f32)((f32)a * (f32)sin(-1.0f * rolag[chan])) + (f32)((f32)b * (f32)cos(-1.0f * rolag[chan])) + centerY[chan]);
        }
    }
}

void __a1_30_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_status->button = (data[1] | (u16)((u16)(data[2] << 8) & 0xFF00)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = (data[1] >> 7) & 1;
}

void __a1_31_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADControlBlock* p_cal;
    p_status->button = (data[1] | (u16)((u16)(data[2] << 8) & 0xFF00)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = (data[1] >> 7) & 1;
    p_cal = _wpdcb[chan];
    p_status->accX = (s16)((s16)((s16)(data[3] << 2) & (s16)0xFFFC) | ((data[1] >> 5) & 3)) - p_cal->conf.acc_0g.x;
    p_status->accY = (s16)((s16)((s16)(data[4] << 2) & (s16)0xFFFC) | (s16)((data[2] >> 4) & 2)) - p_cal->conf.acc_0g.y;
    p_status->accZ = (s16)((s16)((s16)(data[5] << 2) & (s16)0xFFFC) | (s16)((data[2] >> 5) & 2)) - p_cal->conf.acc_0g.z;
}

void __a1_32_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_status->button = (data[1] | (u16)((u16)(data[2] << 8) & 0xFF00)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = (data[1] >> 7) & 1;
    __parse_ext_data(chan, p_status, p_cb->devMode, data + 3, 8);
}

void __a1_33_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADControlBlock* p_cal;
    p_status->button = (data[1] | (data[2] << 8)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = (data[1] >> 7) & 1;
    p_cal = _wpdcb[chan];
    p_status->accX = (s16)((s16)((s16)(data[3] << 2) & (s16)0xFFFC) | ((data[1] >> 5) & 3)) - p_cal->conf.acc_0g.x;
    p_status->accY = (s16)((s16)((s16)(data[4] << 2) & (s16)0xFFFC) | (s16)((data[2] >> 4) & 2)) - p_cal->conf.acc_0g.y;
    p_status->accZ = (s16)((s16)((s16)(data[5] << 2) & (s16)0xFFFC) | (s16)((data[2] >> 5) & 2)) - p_cal->conf.acc_0g.z;
    __parse_dpd_data(chan, &p_status, p_cb->dpdFormat, data + 6, 12);
}

void __a1_34_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_status->button = (data[1] | (u16)((u16)(data[2] << 8) & 0xFF00)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = (data[1] >> 7) & 1;
    __parse_ext_data(chan, p_status, p_cb->devMode, data + 3, 19);
}

void __a1_35_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cal;
    WPADControlBlock* p_cb = _wpdcb[chan];
    p_status->button = (data[1] | (u16)((u16)(data[2] << 8) & 0xFF00)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = (data[1] >> 7) & 1;
    p_cal = _wpdcb[chan];
    p_status->accX = (s16)((s16)((s16)(data[3] << 2) & (s16)0xFFFC) | ((data[1] >> 5) & 3)) - p_cal->conf.acc_0g.x;
    p_status->accY = (s16)((s16)((s16)(data[4] << 2) & (s16)0xFFFC) | (s16)((data[2] >> 4) & 2)) - p_cal->conf.acc_0g.y;
    p_status->accZ = (s16)((s16)((s16)(data[5] << 2) & (s16)0xFFFC) | (s16)((data[2] >> 5) & 2)) - p_cal->conf.acc_0g.z;
    __parse_ext_data(chan, p_status, p_cb->devMode, data + 6, 16);
}

void __a1_36_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADStatus* dpdStatus = p_status;
    p_status->button = (data[1] | (data[2] << 8)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = (data[1] >> 7) & 1;
    __parse_dpd_data(chan, &dpdStatus, p_cb->dpdFormat, data + 3, 10);
    __parse_ext_data(chan, p_status, p_cb->devMode, data + 13, 9);
}

void __a1_37_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADStatus* dpdStatus = p_status;
    WPADControlBlock* p_cal;
    dpdStatus->button = (data[1] | (data[2] << 8)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = (data[1] >> 7) & 1;
    p_cal = _wpdcb[chan];
    dpdStatus->accX = (s16)((s16)((s16)(data[3] << 2) & (s16)0xFFFC) | ((data[1] >> 5) & 3)) - p_cal->conf.acc_0g.x;
    dpdStatus->accY = (s16)((s16)((s16)(data[4] << 2) & (s16)0xFFFC) | (s16)((data[2] >> 4) & 2)) - p_cal->conf.acc_0g.y;
    dpdStatus->accZ = (s16)((s16)((s16)(data[5] << 2) & (s16)0xFFFC) | (s16)((data[2] >> 5) & 2)) - p_cal->conf.acc_0g.z;
    __parse_dpd_data(chan, &dpdStatus, p_cb->dpdFormat, data + 6, 10);
    __parse_ext_data(chan, p_status, p_cb->devMode, data + 16, 6);
}
void __a1_3d_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    __parse_ext_data(chan, p_status, p_cb->devMode, data + 1, 21);
}


// NONMATCHING: compiler register allocation and expression lowering differ.
void __parse_dpdex_data(s32 chan, WPADStatus** p_status, u8 index, u8* data, u8 len) {
#define EX_STATUS ((WPADStatusEx*)*p_status)
    EX_STATUS->obj[index].x = (s16)((data[2] & 0x30) << 4) | (s16)data[0];
    EX_STATUS->obj[index].y = 767 - (s16)((s16)((data[2] & 0xC0) << 2) | (s16)data[1]);
    EX_STATUS->exp[index].pixel = ((s16)((u8)data[7] << 8) | data[8]) * 64;
    EX_STATUS->exp[index].radius = data[2] & 0xF;
    EX_STATUS->exp[index].range_x1 = (s8)data[3] == -1 ? 0 : data[3];
    EX_STATUS->exp[index].range_y1 = (s8)data[4] == -1 ? 0 : data[4];
    EX_STATUS->exp[index].range_x2 = (s8)data[5] == -1 ? 0 : data[5];
    EX_STATUS->exp[index].range_y2 = (s8)data[6] == -1 ? 0 : data[6];
    EX_STATUS->exp[index].range_x1 *= 8;
    EX_STATUS->exp[index].range_y1 = 767 - (s16)(EX_STATUS->exp[index].range_y1 * 8);
    EX_STATUS->exp[index].range_x2 *= 8;
    EX_STATUS->exp[index].range_y2 = 767 - (s16)(EX_STATUS->exp[index].range_y2 * 8);
    EX_STATUS->obj[index].size = (f32)EX_STATUS->exp[index].radius * (f32)EX_STATUS->exp[index].radius * PI;
    if (EX_STATUS->obj[index].size == 0 || EX_STATUS->obj[index].x == 1023 || EX_STATUS->obj[index].y == 767 || EX_STATUS->exp[index].radius == 15) {
        EX_STATUS->obj[index].x = 0;
        EX_STATUS->obj[index].y = 767;
        EX_STATUS->obj[index].size = 0;
        EX_STATUS->exp[index].pixel = 0;
        EX_STATUS->exp[index].radius = 0;
    }
    EX_STATUS->obj[index].traceId = index;
#undef EX_STATUS
}

// NONMATCHING: compiler register allocation, short casts and scheduling differ.
void __a1_3e_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADStatus* dpdStatus = p_status;
    WPADControlBlock* p_cal;
    BOOL enabled;
    if (!p_cb->dpdExtendedReportMask) memset(p_status, 0, WPAD_RX_DATASIZE);
    p_status->button = (data[1] | (data[2] << 8)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = 0;
    p_cal = _wpdcb[chan];
    p_status->accX = (s16)((s16)((s16)(data[3] << 2) & (s16)0xFFFC) | (s16)((data[1] >> 6) & 2)) - p_cal->conf.acc_0g.x;
    p_status->accZ |= (s16)((s16)((s16)(data[2] << 3) & (s16)0xFF00) | ((data[1] << 1) & 0xC0));
    __parse_dpdex_data(chan, &dpdStatus, 0, data + 4, 0);
    __parse_dpdex_data(chan, &dpdStatus, 1, data + 13, 0);
    enabled = OSDisableInterrupts();
    p_cb->dpdExtendedReportMask |= 1;
    if (p_cb->dpdExtendedReportMask == 3) {
        p_status->accZ -= p_cb->conf.acc_0g.z;
        p_cb->rxBufIndex = !p_cb->rxBufIndex;
        p_cb->dpdExtendedReportMask = 0;
    }
    OSRestoreInterrupts(enabled);
}

// NONMATCHING: compiler register allocation, short casts and scheduling differ.
void __a1_3f_data_type(u8 chan, u8* data, WPADStatus* p_status) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADStatus* dpdStatus = p_status;
    WPADControlBlock* p_cal;
    BOOL enabled;
    if (!p_cb->dpdExtendedReportMask) memset(p_status, 0, WPAD_RX_DATASIZE);
    p_status->button = (data[1] | (data[2] << 8)) & HID_WPAD_BUTTON_MASK;
    p_cb->infos.nearempty = 0;
    p_cal = _wpdcb[chan];
    p_status->accY = (s16)((s16)((s16)(data[3] << 2) & (s16)0xFFFC) | (s16)((data[1] >> 6) & 2)) - p_cal->conf.acc_0g.y;
    p_status->accZ |= (s16)(((data[2] >> 1) & 0x30) | ((data[1] >> 3) & 0xC));
    __parse_dpdex_data(chan, &dpdStatus, 2, data + 4, 0);
    __parse_dpdex_data(chan, &dpdStatus, 3, data + 13, 0);
    enabled = OSDisableInterrupts();
    p_cb->dpdExtendedReportMask |= 2;
    if (p_cb->dpdExtendedReportMask == 3) {
        p_status->accZ -= p_cb->conf.acc_0g.z;
        p_cb->rxBufIndex = !p_cb->rxBufIndex;
        p_cb->dpdExtendedReportMask = 0;
    }
    OSRestoreInterrupts(enabled);
}

void __a1_unused_report(u8 chan, u8* data, WPADStatus* p_status) {}

void __parse_bl_data(s32 chan, WPADStatus** p_status, u8 fmt, u8* p_data, u8 len) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    s32 level;

    ((WPADBLStatus*)*p_status)->pressure[0] = (u16)((u16)(p_data[0] << 8) & 0xFF00) | p_data[1];
    ((WPADBLStatus*)*p_status)->pressure[1] = (u16)((u16)(p_data[2] << 8) & 0xFF00) | p_data[3];
    ((WPADBLStatus*)*p_status)->pressure[2] = (u16)((u16)(p_data[4] << 8) & 0xFF00) | p_data[5];
    ((WPADBLStatus*)*p_status)->pressure[3] = (u16)((u16)(p_data[6] << 8) & 0xFF00) | p_data[7];
    ((WPADBLStatus*)*p_status)->temperature = p_data[8];
    ((WPADBLStatus*)*p_status)->battery = p_data[10];

    level = ((WPADBLStatus*)*p_status)->battery * 2;
    if (level >= 260) {
        p_cb->boardBatteryLevel = 4;
    } else if ((u32)(level - 250) <= 9) {
        p_cb->boardBatteryLevel = 3;
    } else if ((u32)(level - 240) <= 9) {
        p_cb->boardBatteryLevel = 2;
    } else if ((u32)(level - 212) <= 27) {
        p_cb->boardBatteryLevel = 1;
    } else {
        p_cb->boardBatteryLevel = 0;
    }
    if (!p_cb->isSetStickOrigin) {
        p_cb->isSetStickOrigin = 1;
    }
}

// NONMATCHING: compiler register allocation, branch sharing and lowering differ.
void __parse_mp_data(s32 chan, WPADStatus** p_status, u8 mode, u8* data, u8 len) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADMPStatus* previous;
    u8 notify;
    s16 x, y;
#define MP_REPORT ((WPADMPStatus*)*p_status)
#define FS_REPORT ((WPADFSStatus*)*p_status)
#define CL_REPORT ((WPADCLStatus*)*p_status)

    if (p_cb->mplsDiscardReports != 0) {
        --p_cb->mplsDiscardReports;
        (*p_status)->err = WPAD_ERR_INVALID;
    }
    if ((*p_status)->err != WPAD_ERR_NONE) {
        return;
    }
    if (mode == 4) {
        if ((data[5] & 3) == 2) {
            MP_REPORT->status = (data[4] & 3) | ((data[3] & 3) << 2) | 0x80;
            MP_REPORT->rotation[0] = (s16)data[2] | (s16)((data[5] & 0xFC) << 6);
            MP_REPORT->rotation[1] = (s16)data[0] | (s16)((data[3] & 0xFC) << 6);
            MP_REPORT->rotation[2] = (s16)data[1] | (s16)((data[4] & 0xFC) << 6);
        } else {
            (*p_status)->err = WPAD_ERR_CORRUPTED;
        }
    } else if (mode == 5 || mode == 7) {
        previous = (WPADMPStatus*)(p_cb->rxBuf + (u8)(p_cb->rxBufIndex == 0) * WPAD_RX_DATASIZE);
        if (data[5] & 1) {
            MP_REPORT->status = 0;
            (*p_status)->err = WPAD_ERR_CORRUPTED;
        } else if (!(data[5] & 2)) {
            MP_REPORT->rotation[0] = previous->rotation[0];
            MP_REPORT->rotation[1] = previous->rotation[1];
            MP_REPORT->rotation[2] = previous->rotation[2];
            MP_REPORT->status = previous->status;
            if ((data[0] == 0xFF && data[1] == 0xFF) || p_cb->mplsPassthroughConfigState != 5 ||
                (p_cb->devType == WPAD_DEV_MPLS_FREESTYLE && p_cb->devMode != 5) ||
                (p_cb->devType == WPAD_DEV_MPLS_CLASSIC && p_cb->devMode != 7) || p_cb->devType == WPAD_DEV_MPLS_FUTURE) {
                MP_REPORT->status &= 0xBF;
            } else {
                MP_REPORT->status |= 0x40 | (data[4] & 1);
                if (p_cb->devType == WPAD_DEV_MPLS_FREESTYLE) {
                    FS_REPORT->fsStickX = data[0];
                    FS_REPORT->fsStickY = data[1];
                    FS_REPORT->fsAccX = (s16)((s16)((s16)((s16)(data[2] << 2) & (s16)~3) | (s16)((data[5] >> 3) & 2)) - p_cb->extConf.fs.acc_0g.x);
                    FS_REPORT->fsAccY = (s16)((s16)((s16)((s16)(data[3] << 2) & (s16)~3) | (s16)((data[5] >> 4) & 2)) - p_cb->extConf.fs.acc_0g.y);
                    FS_REPORT->fsAccZ = (s16)((s16)((s16)((s16)(data[4] << 2) & (s16)~7) | ((data[5] >> 5) & 6)) - p_cb->extConf.fs.acc_0g.z);
                    (*p_status)->button |= ((~data[5]) << 11) & 0x6000;
                    if (!p_cb->isSetStickOrigin) {
                        p_cb->isSetStickOrigin = 1;
                        p_cb->extConf.fs.stick.x = FS_REPORT->fsStickX;
                        p_cb->extConf.fs.stick.y = FS_REPORT->fsStickY;
                    }
                    x = (s16)((u8)FS_REPORT->fsStickX - (u8)p_cb->extConf.fs.stick.x);
                    y = (s16)((u8)FS_REPORT->fsStickY - (u8)p_cb->extConf.fs.stick.y);
                    if (x < -128) FS_REPORT->fsStickX = -128;
                    else if (x > 127) FS_REPORT->fsStickX = 127;
                    else FS_REPORT->fsStickX = x;
                    if (y < -128) FS_REPORT->fsStickY = -128;
                    else if (y > 127) FS_REPORT->fsStickY = 127;
                    else FS_REPORT->fsStickY = y;
                } else if (p_cb->devType == WPAD_DEV_MPLS_CLASSIC) {
                    CL_REPORT->clLStickX = (data[0] << 4) & 0x3E0;
                    CL_REPORT->clLStickY = (data[1] << 4) & 0x3E0;
                    CL_REPORT->clRStickX = (s16)((s16)(data[2] >> 7) | (((data[1] >> 5) & 6) | ((data[0] >> 3) & 0x18))) << 5;
                    CL_REPORT->clRStickY = (data[2] & 0x1F) << 5;
                    CL_REPORT->clTriggerL = (s16)(((data[3] >> 5) & 7) | ((data[2] >> 2) & 0x18)) << 3;
                    CL_REPORT->clTriggerR = (data[3] & 0x1F) << 3;
                    CL_REPORT->clButton = ((data[0] & 1) | ((data[1] & 1) << 1) | (data[5] & 0xFC) | ((data[4] & 0xFE) << 8)) ^ 0xFEFF;
                    CL_REPORT->clLStickX -= 512;
                    CL_REPORT->clLStickY -= 512;
                    CL_REPORT->clRStickX -= 512;
                    CL_REPORT->clRStickY -= 512;
                    if (!p_cb->isSetStickOrigin) {
                        p_cb->isSetStickOrigin = 1;
                        p_cb->extConf.cl.lstk.x = CL_REPORT->clLStickX;
                        p_cb->extConf.cl.lstk.y = CL_REPORT->clLStickY;
                        p_cb->extConf.cl.rstk.x = CL_REPORT->clRStickX;
                        p_cb->extConf.cl.rstk.y = CL_REPORT->clRStickY;
                        p_cb->extConf.cl.triggerL = CL_REPORT->clTriggerL;
                        p_cb->extConf.cl.triggerR = CL_REPORT->clTriggerR;
                    }
                    x = (s16)(CL_REPORT->clLStickX - p_cb->extConf.cl.lstk.x);
                    y = x;
                    if (x < -512) y = -512;
                    if (x > 511) y = 511;
                    CL_REPORT->clLStickX = y;
                    x = (s16)(CL_REPORT->clLStickY - p_cb->extConf.cl.lstk.y);
                    y = x;
                    if (x < -512) y = -512;
                    if (x > 511) y = 511;
                    CL_REPORT->clLStickY = y;
                    x = (s16)(CL_REPORT->clRStickX - p_cb->extConf.cl.rstk.x);
                    y = x;
                    if (x < -512) y = -512;
                    if (x > 511) y = 511;
                    CL_REPORT->clRStickX = y;
                    x = (s16)(CL_REPORT->clRStickY - p_cb->extConf.cl.rstk.y);
                    y = x;
                    if (x < -512) y = -512;
                    if (x > 511) y = 511;
                    CL_REPORT->clRStickY = y;
                    if (CL_REPORT->clTriggerL > 72) CL_REPORT->clButton |= 0x2000;
                    if (CL_REPORT->clTriggerR > 72) CL_REPORT->clButton |= 0x0200;
                    CL_REPORT->clTriggerL = 0;
                    CL_REPORT->clTriggerR = 0;
                }
            }
        } else {
            MP_REPORT->extension = previous->extension;
            (*p_status)->button |= previous->core.button & 0x6000;
            MP_REPORT->status = (data[4] & 3) | ((data[3] & 3) << 2) | 0x80;
            if (MP_REPORT->status & 1) {
                MP_REPORT->status |= previous->status & 0x40;
            }
            MP_REPORT->rotation[0] = (s16)data[2] | (s16)((data[5] & 0xFC) << 6);
            MP_REPORT->rotation[1] = (s16)data[0] | (s16)((data[3] & 0xFC) << 6);
            MP_REPORT->rotation[2] = (s16)data[1] | (s16)((data[4] & 0xFC) << 6);
        }
    } else {
        (*p_status)->err = WPAD_ERR_CORRUPTED;
    }

    if ((*p_status)->err != WPAD_ERR_NONE) {
        return;
    }
    notify = p_cb->mplsPassthroughConfigBusy;
    if (!(MP_REPORT->status & 1)) {
        if (p_cb->mplsPassthroughConfigState != 5) notify = 0;
        p_cb->mplsPassthroughConfigBusy = 0;
        p_cb->mplsPassthroughConfigState = 0;
        p_cb->devType = WPAD_DEV_MPLS;
        MP_REPORT->status &= 0xBE;
        if (notify && p_cb->extensionCallback) {
            p_cb->extensionCallback(chan, p_cb->devType);
        }
    } else if (!notify && p_cb->mplsPassthroughConfigState == 0) {
        if (WPADiSendReadData(&p_cb->extq, p_cb->readBuf, 4, 0x04A400F6, NULL)) {
            p_cb->mplsPassthroughConfigBusy = 1;
            p_cb->mplsPassthroughConfigState = 1;
        }
    }

    if (p_cb->mplsPassthroughConfigState == 2 || p_cb->mplsPassthroughConfigState == 9) {
        if (WPADiSendReadData(&p_cb->extq, p_cb->readBuf, 16, 0x04A40040, NULL)) {
            ++p_cb->mplsPassthroughConfigState;
        }
    } else if (p_cb->mplsPassthroughConfigState == 4) {
        p_cb->mplsPassthroughConfigState = 5;
        switch (_mplsExtensionType[chan]) {
        case 0: p_cb->devType = WPAD_DEV_MPLS_FREESTYLE; break;
        case 1: p_cb->devType = WPAD_DEV_MPLS_CLASSIC; break;
        case 3:
            if (__OSInIPL) {
                p_cb->devType = WPAD_DEV_MPLS_CLASSIC;
            } else {
                p_cb->devType = WPAD_DEV_MPLS_FUTURE;
            }
            break;
        default: p_cb->devType = WPAD_DEV_MPLS_FUTURE; break;
        }
        if (p_cb->extensionCallback) p_cb->extensionCallback(chan, p_cb->devType);
    } else if (p_cb->mplsPassthroughConfigState == 6) {
        if (WPADiIsAvailableCmdQueue(&p_cb->extq, 2)) {
            WPADiSendWriteDataCmd(&p_cb->extq, 1, 0x04A400F3, NULL);
            WPADiSendReadData(&p_cb->extq, &p_cb->mplsRegisterF3, 1, 0x04A400F3, NULL);
            p_cb->mplsPassthroughConfigState = 7;
        }
    }
#undef MP_REPORT
#undef FS_REPORT
#undef CL_REPORT
}

// NONMATCHING: compiler shares clamp paths and lowers packed fields differently.
static inline s16 __wpadClampStick(s16 value, s16 minimum, s16 maximum) {
    s16 result = value;
    if (value < minimum) result = minimum;
    if (value > maximum) result = maximum;
    return result;
}

void __parse_cl_data(s32 chan, WPADStatus** p_status, u8 mode, u8* data, u8 len) {
    WPADControlBlock* p_cb = _wpdcb[chan];
#define CL_STATUS ((WPADCLStatus*)*p_status)
    switch (mode) {
    case 2:
        CL_STATUS->clLStickX = (s16)((s16)(data[0] << 2) & (s16)0xFFFC) | (data[4] & 3);
        CL_STATUS->clRStickX = (s16)((s16)(data[1] << 2) & (s16)0xFFFC) | ((s16)(data[4] >> 2) & 3);
        CL_STATUS->clLStickY = (s16)((s16)(data[2] << 2) & (s16)0xFFFC) | ((s16)(data[4] >> 4) & 3);
        CL_STATUS->clRStickY = (s16)((s16)(data[3] << 2) & (s16)0xFFFC) | (s16)(data[4] >> 6);
        CL_STATUS->clTriggerL = data[5];
        CL_STATUS->clTriggerR = len < 9 ? 0 : data[6];
        CL_STATUS->clButton = len < 9 ? 0 : ((data[7] << 8) | data[8]) ^ 0xFFFF;
        break;
    case 3: {
        s16 raw;
        raw = data[0];
        CL_STATUS->clLStickX = raw << 2;
        raw = data[1];
        CL_STATUS->clRStickX = raw << 2;
        raw = data[2];
        CL_STATUS->clLStickY = raw << 2;
        raw = data[3];
        CL_STATUS->clRStickY = raw << 2;
        CL_STATUS->clTriggerL = data[4];
        CL_STATUS->clTriggerR = data[5];
        CL_STATUS->clButton = len < 8 ? 0 : ((data[6] << 8) | data[7]) ^ 0xFFFF;
        break;
    }
    default:
        CL_STATUS->clLStickX = (data[0] & 0x3F) << 4;
        CL_STATUS->clLStickY = (data[1] & 0x3F) << 4;
        CL_STATUS->clRStickX = (s16)((s16)(data[2] >> 7) | (((s16)(data[0] >> 3) & 0x18) | ((s16)(data[1] >> 5) & 6))) << 5;
        CL_STATUS->clRStickY = (data[2] & 0x1F) << 5;
        CL_STATUS->clTriggerL = (s16)(((s16)(data[2] >> 2) & 0x18) | ((s16)(data[3] >> 5) & 7)) << 3;
        CL_STATUS->clTriggerR = (data[3] & 0x1F) << 3;
        CL_STATUS->clButton = ((data[4] << 8) | data[5]) ^ 0xFFFF;
        break;
    }
    if (p_cb->devType == 2) {
        CL_STATUS->clLStickX -= 512;
        CL_STATUS->clLStickY -= 512;
        CL_STATUS->clRStickX -= 512;
        CL_STATUS->clRStickY -= 512;
    } else if (p_cb->devType != 19) {
        CL_STATUS->clLStickX -= 512;
        CL_STATUS->clLStickY -= 512;
    }
    if (!p_cb->isSetStickOrigin) {
        p_cb->isSetStickOrigin = 1;
        if (p_cb->devType != 19) {
            p_cb->extConf.cl.lstk.x = CL_STATUS->clLStickX;
            p_cb->extConf.cl.lstk.y = CL_STATUS->clLStickY;
        } else {
            p_cb->extConf.cl.lstk.x = 0;
            p_cb->extConf.cl.lstk.y = 0;
        }
        if (p_cb->devType == 2) {
            p_cb->extConf.cl.rstk.x = CL_STATUS->clRStickX;
            p_cb->extConf.cl.rstk.y = CL_STATUS->clRStickY;
            p_cb->extConf.cl.triggerL = CL_STATUS->clTriggerL;
            p_cb->extConf.cl.triggerR = CL_STATUS->clTriggerR;
        } else {
            p_cb->extConf.cl.rstk.x = 0;
            p_cb->extConf.cl.rstk.y = 0;
            p_cb->extConf.cl.triggerL = 0;
            p_cb->extConf.cl.triggerR = 0;
        }
    }
    if (p_cb->devType != 19) {
        CL_STATUS->clLStickX = __wpadClampStick(CL_STATUS->clLStickX - p_cb->extConf.cl.lstk.x, -512, 511);
        CL_STATUS->clLStickY = __wpadClampStick(CL_STATUS->clLStickY - p_cb->extConf.cl.lstk.y, -512, 511);
    } else {
        CL_STATUS->clLStickX = __wpadClampStick(CL_STATUS->clLStickX - p_cb->extConf.cl.lstk.x, 0, 1024);
        CL_STATUS->clLStickY = __wpadClampStick(CL_STATUS->clLStickY - p_cb->extConf.cl.lstk.y, 0, 1024);
    }
    if (p_cb->devType == 2) {
        CL_STATUS->clRStickX = __wpadClampStick(CL_STATUS->clRStickX - p_cb->extConf.cl.rstk.x, -512, 511);
        CL_STATUS->clRStickY = __wpadClampStick(CL_STATUS->clRStickY - p_cb->extConf.cl.rstk.y, -512, 511);
        if (CL_STATUS->clTriggerL > 72) CL_STATUS->clButton |= 0x2000;
        if (CL_STATUS->clTriggerR > 72) CL_STATUS->clButton |= 0x0200;
        CL_STATUS->clTriggerL = 0;
        CL_STATUS->clTriggerR = 0;
    }
    if (_devCmpt[chan]) {
        CL_STATUS->clRStickX = 0;
        CL_STATUS->clRStickY = 0;
        CL_STATUS->clTriggerL = 0;
        CL_STATUS->clTriggerR = 0;
    }
#undef CL_STATUS
}

// NONMATCHING: compiler register allocation and packed-bit lowering differ.
void __parse_vs_data(s32 chan, WPADStatus** p_status, u8 fmt, u8* data, u8 len) {
    WPADControlBlock* p_cb = _wpdcb[chan];
#define VS_STATUS ((WPADVSStatus*)*p_status)
    VS_STATUS->sample[1].data = data[0];
    VS_STATUS->sample[1].sample[0] = (u16)(data[1] << 2) | (data[6] >> 6);
    VS_STATUS->sample[1].sample[1] = ((u16)(data[6] >> 4) & 3) | (u16)(data[2] << 2);
    VS_STATUS->sample[1].sample[2] = ((u16)(data[6] >> 2) & 3) | (u16)(data[3] << 2);
    VS_STATUS->sample[1].sample[3] = ((u16)data[6] & 3) | (u16)(data[4] << 2);
    VS_STATUS->sample[1].sample[4] = (u16)(data[5] << 2) | (data[7] >> 6);
    VS_STATUS->sample[0].data = data[8];
    VS_STATUS->sample[0].sample[0] = (u16)(data[9] << 2) | (data[14] >> 6);
    VS_STATUS->sample[0].sample[1] = ((u16)(data[14] >> 4) & 3) | (u16)(data[10] << 2);
    VS_STATUS->sample[0].sample[2] = ((u16)(data[14] >> 2) & 3) | (u16)(data[11] << 2);
    VS_STATUS->sample[0].sample[3] = ((u16)data[14] & 3) | (u16)(data[12] << 2);
    VS_STATUS->sample[0].sample[4] = (u16)(data[13] << 2) | (data[15] >> 6);
    VS_STATUS->button = ((u16)(data[7] << 4) & 0xF3F0) | ((u16)(data[15] >> 2) & 0xF);
    if (p_cb->trnFormatSelector < 8) {
        VS_STATUS->selector = 0xFF;
        return;
    }
    VS_STATUS->selector = data[15] & 3;
#undef VS_STATUS
}


static const u8 checkZeroData[21] = {0};

// NONMATCHING: compiler shares error and device-dispatch paths differently.
void __parse_ext_data(s32 chan, WPADStatus* p_status, u8 mode, u8* data, u8 len) {
    WPADControlBlock* p_cb = _wpdcb[chan];
    WPADStatus* parseStatus = p_status;
    s8 currentError;
    s16 x, y, value;
    u16 i;
#define FS_REPORT ((WPADFSStatus*)parseStatus)
#define RAW_REPORT ((WPADRawStatus*)parseStatus)
#define GTR_REPORT ((WPADPercussionStatus*)parseStatus)

    if (p_cb->extensionDataError == WPAD_ERR_NO_CONTROLLER) {
        p_cb->extensionDataError = p_status->err;
        p_cb->extensionDataDeviceType = p_cb->devType;
        p_cb->extensionDataLength = len;
        memcpy(p_cb->extensionData, data, len);
    }
    if (p_status->err == WPAD_ERR_NONE) {
        if (memcmp(data, checkZeroData, len) == 0) {
            p_status->err = WPAD_ERR_INVALID;
        } else if (memcmp(data, checkInvalidData, len) == 0) {
            p_status->err = WPAD_ERR_CORRUPTED;
            p_cb->extensionErrorCountdown = 3;
        } else if (!p_cb->infos.attach) {
            p_status->err = WPAD_ERR_CORRUPTED;
            p_cb->extensionErrorCountdown = 3;
        } else if (p_cb->extensionErrorCountdown != 0) {
            p_status->err = WPAD_ERR_CORRUPTED;
            --p_cb->extensionErrorCountdown;
        }
    }
    if (p_cb->extensionDataError == WPAD_ERR_NONE && p_status->err == WPAD_ERR_CORRUPTED) {
        p_cb->extensionDataError = WPAD_ERR_CORRUPTED;
    }
    if (p_cb->extensionDataDeviceType != p_cb->devType) {
        p_cb->extensionDataError = WPAD_ERR_INVALID;
    }
    if (p_cb->extensionDataError == WPAD_ERR_NONE) {
        p_cb->extensionErrorCountdown = 0;
        WPADiDecode(chan, p_cb->extensionData, p_cb->extensionDataLength, 0);
        switch ((u32)p_cb->devType) {
        case WPAD_DEV_FREESTYLE:
            FS_REPORT->fsStickX = p_cb->extensionData[0];
            FS_REPORT->fsStickY = p_cb->extensionData[1];
            FS_REPORT->fsAccX = (s16)((s16)((s16)((s16)(p_cb->extensionData[2] << 2) & (s16)~3) | (s16)((p_cb->extensionData[5] >> 2) & 3)) - p_cb->extConf.fs.acc_0g.x);
            FS_REPORT->fsAccY = (s16)((s16)((s16)((s16)(p_cb->extensionData[3] << 2) & (s16)~3) | (s16)((p_cb->extensionData[5] >> 4) & 3)) - p_cb->extConf.fs.acc_0g.y);
            FS_REPORT->fsAccZ = (s16)((s16)((s16)((s16)(p_cb->extensionData[4] << 2) & (s16)~3) | (s16)(p_cb->extensionData[5] >> 6)) - p_cb->extConf.fs.acc_0g.z);
            parseStatus->button |= ((~p_cb->extensionData[5]) & 3) << 13;
            if (!p_cb->isSetStickOrigin) {
                p_cb->isSetStickOrigin = 1;
                p_cb->extConf.fs.stick.x = FS_REPORT->fsStickX;
                p_cb->extConf.fs.stick.y = FS_REPORT->fsStickY;
            }
            x = (s16)((u8)FS_REPORT->fsStickX - (u8)p_cb->extConf.fs.stick.x);
            value = x;
            if (x < -128) value = -128;
            if (x > 127) value = 127;
            FS_REPORT->fsStickX = value;
            y = (s16)((u8)FS_REPORT->fsStickY - (u8)p_cb->extConf.fs.stick.y);
            value = y;
            if (y < -128) value = -128;
            if (y > 127) value = 127;
            FS_REPORT->fsStickY = value;
            break;
        case WPAD_DEV_CLASSIC:
        case 17: case 18: case 19: case 20:
            __parse_cl_data(chan, &parseStatus, p_cb->devMode, p_cb->extensionData, p_cb->extensionDataLength);
            break;
        case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28: {
            s32 length = p_cb->extensionDataLength;
            for (i = 0; i < length; ++i) {
                RAW_REPORT->data[i] = p_cb->extensionData[i];
            }
            break;
        }
        case 16:
            GTR_REPORT->force[0] = p_cb->extensionData[2];
            GTR_REPORT->force[1] = p_cb->extensionData[3];
            GTR_REPORT->button = ((p_cb->extensionData[6] << 8) | p_cb->extensionData[7]) ^ 0xFFFF;
            break;
        case 3:
            __parse_bl_data(chan, &parseStatus, p_cb->devMode, p_cb->extensionData, p_cb->extensionDataLength);
            break;
        case 4:
            __parse_vs_data(chan, &parseStatus, (u8)p_cb->dataFormat, p_cb->extensionData, p_cb->extensionDataLength);
            break;
        case WPAD_DEV_MPLS: case WPAD_DEV_MPLS_FREESTYLE: case WPAD_DEV_MPLS_CLASSIC: case WPAD_DEV_MPLS_FUTURE:
            __parse_mp_data(chan, &parseStatus, p_cb->devMode, p_cb->extensionData, p_cb->extensionDataLength);
            break;
        }
    }
    currentError = p_status->err;
    p_status->err = p_cb->extensionDataError;
    p_cb->extensionDataError = currentError;
    p_cb->extensionDataDeviceType = p_cb->devType;
    p_cb->extensionDataLength = len;
    memcpy(p_cb->extensionData, data, len);
#undef FS_REPORT
#undef RAW_REPORT
#undef GTR_REPORT
}
