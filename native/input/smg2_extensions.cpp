#include <revolution/kmpls.h>
#include <revolution/kpad/KPADInternal.h>
#include <revolution/os.h>
#include <revolution/wbc.h>

#include <cstdlib>

namespace {
[[noreturn]] void unsupported(const char* function) {
    OSPanic(__FILE__, __LINE__, "%s: native MotionPlus and Balance Board support is not implemented", function);
    std::abort();
}
}  // namespace

extern "C" {
void KPADInit(void) {
    KPADInitEx(nullptr, 0);
}

s32 KPADRead(s32 chan, KPADStatus samplingBufs[], u32 length) {
    return KPADiRead(chan, samplingBufs, length, nullptr, FALSE);
}

BOOL KMPLSIsInit(void) {
    return FALSE;
}

void KMPLSSetKpadRingBuffer(s32, const KPADUnifiedWpadStatus*, const KPADUnifiedWpadStatus*, u32) {
    unsupported("KMPLSSetKpadRingBuffer");
}

void KMPLSSetSamplingCallback(KMPLSSamplingCallback) {
    unsupported("KMPLSSetSamplingCallback");
}

void KMPLSRead(s32, KPADMPLSStatus*, s32, s32, const KPADStatus*, f64) {
    unsupported("KMPLSRead");
}

BOOL WBCSetupCalibration(void) {
    unsupported("WBCSetupCalibration");
}

BOOL WBCGetCalibrationStatus(void) {
    unsupported("WBCGetCalibrationStatus");
}

u32 WBCGetBatteryLevel(u8) {
    unsupported("WBCGetBatteryLevel");
}

s32 WBCRead(WPADBLStatus*, f64*, u32) {
    unsupported("WBCRead");
}

s32 WBCSetZEROPoint(f64*, u32) {
    unsupported("WBCSetZEROPoint");
}

s32 WBCGetTGCWeight(f64, f64*, WPADBLStatus*) {
    unsupported("WBCGetTGCWeight");
}

u8 WPADiGetMplsStatus(s32 chan) {
    if (chan < 0 || chan >= WPAD_MAX_CONTROLLERS) {
        OSPanic(__FILE__, __LINE__, "WPADiGetMplsStatus: invalid channel %d", chan);
        std::abort();
    }
    return 0;
}

s32 WPADiControlMpls(s32, u8, WPADCallback) {
    unsupported("WPADiControlMpls");
}

s32 WPADControlBLC(s32, u8, WPADCallback) {
    unsupported("WPADControlBLC");
}
}  // extern "C"
