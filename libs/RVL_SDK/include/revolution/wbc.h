#ifndef RVL_SDK_WBC_H
#define RVL_SDK_WBC_H

#include "revolution/wpad.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL WBCSetupCalibration(void);
BOOL WBCGetCalibrationStatus(void);
u32 WBCGetBatteryLevel(u8 battery);
s32 WBCRead(WPADBLStatus* report, f64* weight, u32 count);
s32 WBCSetZEROPoint(f64* averagePressure, u32 count);
s32 WBCGetTGCWeight(f64 averageWeight, f64* result, WPADBLStatus* report);

#ifdef __cplusplus
}
#endif
#endif
