#ifndef D_VF_SYS_H
#define D_VF_SYS_H

#include "revolution/vf/vf_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

void (*VFSysGetTimeStampCallback())(struct VFSysTime*);
void (*VFSysSetTimeStampCallback(void (*i_callback)(struct VFSysTime*)))(struct VFSysTime*);

s32 VFSysUnsetDevice(s32 i_handle_idx);
s32 VFSysCheckExistPrfFile(s32 i_handle_idx, const s8* i_prf_file_name_p, void* i_memory_p);
s32 VFSysCheckExistPrfFile_nandflash_sub(const s8* i_prf_file_name_p, u32 i_handle_idx);
s32 VFSysCheckExistPrfFile_nandflash(struct VF_HANDLE_DEVICE* i_device_p, const s8* i_prf_file_name_p, void* i_memory_p, u32 i_handle_idx);
s32 VFSysCheckExistPrfFile_ram(struct VF_HANDLE_DEVICE* i_device_p, const s8* i_prf_file_name_p, void* i_memory_p, u32 i_handle_idx);
s32 VFSysCheckExistPrfFile_dvd(struct VF_HANDLE_DEVICE* i_device_p, const s8* i_prf_file_name_p, void* i_memory_p, u32 i_handle_idx);

struct PF_FILE* VFSysOpenFile(s32 i_handle_idx, const s8* i_path_p, const s8* i_mode);
struct PF_FILE* VFSysOpenFile_current(const s8* i_path_p, const s8* i_mode);
s32 VFSysGetFileSizeByFd(u32* o_size_p, struct PF_FILE* i_file_p);
// Descriptive name: the historical SDK name is unavailable; returns status and writes unsigned drive geometry.
s32 VFSysGetDriveFreeSizeInfo(s32 i_handle_idx, u32* o_free_clusters, u32* o_sectors_per_cluster, u32* o_bytes_per_sector);
void VFSysSetLastError(s32 i_err);
s32 VFSysGetLastError(void);

#ifdef __cplusplus
}
#endif

#endif  // D_VF_SYS_H
