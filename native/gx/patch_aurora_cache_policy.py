#!/usr/bin/env python3
"""Compose cache admission/build identity with the existing Aurora patch chain."""
from pathlib import Path
import sys


def replace(text, old, new):
    if text.count(old) != 1:
        raise ValueError('cache policy anchor mismatch: '+old[:120])
    return text.replace(old, new)


def patch_pipeline(text):
    here=Path(__file__).resolve().parent
    text=f'#include "{here / "pipeline_cache_identity.hpp"}"\n#include "{here / "pipeline_texgen_validity.hpp"}"\n'+text
    text=replace(text,'/ "pipeline_cache.db"','/ PetariPipelineCache::filename("pipeline_cache")')
    text=replace(text,'  ret = sqlite::exec(g_pipelineCacheDb, "PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL;");', '''  if (!PetariPipelineCache::identity(g_pipelineCacheDb, true)) {
    Log.warn("Ignoring foreign-build pipeline cache '{}'; build {}", path, PETARI_PIPELINE_BUILD_KEY);
    pipeline_cache_abort();
    return false;
  }
  ret = sqlite::exec(g_pipelineCacheDb, "PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL;");''')
    text=replace(text,'  return seedDb;\n}', '''  if (!PetariPipelineCache::identity(seedDb)) {
    Log.warn("Ignoring foreign-build pipeline seed '{}'", path);
    sqlite3_close(seedDb);
    return nullptr;
  }
  return seedDb;
}''')
    text=replace(text,'    remember_pipeline_config(type, config, firstFrameUsed, false);', '''    if constexpr (std::is_same_v<PipelineConfig, gx::PipelineConfig>) {
      if (!gx::petari_cached_shader_valid(config.shaderConfig,
            xxh3_hash(config, static_cast<HashType>(type)))) continue;
    }
    remember_pipeline_config(type, config, firstFrameUsed, false);''')
    text=replace(text,'\n      if (!write_pipeline_cache_record(write)) {', '''\n      if (write.type == ShaderType::GX) {
        if (write.configVersion != gx::GXPipelineConfigVersion || write.config.size() != sizeof(gx::PipelineConfig)) {
          ++skippedRows;
          continue;
        }
        gx::PipelineConfig config;
        std::memcpy(&config, write.config.data(), sizeof(config));
        if (config.version != gx::GXPipelineConfigVersion ||
            !gx::petari_cached_shader_valid(config.shaderConfig, write.hash)) {
          ++skippedRows;
          continue;
        }
      }
      if (!write_pipeline_cache_record(write)) {''')
    return text


def patch_dawn(text):
    header=Path(__file__).resolve().with_name('pipeline_cache_identity.hpp')
    text=f'#include "{header}"\n'+text
    text=replace(text,'/ "dawn_cache.db"','/ PetariPipelineCache::filename("dawn_cache")')
    text=replace(text,'  // WAL mode + NORMAL = no need for disk syncs, consistent but not durable is fine.', '''  if (!PetariPipelineCache::identity(db, true)) {
    Log.warn("Ignoring foreign-build Dawn cache '{}'", file);
    return false;
  }
  // WAL mode + NORMAL = no need for disk syncs, consistent but not durable is fine.''')
    return text

if __name__=='__main__':
    mode,source,dest=sys.argv[1:];out=Path(dest);text=patch_dawn(Path(source).read_text()) if mode=='dawn' else patch_pipeline(Path(source).read_text());out.parent.mkdir(parents=True,exist_ok=True)
    if not out.exists() or out.read_text()!=text:out.write_text(text)
