#include "nw4r/db/assert.h"
#include "nw4r/db/console.h"
#include "nw4r/db/directPrint.h"
#include "nw4r/db/mapFile.h"
#include <va_list.h>

namespace nw4r {
    namespace db {
        static u32 sWarningTime = 0;
        static ConsoleHandle sAssertionConsole = NULL;
        static bool sDispWarningAuto = true;

        void WarningAlarmFunc_(OSAlarm*, OSContext*) {
            if (sAssertionConsole != NULL) Console_SetVisible(sAssertionConsole, false);
        }

        static OSAlarm* GetWarningAlarm_() {
            static OSAlarm sWarningAlarm;
            static bool sInitializedAlarm;
            if (!sInitializedAlarm) {
                OSCreateAlarm(&sWarningAlarm);
                sInitializedAlarm = true;
            }
            return &sWarningAlarm;
        }


        static void Assertion_Printf_(const char* fmt, ...) {
            va_list vlist;
            va_start(vlist, fmt);
            if (sAssertionConsole) {
            } else {
                OSVReport(fmt, vlist);
            }
            va_end(vlist);
        }

        static void ShowStack_(u32 sp) {
            Assertion_Printf_("-------------------------------- TRACE\n");
            u32 i;
            u32* p;

            Assertion_Printf_("Address:   BackChain   LR save\n");
            p = (u32*)sp;

            for (i = 0; i < 16; i++) {
                if (p == NULL || (u32)p == 0xffffffff || ((u32)p & 0x80000000) == 0) {
                    break;
                }
                Assertion_Printf_("%08X:  %08X    %08X ", p, p[0], p[1]);
                Assertion_Printf_("\n");

                p = (u32*)*p;
            }

            return;
        }

        __declspec(weak) void VPanic(const char* file, int line, const char* fmt, va_list vlist, bool halt) {
            register u32 stackPointer;
            asm {
        mr  stackPointer, r1
            }
            stackPointer = *((u32*)stackPointer);
            (void)OSDisableInterrupts();
            (void)OSDisableScheduler();

            (void)VISetPostRetraceCallback(NULL);
            (void)VISetPreRetraceCallback(NULL);

            ShowStack_(stackPointer);

            if (sAssertionConsole) {
                Console_Printf(sAssertionConsole, "%s:%d Panic:", file, line);
                Console_Printf(sAssertionConsole, "\n");

                (void)Console_ShowLatestLine(sAssertionConsole);
                (void)Console_SetVisible(sAssertionConsole, true);
            } else {
                OSReport("%s:%d Panic:", file, line);
                OSVReport(fmt, vlist);
                OSReport("\n");
            }

            if (halt) {
                PPCHalt();
            }
        }

        __declspec(weak) void Panic(const char* file, int line, const char* fmt, ...) {
            va_list vlist;
            va_start(vlist, fmt);
            nw4r::db::VPanic(file, line, fmt, vlist, true);
            va_end(vlist);
            PPCHalt();
        }

        void Warning(const char* file, int line, const char* fmt, ...) {
            va_list args;
            va_start(args, fmt);
            if (sAssertionConsole != NULL) {
                Console_Printf(sAssertionConsole, "%s:%d Warning:", file, line);
                Console_Printf(sAssertionConsole, "\n");
                Console_ShowLatestLine(sAssertionConsole);
                if (sDispWarningAuto && sAssertionConsole != NULL) {
                    const u32 time = sWarningTime;
                    OSAlarm* alarm = GetWarningAlarm_();
                    OSCancelAlarm(alarm);
                    Console_SetVisible(sAssertionConsole, true);
                    if (time != 0) OSSetAlarm(alarm, time, WarningAlarmFunc_);
                }
            } else {
                OSReport("%s:%d Warning:", file, line);
                OSVReport(fmt, args);
                OSReport("\n");
            }
            va_end(args);
        }

    };  // namespace db
};  // namespace nw4r
