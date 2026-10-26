#!/usr/bin/env python3
"""Compose strict, build-independent cache admission with pinned Aurora sources."""
from pathlib import Path
import sys


def replace(text, old, new):
    if text.count(old) != 1:
        raise ValueError('cache policy anchor mismatch: '+old[:120])
    return text.replace(old, new)


def patch_pipeline(text):
    here=Path(__file__).resolve().parent
    # GX declarations must precede the typed decoder; later uber patch replaces
    # these declarations with the same generated production types.
    text=replace(text,'namespace aurora::gfx {\nstatic Module',f'#include "{here / "pipeline_cache_migration.hpp"}"\n\nnamespace aurora::gfx {{\nstatic Module')
    text=replace(text,'/ "pipeline_cache.db"','/ PetariPipelineCache::filename("pipeline_cache")')
    text=replace(text,'  ret = sqlite::exec(g_pipelineCacheDb, "PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL;");', '''  if (!PetariPipelineCache::identity(g_pipelineCacheDb, true) ||
      !PetariPipelineCache::recipeContainer(g_pipelineCacheDb) ||
      !PetariPipelineCache::ensureRecipeIntegrity(g_pipelineCacheDb)) {
    Log.warn("Ignoring incompatible/corrupt recipe cache '{}' ABI {}", path, PETARI_PIPELINE_RECIPE_KEY);
    pipeline_cache_abort();
    return false;
  }
  ret = sqlite::exec(g_pipelineCacheDb, "PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL;");''')
    text=replace(text,'  return seedDb;\n}', '''  if (!PetariPipelineCache::identity(seedDb) || !PetariPipelineCache::recipeContainer(seedDb, true)) {
    Log.warn("Ignoring incompatible/corrupt pipeline seed '{}'", path);
    sqlite3_close(seedDb);
    return nullptr;
  }
  return seedDb;
}''')
    text=replace(text,'"SELECT config, first_frame_used FROM pipeline_cache "','"SELECT type,hash,config_version,config_size,config,first_frame_used FROM pipeline_cache "')
    start='''  size_t acceptedRows = 0;
  while ((ret = sqlite3_step(g_pipelineCacheLoadStmt)) == SQLITE_ROW) {'''
    text=replace(text,start,'''  size_t acceptedRows = 0, rejectedRows = 0;
  PetariPipelineCache::RecipeRows validation(g_pipelineCacheDb);
  while ((ret = sqlite3_step(g_pipelineCacheLoadStmt)) == SQLITE_ROW) {
    if (!PetariPipelineCache::admitRecipe(validation, g_pipelineCacheLoadStmt)) { ++rejectedRows; continue; }''')
    text=replace(text,'sqlite3_column_blob(g_pipelineCacheLoadStmt, 0)','sqlite3_column_blob(g_pipelineCacheLoadStmt, 4)')
    text=replace(text,'sqlite3_column_bytes(g_pipelineCacheLoadStmt, 0)','sqlite3_column_bytes(g_pipelineCacheLoadStmt, 4)')
    text=replace(text,'sqlite3_column_int64(g_pipelineCacheLoadStmt, 1)','sqlite3_column_int64(g_pipelineCacheLoadStmt, 5)')
    text=replace(text,'  sqlite3_clear_bindings(g_pipelineCacheLoadStmt);\n  return acceptedRows;','''  sqlite3_clear_bindings(g_pipelineCacheLoadStmt);
  std::fprintf(stderr, "[pipeline recipes] type=%u accepted=%zu rejected=%zu ABI=%s\\n",
      unsigned(type), acceptedRows, rejectedRows, PETARI_PIPELINE_RECIPE_KEY);
  return acceptedRows;''')
    text=replace(text,'    while ((ret = sqlite3_step(seedStmt)) == SQLITE_ROW) {','''    PetariPipelineCache::RecipeRows validation(seedDb);
    while ((ret = sqlite3_step(seedStmt)) == SQLITE_ROW) {
      if (!PetariPipelineCache::admitRecipe(validation, seedStmt)) { ++skippedRows; continue; }''')
    text=replace(text,'static bool write_pipeline_cache_record(const PipelineCacheWrite& write) {','''static bool write_pipeline_cache_record(const PipelineCacheWrite& write) {
  if (!PetariPipelineCache::recipeValid(int(write.type), write.hash, write.configVersion,
                                      write.config.data(), write.config.size())) return false;''')
    text=replace(text,'''  sqlite3_reset(g_pipelineCacheUpsertStmt);
  sqlite3_clear_bindings(g_pipelineCacheUpsertStmt);
  return true;''','''  sqlite3_reset(g_pipelineCacheUpsertStmt);
  sqlite3_clear_bindings(g_pipelineCacheUpsertStmt);
  // The caller owns the same transaction as the recipe upsert.
  return PetariPipelineCache::sealRecipe(g_pipelineCacheDb, int(write.type), write.hash,
      write.configVersion, write.config.data(), write.config.size());''')
    text=replace(text,'  seed_pipeline_cache_path(pipeline_cache_seed_path());\n  petari_seed_global();','''  if (!PetariPipelineCache::migrateRecipes(g_pipelineCacheDb, io::fs_path_from_string(g_config.cachePath),
      [&](int type, uint64_t hash, int version, const void* data, size_t size, uint32_t rank) {
        PipelineCacheWrite write{.type=static_cast<ShaderType>(type), .hash=hash,
            .configVersion=static_cast<uint32_t>(version), .config=ByteBuffer(size), .firstFrameUsed=rank};
        std::memcpy(write.config.data(), data, size);
        return write_pipeline_cache_record(write);
      })) {
    pipeline_cache_abort(); return false;
  }
  // Initial seeds never overwrite learned, compatible rows on a later update.
  if (PetariPipelineCache::emptyRecipes(g_pipelineCacheDb)) {
    seed_pipeline_cache_path(pipeline_cache_seed_path()); petari_seed_global();
  }''')
    return text


def patch_dawn(text):
    # Assert the pinned callback ABI before replacing its unsafe storage internals.
    for signature in ('size_t load_from_cache(', 'void store_to_cache(', 'void cache_prune()', 'void cache_shutdown()'):
        if text.count(signature)!=1: raise ValueError('Dawn callback ABI changed: '+signature)
    path=Path(__file__).resolve().with_name('aurora_dawn_cache.inc')
    return f'#include "{path}"\n'


def patch_device(text):
    if 'PetariPipelineCache::configureAdapter(' in text: raise ValueError('device compatibility hook already applied')
    here=Path(__file__).resolve().parent
    text=f'#include "{here / "pipeline_cache_device.hpp"}"\n'+text
    return replace(text,'    wgpu::DawnCacheDeviceDescriptor cacheDescriptor({.nextInChain = nullptr});', '''    PetariPipelineCache::configureAdapter(g_adapterInfo);
    wgpu::DawnCacheDeviceDescriptor cacheDescriptor({.nextInChain = nullptr});''')

if __name__=='__main__':
    mode,source,dest=sys.argv[1:]
    function={'dawn':patch_dawn,'pipeline':patch_pipeline,'device':patch_device}[mode]
    out=Path(dest);text=function(Path(source).read_text());out.parent.mkdir(parents=True,exist_ok=True)
    if not out.exists() or out.read_text()!=text:out.write_text(text)
