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
bool prepareFrontend(SDL_Window* window) {
    if (!petari_gx_frontend_armed()) return true;
    // Background smoke remains hidden. It does not claim visible preparation.
    if (SmokeBackground::enabled) {
        petari_gx_frontend_finish(0, 0, 0, true);
        return true;
    }
    constexpr Uint64 period = 1000000000 / 60;
    const auto shownAt = SDL_GetTicksNS();
    auto deadline = shownAt, lastPump = shownAt;
    double maxGapMs = 0, elapsedMs = 0;
    unsigned pumps = 0, total = 0, pending = 0, failed = 0;
    bool mainThread = true, started = false, quit = false;
    std::uint64_t submitted = 0, collected = 0;
    petari_boot_splash_progress(0, 0);
    const auto collect = [&] {
        const auto count = PetariBootSplash::presented.load(std::memory_order_acquire);
        if (count > collected) {
            // At most one submission is outstanding. The acquire pairs with
            // the render worker's successful Present, never a guessed frame.
            Seam::recordStartupPresent(PetariBootSplash::lastPresentNs.load());
            collected = count;
        }
    };
    while (!quit) {
        const auto pumpAt = SDL_GetTicksNS();
        maxGapMs = std::max(maxGapMs, (pumpAt - lastPump) / 1e6);
        lastPump = pumpAt; ++pumps;
        mainThread &= petari_boot_splash_pump();
        for (const AuroraEvent* event = aurora_update(); event && event->type != AURORA_NONE; ++event)
            if (event->type == AURORA_EXIT) quit = true;
        collect();
        if (quit || !mainThread) break;
        if (!started && collected && petari_boot_splash_window_ready()) {
            std::fprintf(stderr, "[gx frontend release] finished_launching=1 window_onscreen=1 alpha=1 bounds_nonzero=1 first_present=1 presented_frames=%llu\n",
                (unsigned long long)collected);
            petari_gx_frontend_begin();
            started = true;
        }
        petari_gx_frontend_status(&total, &pending, &failed, &elapsedMs);
        if (started && collected == submitted && (elapsedMs >= 5000 || failed || (!pending && collected >= 3))) break;
        // Failure to create a visible, presented frame is bounded too. No
        // fabricated readiness and no compile work starts in that case.
        if (!started && SDL_GetTicksNS() - shownAt >= 5000000000ull) break;
        if (collected < submitted) {
            SDL_DelayNS(1000000); // keep servicing the main loop while GPU works
            continue;
        }
        const auto now = SDL_GetTicksNS();
        if (now < deadline) { SDL_DelayNS(std::min<Uint64>(deadline - now, 2000000)); continue; }
        if (aurora_begin_frame()) {
            int width = 0, height = 0;
            SDL_GetWindowSize(window, &width, &height);
            const float progress = total ? float(total - pending) / total : 0;
            HomeMenu::drawPreparingGraphics(progress, 0, 0, float(width), float(height));
            // Prep serials 0..N-1 correspond to CSV intervals 1..N-1:
            // row N-1 ends at present N-1, the first present is the anchor.
            PetariNative::LatencyProbe::frame.store(static_cast<unsigned>(submitted + 1));
            aurora_end_frame();
            ++submitted;
            deadline = now + period;
        } else SDL_DelayNS(1000000);
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
