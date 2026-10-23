#!/usr/bin/env python3
"""Compose the opt-in spike over final generated sources, without editing Aurora."""
import sys,re
from pathlib import Path
source,output,kind,root,header=map(Path,sys.argv[1:])
kind=str(kind);here=Path(__file__).resolve().parent
text=source.read_text()
def replace(a,b):
 global text
 if text.count(a)!=1: raise ValueError(f'ubershader {kind} anchor count {text.count(a)}: {a}')
 text=text.replace(a,b)
# Make copied header's dependencies absolute. All consumers must share this header.
if kind=='header':
 text=re.sub(r'#include "([^"]+)"',lambda m:f'#include "{(root/"lib/gx"/m[1]).resolve()}"',text)
 replace('  gfx::Range uniformRange;','  gfx::Range uniformRange;\n  uint32_t uberUniformOffset = UINT32_MAX;')
else:
 text=text.replace('#include "pipeline.hpp"',f'#include "{header}"').replace('#include "../gx/pipeline.hpp"',f'#include "{header}"')
 if kind in ('shader','cache','pipeline','command_processor','frame'):
  text=f'#include "{header}"\n#include "{here / "ubershader.hpp"}"\n'+text
 if kind in ('frame','recording'):
  text='#include <cstdlib>\n#include <cstring>\n'+text
  size='([]{ const auto* p=std::getenv("PETARI_UBERSHADER"); return p && (std::strcmp(p,"1")==0 || std::strcmp(p,"all")==0) ? 5120u : gx::MaxUniformSize; }())'
  text=text.replace('gx::MaxUniformSize',size)
 if kind=='shader':
  replace('  const auto shaderSource = build_shader_source(config, dstAlphaMode, normalAttachment);','  if (petariBuildingUber) return petari_uber_shader(config, dstAlphaMode, normalAttachment);\n  const auto shaderSource = build_shader_source(config, dstAlphaMode, normalAttachment);')
  text+='\n'+(here/'ubershader_runtime.inc').read_text()
  text+='\nstatic constexpr const char* petariUberWgsl = R"UBER('+ (here/'ubershader.wgsl').read_text()+')UBER";\n'
  text+='\n'+(here/'ubershader_source.inc').read_text()
 if kind=='frame':
  replace('  begin_pipeline_frame();','  gx::petari_uber_set_recording_frame(frame.frameId);\n  begin_pipeline_frame();')
  replace('    webgpu::gpu_prof::frame_begin(g_framePackets[frameSlot].encoder);', '    gx::petari_uber_begin_frame(g_framePackets[frameSlot].frameId);\n    webgpu::gpu_prof::frame_begin(g_framePackets[frameSlot].encoder);')
 if kind=='command_processor':
  replace('  gfx::Range uniformRange{};','  gfx::Range uniformRange{};\n  gfx::Range uberUniformRange{};')
  replace('    cache.uniformRange = build_uniform(cache.shaderInfo);','''    cache.uniformRange = build_uniform(cache.shaderInfo);
    cache.uberUniformRange = {};
    if (petari_uber_enabled() && std::strcmp(petari_uber_reason(cache.config.shaderConfig), "covered") == 0)
      cache.uberUniformRange = petari_uber_uniform(cache.config.shaderConfig);''')
  replace('  gfx::push_draw_command(DrawData{','  petari_uber_note(cache.config.shaderConfig);\n  gfx::push_draw_command(DrawData{')
  replace('      .uniformRange = cache.uniformRange,','      .uniformRange = cache.uniformRange,\n      .uberUniformOffset = cache.uberUniformRange.size ? cache.uberUniformRange.offset : UINT32_MAX,')
 if kind=='pipeline':
  replace('''  if (!gfx::get_pipeline(data.pipeline, pipeline)) {
    return;
  }''','''  const bool covered = data.uberUniformOffset != UINT32_MAX;
  const auto uberKey = covered ? petari_uber_lookup(data.pipeline) : 0;
  gfx::CompiledPipeline available;
  bool fallbackReady = covered && petari_uber_ready(uberKey, available);
  bool uber = false;
  const bool force = petari_uber_enabled() && petari_uber_all() && covered;
  const bool missing = petari_uber_enabled() && covered && !fallbackReady;
  if (missing) petari_uber_not_ready(uberKey);
  if (force && fallbackReady) { pipeline = available; uber = true; }
  if (!uber && !gfx::get_pipeline(data.pipeline, pipeline)) {
    if (!fallbackReady) fallbackReady = covered && petari_uber_ready(uberKey, available);
    if (!fallbackReady) {
      if (missing) petari_uber_not_ready(uberKey, true);
      return;
    }
    pipeline = available;
    uber = true;
  }
  if (petari_uber_enabled()) {
    petari_uber_draw(uber, force && uber, missing);
  }''')
  replace('  const std::array offsets{data.uniformRange.offset};','  const std::array offsets{uber ? data.uberUniformOffset : data.uniformRange.offset};')
 if kind=='cache':
  text=f'#include <type_traits>\n#include "{root / "lib/gfx/render_worker.hpp"}"\nnamespace aurora::webgpu::gpu_prof {{ void petari_csv_finish(); }}\n'+text
  # Emit exact pending key membership before a complete-scene wait. Normal
  # pipeline diagnostics do not retain that vector after the frame completes.
  replace('  if (ready()) return;\n  petariPipelineWaiters.fetch_add(1, std::memory_order_relaxed);','''  if (ready()) return;
  if (gx::petari_uber_enabled() && PetariPipeline::diagnostics()) {
    auto unique=keys;std::sort(unique.begin(),unique.end());unique.erase(std::unique(unique.begin(),unique.end()),unique.end());
    for(auto key:unique) if(!g_pipelines.contains(key)) {
      HashType configHash=0,shaderHash=0;const char* reason="unknown_config";unsigned fog=0,indirect=0,tev=0;
      auto sample=petariPipelineSamples.find(key);
      if(sample!=petariPipelineSamples.end()) {
        configHash=sample->second.configKey;auto known=g_knownPipelines.find(configHash);
        if(known!=g_knownPipelines.end()) {
          reason="non_gx";
          if(known->second.type==ShaderType::GX) {
            const auto& c=std::get<gx::PipelineConfig>(known->second.config).shaderConfig;
            shaderHash=xxh3_hash(c);reason=gx::petari_uber_reason(c);fog=c.fogType;indirect=c.numIndStages;tev=c.tevStageCount;
          }
        }
      }
      const auto uber=gx::petari_uber_lookup(key);CompiledPipeline available;
      const bool fallbackReady=uber && gx::petari_uber_ready(uber,available);
      std::fprintf(stderr,"[gx uber wait] frame=%llu key=%016llx config=%016llx shader=%016llx reason=%s fallback_key=%016llx fallback_ready=%u fog=%u indirect=%u tev=%u\\n",
        (unsigned long long)gx::petari_uber_recording_frame(),(unsigned long long)key,(unsigned long long)configHash,(unsigned long long)shaderHash,reason,(unsigned long long)uber,fallbackReady,fog,indirect,tev);
    }
  }
  petariPipelineWaiters.fetch_add(1, std::memory_order_relaxed);''')
  replace('  if (!done) {\n    size_t pending = 0;','''  if (gx::petari_uber_enabled() && PetariPipeline::diagnostics())
    std::fprintf(stderr,"[gx uber wait end] frame=%llu wait_ms=%.3f keys=%zu completed=%u\\n",(unsigned long long)gx::petari_uber_recording_frame(),PetariPipeline::milliseconds(waited),keys.size(),done);
  if (!done) {
    size_t pending = 0;''')
  # Covered draws with a boot-ready fallback queue their specialised pipeline without waiting.
  anchor='''  const PetariPipeline::BoundedDrawScope petariBoundedDraw{'''
  replace(anchor,'''  if (gx::petari_uber_enabled()) {
    const bool covered = std::strcmp(gx::petari_uber_reason(config.shaderConfig), "covered") == 0;
    CompiledPipeline uber;
    if (covered) {
      const auto key = gx::petari_uber_key(config, layout);
      gx::petari_uber_associate(xxh3_hash(layout.key, xxh3_hash(config, static_cast<HashType>(ShaderType::GX))), key);
      const bool ready = gx::petari_uber_ready(key, uber);
      if (!ready) petari_uber_schedule_draw(config, layout);
      petari_uber_observe_title(true, ready);
      if (ready) return resolve_pipeline(ShaderType::GX, config, layout, PipelinePriority::Normal);
    } else petari_uber_observe_title(false, false);
  }
'''+anchor)
  replace('  const size_t loadedCount = load_pipeline_cache();','  gx::petari_uber_boot();\n  const size_t loadedCount = load_pipeline_cache();\n  petari_uber_schedule_known();')
  replace('void shutdown_pipeline_cache() {', 'void shutdown_pipeline_cache() {\n  gx::petari_uber_report();')
  replace('    g_pipelineThreads.clear();', '''    g_pipelineThreads.clear();
    petari_uber_async_stop();
    gx::petari_uber_report();
    petari_uber_async_report();
    gx::petari_uber_release();''')
  replace('    const auto sampleIt = petariPipelineSamples.find(ref);', '    CompiledPipeline uber;\n    if (gx::petari_uber_enabled() && gx::petari_uber_ready(gx::petari_uber_lookup(ref), uber)) return false;\n    const auto sampleIt = petariPipelineSamples.find(ref);')
  # Explicit reporting endpoint and shutdown both expose cumulative coverage.
  replace('extern "C" void petari_gx_pipeline_report() {','''extern "C" void petari_gx_pipeline_report() {
  const auto* gpuCsv = std::getenv("PETARI_GPU_TIMING_CSV");
  if (gpuCsv && *gpuCsv) {
    render_worker::enqueue_work([] { webgpu::gpu_prof::petari_csv_finish(); });
    render_worker::synchronize();
  }
  gx::petari_uber_report();
  petari_uber_async_report();''')
  replace('const auto petariReady = [=] { return g_pipelines.contains(runtimeKey) || g_pipelineThreadEnd; };',
          'const auto petariReady = [=] { return petari_uber_or_specialized_ready(runtimeKey); };')
  replace('std::all_of(keys.begin(), keys.end(), [](auto key) { return g_pipelines.contains(key); });',
          'std::all_of(keys.begin(), keys.end(), [](auto key) { return petari_uber_or_specialized_ready(key); });')
  # Stage manifests already provide validated configs on the recording side.
  # Queue/promote their fallback jobs before submitting specialised stage work.
  replace('    std::visit([&](const auto& config) { submit(item.type, config, hot); }, item.config);', '''    std::visit([&](const auto& config) {
      if constexpr (std::is_same_v<std::decay_t<decltype(config)>, gx::PipelineConfig>) {
        if (!overlay) petari_uber_schedule_stage(config, scene, offscreen, stageName);
      }
      submit(item.type, config, hot);
    }, item.config);''')
  replace('  size_t pending = 0, gatePending = 0;',
          '  if (!overlay) petari_uber_stage_report(stageName);\n  size_t pending = 0, gatePending = 0;')
  text+='\n#include <aurora/gfx.hpp>\n'+(here/'ubershader_boot.inc').read_text()
output.parent.mkdir(parents=True,exist_ok=True)
if not output.exists() or output.read_text()!=text:output.write_text(text)
