#ifndef KPAD_INTERNAL_H
#define KPAD_INTERNAL_H
#include "revolution/kpad.h"
typedef struct {
    Vec2 center;
    s8 error_fg;
    s8 state_fg;
    s8 _fg_1;
    s8 _fg_2;
} KPADObject;

typedef union {
    KPADStatus k;
    KPADUnifiedWpadStatus w;
} KPADTmpStatus;

typedef struct {
    KPADStatus status;

    f32 pos_play_radius;
    f32 pos_sensitivity;

    f32 hori_play_radius;
    f32 hori_sensitivity;

    f32 dist_play_radius;
    f32 dist_sensitivity;

    f32 acc_play_radius;
    f32 acc_sensitivity;
    f32 dist_org;
    Vec2 accXY_nrm_hori;
    Vec2 sec_nrm_hori;
    Vec2 center_org;
    f32 dpd2pos_scale;
    KPADObject kobj_sample[4];
    KPADObject kobj_regular[2];
    s16 valid_objs;
    u8 bufIdx;
    u8 bufCount;
    /* 0x17C */ KPADStatus* mplsWritePos;
    /* 0x180 */ KPADUnifiedWpadStatus uniRingBuf[KPAD_INTERNAL_RING_LENGTH];
    /* 0x5A0 */ KPADUnifiedWpadStatus* externalRingBuf;
    /* 0x5A4 */ u32 externalRingLength;
    f32 sec_length;
    Vec2 sec_nrm;
    f32 sec_dist;
    f32 trust_sec_length;
    Vec hard_acc;
    Vec2 obj_horizon;
    Vec2 acc_horizon;
    /* 0x5D8 */ Vec2 rawPos;
    /* 0x5E0 */ Vec2 ah_circle_pos;
    u16 ah_circle_ct;
    u8 dpd_valid2_ct;
    /* 0x5EB */ u8 readLocked;
    u16 btn_repeat_time;
    u16 btn_repeat_next;
    u16 btn_repeat_delay;
    u16 btn_repeat_pulse;
    u16 btn_cl_repeat_time;
    u16 btn_cl_repeat_next;
    KPADControlDpdCallback dpd_ctrl_callback;
    f32 acc_scale_x;
    f32 acc_scale_y;
    f32 acc_scale_z;
    f32 fs_acc_scale_x;
    f32 fs_acc_scale_y;
    f32 fs_acc_scale_z;
    Vec2 kobj_frame_min;
    Vec2 kobj_frame_max;
    f32 err_dist_speed_1;
    f32 err_dist_speedM_1;
    f32 ah_circle_radius2;
    f32 dist_vv1;
    f32 err_dist_min;
    /* 0x638 */ WPADSamplingCallback appSamplingCallback;
    /* 0x63C */ WPADConnectCallback appConnectCallback;
    /* 0x640 */ u8 resetReq;
    /* 0x641 */ u8 exResetReq;
    /* 0x642 */ u8 dpdRequested;
    /* 0x643 */ u8 dpdEnabled;
    /* 0x644 */ u8 dpdIssued;
    u8 dpdCmd;
    u8 dpdPreCallbackDone;
    u8 dpdPostCallbackDone;
    u8 aimReq;
    u8 aimEnabled;
    u8 fsAccRevise;
    u8 buttonProcessingMode;
    KPADPlayMode pos_play_mode;
    KPADPlayMode hori_play_mode;
    KPADPlayMode dist_play_mode;
    KPADPlayMode acc_play_mode;
    /* 0x65C */ WPADDeviceType deviceType;
    /* 0x660 */ u32 readTicks[3];
    /* 0x66C */ u32 previousSampleTick;
    /* 0x670 */ u32 sampleTick;
    /* 0x674 */ u8 previousReadCount;
    /* 0x675 */ u8 lastReadCount;
    /* 0x676 */ u8 mplsCommandMode;
    /* 0x677 */ u8 mplsAcceptedMode;
    /* 0x678 */ u8 mplsTargetMode;
    /* 0x679 */ u8 mplsRequestedMode;
    /* 0x67A */ u8 mplsCommandPending;
    /* 0x67B */ u8 mplsPreCallbackDone;
    /* 0x67C */ u8 mplsPostCallbackDone;
    /* 0x67D */ u8 mplsWaitCount;
    /* 0x67E */ u8 gravityUnitResetReq;
    /* 0x67F */ u8 _67F;
    /* 0x680 */ KPADControlDpdCallback mpls_ctrl_callback;
} KPADInsideStatus;
#ifdef __cplusplus
extern "C" {
#endif
s32 KPADiRead(s32 chan, KPADStatus* samplingBufs, u32 length, s32* error, BOOL readEx);
#ifdef __cplusplus
}
#endif
#endif
