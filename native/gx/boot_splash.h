#pragma once
#include "frontend_policy.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <cstdio>
// Shared by main and the actual Aurora surface-present callback. No game/GX
// state advances in this phase. Presented means Present() returned success;
// WindowServer evidence is separate (it does not prove scan-out).
namespace PetariBootSplash {
// Set by the windowed app before Aurora init. Other Aurora clients retain
// their original show behavior. The app really keeps the SDL window hidden.
inline bool deferWindowShow = false;
inline void startupEvent(const char* event) {
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    std::fprintf(stderr, "[gx startup call] event=%s monotonic_ns=%lld\n", event, (long long)ns);
}
struct SlowCall {
    const char* call;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    ~SlowCall() {
        const auto stop = std::chrono::steady_clock::now();
        if (stop - start < std::chrono::milliseconds(50)) return;
        const auto ns = [](const auto& t) { return (long long)std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count(); };
        std::fprintf(stderr, "[gx startup span] call=%s start_ns=%lld stop_ns=%lld elapsed_ms=%.3f\n",
            call, ns(start), ns(stop), std::chrono::duration<double, std::milli>(stop - start).count());
    }
};
inline std::atomic<bool> active{false};
inline std::atomic<std::uint64_t> presented{0}, lastPresentNs{0};
inline std::atomic<bool> captureClaimed{false}, captureDone{false};
}
extern "C" void petari_boot_splash_progress(std::size_t published, std::size_t total);
extern "C" bool petari_boot_splash_hidden();
extern "C" bool petari_boot_splash_show_window();
extern "C" bool petari_boot_splash_pump();
extern "C" bool petari_boot_splash_window_ready();
extern "C" void petari_boot_splash_presented(std::uint64_t frame);
extern "C" void petari_boot_splash_finish();
// Finite native frontend phase: arm in init, release only after a visible,
// successfully presented splash, then poll/finish from ordinary main frames.
extern "C" bool petari_gx_frontend_armed();
extern "C" void petari_gx_frontend_begin();
extern "C" void petari_gx_frontend_status(unsigned* total, unsigned* pending, unsigned* failed, double* elapsedMs);
extern "C" bool petari_gx_frontend_finish(unsigned frames, unsigned pumps, double maxGapMs, bool mainThread);
