#!/usr/bin/env python3
"""Run production emitter conversions and follow-flag methods under ASan/UBSan.

Both complete emitter TUs are compiled. The callback harness links real matrix,
trig, AutoEffectInfo, and JPA callback providers. Its sole test double is an inert
MultiEmitter resource-owner constructor; resource allocation is not under test.
Every input flag byte is explicitly supplied: this does not prove a value for the
uninitialized FlagSRT in followSRT's null-AutoEffectInfo path.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'native/tools'))
from encode_legacy_sources import prepare

PREAMBLE = r'''
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <limits>
static unsigned checks=0;
static void check(bool ok, const char* why) {
    ++checks;
    if (!ok) { std::fprintf(stderr,"FAIL %s\n",why); std::exit(1); }
}
// Independent numeric oracle: truncate in a wider domain, then interpret low16.
static short phaseExpected(float deg) {
    const float scaled=182.04445f*deg;
    long long word;
    if (std::isnan(scaled) || scaled < -2147483648.0f) word=-2147483648LL;
    else if (scaled >= 2147483648.0f) word=2147483647LL;
    else word=static_cast<long long>(std::trunc(static_cast<double>(scaled)));
    const unsigned bits=static_cast<unsigned long long>(word)&65535u;
    return static_cast<short>(bits<32768u ? static_cast<int>(bits) : static_cast<int>(bits)-65536);
}
static void conversions() {
    const float edge[]={0.0f,-0.0f,1.0f,-1.0f,179.999f,180.0f,181.0f,360.0f,-360.0f,
        1e7f,-1e7f,11796480.0f,-11796480.0f,std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max(),std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::denorm_min()};
    for(float f:edge) check(toS16(f)==phaseExpected(f),"production phase edge conversion");
    for(int n=-65536;n<=65536;++n) {
        const float f=static_cast<float>(n)/182.04445f;
        check(toS16(f)==phaseExpected(f),"phase boundaries and low16 wrap");
    }
    uint32_t rng=0x52525252u;
    for(unsigned i=0;i<8192;++i) {
        rng=rng*1664525u+1013904223u;
        float f; std::memcpy(&f,&rng,4);
        check(toS16(f)==phaseExpected(f),"sampled float bit patterns");
    }
}
'''

CALLBACK = r'''
#include "Game/Effect/SingleEmitter.hpp"
// Test-only resource-owner fixture. No method under test is replaced.
MultiEmitter::MultiEmitter(const char*) : mAutoEffectInfo(nullptr) {}
static void flagsTest() {
    static_assert(sizeof(MultiEmitterCallBack::FlagSRT)==3,"three byte flags");
    MultiEmitter owner("fixture");
    MultiEmitterCallBack cb(&owner,TVec3f(0,0,0));
    TVec3f host(1,2,3); Mtx mtx={};
    cb.mMtx=mtx;
    // All 16-bit callback combinations, including unnamed/unknown bits, with
    // defined incoming noncanonical bytes. No masking to a named enum domain.
    for(unsigned bits=0;bits<65536;++bits) {
        cb.mFlags=static_cast<u16>(bits);
        MultiEmitterCallBack::FlagSRT f;
        const unsigned char initial[3]={2,255,0};
        std::memcpy(&f,initial,3);
        cb.isFollowSRT(&f,false);
        const unsigned char expected[3]={
            static_cast<unsigned char>((bits&0x42)?0:2),
            static_cast<unsigned char>((bits&0x42)?0:255),
            static_cast<unsigned char>((bits&0x42)?0:!!(bits&0x10))};
        check(std::memcmp(&f,expected,3)==0,"raw byte preservation and force-off precedence");
        check(f.isTransform()==((bits&0x42)==0),"nonzero raw-byte transform decision");
    }
    cb.mFlags=0;
    for(unsigned byte=0;byte<256;++byte) {
        MultiEmitterCallBack::FlagSRT f;
        const unsigned char initial[3]={static_cast<unsigned char>(byte),static_cast<unsigned char>(byte),static_cast<unsigned char>(byte)};
        std::memcpy(&f,initial,3);
        cb.isFollowSRT(&f,true);
        check(std::memcmp(&f,initial,3)==0,"every incoming byte retains LBZ semantics");
        check(f.isTransform()==(byte!=0),"all256 nonzero truth values");
    }
    // Host gates kill only their corresponding byte; live host fields retain it.
    for(unsigned hosts=0;hosts<16;++hosts) {
        cb.mMtx=(hosts&8)?mtx:nullptr;
        cb.mScale=(hosts&1)?&host:nullptr;
        cb.mRotation=(hosts&2)?&host:nullptr;
        cb.mTranslation=(hosts&4)?&host:nullptr;
        MultiEmitterCallBack::FlagSRT f;
        const unsigned char initial[3]={128,2,255}; std::memcpy(&f,initial,3);
        cb.isFollowSRT(&f,true);
        const unsigned char expected[3]={
            static_cast<unsigned char>((hosts&9)?128:0),
            static_cast<unsigned char>((hosts&10)?2:0),
            static_cast<unsigned char>((hosts&12)?255:0)};
        check(std::memcmp(&f,expected,3)==0,"independent host pointer gates");
    }
    AutoEffectInfo info;
    owner.mAutoEffectInfo=&info;
    cb.mMtx=mtx;
    cb.mFlags=0x8000; // Unknown callback bits do not force anything.
    for(unsigned bits=0;bits<65536;++bits) for(bool affect:{false,true}) {
        info.mFlag=static_cast<u16>(bits);
        MultiEmitterCallBack::FlagSRT f;
        const unsigned char initial[3]={255,255,255};std::memcpy(&f,initial,3);
        cb.isFollowSRT(&f,affect);
        const unsigned shift=affect?3:0;
        const unsigned char expected[3]={static_cast<unsigned char>((bits>>shift)&1),
            static_cast<unsigned char>((bits>>(shift+1))&1),static_cast<unsigned char>((bits>>(shift+2))&1)};
        check(std::memcmp(&f,expected,3)==0,"all AutoEffectInfo bits follow/affect mapping");
    }
    // Full caller's null-host path overwrites every byte before observing it.
    owner.mAutoEffectInfo=nullptr;
    cb.mMtx=nullptr;cb.mScale=cb.mRotation=cb.mTranslation=nullptr;cb.mFlags=0;
    cb.followSRT(nullptr,true);cb.followSRT(nullptr,false);
    check(true,"full followSRT null-host path");
}
'''


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',default='clang++')
    parser.add_argument('--overlay',type=Path,help='Temporary canonical dependency include root, before main lands')
    parser.add_argument('--output',type=Path)
    args=parser.parse_args()
    report={'production_units':[], 'negative_controls':[], 'scope':'defined input bytes; no inferred uninitialized stack value'}
    with tempfile.TemporaryDirectory(prefix='petari-emitter-native-') as tmp:
        temp=Path(tmp);legacy=temp/'legacy';prepare(ROOT,legacy)
        common=[args.compiler,'-std=c++17','-DGALAXY_NATIVE=1','-O2','-fno-rtti','-ffp-contract=off',
                '-fno-strict-aliasing','-Wno-register','-Wno-inconsistent-missing-override',
                '-fsanitize=address,undefined,float-cast-overflow','-fno-sanitize-recover=all',
                '-ffunction-sections','-fdata-sections','-Wl,-dead_strip']
        includes=[]
        if args.overlay: includes+=['-I',str(args.overlay.resolve())]
        for directory in ['native/include','native/home_menu/include','include','libs/JSystem/include',
                          'libs/RVL_SDK/include','libs/nw4r/include','libs/RVLFaceLib/include','libs/libms/include']:
            if not directory.startswith('native/'): includes+=['-I',str(legacy/directory)]
            includes+=['-I',str(ROOT/directory)]
        dependencies=['src/JSystem/JParticle/JPAEmitter.cpp','src/JSystem/JParticle/JPAMath.cpp',
                      'src/Game/Util/MathUtil.cpp','src/Game/Effect/EffectSystemUtil.cpp',
                      'src/JSystem/JMath/JMATrigonometric.cpp','src/Game/Effect/AutoEffectInfo.cpp']
        deps=[str(legacy/p) for p in dependencies]+[str(ROOT/'native/src/mtx.cpp')]
        def run(unit,callback=False,mutation=None):
            source=legacy/f'src/Game/Effect/{unit}.cpp'
            extra=[]
            if mutation=='conversion':
                text=source.read_text();old='return JMath::truncToS16(DEGREE_TO_S16 * deg);'
                assert text.count(old)==1
                source=temp/'conversion-mutant.cpp';source.write_text(text.replace(old,'return DEGREE_TO_S16 * deg;'))
            if mutation=='bool':
                header=legacy/'include/Game/Effect/MultiEmitterCallBack.hpp'
                text=header.read_text()
                for field in ['mScale','mRotation','mTranslation']:
                    assert f'u8 {field};' in text
                    text=text.replace(f'u8 {field};',f'bool {field};')
                mutant=temp/'bool-include/Game/Effect/MultiEmitterCallBack.hpp';mutant.parent.mkdir(parents=True,exist_ok=True);mutant.write_text(text)
                extra=['-I',str(temp/'bool-include')]
            cpp=temp/f'{unit}-{mutation or "current"}.cpp'
            cpp.write_text('#include <initializer_list>\n#include '+json.dumps(str(source))+'\n'+PREAMBLE+
                           (CALLBACK if callback else '')+'\nint main(){conversions();'+('flagsTest();' if callback else '')+
                           'std::printf("PASS %u production checks\\n",checks);}\n')
            binary=cpp.with_suffix('')
            cmd=common+extra+includes+[str(cpp)]+(deps if callback else [])+['-o',str(binary)]
            build=subprocess.run(cmd,text=True,capture_output=True)
            if build.returncode: raise RuntimeError(build.stdout+build.stderr)
            result=subprocess.run([str(binary)],text=True,capture_output=True,timeout=45)
            if mutation:
                diagnostic='bool' if mutation=='bool' else 'short'
                if result.returncode==0 or 'runtime error:' not in result.stderr or diagnostic not in result.stderr:
                    raise AssertionError(f'{mutation} negative control did not fail for intended sanitizer check: '+result.stderr)
                report['negative_controls'].append({'mutation':mutation,'rejected':True,'exit':result.returncode})
                print(f'PASS negative control: original {mutation} operation rejected')
            else:
                if result.returncode: raise RuntimeError(result.stdout+result.stderr)
                print(unit+': '+result.stdout.strip())
                report['production_units'].append({'unit':unit,'passed':True,'result':result.stdout.strip(),
                    'source_sha256':hashlib.sha256((ROOT/f'src/Game/Effect/{unit}.cpp').read_bytes()).hexdigest()})
        run('MultiEmitter')
        run('MultiEmitterCallBack',True)
        run('MultiEmitter',mutation='conversion')
        run('MultiEmitterCallBack',True,mutation='bool')
        report['compiler']=subprocess.check_output([args.compiler,'--version'],text=True).splitlines()[0]
        report['command_prefix']=common
        report['dependency_sources']=dependencies+['native/src/mtx.cpp']
        helper=(args.overlay if args.overlay else ROOT/'libs/JSystem/include')/'JSystem/JMath/JMAFloatConversion.hpp'
        report['canonical_header_sha256']=hashlib.sha256(helper.read_bytes()).hexdigest()
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n')
    return 0


if __name__=='__main__': raise SystemExit(main())
