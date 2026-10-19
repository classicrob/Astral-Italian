#ifndef KPAD_H
#define KPAD_H

#include "revolution/mtx.h"
#include "revolution/types.h"
#include "revolution/wpad.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KPAD_BUTTON_MASK 0x0000ffff
#define KPAD_BUTTON_RPT 0x80000000

#define KPAD_BTN_RPT_TIME_MAX 20000
#define KPAD_BTN_NO_RPT_DELAY 40000

#define KPAD_CMOS_RESO_WX 128
#define KPAD_CMOS_RESO_WY 96
#define KPAD_CMOS_RESO_CX (KPAD_CMOS_RESO_WX / 2)
#define KPAD_CMOS_RESO_CY (KPAD_CMOS_RESO_WY / 2)

#define KPAD_DPD_RESO_WX WPAD_DPD_IMG_RESO_WX
#define KPAD_DPD_RESO_WY WPAD_DPD_IMG_RESO_WY
#define KPAD_DPD_RESO_CX (KPAD_DPD_RESO_WX / 2)
#define KPAD_DPD_RESO_CY (KPAD_DPD_RESO_WY / 2)

#define KPAD_USE_OBJECTS 2
#define KPAD_INTERNAL_RING_LENGTH 16

typedef struct Vec2 {
    f32 x;
    f32 y;
} Vec2;

typedef enum KPADPlayMode { KPAD_PLAY_MODE_LOOSE = 0, KPAD_PLAY_MODE_TIGHT } KPADPlayMode;

typedef union KPADEXStatus {
    struct {
        Vec2 stick;
        Vec acc;
        f32 acc_value;
        f32 acc_speed;
    } fs;

    struct {
        u32 hold;
        u32 trig;
        u32 release;
        Vec2 lstick;
        Vec2 rstick;
        f32 ltrigger;
        f32 rtrigger;
    } cl;

    // Balance board (SMG2 SDK; this member makes the union 0x50 bytes)
    struct {
        f64 tgc_weight;
        f64 weight[4];
        f64 weight_ave[4];
        s32 weight_err;
        s32 tgc_weight_err;
    } bl;
} KPADEXStatus;

// MotionPlus sample: angular velocity, integrated angle, then three direction axes.
typedef struct KPADMPLSDirection {
    Vec X;
    Vec Y;
    Vec Z;
} KPADMPLSDirection;

typedef struct KPADMPLSStatus {
    Vec angularVelocity;
    Vec angle;
    KPADMPLSDirection direction;
} KPADMPLSStatus;

typedef struct KPADStatus {
    /* 0x00 */ u32 hold;
    /* 0x04 */ u32 trig;
    /* 0x08 */ u32 release;
    /* 0x0C */ Vec acc;
    /* 0x18 */ f32 acc_value;
    /* 0x1C */ f32 acc_speed;
    /* 0x20 */ Vec2 pos;
    /* 0x28 */ Vec2 vec;
    /* 0x30 */ f32 speed;
    /* 0x34 */ Vec2 horizon;
    /* 0x3C */ Vec2 hori_vec;
    /* 0x44 */ f32 hori_speed;
    /* 0x48 */ f32 dist;
    /* 0x4C */ f32 dist_vec;
    /* 0x50 */ f32 dist_speed;
    /* 0x54 */ Vec2 acc_vertical;
    /* 0x5C */ u8 dev_type;
    /* 0x5D */ s8 wpad_err;
    /* 0x5E */ s8 dpd_valid_fg;
    /* 0x5F */ u8 data_format;
    /* 0x60 */ KPADEXStatus ex_status;
    /* 0xB0 */ KPADMPLSStatus mpls;
    /* 0xEC */ u32 unk_EC;  // SMG2 SDK field, purpose not recovered
} KPADStatus;

typedef struct KPADUnifiedWpadStatus {
    union {
        WPADStatus core;
        WPADFSStatus fs;
        WPADCLStatus cl;
        WPADMPStatus mp;
        WPADBLStatus bl;
        WPADAnalogStatus analog;
        WPADPercussionStatus percussion;
        // SMG2's unified report also carries extended controller data.
        u8 extended[0x40];
    } u;
    u8 fmt;
    u8 padding;
} KPADUnifiedWpadStatus;

typedef void (*KPADControlDpdCallback)(s32 chan, s32 reason);
typedef WPADCallback KPADCallback;
typedef WPADChannel KPADChannel;



void KPADInit();
void KPADInitEx(KPADUnifiedWpadStatus* uniBuf, u32 length);

void KPADSetBtnRepeat(s32, f32, f32);
void KPADSetSensorHeight(s32, f32);
f32 KPADGetSensorHeight(s32 chan);
BOOL KPADIsEnableAimingMode(s32 chan);
void KPADSetAccParam(s32 chan, f32 playRadius, f32 sensitivity);
void KPADSetPosParam(s32, f32, f32);
void KPADSetHoriParam(s32, f32, f32);
void KPADSetDistParam(s32, f32, f32);

void KPADReset(void);
s32 KPADRead(s32, KPADStatus[], u32);
s32 KPADReadEx(s32 chan, KPADStatus samplingBufs[], u32 length, s32* pErr);
WPADConnectCallback KPADSetConnectCallback(s32 chan, WPADConnectCallback callback);



#ifdef __cplusplus
}
#endif

#endif  // KPAD_H
