#pragma once
// Host crash reporting.
//
// On the Wii, CPU exceptions reach handlers registered with OSSetErrorHandler
// with an OSContext holding the PowerPC registers (JUTException prints them on
// screen). Natively there are no PowerPC exceptions or registers: faults are
// host signals with arm64 state. install() catches fatal signals (SIGSEGV,
// SIGBUS, SIGILL, SIGFPE, SIGTRAP, SIGABRT) and writes a report of the true
// host state:
// - signal, code, and fault address;
// - arm64 registers (x0-x28, fp, lr, sp, pc), labelled as host registers;
// - a saved-context frame-pointer backtrace and UUID/load-address image list;
// - the cached app/build/OS/hardware identity and crashed host thread identity;
// - the running OS thread and whether the fault was in interrupt context;
// - the last OSPanic/OSFatal message and recent OSReport lines;
// - the game error handlers registered with OSSetErrorHandler.
// The signal is then re-raised with its default action. Game error handlers
// are not called: they expect a PowerPC context this port cannot provide
// truthfully.

#include <filesystem>

namespace PetariNative::Platform::Crash {

// Installs the signal handlers (on an alternate stack). Reports go to stderr
// and, if reportDirectory is not empty, to petari-crash-<time>-<pid>.txt in it
// (the directory must exist). Call once from the main thread at start-up.
void install(const std::filesystem::path& reportDirectory);

// Gives the calling thread its own alternate signal stack (sigaltstack is per
// thread), so a stack overflow on it can still be reported. install() does this
// for its own thread; the OS layer calls it for every OS thread it starts.
// Allocates with mmap (no malloc); safe to call more than once per thread.
void installThread();

// Path of the report file the next crash will write (empty if none).
std::filesystem::path reportPath();

}  // namespace PetariNative::Platform::Crash
