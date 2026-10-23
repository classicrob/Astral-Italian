#!/usr/bin/env python3
"""Compile actual tier selection/frame accounting with fake pipeline handles."""
from pathlib import Path
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
