#ifndef OS_CLOCK_H
#define OS_CLOCK_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __MWERKS__
u32 __OSBusClock : (0x8000 << 16 | 0x00F8);
#define OS_BUS_CLOCK __OSBusClock
#else
#ifndef OS_BUS_CLOCK
#define OS_BUS_CLOCK 243000000u
#endif
#endif

#ifndef OS_CORE_CLOCK
#define OS_CORE_CLOCK 729000000u
#endif
#ifndef OS_TIMER_CLOCK
#define OS_TIMER_CLOCK (OS_BUS_CLOCK / 4)
#endif

#ifdef __cplusplus
}
#endif

#endif // OS_CLOCK_H
