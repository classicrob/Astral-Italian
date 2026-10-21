"""Fixed chosen-tools SOURCE component; not the full official graph or importer."""
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import stat

import dispatch_platform_exact as exact
import dispatch_full_budget as baseline

ROOT=Path.home()/'MacioGalaxy2-team/scratch-port/solP/dispatch-platform-tools-v1'
LABELS=('python_context','ninja_version','git_version','compiler_rule','consumed_deps')
SOURCE_NAMES=('dispatch_tools_profile.py','dispatch_tools_gate.py','dispatch_tools_chosen.py',
              'dispatch_platform_exact.py','dispatch_full_budget.py','dispatch_exec_gate.py',
              'dispatch_budget.py','dispatch_preflight.py')
UNIT='src/Game/System/WorldMapResultTU13.o'
SPEC_SHA='3b0975d48ca4354862814f13de004121fb16fbc1b90f87fb8e9f78a65155f8d9'
SRC='src/Game/System/solPCompilerContext.cpp'
OBJ='build/SB4E01/src/Game/System/solPCompilerContext.o'
DEP=OBJ[:-2]+'.d'
FIXTURE=b'extern "C" int solP_chosen_compiler_context() { return 17; }\n'
TRANSFORM_SHA='622a2c65f9b10a836c8af3c36ff2db770f5739e1c0285f5f004b82867c965d9f'


def fixed_tool_paths():
    build=Path.home()/'MacioGalaxy2/build'
    return {'wibo':str(build/'tools/wibo'),'wibo_real':str(build/'tools/wibo.real'),
            'sjiswrap':str(build/'tools/sjiswrap.exe'),'compiler':str(build/'compilers/Wii/1.0/mwcceppc.exe'),
            'python_command':'/opt/homebrew/opt/python@3.15/bin/python3.15',
            'ninja':'/opt/homebrew/Cellar/ninja/1.13.2/bin/ninja','git':'/usr/bin/git',
            'env':'/usr/bin/env','sh':'/bin/sh',
            'transform_dep':str(Path.home()/'MacioGalaxy2-team/scratch-port/solP/proof-wm-dispatch-source/tools/transform_dep.py')}


def read_bounded(path,maximum):
    path=Path(path);st=path.lstat()
    if not stat.S_ISREG(st.st_mode) or st.st_size>maximum:raise RuntimeError('bounded regular component artifact required')
    with path.open('rb') as stream:raw=stream.read(maximum+1)
    if len(raw)>maximum:raise RuntimeError('component artifact grew beyond bound')
    return raw


def source_check(pins):
    if type(pins) is not dict or set(pins)!=set(SOURCE_NAMES):raise RuntimeError('exact tools source inventory')
    for name,pin in pins.items():
        if exact.sha(Path(__file__).with_name(name))!=pin:raise RuntimeError('chosen-tools source drift')


def rule_tokens(spec,tools):
    row=spec['units'][UNIT];opt=row['options']
    if row['source']!='src/Game/System/WorldMapResultTU13.cpp' or opt['mw_version']!='Wii/1.0' or opt['shift_jis'] is not True:
        raise RuntimeError('exact representative unit required')
    flags=list(opt['cflags']);extra=list(opt['extra_cflags'])
    if not any(f.startswith('-lang') for f in flags+extra):extra.insert(0,'-lang=c++')
    official=[tools['wibo']['path'],tools['sjiswrap']['path'],tools['compiler']['path']]+shlex.split(' '.join(flags+extra))+[
        '-MMD','-c',row['source'],'-o','build/SB4E01/src/Game/System','&&',tools['python_command']['path'],
        'tools/transform_dep.py','build/SB4E01/src/Game/System/WorldMapResultTU13.d','build/SB4E01/src/Game/System/WorldMapResultTU13.d']
    changed=list(official);indices=[official.index(row['source']),len(official)-2,len(official)-1]
    replacements=[SRC,DEP,DEP]
    substitutions=[]
    for i,value in zip(indices,replacements):
        substitutions.append({'index':i,'before':official[i],'after':value});changed[i]=value
    return official,changed,substitutions


def graph(tokens):
    boundary=tokens.index('&&')
    command=shlex.join(tokens[:boundary])+' && '+shlex.join(tokens[boundary+1:])
    # No flags/order changes; shell quoting only. Ninja dollar escaping is literal.
    command=command.replace('$','$$')
    return ('rule mwcc_context\n  command = '+command+'\n  depfile = '+DEP+'\n  deps = gcc\n'
            +'build '+OBJ+': mwcc_context '+SRC+'\n').encode()


def validate_plan(plan):
    source_check(plan['source_files'])
    if (type(plan['schema']) is not int or plan['schema']!=1 or plan['root']!=str(ROOT)
            or plan['labels']!=list(LABELS) or plan['source_fixture_sha256']!=hashlib.sha256(FIXTURE).hexdigest()):
        raise RuntimeError('exact tools plan identity')
    path=Path(plan['canonical_spec']['path'])
    if exact.sha(path)!=SPEC_SHA or plan['canonical_spec']['sha256']!=SPEC_SHA:raise RuntimeError('official spec pin')
    spec=exact.strict_json(path.read_bytes());official,tokens,substitutions=rule_tokens(spec,plan['tools'])
    if plan['official_tokens']!=official or plan['compile_tokens']!=tokens or plan['substitutions']!=substitutions:
        raise RuntimeError('representative official token/declared substitution drift')
    if plan['graph_sha256']!=hashlib.sha256(graph(tokens)).hexdigest():raise RuntimeError('exact synthetic graph pin')
    if set(plan['tools'])!=set(fixed_tool_paths()):raise RuntimeError('exact fixed tool inventory')
    for name,item in plan['tools'].items():
        if item['path']!=fixed_tool_paths()[name]:raise RuntimeError('fixed tool path differs')
        path=Path(item['path'])
        if not path.is_file() or exact.sha(path)!=item['sha256']:raise RuntimeError('pinned chosen tool changed')
    if plan['tools']['transform_dep']['sha256']!=TRANSFORM_SHA:raise RuntimeError('exact transform source pin')
    if plan['commands']!=commands(plan):raise RuntimeError('fixed ordered tools argv required')


def commands(plan):
    t=plan['tools'];py=plan['python']['resolved'];ninja=t['ninja']['path']
    return [exact.payload(py,plan['operator']),[ninja,'--version'],[t['git']['path'],'--version'],
            [ninja,'-f','compiler.ninja','-j','1','-k','1','-d','keepdepfile',OBJ],
            [ninja,'-f','compiler.ninja','-t','deps',OBJ]]


def context(plan):
    validate_plan(plan);base=baseline.launch_context(ROOT)
    if plan['base_env']!=base['env']:raise RuntimeError('independent exact tools base-eight mapping')
    env,digest=exact.load_operator(plan['operator']['path'],plan['operator']['sha256'],base['env'])
    if digest!=plan['full_env_sha256'] or base['python']!=plan['python']['resolved'] or base['python_sha256']!=plan['python']['sha256']:
        raise RuntimeError('independent tools operator/Python pin')
    targets={a[0]:{'path':str(Path(a[0]).resolve()),'sha256':exact.sha(a[0])} for a in commands(plan)}
    return {'python':base['python'],'python_sha256':base['python_sha256'],
            'gate':str(Path(__file__).with_name('dispatch_tools_gate.py').resolve()),
            'gate_sha256':plan['source_files']['dispatch_tools_gate.py'],'env':env,'targets':targets}


def prepare_fixture(plan):
    fixture=ROOT/'fixture';fixture.mkdir()
    for dirname in ('include','libs/JSystem/include','libs/MSL_C++/include','libs/MSL_C/include',
                    'libs/MetroTRK/include','libs/RVLFaceLib/include','libs/RVL_SDK/include',
                    'libs/Runtime/include','libs/nw4r/include','build/SB4E01/include',
                    'src/Game/System','build/SB4E01/src/Game/System','tools'):
        (fixture/dirname).mkdir(parents=True,exist_ok=True)
    (fixture/SRC).write_bytes(FIXTURE)
    (fixture/'compiler.ninja').write_bytes(graph(plan['compile_tokens']))
    (fixture/'tools/transform_dep.py').write_bytes(Path(plan['tools']['transform_dep']['path']).read_bytes())
    check_fixture(plan)


def check_fixture(plan):
    fixture=ROOT/'fixture'
    expected={SRC:hashlib.sha256(FIXTURE).hexdigest(),'compiler.ninja':plan['graph_sha256'],'tools/transform_dep.py':TRANSFORM_SHA}
    for rel,pin in expected.items():
        path=fixture/rel
        if path.is_symlink() or exact.sha(path)!=pin:raise RuntimeError('consumed fixture/source/graph/transform drift')


def elf_function(raw):
    if len(raw)<52 or len(raw)>1024**2 or raw[:7]!=b'\x7fELF\x01\x02\x01':raise RuntimeError('bounded ELF32 big-endian object required')
    if struct.unpack_from('>HH',raw,16)!=(1,20):raise RuntimeError('relocatable PPC object required')
    shoff=struct.unpack_from('>I',raw,32)[0];entsize,count=struct.unpack_from('>HH',raw,46)
    if entsize!=40 or not 0<count<=1024 or shoff+count*40>len(raw):raise RuntimeError('ELF section bounds')
    sections=[struct.unpack_from('>10I',raw,shoff+i*40) for i in range(count)];found=[]
    def section_data(index):
        if not 0<=index<count:raise RuntimeError('ELF linked section index')
        s=sections[index];offset,size=s[4:6]
        if offset+size>len(raw):raise RuntimeError('ELF section data bounds')
        return raw[offset:offset+size]
    for section_index,s in enumerate(sections):
        if s[1]!=2:continue
        if s[9]!=16 or s[5]%16:raise RuntimeError('ELF symbol table width')
        if not 0<=s[6]<count or sections[s[6]][1]!=3:raise RuntimeError('ELF symbol names type')
        names=section_data(s[6]);data=section_data(section_index)
        for at in range(0,len(data),16):
            name,value,size,info,other,index=struct.unpack_from('>IIIBBH',data,at)
            if name>=len(names):raise RuntimeError('ELF symbol name bounds')
            end=names.find(b'\0',name)
            if end<0:raise RuntimeError('ELF symbol name termination')
            if names[name:end]!=b'solP_chosen_compiler_context':continue
            if info!=0x12 or not 0<index<count or size==0 or size>4096:raise RuntimeError('expected defined global function')
            body_section=sections[index]
            if body_section[1]!=1 or body_section[2]&6!=6 or value+size>body_section[5]:raise RuntimeError('function executable section extent')
            body=section_data(index)[value:value+size];found.append({'size':size,'body_sha256':hashlib.sha256(body).hexdigest()})
    if len(found)!=1:raise RuntimeError('exact one expected synthetic function')
    return found[0]


def dependency_join(text,consumed):
    fixture=ROOT/'fixture'
    if len(text)>65536 or len(consumed)>65536:raise RuntimeError('dependency output bound')
    target,body=text.replace('\\\n',' ').split(':',1)
    if target.strip()!=OBJ:raise RuntimeError('dependency target mismatch')
    def relative(value):
        p=Path(value)
        if '..' in p.parts:raise RuntimeError('dependency traversal')
        if p.is_absolute():
            try:p=p.relative_to(fixture)
            except ValueError:raise RuntimeError('foreign dependency origin')
        return p.as_posix()
    paths=[relative(p) for p in body.split()]
    lines=consumed.splitlines()
    if not lines or not lines[0].startswith(OBJ+': #deps 1,') or not lines[0].endswith('(VALID)'):
        raise RuntimeError('actual Ninja consumed dependency status')
    actual=[relative(line.strip()) for line in lines[1:] if line.strip()]
    if paths!=[SRC] or actual!=[SRC]:raise RuntimeError('exact synthetic-only actual dependency closure')
    return {'target':OBJ,'consumed_paths':actual,'source_sha256':hashlib.sha256(FIXTURE).hexdigest()}
