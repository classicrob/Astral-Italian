#!/usr/bin/env python3
"""Emit exact source-only compiler argv; intentionally no execution/runner API."""
import argparse
import hashlib
import json
from pathlib import Path
from visual_pairs import private_path

PINS=json.loads(Path(__file__).with_name('m1_draw_context_pins.json').read_text())

def need(value,message):
    if not value:raise ValueError(message)

def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def plan(context,source,output):
    source=Path(source).resolve();output=private_path(output)
    need(not output.exists(),'fresh private output root required')
    need(context['inputs']==PINS['inputs'],'actual tool/SDK pins differ')
    entry=context['entry'];args=entry['arguments'];compiler=PINS['inputs'][0]['path']
    need(args[0]==compiler and args[-2:]==['-c',entry['file']],'unexpected canonical TU command')
    need(args.count('-o')==1,'ambiguous canonical output')
    for token in ('-DGALAXY_NATIVE=1','-nostdinc++','-nostdlib++','-std=gnu++17','-ffp-contract=off','-fno-strict-aliasing','-fno-rtti'):
        need(token in args,'canonical compile policy missing: '+token)
    o=args.index('-o');need(o==len(args)-4,'canonical compile order changed')
    prefix=args[:o];folder=source/'native/app/visual';commands=[];files={}
    def compile_(name,canonical=False):
        path=folder/name;need(path.is_file(),'missing frozen fixture source '+name);files[str(path)]=digest(path)
        flags=prefix if canonical else [compiler,'-nostdinc++','-isystem',str(Path(compiler).parent.parent/'include/c++/v1'),'-nostdlib++','-O3','-std=gnu++17','-arch','arm64','-isysroot',args[args.index('-isysroot')+1],'-mmacosx-version-min=15.0','-ffp-contract=off','-fno-strict-aliasing','-fno-rtti']
        stem=Path(name).stem;obj=output/(stem+'.o')
        commands.append(dict(label='compile-'+stem,cwd=entry['directory'],argv=flags+([] if canonical else ['-UNDEBUG'])+['-MMD','-MF',str(output/(stem+'.d')),'-c',str(path),'-o',str(obj)]))
        return str(obj)
    core=[compile_(name) for name in ('draw_observer.cpp','draw_observer_json.cpp')]
    controls=compile_('draw_observer_tests.cpp');bridge=compile_('draw_pilot_bridge.cpp');bridge_tests=compile_('draw_pilot_bridge_tests.cpp')
    for name in ('draw_snapshot.cpp','draw_snapshot_no_xfb.cpp','draw_pilot_host.cpp'):compile_(name,True)
    runtimes=[record['path'] for record in PINS['inputs'] if record['path'].endswith('.a')]
    need(len(runtimes)==3,'static matched runtime trio missing')
    link=[compiler,'-nostdlib++','-arch','arm64','-isysroot',args[args.index('-isysroot')+1],'-mmacosx-version-min=15.0','-fuse-ld=lld','-Wl,-no_exported_symbols','-pthread']
    for name,objects in [('observer-core-controls',core+[controls]),('pilot-bridge-controls',core+[bridge,bridge_tests])]:
        executable=str(output/name)
        commands.append(dict(label='link-'+name,cwd=str(output),argv=link+objects+runtimes+['-o',executable]))
        commands.append(dict(label='control-'+name,cwd=str(output),argv=[executable]))
        commands.append(dict(label='dependencies-'+name,cwd=str(output),argv=['/usr/bin/otool','-L',executable]))
    for name in ('draw_observer.hpp','draw_snapshot_no_xfb.hpp','draw_pilot_bridge.hpp'):files[str(folder/name)]=digest(folder/name)
    return dict(schema_version=1,status='SOURCE_PLAN_ONLY_EXECUTION_BLOCKED',commands=commands,source_files=files,
                tool_inputs=PINS['inputs'],limits=PINS['limits'],runner=PINS['runner_status'],qualification='BLOCKED',qualified_frames=None,
                estimate='45-90 seconds, unmeasured; hard120s no extension',controls_status='UNRUN',installation='NOT_ALLOCATED',
                source_context_directory=entry['directory'],game_brackets=PINS['game_brackets'],
                sampler_safety='OPEN: original XFB/VI fields; no-XFB async17 scene visibility/retirement; compile is not sampling evidence',
                prerequisites=['B/E exact bridge review','reviewed exact compiler child/ack/ownership runner','complete before/after consumed header dependency pins','explicit P/root M1 slot'],
                pending_link_checks=['all normal native consumers','manual partial-link archive inventory'],
                build_scope='standalone core/exporter/bridge controls + three compile-only Game-dependent TUs; no app/Game link/runtime/GPU')

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--context',required=True);parser.add_argument('--source',required=True);parser.add_argument('--output',required=True);parser.add_argument('--plan',required=True)
    a=parser.parse_args();need(digest(a.context)==PINS['context_sha256'],'actual canonical context hash differs')
    result=plan(json.loads(Path(a.context).read_text()),a.source,a.output)
    path=private_path(a.plan)
    with path.open('x') as stream:json.dump(result,stream,indent=2);stream.write('\n')
    print(json.dumps(dict(status=result['status'],commands=len(result['commands']),runner=result['runner'])))
