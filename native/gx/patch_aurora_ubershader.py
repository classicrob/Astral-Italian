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
 replace('  gfx::Range uniformRange;','  gfx::Range uniformRange;\n  gfx::Range uberUniformRange{};\n  gfx::PipelineRef uberPipeline = 0;')
else:
 text=text.replace('#include "pipeline.hpp"',f'#include "{header}"').replace('#include "../gx/pipeline.hpp"',f'#include "{header}"')
 if kind in ('shader','cache','pipeline','command_processor'):
  text=f'#include "{header}"\n#include "{here / "ubershader.hpp"}"\n'+text
 if kind=='shader':
  replace('  const auto shaderSource = build_shader_source(config, dstAlphaMode, normalAttachment);','  if (petariBuildingUber) return petari_uber_shader(dstAlphaMode, normalAttachment);\n  const auto shaderSource = build_shader_source(config, dstAlphaMode, normalAttachment);')
  text+='\n'+(here/'ubershader_runtime.inc').read_text()
  text+='\nstatic constexpr const char* petariUberWgsl = R"UBER('+ (here/'ubershader.wgsl').read_text()+')UBER";\n'
  text+='\n'+(here/'ubershader_source.inc').read_text()
 if kind=='command_processor':
  replace('  gfx::Range uniformRange{};','  gfx::Range uniformRange{};\n  gfx::Range uberUniformRange{};')
  replace('    cache.uniformRange = build_uniform(cache.shaderInfo);','''    cache.uniformRange = build_uniform(cache.shaderInfo);
    cache.uberUniformRange = {};
    if (petari_uber_enabled() && std::strcmp(petari_uber_reason(cache.config.shaderConfig), "covered") == 0)
      cache.uberUniformRange = petari_uber_uniform(cache.config.shaderConfig);''')
  replace('  gfx::push_draw_command(DrawData{','  petari_uber_note(cache.config.shaderConfig);\n  gfx::push_draw_command(DrawData{')
  replace('      .uniformRange = cache.uniformRange,','      .uniformRange = cache.uniformRange,\n      .uberUniformRange = cache.uberUniformRange,\n      .uberPipeline = cache.uberUniformRange.size ? petari_uber_key(cache.config, gfx::get_render_target_layout()) : 0,')
 if kind=='pipeline':
  replace('''  if (!gfx::get_pipeline(data.pipeline, pipeline)) {
    return;
  }''','''  bool uber = false;
  bool force = petari_uber_enabled() && petari_uber_all() && data.uberPipeline;
  bool missing = false;
  if (force) uber = petari_uber_ready(data.uberPipeline, pipeline);
  if (!uber && !gfx::get_pipeline(data.pipeline, pipeline)) {
    if (!data.uberPipeline || !petari_uber_ready(data.uberPipeline, pipeline)) return;
    uber = true;
  }
  if (petari_uber_enabled()) {
    gfx::CompiledPipeline available;
    missing = data.uberPipeline && !petari_uber_ready(data.uberPipeline, available);
    petari_uber_draw(uber, force && uber, missing);
  }''')
  replace('  const std::array offsets{data.uniformRange.offset};','  const std::array offsets{uber ? data.uberUniformRange.offset : data.uniformRange.offset};')
 if kind=='cache':
  # Covered draws with a boot-ready fallback queue their specialised pipeline without waiting.
  anchor='''  const PetariPipeline::BoundedDrawScope petariBoundedDraw{'''
  replace(anchor,'''  if (gx::petari_uber_enabled() && std::strcmp(gx::petari_uber_reason(config.shaderConfig), "covered") == 0) {
    CompiledPipeline uber;
    if (gx::petari_uber_ready(gx::petari_uber_key(config, layout), uber))
      return resolve_pipeline(ShaderType::GX, config, layout, PipelinePriority::Normal);
  }
'''+anchor)
  replace('  const size_t loadedCount = load_pipeline_cache();','  const size_t loadedCount = load_pipeline_cache();\n  gx::petari_uber_boot();')
  replace('  petari_pipeline_summary();\n}', '  petari_pipeline_summary();\n  gx::petari_uber_report();\n}') if '  petari_pipeline_summary();\n}' in text else None
  # Explicit reporting endpoint and shutdown both expose cumulative coverage.
  replace('extern "C" void petari_gx_pipeline_report() {','extern "C" void petari_gx_pipeline_report() {\n  gx::petari_uber_report();')
  text+='''
namespace aurora::gx {
void petari_uber_boot() {
  if (!petari_uber_enabled()) return;
  auto scene=gfx::scene_render_target_layout();auto off=scene;off.colorAttachmentCount=1;off.sampleCount=1;gfx::detail::finalize_render_target_layout(off);
  const auto prepare=[&](const PipelineConfig& c){petari_uber_prepare(c,scene);if(off.key!=scene.key)petari_uber_prepare(c,off);};
  for(auto& [k,item]:gfx::g_knownPipelines) if(item.type==gfx::ShaderType::GX) prepare(std::get<PipelineConfig>(item.config));
  const char* env=std::getenv("PETARI_PIPELINE_SEED_DIR");auto dir=env&&*env?std::filesystem::path(env):std::filesystem::path(g_config.resourcesPath?g_config.resourcesPath:"")/"pipeline-seeds";
  std::error_code ec;
  for(auto& file:std::filesystem::directory_iterator(dir,ec)) {
    if(file.path().extension()!=".db")continue;
    auto* db=gfx::open_pipeline_cache_seed_db(file.path().string());if(!db)continue;
    sqlite3_stmt* stmt=nullptr;
    if(sqlite3_prepare_v2(db,"SELECT config FROM pipeline_cache WHERE type=1 AND config_version=13",-1,&stmt,nullptr)==SQLITE_OK) {
      while(sqlite3_step(stmt)==SQLITE_ROW) if(sqlite3_column_bytes(stmt,0)==sizeof(PipelineConfig)){PipelineConfig c;std::memcpy(&c,sqlite3_column_blob(stmt,0),sizeof(c));if(gx::petari_cached_shader_valid(c.shaderConfig,0))prepare(c);}
    }
    sqlite3_finalize(stmt);sqlite3_close(db);
  }
}
}
'''
output.parent.mkdir(parents=True,exist_ok=True)
if not output.exists() or output.read_text()!=text:output.write_text(text)
