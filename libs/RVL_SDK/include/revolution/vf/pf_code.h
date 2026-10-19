#ifndef PF_CODE_H
#define PF_CODE_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const u8 VFipf_valid_fn_char[96];

int VFiPFCODE_Combine_Width(s16, s16);
void VFiPFCODE_Divide_Width(u32, s16*, s16*);

#ifdef __cplusplus
}
#endif

#endif  // PF_CODE_H
