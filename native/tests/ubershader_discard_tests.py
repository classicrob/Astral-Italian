#!/usr/bin/env python3
"""Compare fallback discard keys against Aurora's actual alpha-test emitter."""
from pathlib import Path
import subprocess
import sys
import tempfile


def function(text, signature):
    start = text.index(signature)
    end = text.index('{', start) + 1
    depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def main():
    aurora, includes = Path(sys.argv[1]), sys.argv[2].split(';')
    compiler = sys.argv[3] if len(sys.argv) > 3 else 'c++'
    native = (aurora / 'lib/gx/shader.cpp').read_text()
    runtime = (Path(__file__).resolve().parents[1] / 'gx/ubershader_runtime.inc').read_text()
    gx = (aurora / 'lib/gx/gx.hpp').read_text()
    helpers = '\n'.join(function(native, signature) for signature in (
        'AlphaCompareExpr alpha_compare_const(', 'AlphaCompareExpr alpha_compare_not(',
        'AlphaCompareExpr alpha_compare_and(', 'AlphaCompareExpr alpha_compare_or(',
        'AlphaCompareExpr alpha_compare_xor(', 'AlphaCompareExpr alpha_compare_xnor(',
        'AlphaCompareExpr alpha_compare(GXCompare'))
    alpha = function(gx, 'struct AlphaCompare {') + ';'
    emitter = function(native, '  if (config.alphaCompare) {')
    predicate = function(runtime, 'static bool petari_uber_needs_discard(')
    variant = function(runtime, 'uint32_t petari_uber_variant(const ShaderConfig& c,PetariUberTier tier)')
    source = r'''
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <array>
#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <cassert>
using namespace std::string_literals;
using namespace std::string_view_literals;
using u8=uint8_t;using u32=uint32_t;
enum GXCompare {GX_NEVER,GX_LESS,GX_EQUAL,GX_LEQUAL,GX_GREATER,GX_NEQUAL,GX_GEQUAL,GX_ALWAYS};
enum GXAlphaOp {GX_AOP_AND,GX_AOP_OR,GX_AOP_XOR,GX_AOP_XNOR};
enum {GX_FOG_NONE=0,GX_CH_RED=0,GX_CH_GREEN=1,GX_CH_BLUE=2,GX_CH_ALPHA=3};
#define DEFAULT_FATAL(...) default:std::abort();
struct AlphaCompareExpr {std::string expr;int constant=-1;};
''' + alpha + r'''
struct ShaderConfig {
 AlphaCompare alphaCompare{};unsigned tevStageCount=3,numIndStages=1,fogType=GX_FOG_NONE;
 struct Stage {unsigned tevSwapTex=0,tevSwapRas=0;};std::array<Stage,16> tevStages{};
 struct Swap {unsigned red=0,green=1,blue=2,alpha=3;};std::array<Swap,4> tevSwapTable{};
};
struct Info {std::bitset<8> sampledTexCoords{7};};
Info build_shader_info(const ShaderConfig&){return {};}
enum class PetariUberTier {Generic,Fast};
unsigned petari_uber_arg_program(const ShaderConfig&){return 0;}
''' + helpers + '\n' + predicate + '\n' + variant + r'''
bool native_emits_discard(const ShaderConfig& config){std::string fragmentFn;
''' + emitter + r'''
 return fragmentFn.find("discard;")!=std::string::npos;}
int main(){
 unsigned checked=0;const unsigned refs[]={0,1,127,254,255};
 for(unsigned a=0;a<8;++a)for(unsigned b=0;b<8;++b)for(unsigned ra:refs)for(unsigned rb:refs)for(unsigned op=0;op<4;++op){
  ShaderConfig c;c.alphaCompare.comp0=GXCompare(a);c.alphaCompare.comp1=GXCompare(b);
  c.alphaCompare.ref0=ra;c.alphaCompare.ref1=rb;c.alphaCompare.op=GXAlphaOp(op);
  const bool expected=native_emits_discard(c);
  assert(petari_uber_needs_discard(c)==expected);
  for(auto tier:{PetariUberTier::Generic,PetariUberTier::Fast})assert(bool(petari_uber_variant(c,tier)&(1u<<14))==!expected);
  if(!expected)assert(petari_uber_variant(c,PetariUberTier::Generic)==petari_uber_variant(c,PetariUberTier::Fast));
  c.numIndStages=0;
  for(auto tier:{PetariUberTier::Generic,PetariUberTier::Fast})assert(!(petari_uber_variant(c,tier)&(1u<<14)));
  ++checked;
 }
 ShaderConfig pass,conditional;conditional.alphaCompare.comp0=GX_GREATER;conditional.alphaCompare.ref0=127;
 assert(petari_uber_variant(pass,PetariUberTier::Generic)!=petari_uber_variant(conditional,PetariUberTier::Generic));
 assert(petari_uber_variant(pass,PetariUberTier::Fast)!=petari_uber_variant(conditional,PetariUberTier::Fast));
 std::printf("%u native alpha-emission/key cases passed in both tiers\n",checked);
}
'''
    with tempfile.TemporaryDirectory(prefix='petari-discard-') as temporary:
        cpp, exe = Path(temporary) / 'test.cpp', Path(temporary) / 'test'
        cpp.write_text(source)
        subprocess.run([compiler, '-std=c++20', '-O2', '-UNDEBUG', *['-I' + p for p in includes if p], str(cpp), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    main()
