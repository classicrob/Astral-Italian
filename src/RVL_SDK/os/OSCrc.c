#include "revolution/types.h"

static const u32 s_crc32Table[16] = {
    0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC,
    0x76DC4190, 0x6B6B51F4, 0x4DB26158, 0x5005713C,
    0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
    0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C,
};

// NONMATCHING: only the volatile r8/r9 allocation is exchanged; all
// instructions, branches and the complete 16-entry table agree after renaming.
u32 fn_805BFF30(const void* data, u32 length) {
    const u8* bytes = data;
    u32 crc = 0xFFFFFFFF;
    while (length) {
        u32 value = *bytes++;
        crc = (crc >> 4) ^ s_crc32Table[(crc ^ value) & 0xF];
        crc = (crc >> 4) ^ s_crc32Table[(crc ^ (value >> 4)) & 0xF];
        length--;
    }
    return ~crc;
}
