#!/usr/bin/env python3
"""Sanitize the full CShader TU, its actual packed-read expressions and length-map methods.

The display-list read statements are extracted verbatim into tiny accessors to
exercise every alignment/byte order without constructing a renderer/model graph.
The full production TU is included; getCount and CLengthMap run directly. No game
ABI declarations, provider stubs, SDK changes or renderer execution are involved.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'native/tools'))
from encode_legacy_sources import prepare

HARNESS=r'''
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <limits>
static unsigned checks=0;
static void check(bool ok,const char* why) {
    ++checks;
    if(!ok) {std::fprintf(stderr,"FAIL %s\n",why);std::exit(1);}
}
static void put16(u8* p,unsigned value) {p[0]=value>>8;p[1]=value;}
static void counts() {
    alignas(16) u8 bytes[16]={};
    for(unsigned offset=0;offset<4;++offset) for(unsigned n=0;n<65536;++n) {
        u8* p=bytes+offset;p[0]=0x98;put16(p+1,n);
        check(getCount(p)==static_cast<s32>(n),"actual getCount BE unsigned16");
        check(borderCount(p)==static_cast<s32>(n),"actual border count BE unsigned16");
    }
}
static void vertices() {
    alignas(16) u8 bytes[128]={};
    for(unsigned start=0;start<4;++start) for(int stride:{6,7,9}) for(int i=0;i<3;++i) {
        u8* p=bytes+start;
        const unsigned expected[3]={static_cast<unsigned>(0x123+i),static_cast<unsigned>(0x4567+i),static_cast<unsigned>(0x89ab+i)};
        for(unsigned k=0;k<3;++k)put16(p+3+stride*i+2*k,expected[k]);
        const auto got=vertex(p,stride,i,0,2,4);
        check(got.pos==expected[0]&&got.nrm==expected[1]&&got.tex==expected[2],"actual makeIndexData vertex read statements");
        check(borderVertex(p,stride,i,0)==expected[0],"actual border position read statement");
    }
}
static void maps() {
    static_assert(sizeof(ResTIMG)==32,"actual texture header width");
    CShader::CLengthMap absent(nullptr);
    check(absent._0==nullptr&&reinterpret_cast<std::uintptr_t>(absent._4)==32&&absent._8==1,"null constructor preserves Wii address word32 and inactive flag");
    const float special[]={0.0f,1.0f,-1.0f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()};
    for(float u:special)for(float v:special)check(absent.refer(u,v)==1.0f,"inactive map returns1 before pointer or FP input consumption");
    struct Image {ResTIMG header;u8 pixels[64];} image{};
    image.header.mFormat=GX_TF_I8;image.header.mWidth=8;image.header.mHeight=4;
    image.header.mWrapS=image.header.mWrapT=GX_CLAMP;
    image.header.mImageDataOffset=64; // This routine deliberately uses header+32, as the DOL does.
    for(unsigned n=0;n<64;++n)image.pixels[n]=static_cast<u8>(n*3);
    CShader::CLengthMap valid(&image.header);
    check(valid._0==&image.header&&valid._4==image.pixels&&valid._8==0,"I8 data begins immediately after actual header");
    check(valid.refer(0,0)==float(image.pixels[0])/255.0f,"I8 first pixel");
    check(valid.refer(1,0)==float(image.pixels[7])/255.0f,"I8 last column");
    check(valid.refer(0,1)==float(image.pixels[24])/255.0f,"I8 last row");
    check(valid.refer(1,1)==float(image.pixels[31])/255.0f,"I8 tile endpoint");
    valid.setLengthMap(nullptr);
    check(valid._0==&image.header&&valid._4==image.pixels&&valid._8==1&&valid.refer(1,1)==1.0f,"null setter retains addresses and disables reads");
    image.header.mFormat=GX_TF_RGBA8;
    CShader::CLengthMap unsupported(&image.header);
    check(unsupported._0==&image.header&&unsupported._4==image.pixels&&unsupported._8==1&&unsupported.refer(0,0)==1.0f,"unsupported format inactive contract");
}
'''


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--compiler',default='clang++');ap.add_argument('--output',type=Path);args=ap.parse_args()
    report={'source_sha256':hashlib.sha256((ROOT/'src/Game/Util/CShader.cpp').read_bytes()).hexdigest(),
            'runtime_scope':'full TU; direct getCount/CLengthMap, verbatim extracted vertex/count read statements; no model traversal/renderer claim',
            'negative_controls':[]}
    with tempfile.TemporaryDirectory(prefix='petari-cshader-') as directory:
        temp=Path(directory);legacy=temp/'legacy';prepare(ROOT,legacy)
        source=(legacy/'src/Game/Util/CShader.cpp').read_text()
        common=[args.compiler,'-std=c++17','-DGALAXY_NATIVE=1','-O2','-fno-rtti','-ffp-contract=off','-fno-strict-aliasing',
                '-Wno-register','-Wno-inconsistent-missing-override','-fsanitize=address,undefined,float-cast-overflow',
                '-fno-sanitize-recover=all','-ffunction-sections','-fdata-sections','-Wl,-dead_strip']
        for inc in ['native/include','native/home_menu/include','include','libs/JSystem/include','libs/RVL_SDK/include','libs/nw4r/include','libs/RVLFaceLib/include','libs/libms/include']:
            if not inc.startswith('native/'):common+=['-I',str(legacy/inc)]
            common+=['-I',str(ROOT/inc)]
        def fragment(text,start,end):
            begin=text.index(start);return text[begin:text.index(end,begin)]
        def compile_run(text,label):
            tu=temp/(label+'-source.cpp');tu.write_text(text)
            vertex_reads=fragment(text,'                u8* pVertex = pRead + 3 + stride * i;','                mIndexArray[pos]._0')
            count=fragment(text,'#ifdef GALAXY_NATIVE\n                s32 count =','\n\n                for (s32 i =')
            border=fragment(text,'#ifdef GALAXY_NATIVE\n                    u16 pos =','\n                    mIndexArray[pos]._0')
            cpp=temp/(label+'.cpp')
            accessors='''
struct Indices {u16 pos,nrm,tex;};
static Indices vertex(u8* pRead,int stride,int i,int posOffset,int nrmOffset,int texOffset) {
'''+vertex_reads+'''return {pos,nrm,tex};}
static s32 borderCount(u8* pRead) {
'''+count+'''\nreturn count;}
static u16 borderVertex(u8* pRead,int stride,int i,int posOffset) {
'''+border+'''\nreturn pos;}
'''
            cpp.write_text('#include <initializer_list>\n#include '+json.dumps(str(tu))+'\n'+accessors+HARNESS+'\nint main(){counts();vertices();maps();std::printf("PASS %u production-read/map checks\\n",checks);}\n')
            binary=temp/label;cmd=common+[str(cpp),'-o',str(binary)]
            build=subprocess.run(cmd,text=True,capture_output=True)
            if build.returncode:raise RuntimeError(build.stdout+build.stderr)
            return subprocess.run([str(binary)],text=True,capture_output=True,timeout=30)
        result=compile_run(source,'current')
        if result.returncode:raise RuntimeError(result.stdout+result.stderr)
        print(result.stdout,end='');report['result']=result.stdout.strip()
        operations=[
            ('count','return PetariNative::readU16BE(pRead + 1);','return *reinterpret_cast< u16* >(pRead + 1);'),
            ('position','u16 pos = PetariNative::readU16BE(pVertex + posOffset);','u16 pos = *reinterpret_cast< u16* >(pVertex + posOffset);'),
            ('normal','u16 nrm = PetariNative::readU16BE(pVertex + nrmOffset);','u16 nrm = *reinterpret_cast< u16* >(pVertex + nrmOffset);'),
            ('texture','u16 tex = PetariNative::readU16BE(pVertex + texOffset);','u16 tex = *reinterpret_cast< u16* >(pVertex + texOffset);'),
            ('border-count','s32 count = PetariNative::readU16BE(pRead + 1);','s32 count = *reinterpret_cast< u16* >(pRead + 1);'),
            ('border-position','u16 pos = PetariNative::readU16BE((pRead + stride * i) + (posOffset + 3));','u16 pos = *reinterpret_cast< u16* >((pRead + stride * i) + (posOffset + 3));'),
            ('null-image','_4 = reinterpret_cast< const u8* >(reinterpret_cast< std::uintptr_t >(pTimg) + sizeof(ResTIMG));','_4 = reinterpret_cast< const u8* >(pTimg) + sizeof(ResTIMG);'),
            ('null-zero','_4 = reinterpret_cast< const u8* >(reinterpret_cast< std::uintptr_t >(pTimg) + sizeof(ResTIMG));','_4 = pTimg ? reinterpret_cast< const u8* >(pTimg) + sizeof(ResTIMG) : nullptr;'),
            ('reversed-endian','return PetariNative::readU16BE(pRead + 1);','return static_cast<u16>(pRead[1]) | (static_cast<u16>(pRead[2]) << 8);')]
        for label,fixed,original in operations:
            assert source.count(fixed)==1,label
            result=compile_run(source.replace(fixed,original),label)
            if result.returncode==0:raise AssertionError(label+' original operation escaped detection')
            reason='sanitizer' if 'runtime error:' in result.stderr else 'value mismatch'
            if reason=='value mismatch' and 'FAIL ' not in result.stderr:raise RuntimeError(result.stderr)
            if label=='null-image' and 'runtime error:' not in result.stderr:raise AssertionError('null pointer arithmetic must trigger sanitizer')
            print('PASS original-operation negative control: '+label+' ('+reason+')')
            report['negative_controls'].append({'site':label,'rejected':True,'reason':reason})
        report['compiler']=subprocess.check_output([args.compiler,'--version'],text=True).splitlines()[0]
        report['command_prefix']=common
    if args.output:args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n')
    return 0


if __name__=='__main__':raise SystemExit(main())
