#pragma once
#include "gx.hpp"

namespace aurora::gx {
bool petari_uber_enabled();
bool petari_uber_all();
const char* petari_uber_reason(const ShaderConfig&);
gfx::Range petari_uber_uniform(const ShaderConfig&);
gfx::PipelineRef petari_uber_key(const PipelineConfig&, const gfx::RenderTargetLayout&);
void petari_uber_associate(gfx::PipelineRef, gfx::PipelineRef);
gfx::PipelineRef petari_uber_lookup(gfx::PipelineRef);
bool petari_uber_ready(gfx::PipelineRef, gfx::CompiledPipeline&);
void petari_uber_prepare(const PipelineConfig&, const gfx::RenderTargetLayout&);
void petari_uber_boot();
void petari_uber_report();
void petari_uber_note(const ShaderConfig&);
void petari_uber_begin_frame(uint64_t);
void petari_uber_set_recording_frame(uint64_t);
uint64_t petari_uber_recording_frame();
void petari_uber_draw(bool fallback, bool forced, bool missing);
// Counts covered draw attempts before the draw's possible early return.
void petari_uber_not_ready(gfx::PipelineRef, bool skipped = false);
void petari_uber_scheduled(gfx::PipelineRef);
void petari_uber_release(); // after compilation workers have joined
extern thread_local bool petariBuildingUber;
uint32_t petari_uber_variant(const ShaderConfig&);
uint32_t petari_uber_arg_program(const ShaderConfig&);
wgpu::ShaderModule petari_uber_shader(const ShaderConfig&, DstAlphaMode, uint32_t);
}
