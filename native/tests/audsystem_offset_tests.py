#!/usr/bin/env python3
"""Regress the actual AudSystem setter against its owning native class layout.

Compile the complete production TU to LLVM IR, check its single signed32 input
store through msBasic against an independent canonical-field address expression,
and reject the old raw-offset and wrong-neighbor stores. A sanitized layout-only
probe measures actual headers; no AudSystem is fabricated, constructed or run.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'native/tools'))
from encode_legacy_sources import prepare

PROBE=r'''
#include "Game/AudioLib/AudSystem.hpp"
#include <cstddef>
#include <cstdio>
#include <type_traits>
static_assert(std::is_same<decltype(AudSystem::mRestrictedSoundFrames),s32>::value,"canonical signed32 field");
static_assert(std::is_same<decltype(AudSystem::msBasic),AudSystem*>::value,"canonical singleton pointer");
static_assert(sizeof(s32)==4 && s32(-1)<0,"native signed32 argument/store");
int main() {
    const size_t oldVictim=offsetof(AudSystem,mSeMgr)+offsetof(JAISeMgr,mParams)
        +offsetof(JAISoundParamsMove,mParams)+offsetof(JASSoundParams,mFxMix);
    std::printf("%zu %zu %zu %zu %zu %zu\n",sizeof(AudSystem),sizeof(JAUSoundMgr),
        offsetof(AudSystem,mRestrictedSoundFrames),sizeof(AudSystem::mRestrictedSoundFrames),
        offsetof(AudSystem,mSystemSoundOnlyFrames),oldVictim);
}
'''
ORACLE=r'''
extern "C" const s32* audRestrictedAddressForTest(const AudSystem* system) {
    return &system->mRestrictedSoundFrames;
}
'''


def function(ir,name):
    match=re.search(r'^define [^\n]*@'+re.escape(name)+r'\([^\n]*\)[^\n]*\{\n(.*?)^}',ir,re.M|re.S)
    if not match:raise AssertionError('missing compiled function '+name)
    return match.group(0)


def verify(ir):
    setter=function(ir,'_ZN9AudSystem11setField834Ei')
    oracle=function(ir,'audRestrictedAddressForTest')
    parameter=re.search(r'@_ZN9AudSystem11setField834Ei\(i32(?: [A-Za-z_][A-Za-z_0-9]*)* (%[A-Za-z_0-9.]+)\)',setter)
    if not parameter:raise AssertionError('setter must keep one signed32 ABI input')
    loaded=re.findall(r'(%[A-Za-z_0-9.]+) = load ptr, ptr @_ZN9AudSystem7msBasicE\b',setter)
    if len(loaded)!=1:raise AssertionError('must load the canonical AudSystem::msBasic exactly once')
    gep=re.findall(r'(%[A-Za-z_0-9.]+) = getelementptr inbounds ([^\n]+)',setter)
    expected=re.findall(r'getelementptr inbounds ([^\n]+)',oracle)
    if len(gep)!=1 or len(expected)!=1:raise AssertionError('one canonical field address required')
    signature=re.sub(r', ptr %[A-Za-z_0-9.]+,',', ptr BASE,',gep[0][1])
    canonical=re.sub(r', ptr %[A-Za-z_0-9.]+,',', ptr BASE,',expected[0])
    if signature!=canonical:raise AssertionError('store address differs from canonical mRestrictedSoundFrames field')
    if ', ptr '+loaded[0]+',' not in gep[0][1]:raise AssertionError('field address must use the loaded singleton base')
    stores=re.findall(r'\bstore i32 (%[A-Za-z_0-9.]+), ptr (%[A-Za-z_0-9.]+)',setter)
    if stores!=[(parameter[1],gep[0][0])]:raise AssertionError('one unchanged input word must reach the canonical field')
    if len(re.findall(r'\bstore\b',setter))!=1 or re.search(r'\b(call|invoke|br|switch)\b',setter):
        raise AssertionError('setter must retain its unconditional single-store footprint')
    return {'setter_ir':setter,'field_oracle_ir':oracle,'canonical_gep':signature,'input_store_bits':32,
            'global':'AudSystem::msBasic','scope':'all signed32 input bit patterns by direct dataflow; no audio-system execution'}


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--compiler',default='clang++');ap.add_argument('--output',type=Path);args=ap.parse_args()
    path=ROOT/'src/Game/AudioLib/AudSystem.cpp';report={'source_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'negative_controls':[]}
    with tempfile.TemporaryDirectory(prefix='petari-audsystem-offset-') as directory:
        temp=Path(directory);legacy=temp/'legacy';prepare(ROOT,legacy)
        flags=[args.compiler,'-std=c++17','-DGALAXY_NATIVE=1','-O2','-fno-rtti','-ffp-contract=off','-fno-strict-aliasing',
               '-Wno-register','-Wno-inconsistent-missing-override']
        for inc in ['native/include','native/home_menu/include','include','libs/JSystem/include','libs/RVL_SDK/include','libs/nw4r/include','libs/RVLFaceLib/include','libs/libms/include']:
            if not inc.startswith('native/'):flags+=['-I',str(legacy/inc)]
            flags+=['-I',str(ROOT/inc)]
        probe=temp/'layout.cpp';probe.write_text(PROBE);binary=temp/'layout'
        command=flags+['-fsanitize=address,undefined','-fno-sanitize-recover=all',str(probe),'-o',str(binary)]
        result=subprocess.run(command,text=True,capture_output=True)
        if result.returncode:raise RuntimeError(result.stderr)
        result=subprocess.run([str(binary)],text=True,capture_output=True,timeout=15)
        if result.returncode:raise RuntimeError(result.stderr)
        values=list(map(int,result.stdout.split()))
        report['native_layout']=dict(zip(['sizeof_AudSystem','sizeof_JAUSoundMgr','restricted_offset','restricted_size','neighbor_offset','old_raw_victim_fxmix_offset'],values))
        if values[2]==0x834 or values[3]!=4 or values[4]==values[2]:raise AssertionError('probe must expose distinct native field positions and signed32 width')
        source=(legacy/'src/Game/AudioLib/AudSystem.cpp').read_text()
        def compiled(text,label):
            cpp=temp/(label+'.cpp');cpp.write_text(text+'\n'+ORACLE);ir=temp/(label+'.ll')
            result=subprocess.run(flags+['-S','-emit-llvm',str(cpp),'-o',str(ir)],text=True,capture_output=True)
            if result.returncode:raise RuntimeError(result.stderr)
            return ir.read_text()
        report['production_ir']=verify(compiled(source,'current'))
        fixed='msBasic->mRestrictedSoundFrames = value;'
        if source.count(fixed)!=1:raise AssertionError('one actual native assignment expected')
        for label,wrong in [('old-raw-offset','*reinterpret_cast<s32*>(reinterpret_cast<u8*>(msBasic)+0x834)=value;'),
                            ('wrong-neighbor','msBasic->mSystemSoundOnlyFrames = value;'),
                            ('narrowed-input','msBasic->mRestrictedSoundFrames = static_cast<u16>(value);')]:
            ir=compiled(source.replace(fixed,wrong),label)
            try:verify(ir)
            except AssertionError as error:report['negative_controls'].append({'mutation':label,'rejected':True,'reason':str(error)})
            else:raise AssertionError(label+' escaped field/store regression')
        report['compiler']=subprocess.check_output([args.compiler,'--version'],text=True).splitlines()[0]
        report['compile_prefix']=flags;report['sanitized_probe_command']=command
    print('PASS actual native layout + full-TU signed32 singleton field/store regression; 3 mutants rejected')
    print('restricted offset: '+hex(values[2])+'; old raw offset '+hex(0x834)+'; width '+str(values[3]))
    if args.output:args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n')
    return 0


if __name__=='__main__':raise SystemExit(main())
