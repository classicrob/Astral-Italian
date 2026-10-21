#!/usr/bin/env python3
"""Texture prewarm for the native app (native/app/hud_prewarm.cpp). The game builds its HUD layouts at scene
initialization and draws them later; the first draw created their GPU textures (a 20-37 ms frame about 85 frames into
every stage). petari_gx_texture_prewarm(GXTexObj*) queues a texture object (any thread, under a mutex); the queue is
drained in texture::end_frame, which runs after the FIFO drain so the resolver is idle, and resolved through the
ordinary static-texture path (object cache, content cache, upload) under a per-frame time budget
(PETARI_PREWARM_MS, default 1.5 ms, at least one texture per frame), so a large batch (every model and effect texture of a stage) is
spread over the loading/intro frames. A batch is dropped 900 frames after it was queued, and petari_gx_texture_prewarm_cancel()
drops what is left. A later draw of the same texels then hits the content cache. Applied to lib/gx/texture.cpp after patch_aurora_texture_timing.py; fails if Aurora's source changes.

  patch_aurora_texture_prewarm.py INPUT OUTPUT   (the two may be the same file)
"""
from pathlib import Path
import sys

source, output = map(Path, sys.argv[1:])
text = source.read_text()


def replace_once(old, new, what):
    global text
    if text.count(old) != 1:
        raise SystemExit(f'Aurora texture prewarm patch anchor mismatch: {what}')
    text = text.replace(old, new)


replace_once('#include <list>\n', '#include <chrono>\n#include <list>\n#include <mutex>\n#include <vector>\n', 'includes')

# Queue and drain, defined where resolve_static_texture is already declared, just before end_frame.
replace_once('void end_frame() noexcept {\n  const auto streamingStats', '''namespace {
std::mutex sPetariPrewarmMutex;
std::vector<GXTexObj_> sPetariPrewarmQueue;
std::atomic<bool> sPetariPrewarmPending{false};
std::atomic<std::uint32_t> sPetariPrewarmCancel{0};
std::vector<GXTexObj_> sPetariPrewarmWork;  // end_frame thread only
std::size_t sPetariPrewarmPos = 0;
std::uint32_t sPetariPrewarmCancelSeen = 0;
std::uint64_t sPetariPrewarmQueuedFrame = 0;
constexpr std::size_t kPetariPrewarmMax = 8192;
constexpr std::uint64_t kPetariPrewarmLifeFrames = 900;
double petariPrewarmBudgetMs() {
  static const double ms = [] {
    const char* value = std::getenv("PETARI_PREWARM_MS");
    return value != nullptr && value[0] != '\0' ? std::atof(value) : 1.5;
  }();
  return ms;
}
}  // namespace

void petari_enqueue_texture_prewarm(const GXTexObj_& obj) {
  std::lock_guard lock{sPetariPrewarmMutex};
  if (sPetariPrewarmQueue.size() < kPetariPrewarmMax) {
    sPetariPrewarmQueue.push_back(obj);
  }
  sPetariPrewarmPending.store(true, std::memory_order_release);
}

void petari_cancel_texture_prewarm() {
  std::lock_guard lock{sPetariPrewarmMutex};
  sPetariPrewarmQueue.clear();
  sPetariPrewarmCancel.fetch_add(1, std::memory_order_acq_rel);
  sPetariPrewarmPending.store(true, std::memory_order_release);
}

// One atomic load per frame when idle; otherwise resolves queued textures until the frame's budget is spent.
static void petari_drain_texture_prewarm() {
  if (sPetariPrewarmPending.load(std::memory_order_acquire)) {
    std::lock_guard lock{sPetariPrewarmMutex};
    const auto cancel = sPetariPrewarmCancel.load(std::memory_order_acquire);
    if (cancel != sPetariPrewarmCancelSeen) {
      sPetariPrewarmCancelSeen = cancel;
      sPetariPrewarmWork.clear();
      sPetariPrewarmPos = 0;
    }
    if (!sPetariPrewarmQueue.empty()) {
      if (sPetariPrewarmPos >= sPetariPrewarmWork.size()) {
        sPetariPrewarmWork.clear();
        sPetariPrewarmPos = 0;
      }
      sPetariPrewarmWork.insert(sPetariPrewarmWork.end(), sPetariPrewarmQueue.begin(), sPetariPrewarmQueue.end());
      sPetariPrewarmQueue.clear();
      sPetariPrewarmQueuedFrame = s_frameCount;
    }
    sPetariPrewarmPending.store(false, std::memory_order_release);
  }
  if (sPetariPrewarmPos >= sPetariPrewarmWork.size()) {
    return;
  }
  if (s_frameCount - sPetariPrewarmQueuedFrame > kPetariPrewarmLifeFrames) {
    sPetariPrewarmWork.clear();
    sPetariPrewarmPos = 0;
    return;
  }
  const auto start = std::chrono::steady_clock::now();
  const double budgetMs = petariPrewarmBudgetMs();
  while (sPetariPrewarmPos < sPetariPrewarmWork.size()) {
    const auto& obj = sPetariPrewarmWork[sPetariPrewarmPos++];
    if (obj.has_data() && obj.width() != 0 && obj.height() != 0) {
      resolve_static_texture(obj);
    }
    if (budgetMs > 0.0 &&
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() >= budgetMs) {
      break;
    }
  }
  if (sPetariPrewarmPos >= sPetariPrewarmWork.size()) {
    sPetariPrewarmWork.clear();
    sPetariPrewarmPos = 0;
  }
}

void end_frame() noexcept {
  petari_drain_texture_prewarm();
  const auto streamingStats''', 'end_frame')

# C entry point at the end of the file, outside every namespace.
text = text.rstrip('\n') + '''

// A static (non-palette) texture object, as built by GXInitTexObj, to be created in the texture cache at the next
// aurora_end_frame. Game thread.
extern "C" void petari_gx_texture_prewarm(const void* texObj) {
  PetariNative::HostAllocationScope petariHostAllocations;
  aurora::gx::texture::petari_enqueue_texture_prewarm(*static_cast<const GXTexObj_*>(texObj));
}

// Drops the textures still waiting to be created (a new stage is about to queue its own). Game thread.
extern "C" void petari_gx_texture_prewarm_cancel() {
  aurora::gx::texture::petari_cancel_texture_prewarm();
}
'''
output.write_text(text)
