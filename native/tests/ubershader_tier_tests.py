#!/usr/bin/env python3
"""Compile actual tier selection/frame accounting with fake pipeline handles."""
from pathlib import Path
import ast
import os
import re
import subprocess
import tempfile
import unittest

GX = Path(__file__).resolve().parents[1] / 'gx'


def function(text, signature):
    start = text.index(signature)
    end = text.index('{', start) + 1
    depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


PREFIX = r'''
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <map>
#include <mutex>
namespace aurora::gfx {using PipelineRef=uint64_t;struct CompiledPipeline {bool main=false;};}
namespace aurora::gx {
struct PetariUberKeys {gfx::PipelineRef generic,fast;};
std::mutex uberMutex;
std::map<gfx::PipelineRef,gfx::CompiledPipeline> uberPipelines;
std::map<gfx::PipelineRef,PetariUberKeys> uberAssociations;
thread_local uint64_t uberEncodingFrame=0;
thread_local bool uberFrameUsed=false,uberFrameForced=false,uberFrameGeneric=false,uberFrameFast=false;
std::atomic<uint64_t> uberFrames{0},uberFallbackFrames{0},uberForcedFrames{0};
std::atomic<uint64_t> uberGenericFrames{0},uberFastFrames{0},uberMixedFrames{0},uberGenericDraws{0},uberFastDraws{0};
std::atomic<uint64_t> uberFallback{0},uberForced{0},uberSpecialized{0};
bool enabled=true;
bool petari_uber_enabled(){return enabled;}
'''
TESTS = r'''
}
int main(){using namespace aurora::gx;
  bool fast=true;
  assert(petari_uber_lookup(999,&fast)==0 && !fast);
  petari_uber_associate(1,{10,11});
  assert(petari_uber_lookup(1,&fast)==10 && !fast); // both pending
  uberPipelines[10].main=true;
  assert(petari_uber_lookup(1,&fast)==10 && !fast); // upgrade pending
  uberPipelines[11].main=true;
  assert(petari_uber_lookup(1,&fast)==11 && fast); // ready upgrade preferred
  uberPipelines[10].main=false;
  assert(petari_uber_lookup(1,&fast)==11 && fast); // upgrade alone ready
  uberPipelines[11].main=false;
  assert(petari_uber_lookup(1,&fast)==10 && !fast); // failed upgrade doesn't hide base
  petari_uber_associate(2,{20,20});uberPipelines[20].main=true;
  assert(petari_uber_lookup(2,&fast)==20 && !fast); // aliased keys count as generic

  petari_uber_begin_frame(42);
  petari_uber_draw(true,false,false,false);
  petari_uber_draw(true,false,false,true);
  petari_uber_end_frame();petari_uber_end_frame(); // no duplicate event
  assert(uberFallbackFrames==1 && uberGenericFrames==1 && uberFastFrames==1 && uberMixedFrames==1);
  petari_uber_begin_frame(43);
  petari_uber_draw(true,true,false,true);petari_uber_draw(true,true,false,true);
  petari_uber_end_frame();
  assert(uberForcedFrames==1 && uberFastFrames==2 && uberMixedFrames==1);
  petari_uber_begin_frame(44);petari_uber_draw(false,false,false,false);petari_uber_end_frame();
  assert(uberFallbackFrames==1 && uberForcedFrames==1);
  petari_uber_begin_frame(45);petari_uber_draw(true,false,false,false);petari_uber_end_frame();
  assert(uberFrames==4 && uberFallbackFrames==2 && uberForcedFrames==1);
  assert(uberGenericFrames==2 && uberFastFrames==2 && uberMixedFrames==1);
  assert(uberGenericDraws==2 && uberFastDraws==3 && uberFallback==3 && uberForced==2 && uberSpecialized==1);
  enabled=false;petari_uber_begin_frame(46);petari_uber_end_frame();assert(uberFrames==4);
}
'''


class Tiers(unittest.TestCase):
    def test_actual_record_readiness_memo(self):
        patch = ast.parse((GX / 'patch_aurora_ubershader.py').read_text())
        replacement = next(node.args[1].value for node in ast.walk(patch)
                           if isinstance(node, ast.Call) and isinstance(node.func, ast.Name)
                           and node.func.id == 'replace' and len(node.args) == 2
                           and isinstance(node.args[0], ast.Constant)
                           and node.args[0].value == 'bool get_pipeline(PipelineRef ref, CompiledPipeline& pipeline) {')
        source = '#include "' + str(GX / 'pipeline_fast_path.hpp') + '"\n' + r'''
#include <cassert>
#include <map>
#include <mutex>
namespace aurora::gfx {
using PipelineRef=uint64_t;struct CompiledPipeline {bool main=false;};
unsigned locks=0;
struct Mutex {void lock(){++locks;}void unlock(){}} g_pipelineMutex;
std::map<PipelineRef,CompiledPipeline> g_pipelines;
''' + function(replacement, 'bool petari_specialized_ready(') + r'''
}
int main(){using namespace aurora::gfx;
 g_pipelines[1].main=true;assert(petari_specialized_ready(1) && locks==1);
 assert(petari_specialized_ready(1) && locks==1); // no warm global lock
 g_pipelines[2].main=false;assert(!petari_specialized_ready(2) && locks==2);
 assert(!petari_specialized_ready(2) && locks==3); // failed/pending not memoized
 g_pipelines[2].main=true;assert(petari_specialized_ready(2) && locks==4);
 assert(petari_specialized_ready(2) && locks==4);
 assert(!petari_specialized_ready(3) && locks==5); // unknown key
 g_pipelines[2049].main=true;assert(petari_specialized_ready(2049) && locks==6);
 assert(petari_specialized_ready(1) && locks==7); // direct-map collision is a miss
 assert(petari_specialized_ready(1) && locks==7);
 PetariFastPipeline::invalidate();g_pipelines.clear();
 assert(!petari_specialized_ready(1) && locks==8); // stage/device/cache reset
 g_pipelines[1].main=true;assert(petari_specialized_ready(1) && locks==9);
 assert(petari_specialized_ready(1) && locks==9);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            cpp, exe = Path(tmp) / 'memo.cpp', Path(tmp) / 'memo'
            cpp.write_text(source)
            subprocess.run(['clang++', '-std=c++20', '-pthread', str(cpp), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

    def test_actual_warm_path_decisions(self):
        runtime = (GX / 'ubershader_runtime.inc').read_text()
        patch = ast.parse((GX / 'patch_aurora_ubershader.py').read_text())
        replacements = {node.args[0].value: node.args[1].value for node in ast.walk(patch)
                        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name)
                        and node.func.id == 'replace' and len(node.args) == 2
                        and all(isinstance(a, ast.Constant) and isinstance(a.value, str) for a in node.args)}
        # Execute the actual generated recording code, not a duplicate decision.
        record = replacements['    cache.uniformRange = build_uniform(cache.shaderInfo);']
        reset = replacements['    cache.pipelineRef = gfx::pipeline_ref(cache.config);']
        source = PREFIX + r'''
struct ShaderConfig {};struct PipelineConfig {ShaderConfig shaderConfig;};
struct DrawData {gfx::PipelineRef pipeline=1;uint32_t uberUniformOffset=256;};
bool all=false,specialized=false,fallback=false,fastVariant=false;
unsigned lookups=0,readyCalls=0,probes=0,nativeCalls=0,missing=0,skipped=0,hashes=0,reasons=0;
unsigned nativeUniforms=0,uberUniforms=0;
bool supported=true;
std::atomic<uint64_t> uberDraws{0},uberCovered{0};
using HashType=uint64_t;std::map<HashType,bool> uberConfigs;
bool petari_uber_all(){return all;}
const char* petari_uber_reason(const ShaderConfig&){++reasons;return supported?"covered":"uncovered";}
HashType xxh3_hash(const ShaderConfig&){++hashes;return 42;}
gfx::PipelineRef petari_uber_lookup(gfx::PipelineRef,bool* fast){++lookups;*fast=fastVariant;return 10;}
bool petari_uber_ready(gfx::PipelineRef,gfx::CompiledPipeline& p){++readyCalls;p.main=fallback;return fallback;}
void petari_uber_not_ready(gfx::PipelineRef,bool skip=false){if(skip)++skipped;else ++missing;}
}
namespace aurora::gfx {
bool get_pipeline(PipelineRef,CompiledPipeline& p){++gx::nativeCalls;p.main=gx::specialized;return p.main;}
bool petari_specialized_ready(PipelineRef){++gx::probes;return gx::specialized;}
PipelineRef pipeline_ref(const gx::PipelineConfig&){return 1;}
struct Range {unsigned offset=0,size=0;};
}
namespace aurora::gx {
struct Cache {PipelineConfig config;int shaderInfo=0;gfx::Range uniformRange,uberUniformRange;
 gfx::PipelineRef pipelineRef=1;bool specializedReady=false;} cache;
gfx::Range build_uniform(int){++nativeUniforms;return {0,1024};}
gfx::Range petari_uber_uniform(const ShaderConfig&){++uberUniforms;return {256,4768};}
''' + '\n'.join(function(runtime, sig) for sig in (
            'bool petari_uber_diagnostics(', 'bool petari_uber_needs_uniform(',
            'void petari_uber_begin_frame(', 'void petari_uber_end_frame(', 'void petari_uber_draw(',
            'bool petari_uber_select_pipeline(', 'void petari_uber_note(')) + '\nvoid record(){\n' + record + r'''
}
void reset_pipeline(){
''' + reset + r'''
}
}
int main(int argc,char** argv){using namespace aurora::gx;
  // A ready specialised pipeline needs no fallback lock/lookup/copy or uniform.
  specialized=true;record();assert(nativeUniforms==1 && uberUniforms==0 && probes==1);
  record();assert(probes==1 && uberUniforms==0);
  aurora::gfx::CompiledPipeline p;bool uber=true;DrawData d;
  petari_uber_begin_frame(1);assert(petari_uber_select_pipeline(d,p,uber) && !uber);
  petari_uber_end_frame();assert(lookups==0 && readyCalls==0 && uberFallbackFrames==0);
  // A new pending pipeline must carry its full fallback uniform and draw fallback.
  reset_pipeline();assert(!cache.specializedReady);specialized=false;record();
  assert(uberUniforms==1 && cache.uberUniformRange.size==4768);
  fallback=true;petari_uber_begin_frame(2);
  assert(petari_uber_select_pipeline(d,p,uber) && uber);petari_uber_end_frame();
  assert(lookups==1 && readyCalls==1 && uberFallbackFrames==1 && uberGenericFrames==1);
  // Becoming ready suppresses future packing and lookup, even with a stale uber range.
  specialized=true;record();auto oldLookups=lookups;
  assert(uberUniforms==1 && cache.uberUniformRange.size==0);
  petari_uber_begin_frame(3);assert(petari_uber_select_pipeline(d,p,uber) && !uber);
  petari_uber_end_frame();assert(lookups==oldLookups && uberFallbackFrames==1);
  // All mode bypasses native readiness, always packs, and reports forced/tier flags.
  all=true;auto oldProbes=probes;auto oldNative=nativeCalls;record();
  assert(uberUniforms==2 && probes==oldProbes);fastVariant=true;
  petari_uber_begin_frame(4);assert(petari_uber_select_pipeline(d,p,uber) && uber);
  petari_uber_end_frame();assert(nativeCalls==oldNative && uberForcedFrames==1 && uberFastFrames==1);
  // Missing fallback preserves today's native-ready or skip behavior in all mode.
  fallback=false;petari_uber_begin_frame(5);
  assert(petari_uber_select_pipeline(d,p,uber) && !uber);petari_uber_end_frame();
  assert(missing==1 && skipped==0 && uberForcedFrames==1);
  specialized=false;assert(!petari_uber_select_pipeline(d,p,uber));assert(missing==2 && skipped==1);
  all=false;fallback=true;d.uberUniformOffset=UINT32_MAX;oldLookups=lookups;
  assert(!petari_uber_select_pipeline(d,p,uber));assert(lookups==oldLookups);
  // Flag-off does not query or pack any fallback, and unsupported draws do not pack.
  enabled=false;oldProbes=probes;record();assert(probes==oldProbes && uberUniforms==2);
  enabled=true;reset_pipeline();supported=false;record();assert(uberUniforms==2);
  // Actual diagnostic gate: no config hash/coverage work unless explicitly enabled.
  hashes=0;reasons=0;supported=true;petari_uber_note({});
  bool diag=argc>1 && std::string(argv[1])=="on";
  assert(hashes==unsigned(diag) && reasons==unsigned(diag) && uberDraws==unsigned(diag));
  enabled=false;petari_uber_note({});assert(hashes==unsigned(diag));
}
'''
        source = '#include <cstdlib>\n#include <cstring>\n#include <string>\n' + source
        with tempfile.TemporaryDirectory() as tmp:
            cpp, exe = Path(tmp) / 'warm.cpp', Path(tmp) / 'warm'
            cpp.write_text(source)
            subprocess.run(['clang++', '-std=c++20', '-pthread', str(cpp), '-o', str(exe)], check=True)
            base = {k: v for k, v in os.environ.items()
                    if k not in ('PETARI_UBERSHADER_DIAG', 'PETARI_PIPELINE_DIAG')}
            for env, diag in (({}, 'off'), ({'PETARI_UBERSHADER_DIAG': '1'}, 'on'),
                              ({'PETARI_PIPELINE_DIAG': '1'}, 'on'),
                              ({'PETARI_PIPELINE_DIAG': '0', 'PETARI_UBERSHADER_DIAG': '0'}, 'off')):
                subprocess.run([str(exe), diag], env={**base, **env}, capture_output=True, text=True, check=True)

    def test_actual_selection_and_packet_accounting(self):
        runtime = (GX / 'ubershader_runtime.inc').read_text()
        source = PREFIX + '\n'.join(function(runtime, sig) for sig in (
            'void petari_uber_associate(', 'gfx::PipelineRef petari_uber_lookup(',
            'void petari_uber_begin_frame(', 'void petari_uber_end_frame(', 'void petari_uber_draw(')) + TESTS
        with tempfile.TemporaryDirectory() as tmp:
            cpp = Path(tmp) / 'tier.cpp'
            exe = Path(tmp) / 'tier'
            cpp.write_text(source)
            subprocess.run(['clang++', '-std=c++20', '-pthread', str(cpp), '-o', str(exe)], check=True)
            result = subprocess.run([str(exe)], capture_output=True, text=True, check=True)
        events = [dict(re.findall(r'(\w+)=(\d+)', line)) for line in result.stderr.splitlines()]
        self.assertEqual(events, [
            dict(frame='42', fallback='1', forced='0', generic='1', fast='1'),
            dict(frame='43', fallback='1', forced='1', generic='0', fast='1'),
            dict(frame='45', fallback='1', forced='0', generic='1', fast='0')])


if __name__ == '__main__':
    unittest.main()
