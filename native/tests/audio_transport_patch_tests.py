#!/usr/bin/env python3
"""Pinned SDL operation-token guard; no compiler, devices, cache, or subprocess.

The hashes cover every token of the three upstream release-3.4.10 audio TUs.
Only the named diagnostic extension/count block/hooks and the equivalent
single-return temporary are erased. Release codegen remains a separate gate.
"""
import hashlib
from pathlib import Path
import re
import sys
import unittest
SDL=Path(sys.argv.pop(1))
ROOT=Path(__file__).resolve().parents[2]
EXPECTED={'src/audio/SDL_audio.c': '48c389e1c9ef7565685a67dace5fb341951906e82864628216083c7e9472a7bd', 'src/audio/SDL_audiocvt.c': 'a31672ca85e0caffbf088fc1d40751b0b865b8c7c76b06527b20c89234058dce', 'src/audio/coreaudio/SDL_coreaudio.m': 'd8c43829b3d1b6d0ee429f3b564042b4548e288f781c4e6366db2f914287f5fc'}
TOKEN=re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|\w+|[^\s]')
def token_hash(s):
    tokens=[t for t in TOKEN.findall(s) if not t.startswith(('//','/*'))]
    return hashlib.sha256('\0'.join(tokens).encode()).hexdigest()
def erase(s):
    s=re.sub(r'#include "(?:\.\./)?SDL_petari_audio_diag.h"\n','',s)
    s=re.sub(r'SDL_PetariAudioObserver SDL_petari_audio_observer = NULL;[\s\S]*?(?=#include "SDL_sysaudio.h")','',s)
    s=re.sub(r'\s*SDL_PetariAudioBoundary\([^;]*\);','',s)
    s=re.sub(r'    // Diagnostic only: default calculation above remains unchanged\.[\s\S]*?(?=    device->hidden->numAudioBuffers =)', '',s)
    s=re.sub(r'const bool ret = PutAudioStreamBuffer\(stream, buf, len, NULL, NULL\);\s*return ret;',
             'return PutAudioStreamBuffer(stream, buf, len, NULL, NULL);',s)
    return s
class Tests(unittest.TestCase):
    def test_off_preserves_all_pinned_audio_operations(self):
        for rel,wanted in EXPECTED.items():
            with self.subTest(rel=rel): self.assertEqual(token_hash(erase((SDL/rel).read_text())),wanted)
    def test_null_cached_observer_precedes_any_hook_work(self):
        private=re.sub(r'/\*[\s\S]*?\*/','',(SDL/'src/audio/SDL_petari_audio_diag.h').read_text())
        self.assertEqual(private.count('__atomic_load_n('),1)
        self.assertIn('if (observer)',private)
        for forbidden in ('clock','thread_local','SDL_Lock','pthread','malloc','fprintf'):
            self.assertNotIn(forbidden,private)
        audio=(SDL/'src/audio/SDL_audio.c').read_text()
        self.assertIn('SDL_petari_audio_observer = NULL;',audio)
        self.assertIn('(buffers && !observer)',audio)
        self.assertIn('buffers != 0 && buffers != 6 && buffers != 18',audio)
    def test_count_only_and_format_bound_fail_closed(self):
        s=(SDL/'src/audio/coreaudio/SDL_coreaudio.m').read_text()
        block=s[s.index('// Diagnostic only:'):s.index('device->hidden->numAudioBuffers =')]
        self.assertIn('!device->recording && diagnosticBuffers',block)
        self.assertIn('device->sample_frames != 256 || device->spec.freq != 44100',block)
        self.assertIn('return SDL_SetError(',block)
        self.assertEqual(block.count('numAudioBuffers = diagnosticBuffers;'),1)
        self.assertNotIn('device->sample_frames =',block)
    def test_producer_has_no_sdl_put_and_fill_balances_exit(self):
        s=(ROOT/'native/audio/sdl_sink.cpp').read_text()
        producer=s[s.index('void produce('):s.index('void SDLCALL fill(')]
        self.assertNotIn('SDL_Put',producer)
        self.assertNotIn('noteTransport',producer)
        self.assertIn('~FillBoundary() { if(active) WG::noteTransport(SDL_PETARI_FILL_EXIT',s)
        self.assertIn('boundary{WG::observing.load',s)
        self.assertIn('WG::noteTransport(SDL_PETARI_FILL_ENTRY',s)
        self.assertIn('PETARI_AUDIO_AQ_BUFFERS_DIAG',s)
        self.assertIn('!diagnostics || !WG::observing.load',s)
if __name__=='__main__': unittest.main()
