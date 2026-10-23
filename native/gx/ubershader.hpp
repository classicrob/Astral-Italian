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
void petari_uber_draw(bool fallback, bool forced, bool missing);
extern thread_local bool petariBuildingUber;
wgpu::ShaderModule petari_uber_shader(DstAlphaMode, uint32_t);
}
