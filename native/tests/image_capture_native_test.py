#!/usr/bin/env python3
"""Actual capture adapter versus signed-word DOL operations, with sanitizer negative control."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HARNESS = r'''
#include <petari/ppc_integer.hpp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>
using s32=std::int32_t; using u8=std::uint8_t;
enum GXTexFmt { TestFormat=4 };
static s32 width, height;
static std::vector<int> events;
namespace MR {
s32 getFrameBufferHeight() {events.push_back(1); return height;}
s32 getFrameBufferWidth() {events.push_back(2); return width;}
}
struct JUTTexture {
    s32 x=0,y=0; GXTexFmt format=TestFormat; bool clear=false; u8 filter=0;
    void capture(s32 px,s32 py,GXTexFmt pf,bool pc,u8 pi) {
        events.push_back(3); x=px; y=py; format=pf; clear=pc; filter=pi;
    }
};
namespace ImageEffectLocalUtil {
void capture(JUTTexture*, s32, s32, GXTexFmt, bool, u8);
}
BODY
static int checks;
static void check(bool ok) {++checks; if (!ok) std::abort();}
static s32 word(std::uint64_t bits) {
    const std::int64_t low=bits & 0xffffffffULL;
    return s32(low >= 0x80000000LL ? low-0x100000000LL : low);
}
static s32 quotient(s32 a,s32 b) {
    if (b==0) return a<0 ? -1 : 0;
    const std::int64_t q=std::int64_t(a)/b;
    if (q>std::numeric_limits<s32>::max()) return -1;
    return s32(q);
}
static void test(s32 tile,s32 divisions,s32 w,s32 h) {
    width=w; height=h; events.clear(); JUTTexture texture;
    ImageEffectLocalUtil::capture(&texture,divisions,tile,TestFormat,true,7);
    const auto row=quotient(tile,divisions);
    const auto y=quotient(word(std::uint64_t(std::int64_t(row)*h)),divisions);
    const auto remainder=word(std::uint64_t(std::int64_t(tile)-std::int64_t(row)*divisions));
    const auto x=quotient(word(std::uint64_t(std::int64_t(remainder)*w)),divisions);
    check(texture.x==x && texture.y==y);
    check(texture.format==TestFormat && texture.clear && texture.filter==7);
    check(events==std::vector<int>({1,2,3}));
}
int main() {
    test(0,0,640,480);  // U025 negative control reaches zero division immediately
    const s32 min=std::numeric_limits<s32>::min(),max=std::numeric_limits<s32>::max();
    const s32 values[]={min,min+1,-1000,-9,-1,0,1,2,9,1000,max};
    const s32 divisors[]={min,-64,-4,-1,0,1,2,4,64,max};
    for (s32 tile:values) for(s32 divisions:divisors) {
        test(tile,divisions,640,480); test(tile,divisions,0,0);
        test(tile,divisions,-640,480); test(tile,divisions,max,min);
    }
    using namespace PetariNative::PPC;
    check(divideSignedWord(-1,0)==-1 && divideSignedWord(0,0)==0 && divideSignedWord(1,0)==0);
    check(divideSignedWord(min,-1)==-1);
    std::uint32_t rng=0x12345678;
    for(int i=0;i<10000;++i) {
        rng=rng*1664525u+1013904223u; const s32 a=word(rng);
        rng=rng*1664525u+1013904223u; const s32 b=word(rng);
        check(divideSignedWord(a,b)==quotient(a,b));
        check(multiplyWord(a,b)==word(std::uint64_t(std::int64_t(a)*b)));
        check(subtractWord(a,b)==word(std::uint64_t(std::int64_t(a)-b)));
    }
    std::printf("%d PPC-word/capture-adapter checks passed\n",checks);
}
'''


def main():
    source = ROOT / "src/Game/Screen/ImageEffectLocalUtil.cpp"
    text = source.read_text()
    start = text.index("void ImageEffectLocalUtil::capture(")
    end = text.index("\n}", start) + len("\n}")
    results = []
    with tempfile.TemporaryDirectory(prefix="smg2-capture-ubsan-") as temp:
        temp = Path(temp)
        cpp = temp / "test.cpp"
        cpp.write_text(HARNESS.replace("BODY", text[start:end]))
        for native in (True, False):
            exe = temp / ("native" if native else "legacy-negative")
            cmd = ["clang++", "-std=c++17", "-O1", "-fsanitize=address,undefined",
                   "-fno-sanitize-recover=all", "-fno-omit-frame-pointer",
                   "-I", str(ROOT / "native/include"), str(cpp), "-o", str(exe)]
            if native:
                cmd.insert(1,"-DGALAXY_NATIVE=1")
            subprocess.run(cmd,check=True,capture_output=True,text=True)
            result = subprocess.run([str(exe)],capture_output=True,text=True)
            if native:
                assert result.returncode==0,result.stderr
            else:
                assert result.returncode!=0 and "division by zero" in result.stderr,result.stderr
            results.append({"native":native,"exit":result.returncode,
                            "stdout":result.stdout,"stderr":result.stderr})
    print(json.dumps({"source_sha256":hashlib.sha256(source.read_bytes()).hexdigest(),
                      "helper_sha256":hashlib.sha256((ROOT/"native/include/petari/ppc_integer.hpp").read_bytes()).hexdigest(),
                      "results":results},indent=2))


if __name__=="__main__":
    main()
