#ifndef KMPLS_INTERNAL_H
#define KMPLS_INTERNAL_H
#include "revolution/kmpls.h"

typedef struct KMPLSState {
    /* 0x000 */ KPADMPLSStatus sample;
    /* 0x03C */ Vec previousRate;
    /* 0x048 */ Vec rateDifference;
    /* 0x054 */ Vec rateScale;
    /* 0x060 */ KPADMPLSDirection previousDirection;
    /* 0x084 */ KPADMPLSDirection directionChange;
    /* 0x0A8 */ f32 rotationStep;
    /* 0x0AC */ f32 samplePeriod;
    /* 0x0B0 */ f32 degreesPerSample;
    /* 0x0B4 */ f32 radiansPerSample;
    /* 0x0B8 */ u8 calibrating;
    /* 0x0B9 */ u8 unstableCount[3];
    /* 0x0BC */ u8 reportMode;
    /* 0x0BD */ u8 zeroPlayEnabled;
    /* 0x0BE */ u8 zeroAdjustEnabled;
    /* 0x0BF */ u8 orientationCorrection;
    /* 0x0C0 */ u8 accelCorrection;
    /* 0x0C1 */ u8 dpdCorrection;
    /* 0x0C2 */ s8 calibrationResult;
    /* 0x0C3 */ u8 reservedC3;
    /* 0x0C4 */ f32 calibrationWindow;
    /* 0x0C8 */ f32 zeroPlayRadius;
    /* 0x0CC */ f32 defaultCalibrationWindow;
    /* 0x0D0 */ s32 historyWindow;
    /* 0x0D4 */ f32 zeroAdjustSpeed;
    /* 0x0D8 */ f32 orientationCorrectionRate;
    /* 0x0DC */ f32 accelCorrectionRate;
    /* 0x0E0 */ f32 accelMagnitudeTolerance;
    /* 0x0E4 */ f32 dpdCorrectionRate;
    /* 0x0E8 */ f32 zeroTrust;
    /* 0x0EC */ f32 historyTrust;
    /* 0x0F0 */ f32 orientationTrust;
    /* 0x0F4 */ f32 accelTrust;
    /* 0x0F8 */ f32 dpdTrust;
    /* 0x0FC */ f32 dpdReviseScale;
    /* 0x100 */ u32 aimingMode;
    /* 0x104 */ f32 dpdYOffset;
    /* 0x108 */ f64 rateConversion[2][3];
    /* 0x138 */ WPADMplsCalibration calibration[2];
    /* 0x170 */ Vec zeroCenter;
    /* 0x17C */ u32 historyHead;
    /* 0x180 */ s32 history[3][256];
    /* 0xD80 */ f64 filteredRaw[3];
    /* 0xD98 */ const KPADUnifiedWpadStatus* internalRing;
    /* 0xD9C */ const KPADUnifiedWpadStatus* externalRing;
    /* 0xDA0 */ u32 externalLength;
    /* 0xDA4 */ Vec rawScale;
} KMPLSState;

typedef struct KMPLSWork {
    f32 invalidSampleDecay;
    KMPLSSamplingCallback samplingCallback;
    KMPLSState channels[4];
} KMPLSWork;

#ifdef __cplusplus
extern "C" {
#endif
extern KPADMPLSDirection kmplsIdentity;
void wpad_callback_func(s32 chan, s32 result);
void work_calibration(s32 chan);
void dpd_revise_scale(s32 chan);
f32 move_mpls_orient(KMPLSState* state, Vec* direction, Vec* previous, Vec* change);
f32 revise_dir_acc(s32 chan, KPADMPLSDirection* direction, const Vec* acceleration);
f32 revise_dir_dpd(s32 chan, KPADMPLSDirection* direction, KPADStatus* context);
void calc_mpls_dir(KMPLSState* state);
s32 get_mpls_data_x(KMPLSState* state, WPADMPStatus* report);
s32 get_mpls_data_y(KMPLSState* state, WPADMPStatus* report);
s32 get_mpls_data_z(KMPLSState* state, WPADMPStatus* report);
void read_mpls(s32 chan, s32 ringEnd, s32 count);
void set_calibration_data(s32 chan);
s32 __KMPLS_f2i(f32 value);
f32 __KMPLS_normalize_Fxyz(Vec* value);
void __KMPLS_normalize_directionXYZ(KPADMPLSDirection* direction, f32 minimum);
void __KMPLS_make_vec_dir(KPADMPLSDirection* direction, Vec* from, Vec* to);
void __KMPLS_mult_dir(KPADMPLSDirection* left, KPADMPLSDirection* right,
                      KPADMPLSDirection* result);
void __KMPLS_linear_Direction(KPADMPLSDirection* from, KPADMPLSDirection* to,
                             KPADMPLSDirection* result, f32 amount);
#ifdef __cplusplus
}
#endif
#endif
