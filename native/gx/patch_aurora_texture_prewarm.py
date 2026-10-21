#!/usr/bin/env python3
"""Texture prewarm for the native app (native/app/hud_prewarm.cpp). The game builds its HUD layouts at scene
initialization and draws them later; the first draw created their GPU textures (a 20-37 ms frame about 85 frames into
every stage). petari_gx_texture_prewarm(GXTexObj*) queues a texture object (any thread, under a mutex); the queue is
drained in texture::end_frame, which runs after the FIFO drain so the resolver is idle, and resolved through the
ordinary static-texture path (object cache, content cache, upload). A later draw of the same texels then hits the
content cache. Applied to lib/gx/texture.cpp after patch_aurora_texture_timing.py; fails if Aurora's source changes.

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


replace_once('#include <list>\n', '#include <list>\n#include <mutex>\n#include <vector>\n', 'includes')

# Queue and drain, defined where resolve_static_texture is already declared, just before end_frame.
replace_once('void end_frame() noexcept {\n  const auto streamingStats', '''namespace {
std::mutex sPetariPrewarmMutex;
std::vector<GXTexObj_> sPetariPrewarmQueue;
std::atomic<bool> sPetariPrewarmPending{false};
}  // namespace

void petari_enqueue_texture_prewarm(const GXTexObj_& obj) {
  std::lock_guard lock{sPetariPrewarmMutex};
  sPetariPrewarmQueue.push_back(obj);
  sPetariPrewarmPending.store(true, std::memory_order_release);
}

// One relaxed-cost check per frame; the work only happens after a prewarm request.
static void petari_drain_texture_prewarm() {
  if (!sPetariPrewarmPending.load(std::memory_order_acquire)) {
    return;
  }
  std::vector<GXTexObj_> items;
  {
    std::lock_guard lock{sPetariPrewarmMutex};
    items.swap(sPetariPrewarmQueue);
    sPetariPrewarmPending.store(false, std::memory_order_release);
  }
  for (const auto& obj : items) {
    if (obj.has_data() && obj.width() != 0 && obj.height() != 0) {
      resolve_static_texture(obj);
    }
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
'''
output.write_text(text)
