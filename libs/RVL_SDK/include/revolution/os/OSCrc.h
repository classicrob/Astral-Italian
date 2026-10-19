#ifndef RVL_SDK_OS_CRC_H
#define RVL_SDK_OS_CRC_H
#include "revolution/types.h"
#ifdef __cplusplus
extern "C" {
#endif
// Descriptive name; historical SDK spelling is unverified.
u32 __OSCalcCRC32(const void* data, u32 length);
#ifdef __cplusplus
}
#endif
#endif
