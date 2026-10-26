#!/usr/bin/env python3
"""Exercise the actual pinned production patch chain without compilation."""
from pathlib import Path
import sys,importlib.util,tempfile,unittest
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'native/gx'))
import patch_aurora_pipeline as pipeline
import patch_aurora_cache_policy as policy

class PatchTests(unittest.TestCase):
    def test_pipeline(self):
        source=(AURORA/'lib/gfx/pipeline_cache.cpp').read_text();out=pipeline.patch(source)
        self.assertIn('admitRecipe(validation, g_pipelineCacheLoadStmt)',out)
        self.assertIn('admitRecipe(validation, seedStmt)',out)
        self.assertIn('admitRecipe(validation, query)',out)
        self.assertIn('migrateRecipes(g_pipelineCacheDb',out)
        self.assertIn('emptyRecipes(g_pipelineCacheDb)',out)
        self.assertIn('sealRecipe(g_pipelineCacheDb',out)
        self.assertNotIn('SELECT config, first_frame_used FROM pipeline_cache',out)
        self.assertEqual(out.count('#include "'+str(ROOT/'native/gx/pipeline_cache_migration.hpp')+'"'),1)
    def test_dawn_and_device(self):
        patched=policy.patch_dawn((AURORA/'lib/webgpu/gpu_cache.cpp').read_text())
        self.assertIn('aurora_dawn_cache.inc',patched)
        device=policy.patch_device((AURORA/'lib/webgpu/gpu.cpp').read_text())
        self.assertLess(device.index('configureAdapter(g_adapterInfo)'),device.index('wgpu::DawnCacheDeviceDescriptor cacheDescriptor'))
        with self.assertRaises(ValueError):policy.patch_device(device)
        with self.assertRaises(ValueError):policy.patch_dawn('unknown callback ABI')
    def test_no_index_only_or_destructive_prune(self):
        text=(ROOT/'native/gx/aurora_dawn_cache.inc').read_text()
        self.assertNotIn('XXH128',text);self.assertNotIn('DELETE',text)
        loader=(ROOT/'native/gx/ubershader_boot.inc').read_text()
        self.assertEqual(loader.count('PetariPipelineCache::admitRecipe(validation,'),3)
    def test_cmake_has_no_source_glob_in_identity(self):
        cmake=(ROOT/'native/gx/CMakeLists.txt').read_text()
        section=cmake[cmake.index('# Compatibility is an explicit ABI'):cmake.index('add_library(petari_pipeline_cache_identity')]
        for forbidden in ('GLOB','native/app/CMakeLists.txt','pipeline-seeds','pipeline_worker','boot_splash'):
            self.assertNotIn(forbidden,section)
        self.assertIn('file(SHA256 "${_petari_dawn_library}"',section)

if __name__=='__main__':
    AURORA=Path(sys.argv.pop(1));unittest.main()
