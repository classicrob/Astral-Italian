#include "JSystem/JAudio2/dspproc.hpp"
#include "JSystem/JAudio2/JASDSPInterface.hpp"
#include "JSystem/JAudio2/dsptask.hpp"

#ifdef GALAXY_NATIVE
// Host pointers do not fit the DSP's 32-bit mails; the native DSP device
// (native/platform/audio) hands out 32-bit handles for them.
#include <petari/platform/dsp.hpp>
#define DSP_MAIL_ADDR(address) PetariNative::Platform::DSP::addressHandle(address)
#else
#define DSP_MAIL_ADDR(address) (address)
#endif

void DSPReleaseHalt2(u32 msg) {
    u32 msgs[2];
    u16 dspMap = DSP_CreateMap2(msg);
    msgs[0] = (msg << 16) | dspMap;

    DSPSendCommands2(msgs, 0, NULL);
}

#ifdef GALAXY_NATIVE
// Shared with the DSP interrupt handler, which natively runs on another host
// thread: acquire/release accesses instead of plain volatile ones.
#define DSP_SHARED_LOAD(v) __atomic_load_n(&(v), __ATOMIC_ACQUIRE)
#define DSP_SHARED_STORE(v, x) __atomic_store_n(&(v), (x), __ATOMIC_RELEASE)
#else
#define DSP_SHARED_LOAD(v) (v)
#define DSP_SHARED_STORE(v, x) ((v) = (x))
#endif

static volatile BOOL flag;
static volatile BOOL d_waitflag;

static void setup_callback(u16 param_0) {
    OSReport("Finish %d\n", param_0);
    flag = FALSE;
}

void DsetupTable(u32 param_0, JASDspAddr param_1, JASDspAddr param_2, JASDspAddr param_3, JASDspAddr param_4) {
    u32 table[5];
    table[0] = (param_0 & 0xFFFF) | 0x81000000;
#ifdef GALAXY_NATIVE
    table[1] = DSP_MAIL_ADDR(param_1);
    table[2] = DSP_MAIL_ADDR(param_2);
    table[3] = DSP_MAIL_ADDR(param_3);
    table[4] = DSP_MAIL_ADDR(param_4);
#else
    table[1] = param_1;
    table[2] = param_2;
    table[3] = param_3;
    table[4] = param_4;
#endif
    static const char tableSetupMessage[] = "Table Setup\n";
    OSReport(tableSetupMessage);

    flag = TRUE;
    DSPSendCommands2(table, 5, setup_callback);
    do {
    } while (flag);
}

static u16 DSP_MIXERLEVEL = 0x4000;

void DsetMixerLevel(f32 level) {
    DSP_MIXERLEVEL = 4096.0f * level;
}

void DsyncFrame2ch(u32 param_0, JASDspAddr param_1, JASDspAddr param_2) {
    u32 msgs[5];
    msgs[0] = ((param_0 & 0xff) << 16) | 0x82000000;
    msgs[0] |= DSP_MIXERLEVEL;
#ifdef GALAXY_NATIVE
    msgs[1] = DSP_MAIL_ADDR(param_1);
    msgs[2] = DSP_MAIL_ADDR(param_2);
#else
    msgs[1] = param_1;
    msgs[2] = param_2;
#endif
    msgs[3] = 0;
    msgs[4] = 0;
    DSPSendCommands2(msgs, 5, 0);
}

void DsyncFrame4ch(u32 param_0, JASDspAddr param_1, JASDspAddr param_2, JASDspAddr param_3, JASDspAddr param_4) {
    u32 msgs[5];
    msgs[0] = ((param_0 & 0xff) << 16) | 0x82000000;
    msgs[0] |= DSP_MIXERLEVEL;
#ifdef GALAXY_NATIVE
    msgs[1] = DSP_MAIL_ADDR(param_1);
    msgs[2] = DSP_MAIL_ADDR(param_2);
    msgs[3] = DSP_MAIL_ADDR(param_3);
    msgs[4] = DSP_MAIL_ADDR(param_4);
#else
    msgs[1] = param_1;
    msgs[2] = param_2;
    msgs[3] = param_3;
    msgs[4] = param_4;
#endif
    DSPSendCommands2(msgs, 5, 0);
}

static void dummy_callback(u16 param_0) {
    d_waitflag = FALSE;
    OSReport("D-Wait end\n");
}

void DsetVARAM(JASDspAddr param_0) {
    u32 msgs[2];
    msgs[0] = 0x8E000000;
#ifdef GALAXY_NATIVE
    msgs[1] = DSP_MAIL_ADDR(param_0);
#else
    msgs[1] = param_0;
#endif

    d_waitflag = TRUE;
    DSPSendCommands2(msgs, 2, dummy_callback);
    do {
    } while (d_waitflag);
}

// NONMATCHING: remaining differences are register allocation, scheduling and SDK
// inline/tail-call lowering. Target calls, fields, diagnostic strings and constants
// were checked independently; original DSP hardware operations are preserved.
