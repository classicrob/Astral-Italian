#include <algorithm>
#include <cstdint>
#include <csignal>
#include <execinfo.h>
#include <sys/mman.h>
#include <sys/ucontext.h>
#include <unistd.h>
#if defined(__aarch64__)
#include <mach/arm/thread_status.h>
#endif

namespace {
void trapWrite(const char* text) {
    std::size_t size = 0;
    while (text[size]) ++size;
    while (size) {
        const auto written = ::write(STDERR_FILENO, text, size);
        if (written <= 0) return;
        text += written;
        size -= static_cast<std::size_t>(written);
    }
}
void trapHex(std::uintptr_t value) {
    char buffer[2 + sizeof(value) * 2 + 2];
    buffer[0] = '0'; buffer[1] = 'x';
    for (std::size_t index = 0; index < sizeof(value) * 2; ++index)
        buffer[2 + index] = "0123456789abcdef"[(value >> (4 * (sizeof(value) * 2 - 1 - index))) & 15];
    buffer[sizeof(buffer) - 2] = '\n'; buffer[sizeof(buffer) - 1] = 0;
    trapWrite(buffer);
}
volatile std::sig_atomic_t trapInHandler = 0;
void trapFault(int signal, siginfo_t* info, void* rawContext) {
    if (trapInHandler) _Exit(128 + signal);
    trapInHandler = 1;
    const auto address = reinterpret_cast<std::uintptr_t>(info->si_addr);
    const TrapGuard* found = nullptr;
    for (const auto& guard : trapGuards) {
        const auto start = reinterpret_cast<std::uintptr_t>(guard.start);
        if (address >= start && address - start < guard.size) { found = &guard; break; }
    }
    trapWrite(found ? "PLACEHOLDER(port): undefined data access: " : "PLACEHOLDER(port): diagnostic crash outside guarded data: ");
    trapWrite(found ? found->name : "unmapped symbol"); trapWrite("\nfault address: "); trapHex(address);
    const auto* context = static_cast<const ucontext_t*>(rawContext);
    std::uintptr_t pc = 0;
#if defined(__aarch64__)
    pc = arm_thread_state64_get_pc(context->uc_mcontext->__ss);
#elif defined(__x86_64__)
    pc = context->uc_mcontext->__ss.__rip;
#endif
    trapWrite("fault PC: "); trapHex(pc);
    void* stack[32];
    const int count = ::backtrace(stack, 32);
    ::backtrace_symbols_fd(stack, count, STDERR_FILENO);
    std::abort();
}
__attribute__((constructor(101))) void trapProtectData() {
    const long page = ::sysconf(_SC_PAGESIZE);
    if (page <= 0) { trapWrite("PLACEHOLDER(port): cannot determine guard page size\n"); _Exit(2); }
    std::uintptr_t low = UINTPTR_MAX, high = 0;
    for (const auto& guard : trapGuards) {
        const auto start = reinterpret_cast<std::uintptr_t>(guard.start);
        if (start % page || guard.size % page) { trapWrite("PLACEHOLDER(port): guard range is not page aligned\n"); _Exit(2); }
        low = std::min(low, start); high = std::max(high, start + guard.size);
    }
    void* warmup[8]; ::backtrace(warmup, 8);
    struct sigaction action{};
    action.sa_sigaction = trapFault; action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    if (::sigaction(SIGSEGV, &action, nullptr) || ::sigaction(SIGBUS, &action, nullptr)) {
        trapWrite("PLACEHOLDER(port): cannot install diagnostic fault handler\n"); _Exit(2);
    }
    if (high && ::mprotect(reinterpret_cast<void*>(low), high - low, PROT_NONE)) {
        trapWrite("PLACEHOLDER(port): cannot protect diagnostic data section\n"); _Exit(2);
    }
}
}
