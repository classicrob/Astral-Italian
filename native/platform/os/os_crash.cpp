// Host crash reporting, the OS error-handler registry, OSContext bookkeeping,
// and the PowerPC machine-state calls game code reaches. See
// petari/platform/crash.hpp.
//
// No PowerPC state is invented:
// - OSContext objects are kept and handed out as the SDK does (current
//   context per thread, OSClearContext resets their FPU mode/state), but
//   nothing fills their registers.
// - OSFillFPUContext and OSProtectRange need PowerPC FPU state or MEM1
//   protection channels; they abort with an explanation if reached (only
//   JUTException's exception path, which native faults never enter, calls
//   them).
// - PPCMfmsr/PPCMtmsr model the two MSR fields with host meaning: EE (the
//   calling thread's interrupt state) and FE0/FE1 (floating-point exception
//   enables, which Apple silicon cannot trap: clearing is honoured, setting
//   aborts). Other MSR bits read as zero.
// - OSGetStackPointer returns 0: there is no PowerPC stack, and a truncated
//   host pointer would mislead.
// The signal handler is async-signal-safe: it formats into a fixed buffer with
// its own integer/hex/string appenders (no stdio, no locale, no malloc) and
// writes with write(2). Image metadata and build identity are cached before
// faults; the saved context is unwound with bounded, checked memory reads.
// A fault inside the handler itself does not recurse: it restores the default
// action and terminates.

#include <revolution/base/PPCArch.h>
#include <revolution/os.h>
#include <revolution/os/OSError.h>

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>
#include <ptrauth.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/ucontext.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <mutex>

#include "os_internal.hpp"
#include "petari/platform/build_identity.hpp"
#include "petari/host_allocation.hpp"
#include "petari/platform/crash.hpp"

namespace OS = PetariNative::Platform::OS;

namespace {

constexpr u32 kMsrEe = 0x8000;
constexpr u32 kMsrFe0 = 0x0800;
constexpr u32 kMsrFe1 = 0x0100;

OSErrorHandler gErrorHandlers[OS_ERROR_MAX];
thread_local OSContext* tCurrentContext = nullptr;
OSContext gHostThreadContext;

// Recent OSReport output and the last panic, for crash reports.
constexpr int kReportLines = 32;
constexpr int kLineLength = 160;
char gReportRing[kReportLines][kLineLength];
std::atomic<unsigned> gReportNext{0};
char gPanicMessage[512];
std::atomic<bool> gHavePanic{false};

char gReportPath[1024];
bool gInstalled;

// dyld serializes callbacks. Publish immutable entries only after filling them;
// removal merely clears a lock-free flag, so a signal never waits on dyld.
struct Image {
    uintptr_t base, textStart, textEnd;
    char uuid[37];
    char path[1024];
    const char* name;
    std::atomic<bool> active{false};
};
constexpr unsigned kMaxImages = 2048;
Image gImages[kMaxImages];
std::atomic<unsigned> gImageCount{0};
static_assert(std::atomic<unsigned>::is_always_lock_free && std::atomic<bool>::is_always_lock_free);

void snapshotImage(const mach_header* header, intptr_t slide, const char* path) {
    if (header->magic != MH_MAGIC_64) return;
    const unsigned index = gImageCount.fetch_add(1);
    if (index >= kMaxImages) return;
    auto& image = gImages[index];
    image.base = reinterpret_cast<uintptr_t>(header);
    image.textStart = image.textEnd = image.base;
    const auto* h = reinterpret_cast<const mach_header_64*>(header);
    const auto* command = reinterpret_cast<const load_command*>(h + 1);
    for (uint32_t i = 0; i < h->ncmds; ++i) {
        if (command->cmd == LC_SEGMENT_64) {
            const auto* segment = reinterpret_cast<const segment_command_64*>(command);
            if (segment->initprot & VM_PROT_EXECUTE) {
                image.textStart = segment->vmaddr + slide;
                image.textEnd = image.textStart + segment->vmsize;
            }
        }
        command = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(command) + command->cmdsize);
    }
    PetariNative::Platform::BuildIdentity::imageUUID(header, image.uuid);
    if (path) std::snprintf(image.path, sizeof(image.path), "%s", path);
    const char* slash = std::strrchr(image.path, '/');
    image.name = slash ? slash + 1 : image.path;
    image.active.store(true, std::memory_order_release);
}
void addImage(const mach_header* header, intptr_t slide) {
    const char* path = nullptr;
    for (uint32_t i = 0; i < _dyld_image_count(); ++i) {
        if (_dyld_get_image_header(i) == header) {
            path = _dyld_get_image_name(i);
            break;
        }
    }
    snapshotImage(header, slide, path);
}
void removeImage(const mach_header* header, intptr_t) {
    for (unsigned i = 0; i < std::min(gImageCount.load(), kMaxImages); ++i) {
        if (gImages[i].active.load(std::memory_order_acquire) && gImages[i].base == reinterpret_cast<uintptr_t>(header)) gImages[i].active.store(false);
    }
}
uintptr_t codeAddress(uintptr_t address) {
    return reinterpret_cast<uintptr_t>(ptrauth_strip(reinterpret_cast<void*>(address), ptrauth_key_return_address));
}

const char* signalName(int sig) {
    switch (sig) {
    case SIGSEGV: return "SIGSEGV";
    case SIGBUS: return "SIGBUS";
    case SIGILL: return "SIGILL";
    case SIGFPE: return "SIGFPE";
    case SIGTRAP: return "SIGTRAP";
    case SIGABRT: return "SIGABRT";
    default: return "signal";
    }
}

// The Wii exception a game handler would have been registered for.
const char* errorName(int error) {
    static const char* names[OS_ERROR_MAX] = {"SYSTEM_RESET", "MACHINE_CHECK", "DSI", "ISI", "EXTERNAL_INTERRUPT", "ALIGNMENT",
                                              "PROGRAM", "FLOATING_POINT", "DECREMENTER", "SYSTEM_CALL", "TRACE", "PERFORMANCE_MONITOR",
                                              "BREAKPOINT", "SYSTEM_INTERRUPT", "THERMAL_INTERRUPT", "PROTECTION", "FPE"};
    return (error >= 0 && error < OS_ERROR_MAX) ? names[error] : "?";
}

// Async-signal-safe line builder: fixed storage, no stdio/locale/malloc.
class Line {
public:
    Line& str(const char* text) {
        if (text == nullptr) {
            text = "(null)";
        }
        while (*text && mLength < sizeof(mBuffer)) {
            mBuffer[mLength++] = *text++;
        }
        return *this;
    }
    Line& dec(long long value) {
        char digits[24];
        int n = 0;
        unsigned long long magnitude = value < 0 ? 0ULL - static_cast<unsigned long long>(value) : static_cast<unsigned long long>(value);
        do {
            digits[n++] = static_cast<char>('0' + magnitude % 10);
            magnitude /= 10;
        } while (magnitude != 0 && n < 24);
        if (value < 0) {
            str("-");
        }
        while (n > 0 && mLength < sizeof(mBuffer)) {
            mBuffer[mLength++] = digits[--n];
        }
        return *this;
    }
    // Lowercase hex; width pads with zeros (0: no padding).
    Line& hex(unsigned long long value, int width = 0) {
        char digits[16];
        int n = 0;
        do {
            digits[n++] = "0123456789abcdef"[value & 0xF];
            value >>= 4;
        } while (value != 0 && n < 16);
        while (n < width && n < 16) {
            digits[n++] = '0';
        }
        while (n > 0 && mLength < sizeof(mBuffer)) {
            mBuffer[mLength++] = digits[--n];
        }
        return *this;
    }
    Line& ptr(const void* pointer) { return str("0x").hex(reinterpret_cast<uintptr_t>(pointer)); }
    // Left-aligned in a field of width characters (for register names).
    Line& padTo(size_t column) {
        while (mLength < column && mLength < sizeof(mBuffer)) {
            mBuffer[mLength++] = ' ';
        }
        return *this;
    }
    size_t length() const { return mLength; }
    void emit(int fd) {
        ssize_t w = ::write(STDERR_FILENO, mBuffer, mLength);
        if (fd >= 0) {
            w = ::write(fd, mBuffer, mLength);
        }
        (void)w;
        mLength = 0;
    }

private:
    char mBuffer[2048];
    size_t mLength = 0;
};

void writeReport(int sig, siginfo_t* info, void* ucontextRaw) {
    const int fd = gReportPath[0] ? ::open(gReportPath, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644) : -1;
    Line line;
    line.str("==== Petari native crash report ====\n").emit(fd);
    const auto& build = PetariNative::Platform::BuildIdentity::get();
    line.str("report version: 2\n").emit(fd);
    line.str("app version: ").str(build.appVersion).str("\n").emit(fd);
    line.str("source commit: ").str(build.sourceCommit).str("\n").emit(fd);
    line.str("executable UUID: ").str(build.executableUUID).str("\n").emit(fd);
    line.str("bundle executable UUID: ").str(build.bundleExecutableUUID).str("\n").emit(fd);
    line.str("macOS: ").str(build.macos).str("\n").emit(fd);
    line.str("model: ").str(build.model).str("\n").emit(fd);
    const mach_port_t threadPort = mach_thread_self();
    thread_extended_info_data_t thread{};
    mach_msg_type_number_t threadCount = THREAD_EXTENDED_INFO_COUNT;
    thread_info(threadPort, THREAD_EXTENDED_INFO, reinterpret_cast<thread_info_t>(&thread), &threadCount);
    thread.pth_name[sizeof(thread.pth_name) - 1] = 0;
    thread_identifier_info_data_t identifier{};
    threadCount = THREAD_IDENTIFIER_INFO_COUNT;
    thread_info(threadPort, THREAD_IDENTIFIER_INFO, reinterpret_cast<thread_info_t>(&identifier), &threadCount);
    // These Mach queries use fixed stack messages and kernel state, including
    // on host threads that have never installed an alternate signal stack.
    line.str("host thread: ").str(thread.pth_name).str("; thread id: ").dec(identifier.thread_id)
        .str("; mach thread id: ").dec(threadPort).str("\n").emit(fd);
    line.str("signal: ").str(signalName(sig)).str(" (").dec(sig).str("), code ").dec(info ? info->si_code : 0).str(", fault address ")
        .ptr(info ? info->si_addr : nullptr).str("\n").emit(fd);
    if (gHavePanic.load()) {
        line.str("last panic: ").str(gPanicMessage).str("\n").emit(fd);
    }
    OSThread* bound = OS::boundThread();
    OSThread* running = OSGetCurrentThread();
    if (bound) {
        line.str("OS thread: ").ptr(bound).str(" (priority ").dec(bound->priority).str(", base ").dec(bound->base)
            .str(") - faulting code ran on this game thread\n").emit(fd);
    } else {
        line.str("OS thread: none on this host thread (interrupt/host context); running OS thread ").ptr(running).str("\n").emit(fd);
    }
    line.str("interrupts disabled on faulting thread: ").str(OS::interruptsDisabled() ? "yes" : "no").str("\n").emit(fd);

    if (ucontextRaw) {
        auto* uc = static_cast<ucontext_t*>(ucontextRaw);
        const auto& ss = uc->uc_mcontext->__ss;
        line.str("host arm64 registers (not PowerPC):\n").emit(fd);
        line.str("  pc ").hex(__darwin_arm_thread_state64_get_pc(ss), 16).str("  lr ").hex(__darwin_arm_thread_state64_get_lr(ss), 16)
            .str("  sp ").hex(__darwin_arm_thread_state64_get_sp(ss), 16).str("  fp ").hex(__darwin_arm_thread_state64_get_fp(ss), 16)
            .str("\n").emit(fd);
        for (int i = 0; i < 29; i += 4) {
            for (int j = i; j < i + 4 && j < 29; ++j) {
                const size_t start = line.length();
                line.str("  x").dec(j);
                line.padTo(start + 6);
                line.hex(ss.__x[j], 16);
            }
            line.str("\n").emit(fd);
        }
    }

    line.str("host backtrace:\n").emit(fd);
    uintptr_t frames[64]{};
    unsigned count = 0;
    if (ucontextRaw) {
        const auto& ss = static_cast<ucontext_t*>(ucontextRaw)->uc_mcontext->__ss;
        frames[count++] = codeAddress(__darwin_arm_thread_state64_get_pc(ss));
        const uintptr_t lr = codeAddress(__darwin_arm_thread_state64_get_lr(ss));
        if (lr && lr != frames[0]) frames[count++] = lr;
        uintptr_t fp = __darwin_arm_thread_state64_get_fp(ss);
        const uintptr_t sp = __darwin_arm_thread_state64_get_sp(ss);
        unsigned walked = 0;
        while (count < 64 && walked++ < 64 && fp >= sp && fp && (fp & 15) == 0) {
            uintptr_t pair[2]{};
            mach_vm_size_t read = 0;
            // Mach copies into our fixed buffer and returns an error for an
            // unreadable frame (including guard pages). No direct dereference
            // of a corrupt stack and no libc unwinder or allocator in a fault.
            if (mach_vm_read_overwrite(mach_task_self(), fp, sizeof(pair),
                    reinterpret_cast<mach_vm_address_t>(pair), &read) != KERN_SUCCESS || read != sizeof(pair)) break;
            const uintptr_t address = codeAddress(pair[1]);
            if (address && !(walked == 1 && address == frames[count - 1])) frames[count++] = address;
            if (pair[0] <= fp) break;
            fp = pair[0];
        }
    }
    const unsigned images = std::min(gImageCount.load(), kMaxImages);
    for (unsigned f = 0; f < count; ++f) {
        const Image* found = nullptr;
        for (unsigned i = 0; i < images; ++i) {
            const auto& image = gImages[i];
            if (image.active.load(std::memory_order_acquire) && frames[f] >= image.textStart && frames[f] < image.textEnd) {
                found = &image;
                break;
            }
        }
        line.str("#").dec(f).str(" 0x").hex(frames[f]).str("  ").str(found ? found->name : "???")
            .str(" +0x").hex(found ? frames[f] - found->base : frames[f]).str("\n").emit(fd);
    }
    line.str("Binary Images:\n").emit(fd);
    for (unsigned i = 0; i < images; ++i) {
        const auto& image = gImages[i];
        if (image.active.load(std::memory_order_acquire)) {
            line.str("0x").hex(image.base).str(" ").str(image.uuid).str(" ").str(image.path).str("\n").emit(fd);
        }
    }
    if (images == kMaxImages) line.str("(image snapshot capacity reached)\n").emit(fd);

    line.str("game error handlers (registered, not called: no PowerPC context natively):\n").emit(fd);
    bool any = false;
    for (int e = 0; e < OS_ERROR_MAX; ++e) {
        if (gErrorHandlers[e]) {
            line.str("  ").str(errorName(e)).str(" -> ").ptr(reinterpret_cast<void*>(gErrorHandlers[e])).str("\n").emit(fd);
            any = true;
        }
    }
    if (!any) {
        line.str("  (none)\n").emit(fd);
    }

    line.str("recent OSReport output:\n").emit(fd);
    const unsigned next = gReportNext.load();
    for (unsigned i = (next > kReportLines ? next - kReportLines : 0); i < next; ++i) {
        line.str("  ").str(gReportRing[i % kReportLines]).str("\n").emit(fd);
    }
    line.str("==== end of report ====\n").emit(fd);
    if (fd >= 0) {
        ::close(fd);
    }
}

std::atomic<bool> gInHandler{false};

void crashHandler(int sig, siginfo_t* info, void* ucontext) {
    if (gInHandler.exchange(true)) {
        // A fault while writing the report (or a second fatal signal): do not
        // recurse into the report; terminate with this signal.
        static const char kNested[] = "==== Petari native crash report: fault inside the crash handler; terminating ====\n";
        ssize_t w = ::write(STDERR_FILENO, kNested, sizeof(kNested) - 1);
        (void)w;
        signal(sig, SIG_DFL);
        raise(sig);
        return;
    }
    writeReport(sig, info, ucontext);
    // Default action: terminate with this signal (debuggers and the system
    // crash reporter still see it).
    signal(sig, SIG_DFL);
    raise(sig);
}

}  // namespace

namespace PetariNative::Platform::OS {

void recordReport(const char* text) {
    // Split into lines; keep the last kReportLines.
    const char* p = text;
    while (*p) {
        const char* end = std::strchr(p, '\n');
        const size_t len = end ? static_cast<size_t>(end - p) : std::strlen(p);
        if (len > 0) {
            const unsigned slot = gReportNext.fetch_add(1) % kReportLines;
            const size_t n = std::min<size_t>(len, kLineLength - 1);
            std::memcpy(gReportRing[slot], p, n);
            gReportRing[slot][n] = '\0';
        }
        if (!end) {
            break;
        }
        p = end + 1;
    }
}

void recordPanic(const char* message) {
    std::snprintf(gPanicMessage, sizeof(gPanicMessage), "%s", message);
    gHavePanic.store(true);
}

}  // namespace PetariNative::Platform::OS

namespace PetariNative::Platform::Crash {

void install(const std::filesystem::path& reportDirectory) {
    if (gInstalled) {
        return;
    }
    gInstalled = true;
    PetariNative::HostAllocationScope hostAllocations;
    if (!reportDirectory.empty()) {
        std::snprintf(gReportPath, sizeof(gReportPath), "%s/petari-crash-%ld-%d.txt", reportDirectory.c_str(), static_cast<long>(std::time(nullptr)),
                      static_cast<int>(::getpid()));
    }
    PetariNative::Platform::BuildIdentity::initialize();
    _dyld_register_func_for_add_image(addImage);
    _dyld_register_func_for_remove_image(removeImage);
    task_dyld_info_data_t dyldInfo{};
    mach_msg_type_number_t dyldInfoCount = TASK_DYLD_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_DYLD_INFO, reinterpret_cast<task_info_t>(&dyldInfo), &dyldInfoCount) == KERN_SUCCESS) {
        const auto* info = reinterpret_cast<const dyld_all_image_infos*>(dyldInfo.all_image_info_addr);
        const auto* header = info->dyldImageLoadAddress;
        if (header && header->magic == MH_MAGIC_64) {
            const auto* h = reinterpret_cast<const mach_header_64*>(header);
            const auto* command = reinterpret_cast<const load_command*>(h + 1);
            for (uint32_t i = 0; i < h->ncmds; ++i) {
                if (command->cmd == LC_SEGMENT_64) {
                    const auto* segment = reinterpret_cast<const segment_command_64*>(command);
                    if (std::strcmp(segment->segname, SEG_TEXT) == 0) {
                        snapshotImage(header, reinterpret_cast<uintptr_t>(header) - segment->vmaddr, "/usr/lib/dyld");
                        break;
                    }
                }
                command = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(command) + command->cmdsize);
            }
        }
    }
    // Faults caused by stack overflow need their own stack.
    installThread();
    struct sigaction action{};
    action.sa_sigaction = crashHandler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_NODEFER;
    sigemptyset(&action.sa_mask);
    for (int sig : {SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGTRAP, SIGABRT}) {
        sigaction(sig, &action, nullptr);
    }
}

void installThread() {
    stack_t current{};
    if (sigaltstack(nullptr, &current) == 0 && !(current.ss_flags & SS_DISABLE) && current.ss_sp != nullptr) {
        return;  // this thread already has one
    }
    constexpr size_t kAltStackSize = 128 * 1024;
    void* stack = ::mmap(nullptr, kAltStackSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (stack == MAP_FAILED) {
        return;
    }
    stack_t ss{};
    ss.ss_sp = stack;
    ss.ss_size = kAltStackSize;
    if (sigaltstack(&ss, nullptr) != 0) {
        ::munmap(stack, kAltStackSize);
    }
}

std::filesystem::path reportPath() {
    return gReportPath[0] ? std::filesystem::path(gReportPath) : std::filesystem::path();
}

}  // namespace PetariNative::Platform::Crash

extern "C" {

OSErrorHandler OSSetErrorHandler(OSError error, OSErrorHandler handler) {
    if (error >= OS_ERROR_MAX) {
        return nullptr;
    }
    OS::InterruptGuard guard;
    OSErrorHandler previous = gErrorHandlers[error];
    gErrorHandlers[error] = handler;
    return previous;
}

// SDK default (OSError.c). FPSCR exception enables have no host effect.
u32 __OSFpscrEnableBits = 0xF8;

OSContext* OSGetCurrentContext(void) {
    if (tCurrentContext) {
        return tCurrentContext;
    }
    if (OSThread* bound = OS::boundThread()) {
        return &bound->context;
    }
    return &gHostThreadContext;
}

void OSSetCurrentContext(OSContext* context) {
    tCurrentContext = context;
}

// Resets the context's FPU ownership state, as the SDK does; registers are untouched.
void OSClearContext(OSContext* context) {
    context->mode = 0;
    context->state = 0;
}

void OSFillFPUContext(OSContext*) {
    OSPanic(__FILE__, __LINE__, "OSFillFPUContext(): there is no PowerPC FPU state on this host");
}

void OSProtectRange(u32 channel, void*, u32, u32) {
    OSPanic(__FILE__, __LINE__, "OSProtectRange(%u): MEM1 protection channels are not available on this host", channel);
}

u32 OSGetStackPointer(void) {
    return 0;  // no PowerPC stack natively
}

u32 PPCMfmsr(void) {
    u32 msr = OS::interruptsDisabled() ? 0 : kMsrEe;
    // Floating-point exceptions are never enabled on the host.
    return msr;
}

void PPCMtmsr(u32 value) {
    if (value & (kMsrFe0 | kMsrFe1)) {
        OSPanic(__FILE__, __LINE__, "PPCMtmsr(%08x): floating-point exceptions cannot be enabled on Apple silicon", value);
    }
    if (value & kMsrEe) {
        OSEnableInterrupts();
    } else {
        OSDisableInterrupts();
    }
}

void PPCHalt(void) {
    OSPanic(__FILE__, __LINE__, "PPCHalt(): the game halted the processor");
}

void OSFatal(GXColor, GXColor, const char* message) {
    OS::recordPanic(message);
    std::fprintf(stderr, "OSFatal: %s\n", message);
    std::fflush(nullptr);
    std::abort();
}

}  // extern "C"
