#include "test_crash.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

namespace PetariNative::App::TestCrash {
namespace {
// Distinct non-inlined frames, including in optimized Release builds. Keep
// tail calls disabled so both our reports and macOS reports verify the chain.
__attribute__((noinline, disable_tail_calls)) void testCrashLeaf(const char* kind) {
    if (std::strcmp(kind, "null-call") == 0) {
        void (*volatile function)() = nullptr;
        function();
    } else if (std::strcmp(kind, "segv-write") == 0) {
        volatile std::uintptr_t address = 0;
        *reinterpret_cast<volatile int*>(address) = 1;
    } else {
        std::abort();
    }
}
__attribute__((noinline, disable_tail_calls)) void testCrashMiddle(const char* kind) {
    testCrashLeaf(kind);
}
__attribute__((noinline, disable_tail_calls)) void testCrashOuter(const char* kind) {
    testCrashMiddle(kind);
}
}  // namespace

void trigger(const char* kind) { testCrashOuter(kind); }

void frame() {
    static const char* kind = std::getenv("PETARI_TEST_CRASH");
    if (!kind) return;
    static bool fired = false;
    static const auto first = std::chrono::steady_clock::now();
    if (fired || std::chrono::steady_clock::now() - first < std::chrono::seconds(3)) return;
    fired = true;
    if (std::strcmp(kind, "null-call") && std::strcmp(kind, "segv-write") && std::strcmp(kind, "abort")) {
        std::fprintf(stderr, "PETARI_TEST_CRASH: choose null-call, segv-write, or abort. No test crash was triggered.\n");
        return;
    }
    std::fprintf(stderr, "PETARI_TEST_CRASH: deliberately triggering %s for release verification.\n", kind);
    trigger(kind);
}
}  // namespace PetariNative::App::TestCrash
