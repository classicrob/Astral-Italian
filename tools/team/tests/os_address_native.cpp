// Exercise the SDK macros with addresses above the Wii's 32-bit range.
#include "revolution/os.h"

int main() {
    static_assert(sizeof(uintptr_t) == sizeof(void*), "address type must hold a pointer");
    const uintptr_t address = sizeof(uintptr_t) > 4 ? (uintptr_t)0x1234567880001234ULL : (uintptr_t)0x80001234u;
    const void* pointer = reinterpret_cast<const void*>(address);
    if (OSRoundDown32B(pointer) != address - 0x14) return 1;
    if (OSRoundUp32B(pointer) != address + 0x0C) return 2;
    if (OSCachedToPhysical(pointer) != address - 0x80000000u) return 3;
    if (reinterpret_cast<uintptr_t>(OSPhysicalToCached(OSCachedToPhysical(pointer))) != address) return 4;
    if (reinterpret_cast<uintptr_t>(OSPhysicalToUncached(OSCachedToPhysical(pointer))) != address + 0x40000000u) return 5;
    const uintptr_t uncached = address + 0x40000000u;
    if (OSUncachedToPhysical(reinterpret_cast<void*>(uncached)) != address - 0x80000000u) return 6;
    if (!OSIsMEM1Region(pointer) || OSIsMEM2Region(pointer)) return 7;
    if (OSIsMEM1Region(reinterpret_cast<void*>(address + 0x10000000u)) ||
        !OSIsMEM2Region(reinterpret_cast<void*>(address + 0x10000000u))) return 8;
    return 0;
}
