#!/usr/bin/env python3
"""Prepare two source-bound native VAT-contract fixtures; never executes a tool.

This extracts the actual GXInit VAT fragments, actual SETVAT/setter functions,
__GXSetVAT, canonical SDK header and production inline FIFO writer. It does not
replace the native parser or assert whole-GXInit/app acceptance.
"""
import argparse
import hashlib
import json
from pathlib import Path


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def function(text, signature):
    assert text.count(signature) == 1, signature
    begin = text.index(signature)
    start = text.index('{', begin)
    depth = 0
    for pos in range(start, len(text)):
        if text[pos] == '{':
            depth += 1
        elif text[pos] == '}':
            depth -= 1
            if depth == 0:
                return text[begin:pos + 1]
    raise AssertionError('unterminated source function')


PRELUDE = r'''
#include "__gx.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#undef CHECK
#define CHECK(condition, ...) do { if (!(condition)) std::abort(); } while (0)

// Canonical types/declarations come from the actual provider header.
static __GXData_struct sGXData;
__GXData_struct* __gx = &sGXData;
static std::array<unsigned char, 16384> fifoBytes{};
namespace aurora::gx::fifo {
namespace detail {
uint8_t* sBufferData = fifoBytes.data();
uint32_t sBufferSize = 0;
uint32_t sBufferCapacity = fifoBytes.size();
bool sInDisplayList = false;
uint8_t* sDlBuffer = nullptr;
uint32_t sDlSize = 0;
uint32_t sDlWritePos = 0;
}
void write_data_grow(const void*, uint32_t) { std::abort(); }
}
'''

CHECKS = r'''
static void require(bool good, const char* name) {
  if (!good) { std::printf("FIXTURE_UNEXPECTED_FAILURE %s\n", name); std::exit(1); }
}
static uint32_t be32(size_t pos) {
  return uint32_t(fifoBytes[pos]) << 24 | uint32_t(fifoBytes[pos + 1]) << 16 |
         uint32_t(fifoBytes[pos + 2]) << 8 | uint32_t(fifoBytes[pos + 3]);
}
static unsigned directWidth(unsigned count, unsigned type, bool position) {
  require(type == GX_F32 || type == GX_U8, "fixture covers F32 and U8 fields");
  return (type == GX_F32 ? 4u : 1u) * (count + (position ? 2u : 1u));
}
static unsigned inheritedWidth1() {
  const auto a = __gx->vatA[1], b = __gx->vatB[1];
  return directWidth(a & 1, (a >> 1) & 7, true) +
         directWidth((a >> 21) & 1, (a >> 22) & 7, false) +
         directWidth(b & 1, (b >> 1) & 7, false);
}
int main() {
  initContractFragment();
  // Independently pinned SDK table values, with original A30/B31 retained.
  constexpr uint32_t expectedA = 0x41377009, expectedB = 0xc8241209, expectedC = 0x04824120;
  bool allDefaults = true;
  for (unsigned bank = 0; bank < 8; ++bank) {
    allDefaults &= __gx->vatA[bank] == expectedA && __gx->vatB[bank] == expectedB && __gx->vatC[bank] == expectedC;
    require((__gx->vatA[bank] & (1u << 30)) != 0, "A30 retained");
    require((__gx->vatB[bank] & (1u << 31)) != 0, "B31 retained");
  }
  require(aurora::gx::fifo::detail::sBufferSize == 8 * 6, "initial eight VAT-B writes unchanged; setters only dirty shadow");
  for (unsigned bank = 0; bank < 8; ++bank) {
    require(fifoBytes[bank * 6] == 8 && fifoBytes[bank * 6 + 1] == (0x80 | bank), "original VAT-B order");
    require(be32(bank * 6 + 2) == 0x80000000u, "initial special B31 value unchanged");
  }
  if (allDefaults) {
    require(__gx->dirtyVAT == 0xff && (__gx->dirtyState & 0x10), "all8 banks dirty before flush");
    const auto begin = aurora::gx::fifo::detail::sBufferSize;
    __GXSetVAT();
    require(__gx->dirtyVAT == 0, "normal VAT flush clears dirty bank mask");
    require(aurora::gx::fifo::detail::sBufferSize == begin + 24 * 6, "exact24 CP writes");
    for (unsigned bank = 0; bank < 8; ++bank) {
      const uint32_t expected[] = {expectedA, expectedB, expectedC};
      for (unsigned word = 0; word < 3; ++word) {
        const auto pos = begin + (bank * 3 + word) * 6;
        require(fifoBytes[pos] == 8 && fifoBytes[pos + 1] == ((0x70 + word * 0x10) | bank), "normal CP A/B/C ordering");
        require(be32(pos + 2) == expected[word], "raw CP default words");
      }
    }
  }
  // Prior ordinary bank1 POS/TEX0 setup followed by the unchanged retail draw's
  // bank0 TEX1 setter. Bank1 TEX1 must remain inherited rather than be rewritten.
  const auto beforeBank1B = __gx->vatB[1];
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
  require(__gx->vatB[1] == beforeBank1B, "bank0 setter cannot repair or overwrite bank1 TEX1");
  const auto width = inheritedWidth1();
  if (!allDefaults) {
    require(width == 21 && ((__gx->vatB[1] & 0x1ffu) == 0), "original negative matches missing TEX1 state");
    std::printf("OLD_NEGATIVE_INIT_DEFAULTS_MISSING inheritedWidth1=%u expected=28\n", width);
    return 17;
  }
  require(width == 28 && 4 * width == 112, "unchanged four-vertex two-UV wire layout");
  // Source-bound reset fragment must restore every bank, not just bank1.
  std::memset(&sGXData, 0xa5, sizeof(sGXData));
  aurora::gx::fifo::detail::sBufferSize = 0;
  initContractFragment();
  for (unsigned bank = 0; bank < 8; ++bank)
    require(__gx->vatA[bank] == expectedA && __gx->vatB[bank] == expectedB && __gx->vatC[bank] == expectedC, "repeat init restores all8");
  std::puts("CANDIDATE_VAT_CONTRACT_PASS banks=8 CPwrites=24 inheritedWidth1=28");
}
'''


def prepare(aurora, baseline, candidate, output):
    assert not output.exists(), 'fresh output directory required'
    output.mkdir(mode=0o700)
    geometry = aurora / 'lib/dolphin/gx/GXGeometry.cpp'
    header = aurora / 'lib/dolphin/gx/__gx.h'
    g = geometry.read_text()
    shared = '\n\n'.join(function(g, signature) for signature in (
        'static inline void SETVAT(', 'void GXSetVtxAttrFmt(', 'void GXSetVtxAttrFmtv('))
    pins = {'generator_sha256': sha(__file__), 'geometry_sha256': sha(geometry),
            'canonical_header_sha256': sha(header), 'arms': {},
            'expected_raw_default_VAT': ['41377009', 'c8241209', '04824120'],
            'limits': 'Actual extracted VAT contract fragments/setters/flush and inline FIFO writer; not full GXInit, parser, threading or app acceptance',
            'execution': 'UNRUN; source preparation only'}
    for name, provider in [('old', baseline), ('candidate', candidate)]:
        t = provider.read_text()
        prefix = t[t.index('  std::memset(&sGXData, 0, sizeof(sGXData));'):t.index('  // Initialize FIFO subsystem')]
        vat = t[t.index('  __gx->dirtyState = 0;'):t.index('  // XF register init')]
        anchor = '  GXInvalidateVtxCache();\n'
        assert t.count(anchor) == 1
        defaults = t[t.index(anchor) + len(anchor):t.index('  GXSetLineWidth(6, GX_TO_ZERO);')]
        init = 'static void initContractFragment() {\n  u32 i;\n' + prefix + vat + defaults + '}\n'
        text = PRELUDE + shared + '\n' + function(t, 'void __GXSetVAT()') + '\n' + init + CHECKS
        path = output / (name + '.cpp')
        path.write_text(text)
        pins['arms'][name] = {'provider': str(provider), 'provider_sha256': sha(provider),
                              'fixture': str(path), 'fixture_sha256': sha(path),
                              'expected_exit': 17 if name == 'old' else 0}
    (output / 'FIXTURE-PINS.json').write_text(json.dumps(pins, indent=2) + '\n')
    return pins


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--aurora', type=Path, required=True)
    p.add_argument('--baseline-provider', type=Path, required=True)
    p.add_argument('--candidate-provider', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    print(json.dumps(prepare(a.aurora, a.baseline_provider, a.candidate_provider, a.output), indent=2))
