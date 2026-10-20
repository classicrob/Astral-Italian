#pragma once
// One-key playtest capture (F9): when the player presses it, the game thread writes a timestamped folder under
// PETARI_CAPTURE_DIR with
//   route.csv   the last 300 frames of Mario's state and the bound inputs (the PETARI_ROUTE_RECORD format, oldest first),
//   state.txt   the scene, stage, scenario, Mario's position, velocity, gravity and the camera at the press,
//   screen.png  the next presented frame.
// Inert unless PETARI_CAPTURE_DIR is set (native/play/play.sh sets ~/MacioGalaxy2-play/captures) and off in background
// smoke runs. Each press logs "[capture] saved <folder>". PETARI_CAPTURE_TEST_FRAME=<n> presses it at recorder frame n.
#include "route_record_writer.hpp"

#include <atomic>

namespace PetariNative::App::PlaytestCapture {
// Any thread (the input event handler): ask for a capture at the next game frame. Header-inline so the input
// layer (and its standalone tests) need no game code.
inline std::atomic<int> gRequests{0};
inline void request() { gRequests.fetch_add(1, std::memory_order_relaxed); }
// True when captures are configured (the per-frame recorder then samples every frame).
bool enabled(bool background);
// Game thread, once per frame from recordRouteFrame: keeps the history and writes a requested capture.
void frame(const RouteRecordFrame& row);
}  // namespace PetariNative::App::PlaytestCapture
