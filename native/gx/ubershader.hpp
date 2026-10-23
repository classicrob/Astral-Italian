#pragma once
#include "gx.hpp"

namespace aurora::gx {
bool petari_uber_enabled();
bool petari_uber_all();
const char* petari_uber_reason(const ShaderConfig&);
gfx::Range petari_uber_uniform(const ShaderConfig&);
enum class PetariUberTier { Generic, Fast };
struct PetariUberKeys { gfx::PipelineRef generic, fast; };
gfx::PipelineRef petari_uber_key(const PipelineConfig&, const gfx::RenderTargetLayout&);
gfx::PipelineRef petari_uber_key(const PipelineConfig&, const gfx::RenderTargetLayout&, PetariUberTier);
PetariUberKeys petari_uber_keys(const PipelineConfig&, const gfx::RenderTargetLayout&);
void petari_uber_associate(gfx::PipelineRef, PetariUberKeys);
gfx::PipelineRef petari_uber_lookup(gfx::PipelineRef, bool* fast = nullptr);
bool petari_uber_ready(gfx::PipelineRef, gfx::CompiledPipeline&);
void petari_uber_prepare(const PipelineConfig&, const gfx::RenderTargetLayout&);
void petari_uber_prepare(const PipelineConfig&, const gfx::RenderTargetLayout&, PetariUberTier);
void petari_uber_boot();
void petari_uber_report();
void petari_uber_note(const ShaderConfig&);
void petari_uber_begin_frame(uint64_t);
void petari_uber_end_frame();
void petari_uber_set_recording_frame(uint64_t);
uint64_t petari_uber_recording_frame();
void petari_uber_draw(bool fallback, bool forced, bool missing, bool fast);
// Counts covered draw attempts before the draw's possible early return.
void petari_uber_not_ready(gfx::PipelineRef, bool skipped = false);
void petari_uber_scheduled(gfx::PipelineRef);
void petari_uber_release(); // after compilation workers have joined
extern thread_local bool petariBuildingUber;
extern thread_local PetariUberTier petariBuildingUberTier;
uint32_t petari_uber_variant(const ShaderConfig&);
uint32_t petari_uber_variant(const ShaderConfig&, PetariUberTier);
uint32_t petari_uber_arg_program(const ShaderConfig&);
wgpu::ShaderModule petari_uber_shader(const ShaderConfig&, DstAlphaMode, uint32_t);
}
