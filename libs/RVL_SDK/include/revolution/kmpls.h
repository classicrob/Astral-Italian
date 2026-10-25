#ifndef KMPLS_H
#define KMPLS_H
#include "revolution/kpad.h"
#ifdef __cplusplus
extern "C" {
#endif
// The callback receives one 0x3C-byte result, valid for the duration of the call.
typedef void (*KMPLSSamplingCallback)(s32 chan, KPADMPLSStatus* sample);
BOOL KMPLSIsInit(void);
void KMPLSSetKpadRingBuffer(s32 chan, const KPADUnifiedWpadStatus* internalRing,
                          const KPADUnifiedWpadStatus* externalRing, u32 externalLength);
void KMPLSSetSamplingCallback(KMPLSSamplingCallback callback);
void KMPLSRead(s32 chan, KPADMPLSStatus* result, s32 ringEnd, s32 count,
               const KPADStatus* context, f64 samplePeriod);
#ifdef __cplusplus
}
#endif
#endif
