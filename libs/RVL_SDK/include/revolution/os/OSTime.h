#ifndef OSTIME_H
#define OSTIME_H

#include "revolution/types.h"
#include "revolution/os/OSClock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef u32 OSTick;
typedef s64 OSTime;

typedef struct OSCalendarTime {
    int sec;
    int min;
    int hour;
    int mday;
    int mon;
    int year;
    int wday;
    int yday;
    int msec;
    int usec;
} OSCalendarTime;

OSTime OSGetTime(void);
OSTick OSGetTick(void);
OSTime __OSGetSystemTime(void);
OSTime __OSTimeToSystemTime(OSTime);
void OSTicksToCalendarTime(OSTime, OSCalendarTime*);

#ifdef __MWERKS__
vu32 OS_BUS_CLOCK_SPEED : 0x800000F8;
#define OS_TIME_SPEED (OS_BUS_CLOCK_SPEED / 4)
#else
#define OS_TIME_SPEED OS_TIMER_CLOCK
#endif

#define OS_SEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED))
#define OS_MSEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED / 1000))
#define OS_USEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED / 125000) / 8)
#define OS_NSEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED / 125000) / 8000)

#ifdef __cplusplus
}
#endif

#endif  // OSTIME_H
