#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
// Shared by main and the actual Aurora surface-present callback. No game/GX
// state advances in this phase. Presented means Present() returned success;
// WindowServer evidence is separate (it does not prove scan-out).
namespace PetariBootSplash {
inline std::atomic<bool> active{false};
inline std::atomic<std::uint64_t> presented{0}, lastPresentNs{0};
inline std::atomic<bool> captureClaimed{false}, captureDone{false};
}
extern "C" void petari_boot_splash_progress(std::size_t published, std::size_t total);
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
