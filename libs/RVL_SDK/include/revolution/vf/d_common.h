#ifndef D_COMMON_H
#define D_COMMON_H

#include "revolution/vf/vf_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

enum FatType dCommon_GetNiceFatType(u32* spf, u32 SPU, u32 SPC, u32 BPS) NO_INLINE;

s32 dCommon_FlushFromVol(PF_VOLUME* i_vol_p, int i_setLastDeviceError);

u32 dCommon_getResvSecNumFromDisk(struct PDM_DISK* p_disk);

#ifdef __cplusplus
}
#endif

#endif  // D_COMMON_H
