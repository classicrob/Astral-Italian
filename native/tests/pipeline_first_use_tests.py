#!/usr/bin/env python3
"""CPU-only exact recorder/normal-exit/crash-prefix tests; run on the build server."""
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time

root = Path(__file__).resolve().parents[2]
source = r'''
#include "pipeline_first_use.hpp"
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>
namespace aurora::gfx {
struct FakePacket { uint64_t frameId = 480; };
struct FakeRecorder {
  FakePacket packet;
  bool isActive = true;
  bool active() const { return isActive; }
  FakePacket& frame() { return packet; }
} g_recorder;
}
#include "pipeline_first_use_clock.inc"
int main(int argc, char** argv) {
  // begin_frame and GX pipeline lookup need not use the same native thread.
  uint64_t observed = 0;
  std::thread lookup([&] { observed = aurora::gfx::petari_pipeline_recording_frame_id(); });
  lookup.join();
  if (observed != 480) throw std::runtime_error("lookup thread lost the active packet clock");
  aurora::gfx::g_recorder.isActive = false;
  if (aurora::gfx::petari_pipeline_recording_frame_id() != UINT64_MAX) throw std::runtime_error("inactive packet is not a draw");
  if (argc > 1) setenv("PETARI_PIPELINE_RECORD_FIRST_USE", argv[1], 1);
  PetariFirstUse::begin("A");
  if (argc > 2 && std::strcmp(argv[2], "frozen") == 0) {
    PetariFirstUse::note(11, 0); PetariFirstUse::note(12, 0);
    PetariFirstUse::finish(); return 0;
  }
  PetariFirstUse::note(11, 480); PetariFirstUse::note(12, 480);
  PetariFirstUse::note(11, 481); PetariFirstUse::note(13, 481);
  if (argc > 2) { std::this_thread::sleep_for(std::chrono::seconds(30)); return 0; }
  PetariFirstUse::begin("A"); PetariFirstUse::note(13, 490); // repeated attribution is idempotent
  PetariFirstUse::begin("B"); PetariFirstUse::note(11, 500);
  PetariFirstUse::begin("A"); PetariFirstUse::note(11, 600);
  PetariFirstUse::finish(); PetariFirstUse::finish();
}
'''
with tempfile.TemporaryDirectory(prefix='pipeline-first-use-') as directory:
    tmp = Path(directory)
    cpp, binary = tmp / 'capture.cpp', tmp / 'capture'
    cpp.write_text(source)
    cmd = [sys.argv[1], '-std=c++20', '-O2', '-I', str(root / 'native/gx')]
    if sys.platform == 'darwin':
        cmd += ['-isysroot', subprocess.check_output(['xcrun', '--show-sdk-path'], text=True).strip()]
    subprocess.run(cmd + [str(cpp), '-o', str(binary)], check=True)
    env = {k: v for k, v in os.environ.items() if k != 'PETARI_PIPELINE_RECORD_FIRST_USE'}
    subprocess.run([str(binary)], check=True, env=env)
    path = tmp / 'normal.jsonl'
    subprocess.run([str(binary), str(path)], check=True, env=env)
    rows = [json.loads(line) for line in path.read_text().splitlines()]
    assert rows[0]['version'] == 2 and rows[0]['clock'] == 'active-frame-packet-id'
    assert [(r['stage'], r['visit'], r['relative_frame']) for r in rows[1:-1]] == [
        ('A', 1, 0), ('A', 1, 0), ('A', 1, 1), ('B', 2, 0), ('A', 3, 0)]
    assert rows[-1] == {'event': 'end', 'complete': True, 'records': 5, 'clock_first_frame': 480,
                        'clock_last_frame': 600, 'clock_transitions': 4, 'lookup_calls': 7}
    frozen = tmp / 'frozen.jsonl'
    subprocess.run([str(binary), str(frozen), 'frozen'], check=True, env=env)
    assert json.loads(frozen.read_text().splitlines()[-1])['complete'] is False
    before = path.read_bytes()
    subprocess.run([str(binary), str(path)], check=True, env=env)
    assert path.read_bytes() == before, 'capture must never overwrite an existing observation'
    killed = tmp / 'killed.jsonl'
    process = subprocess.Popen([str(binary), str(killed), 'wait'], env=env)
    try:
        until = time.monotonic() + 5
        while (not killed.exists() or len(killed.read_text().splitlines()) < 4) and time.monotonic() < until:
            time.sleep(0.01)
        process.send_signal(signal.SIGTERM)
        process.wait(timeout=5)
        assert process.returncode != 0
        assert all(json.loads(line)['event'] != 'end' for line in killed.read_text().splitlines())
    finally:
        if process.poll() is None:
            process.kill(); process.wait()
patch = (root / 'native/gx/patch_aurora_pipeline.py').read_text()
hook = patch[patch.index('PipelineRef find_pipeline(const gx::PipelineConfig& config, const RenderTargetLayout& layout) {\n  if (PetariFirstUse'):]
assert hook.index('PetariFirstUse::note') < hook.index('PetariFastPipeline::hit')
assert 'petari_pipeline_recording_frame_id()' in hook and 'gx::petari_uber_recording_frame()' not in hook
print('Exact frame groups, dedup, visits, normal footer, killed prefix, exclusive output, before-memo hook PASS')
