#include <petari/gpu_trace.hpp>
#include <petari/pipeline_startup.hpp>
#include <petari/home_menu.hpp>
#include <petari/latency_probe.hpp>
#include "host.hpp"
#include "smoke_background.hpp"
#include "../gx/boot_splash.h"
#include <aurora/aurora.h>
#include <aurora/event.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <cstdio>

namespace PetariNative::App {
namespace GpuTrace = PetariNative::GpuTrace;
bool prepareFrontend(SDL_Window* window) {
    // Background smoke remains hidden. It does not claim visible preparation.
    if (SmokeBackground::enabled) {
        if (petari_gx_frontend_armed()) petari_gx_frontend_finish(0, 0, 0, true);
        return true;
    }
    if (PetariBootSplash::deferWindowShow) {
        // Bootstrap the platform once while the actual SDL window is hidden.
        // Menus/fonts are already installed. The first update initializes SDL
        // input and handles its device-add events; no game/route loop exists.
        if (!petari_boot_splash_hidden()) {
            std::fputs("[gx boot bootstrap] result=failed reason=window-already-visible\n", stderr);
            return false;
        }
        PetariBootSplash::startupEvent("hidden_bootstrap_enter");
        if (!petari_boot_splash_pump()) return false;
        for (const AuroraEvent* event = aurora_update(); event && event->type != AURORA_NONE; ++event)
            if (event->type == AURORA_EXIT) return false;
        if (!petari_boot_splash_hidden()) {
            std::fputs("[gx boot bootstrap] result=failed reason=window-became-visible\n", stderr);
            return false;
        }
        PetariBootSplash::startupEvent("hidden_bootstrap_exit");
    }
    if (!petari_gx_frontend_armed()) return petari_boot_splash_show_window();
    constexpr Uint64 period = 1000000000 / 60;
    const auto shownAt = SDL_GetTicksNS();
    auto deadline = shownAt, lastPump = shownAt;
    double maxGapMs = 0, elapsedMs = 0;
    unsigned pumps = 0, total = 0, pending = 0, failed = 0;
    bool mainThread = true, started = false, quit = false;
    std::uint64_t submitted = 0, collected = 0;
    petari_boot_splash_progress(0, 0);
    const auto delay = [](Uint64 ns) { PetariBootSplash::SlowCall span{"splash_delay"};
        PETARI_GPU_TRACE(GpuTrace::waitBegin("SDL-DelayNS", GpuTrace::deadlineAfter(ns)));
        SDL_DelayNS(ns);
        PETARI_GPU_TRACE(GpuTrace::waitEnd()); };
    const auto collect = [&] {
        PETARI_GPU_TRACE(GpuTrace::emit("startup_collect_begin"));
        const auto count = PetariBootSplash::presented.load(std::memory_order_acquire);
        if (count > collected) {
            // At most one submission is outstanding. The acquire pairs with
            // the render worker's successful Present, never a guessed frame.
            Seam::recordStartupPresent(PetariBootSplash::lastPresentNs.load());
            collected = count;
        }
        PETARI_GPU_TRACE(GpuTrace::emit("startup_collect_end"));
    };
    while (!quit) {
        const auto pumpAt = SDL_GetTicksNS();
        maxGapMs = std::max(maxGapMs, (pumpAt - lastPump) / 1e6);
        lastPump = pumpAt; ++pumps;
        PETARI_GPU_TRACE(GpuTrace::emitScheduling("startup_pump_begin"));
        mainThread &= petari_boot_splash_pump();
        PETARI_GPU_TRACE(GpuTrace::emit("startup_pump_end"));
        PETARI_GPU_TRACE(GpuTrace::emit("startup_aurora_update_begin"));
        const auto* events = [] { PetariBootSplash::SlowCall span{"splash_aurora_update"}; return aurora_update(); }();
        PETARI_GPU_TRACE(GpuTrace::emit("startup_aurora_update_end"));
        for (const AuroraEvent* event = events; event && event->type != AURORA_NONE; ++event)
            if (event->type == AURORA_EXIT) quit = true;
        collect();
        if (quit || !mainThread) break;
        if (!started && collected && petari_boot_splash_window_ready()) {
            std::fprintf(stderr, "[gx frontend release] finished_launching=1 window_onscreen=1 alpha=1 bounds_nonzero=1 first_present=1 presented_frames=%llu\n",
                (unsigned long long)collected);
            { PetariBootSplash::SlowCall span{"frontend_admission"}; petari_gx_frontend_begin(); }
            started = true;
        }
        PETARI_GPU_TRACE(GpuTrace::emit("startup_frontend_status_begin"));
        { PetariBootSplash::SlowCall span{"frontend_status"}; petari_gx_frontend_status(&total, &pending, &failed, &elapsedMs); }
        PETARI_GPU_TRACE(GpuTrace::emit("startup_frontend_status_end"));
        if (started && collected == submitted && (elapsedMs >= PetariBootSplash::prepCapMs || failed || (!pending && collected >= 3))) break;
        // Failure to create a visible, presented frame is bounded too. No
        // fabricated readiness and no compile work starts in that case.
        if (!started && SDL_GetTicksNS() - shownAt >= 5000000000ull) break;
        if (collected < submitted) {
            delay(1000000); // keep servicing the main loop while GPU works
            continue;
        }
        const auto now = SDL_GetTicksNS();
        if (now < deadline) { delay(std::min<Uint64>(deadline - now, 2000000)); continue; }
        if ([] { PetariBootSplash::SlowCall span{"splash_begin_frame"}; return aurora_begin_frame(); }()) {
            int width = 0, height = 0;
            SDL_GetWindowSize(window, &width, &height);
            const float progress = total ? float(total - pending) / total : 0;
            { PetariBootSplash::SlowCall span{"splash_draw"}; HomeMenu::drawPreparingGraphics(progress, 0, 0, float(width), float(height)); }
            // Prep serials 0..N-1 correspond to CSV intervals 1..N-1:
            // row N-1 ends at present N-1, the first present is the anchor.
            PetariNative::LatencyProbe::frame.store(static_cast<unsigned>(submitted + 1));
            { PetariBootSplash::SlowCall span{"splash_end_frame"}; aurora_end_frame(); }
            ++submitted;
            deadline = now + period;
        } else delay(1000000);
    }
    collect();
    petari_gx_frontend_finish(static_cast<unsigned>(collected), pumps, maxGapMs, mainThread);
    // No game or route loop ran. openFirstFrame retains this recorder and its
    // last present seam; loading before the first game frame is counted too.
    petari_boot_splash_finish();
    std::fprintf(stderr, "[gx boot splash frames] presented=%llu csv_intervals=%llu first_serial=0 last_serial=%llu elapsed_ms=%.3f game_advanced=0 route_advanced=0\n",
        (unsigned long long)collected, (unsigned long long)Seam::startupFrameCount(),
        (unsigned long long)(collected ? collected - 1 : 0), (SDL_GetTicksNS() - shownAt) / 1e6);
    return !quit;
}
}
