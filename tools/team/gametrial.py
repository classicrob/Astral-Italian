#!/usr/bin/env python3
"""Read-only Game-unit compiler matrix with a fresh, frozen default control.

Only the compiler executable and private output directory vary. Every unit is
compiled once per version; reports use functionRelocDiffs=none. Each object is
deleted immediately after scoring. No Ninja, configure, split or source edits.
"""
from __future__ import annotations
import argparse
import ast
from concurrent.futures import ThreadPoolExecutor
import csv
import difflib
import hashlib
import json
import os
from pathlib import Path
import shutil
import statistics
import subprocess
import sys
import tempfile
import time

import flagsearch
import fnsearch

SCRATCH=Path.home()/'MacioGalaxy2-team/scratch-decomp2/rules'
LOCK=Path.home()/'MacioGalaxy2-team/slots/server-heavy.lock'
CODE={'.text','.init'}


def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def slug(text):return text.replace('/','_').replace('.','_')
def write_json(path,value):
    temporary=path.with_suffix(path.suffix+'.tmp')
    temporary.write_text(json.dumps(value,indent=2)+'\n');temporary.replace(path)


def source_state(root):
    revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()
    dirty=subprocess.check_output(['git','status','--porcelain','--untracked-files=all','--','src','include','libs'],cwd=root,text=True)
    if dirty:raise ValueError('source tree is dirty: '+dirty[:2000])
    trees={p:subprocess.check_output(['git','rev-parse','HEAD:'+p],cwd=root,text=True).strip() for p in ('src','include','libs')}
    generated={str(p.relative_to(root)):sha(p) for p in sorted((root/'build/SB4E01/include').rglob('*')) if p.is_file()}
    return dict(revision=revision,trees=trees,generated=generated)


def check_inputs(manifest):
    root=Path(manifest['root']);state=source_state(root)
    if state['trees']!=manifest['source_state']['trees'] or state['generated']!=manifest['source_state']['generated']:
        raise ValueError('frozen source/generated include inputs changed')
    for path,digest in manifest['pins'].items():
        if not Path(path).is_file() or sha(path)!=digest:raise ValueError('pinned input changed: '+path)
    if shutil.disk_usage(manifest['out']).free<50_000_000_000:raise ValueError('less than 50 GB free')


def compiler_argv(edge,version,output):
    argv=edge['argv'].copy();index=edge['compiler_index']
    if version!='baseline':argv[index]=str(Path('build/compilers')/version/'mwcceppc.exe')
    argv[argv.index('-o')+1]=str(output)
    return argv


def report_scores(data):
    if len(data.get('units',[]))!=1:raise ValueError('expected one report unit')
    u=data['units'][0]
    functions={f['name']:dict(score=float(f.get('fuzzy_match_percent') or 0),size=int(f['size'])) for f in u.get('functions',[])}
    sections={s['name']:dict(score=float(s.get('fuzzy_match_percent') or 0),size=int(s['size'])) for s in u.get('sections',[]) if s['name'] not in CODE}
    measures=u.get('measures',{})
    return dict(functions=functions,sections=sections,
                matched_code=int(measures.get('matched_code',sum(f['size'] for f in functions.values() if f['score']==100))),
                matched_data=int(measures.get('matched_data',sum(s['size'] for s in sections.values() if s['score']==100))))


def compare(before,after):
    fd={n:[f['score'],after['functions'].get(n,{}).get('score',0)] for n,f in before['functions'].items()
        if after['functions'].get(n,{}).get('score',0)<f['score']}
    dd={n:[s['score'],after['sections'].get(n,{}).get('score',0)] for n,s in before['sections'].items()
        if after['sections'].get(n,{}).get('score',0)<s['score']}
    improved={n:[f['score'],after['functions'].get(n,{}).get('score',0),f['size']] for n,f in before['functions'].items()
              if after['functions'].get(n,{}).get('score',0)>f['score']}
    data_up={n:[s['score'],after['sections'].get(n,{}).get('score',0),s['size']] for n,s in before['sections'].items()
             if after['sections'].get(n,{}).get('score',0)>s['score']}
    code_gain=after['matched_code']-before['matched_code'];data_gain=after['matched_data']-before['matched_data']
    no_drops=not fd and not dd and code_gain>=0 and data_gain>=0
    return dict(function_drops=fd,data_section_drops=dd,improved_functions=improved,improved_data_sections=data_up,
                matched_code_gain=code_gain,matched_data_gain=data_gain,
                new_exact_bytes=sum(size for old,new,size in improved.values() if new==100),
                fuzzy_code_gain=sum(size*(new-old)/100 for old,new,size in improved.values()),
                zero_drops=no_drops,strict_gain=no_drops and bool(improved or data_up or code_gain or data_gain))


def prepare(args):
    root=args.root.resolve();edge_root=args.edge_root.resolve();out=args.out.resolve()
    if out.exists() and any(out.iterdir()):raise ValueError('prepare requires an empty output directory')
    if sha(root/'configure.py')!=sha(edge_root/'configure.py'):
        raise ValueError('command-graph configure.py differs from the frozen source baseline')
    state=source_state(root);out.mkdir(parents=True,exist_ok=True)
    graph=out/'inputs';graph.mkdir()
    graph_bytes=(edge_root/'build.ninja').read_bytes();(graph/'build.ninja').write_bytes(graph_bytes)
    cfg_bytes=(edge_root/'objdiff.json').read_bytes();(graph/'objdiff.json').write_bytes(cfg_bytes)
    (graph/'configure.py').write_bytes((root/'configure.py').read_bytes())
    cache={};units=[];skipped=[]
    for unit in json.loads(cfg_bytes)['units']:
        if not unit['name'].startswith('main/Game/'):continue
        try:
            if not unit.get('base_path'):raise ValueError('no configured source object')
            edge=fnsearch.compiler_edge(graph,unit['base_path'],cache)
            relative=Path(edge['source'])
            if relative.is_absolute() or '..' in relative.parts:raise ValueError('source is not repo-relative')
            if not (root/relative).is_file():raise ValueError('source file absent')
            if any(str(edge_root) in a for a in edge['argv']):raise ValueError('absolute graph-owner path in command')
            target=root/unit['target_path']
            if not target.is_file():raise ValueError('target object absent')
            units.append(dict(unit=unit,edge=edge,source_sha256=sha(root/relative)))
        except ValueError as error:skipped.append(dict(unit=unit['name'],reason=str(error)))
    pins={str(root/'configure.py'):sha(root/'configure.py')}
    for module in (__file__,fnsearch.__file__,flagsearch.__file__,root/'tools/team/sg.py',root/'tools/team/permute.py'):
        pins[str(Path(module).resolve())]=sha(module)
    for path in graph.iterdir():pins[str(path)]=sha(path)
    tools={'build/tools/wibo','build/tools/wibo.real','build/tools/sjiswrap.exe','build/tools/objdiff-cli','build/tools/dtk'}
    tools.update(item['edge']['argv'][item['edge']['compiler_index']] for item in units)
    tools.update(str(Path('build/compilers')/v/'mwcceppc.exe') for v in args.versions)
    for tool in tools:
        path=(root/tool).resolve()
        if not path.is_file():raise ValueError('required tool missing: '+str(path))
        pins[str(path)]=sha(path)
    manifest=dict(root=str(root),edge_root=str(edge_root),out=str(out),versions=args.versions,workers=args.workers,
                  hold_seconds=args.hold_seconds,pause_seconds=args.pause_seconds,source_state=state,pins=pins,
                  units=units,skipped=skipped,scoring_config={'functionRelocDiffs':'none'},targets_frozen=False,
                  edge_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=edge_root,text=True).strip())
    write_json(out/'manifest.json',manifest)
    print(f'Prepared {len(units)} Game compile edges; {len(skipped)} skipped; fresh default control plus {args.versions}',flush=True)
    for item in skipped:print('SKIP',item,flush=True)


def stage_rows(out,version):
    path=out/(slug(version)+'.jsonl')
    return [json.loads(line) for line in path.read_text().splitlines()] if path.is_file() else []


def freeze_targets(manifest):
    out=Path(manifest['out']);root=Path(manifest['root'])
    if manifest['targets_frozen']:return
    target_dir=out/'targets';target_dir.mkdir(exist_ok=True)
    for index,item in enumerate(manifest['units']):
        path=target_dir/f'{index:04d}.o';shutil.copyfile(root/item['unit']['target_path'],path)
        item['target']=str(path);item['target_sha256']=sha(path)
    manifest['targets_frozen']=True;write_json(out/'manifest.json',manifest)


def compile_unit(manifest,item,version,deadline):
    if time.monotonic()>=deadline-2:return None
    if shutil.disk_usage(manifest['out']).free<50_000_000_000:raise ValueError('less than 50 GB free')
    root=Path(manifest['root']);edge=item['edge'];unit=item['unit']
    if sha(root/edge['source'])!=item['source_sha256']:raise ValueError('source changed: '+edge['source'])
    started=time.monotonic()
    with tempfile.TemporaryDirectory(prefix='decomp-gametrial-') as directory:
        work=Path(directory);output=work/'obj';output.mkdir()
        argv=compiler_argv(edge,version,output)
        result=fnsearch.run(argv,root,max(.05,min(40,deadline-time.monotonic())))
        if result['returncode']==124 and time.monotonic()>=deadline-.1:return None
        row=dict(unit=unit['name'],version=version,compile_seconds=result['seconds'],status='compile_failed',
                 baseline_compiler=edge['argv'][edge['compiler_index']],source_sha256=item['source_sha256'])
        diagnostic=result['stdout']+result['stderr']
        if result['returncode'] or flagsearch.UNSUPPORTED.search(diagnostic):
            row['diagnostic']=diagnostic[-2400:];return row
        obj=output/(Path(edge['source']).stem+'.o')
        if not obj.is_file():row['diagnostic']='compiler object missing';return row
        for tail in edge['tails']:
            if len(tail)==7:
                post=fnsearch.run([str(root/tail[0]),*tail[1:5],str(obj),str(obj)],root,max(.05,min(30,deadline-time.monotonic())))
                if post['returncode']==124 and time.monotonic()>=deadline-.1:return None
                if post['returncode']:row.update(status='postprocess_failed',diagnostic=post['stderr'][-2400:]);return row
        row['object_sha256']=sha(obj)
        project=work/'project';project.mkdir()
        (project/'objdiff.json').write_text(json.dumps({'units':[{**unit,'target_path':item['target'],'base_path':str(obj)}]}))
        report=fnsearch.run([str(root/'build/tools/objdiff-cli'),'-L','error','report','generate','-c','functionRelocDiffs=none','-p',str(project),'-o','-'],root,max(.05,min(30,deadline-time.monotonic())))
        row['report_seconds']=report['seconds']
        if report['returncode']==124 and time.monotonic()>=deadline-.1:return None
        if report['returncode']:row.update(status='report_failed',diagnostic=report['stderr'][-2400:]);return row
        row.update(status='scored',scores=report_scores(json.loads(report['stdout'])),wall_seconds=time.monotonic()-started)
        return row


def run_stage(args):
    out=args.out.resolve();manifest=json.loads((out/'manifest.json').read_text());check_inputs(manifest)
    if os.environ.get('GAMETRIAL_HEAVY_LOCKED')!=str(LOCK):raise ValueError('stage requires the shared heavy lock')
    start=time.monotonic();freeze_targets(manifest)
    existing={r['unit'] for r in stage_rows(out,args.version)}
    pending=[item for item in manifest['units'] if item['unit']['name'] not in existing]
    baseline={r['unit']:r for r in stage_rows(out,'baseline')}
    deadline=start+manifest['hold_seconds'];completed=0
    path=out/(slug(args.version)+'.jsonl')
    with path.open('a') as log,ThreadPoolExecutor(manifest['workers']) as executor:
        for row in executor.map(lambda item:compile_unit(manifest,item,args.version,deadline),pending):
            if row is None:continue
            if args.version!='baseline' and row['status']=='scored':
                before=baseline.get(row['unit'])
                if before and before['status']=='scored':row['comparison']=compare(before['scores'],row['scores'])
                else:row['status']='baseline_unavailable'
            log.write(json.dumps(row)+'\n');log.flush();completed+=1
            if completed%100==0:print(args.version,len(existing)+completed,'/',len(manifest['units']),flush=True)
    check_inputs(manifest)
    print(f"Released {args.version}: {completed} new units; held {time.monotonic()-start:.2f}s",flush=True)


def config_patch(source,winners):
    tree=ast.parse(source);lines=source.splitlines(keepends=True);starts=[0]
    for line in lines:starts.append(starts[-1]+len(line))
    def offset(node,end=False):return starts[(node.end_lineno if end else node.lineno)-1]+(node.end_col_offset if end else node.col_offset)
    changes=[];seen=set()
    for node in ast.walk(tree):
        if not isinstance(node,ast.Call) or not isinstance(node.func,ast.Name) or node.func.id!='Object' or len(node.args)<2:continue
        name=node.args[1]
        if not isinstance(name,ast.Constant) or not isinstance(name.value,str):continue
        unit='main/'+name.value.removesuffix('.cpp').removesuffix('.c')
        if unit not in winners:continue
        if unit in seen:raise ValueError('duplicate Object call: '+unit)
        seen.add(unit);text=json.dumps(winners[unit])
        keyword=next((k for k in node.keywords if k.arg=='mw_version'),None)
        if keyword:changes.append((offset(keyword.value),offset(keyword.value,True),text))
        else:
            last=node.keywords[-1].value if node.keywords else node.args[-1]
            at=offset(last,True);changes.append((at,at,', mw_version='+text))
    missing=set(winners)-seen
    if missing:raise ValueError('Object call absent: '+', '.join(sorted(missing)))
    result=source
    for a,b,text in sorted(changes,reverse=True):result=result[:a]+text+result[b:]
    ast.parse(result)
    return ''.join(difflib.unified_diff(source.splitlines(keepends=True),result.splitlines(keepends=True),fromfile='a/configure.py',tofile='b/configure.py'))


def summarize(args):
    out=args.out.resolve();manifest=json.loads((out/'manifest.json').read_text())
    baseline={r['unit']:r for r in stage_rows(out,'baseline')};candidates=[];all_rows=[];stats=[]
    for version in manifest['versions']:
        rows=stage_rows(out,version);all_rows.extend(rows)
        gains=[r for r in rows if r.get('comparison',{}).get('strict_gain')]
        for row in gains:
            c=row['comparison'];b=baseline[row['unit']]['scores'];a=row['scores']
            candidates.append(dict(unit=row['unit'],version=version,baseline_compiler=row['baseline_compiler'],
                                   baseline_matched_code=b['matched_code'],variant_matched_code=a['matched_code'],matched_code_gain=c['matched_code_gain'],
                                   matched_data_gain=c['matched_data_gain'],new_exact_bytes=c['new_exact_bytes'],improved_functions=len(c['improved_functions']),
                                   improved_data_sections=len(c['improved_data_sections']),fuzzy_code_gain=c['fuzzy_code_gain'],
                                   function_drops=0,data_section_drops=0,compile_seconds=row['compile_seconds']))
        times=[r['compile_seconds'] for r in rows]
        stats.append(dict(version=version,attempted=len(rows),scored=sum(r['status']=='scored' for r in rows),strict_gains=len(gains),
                          code_gain_units=sum(r.get('comparison',{}).get('matched_code_gain',0)>0 and r['comparison']['strict_gain'] for r in rows),
                          mean_compile_seconds=statistics.mean(times) if times else None,median_compile_seconds=statistics.median(times) if times else None))
    candidates.sort(key=lambda r:(-r['matched_code_gain'],-r['matched_data_gain'],-r['fuzzy_code_gain'],r['unit'],r['version']))
    columns=['unit','version','baseline_compiler','baseline_matched_code','variant_matched_code','matched_code_gain','matched_data_gain','new_exact_bytes','improved_functions','improved_data_sections','fuzzy_code_gain','function_drops','data_section_drops','compile_seconds']
    with (out/'gametrial.tsv').open('w') as f:
        w=csv.DictWriter(f,fieldnames=columns,delimiter='\t',lineterminator='\n');w.writeheader();w.writerows(candidates)
    with (out/'all-trials.tsv').open('w') as f:
        columns=['unit','version','status','strict_gain','matched_code_gain','matched_data_gain','function_drops','data_section_drops','compile_seconds']
        w=csv.DictWriter(f,fieldnames=columns,delimiter='\t',lineterminator='\n');w.writeheader()
        for row in all_rows:
            comparison=row.get('comparison',{})
            w.writerow(dict(unit=row['unit'],version=row['version'],status=row['status'],strict_gain=comparison.get('strict_gain',False),
                            matched_code_gain=comparison.get('matched_code_gain',''),matched_data_gain=comparison.get('matched_data_gain',''),
                            function_drops=len(comparison.get('function_drops',{})),data_section_drops=len(comparison.get('data_section_drops',{})),compile_seconds=row['compile_seconds']))
    winners={}
    for row in candidates:
        if row['unit'] not in winners:winners[row['unit']]=row['version']
    source=(out/'inputs/configure.py').read_text();patch=config_patch(source,winners)
    (out/'best-config.patch').write_text(patch)
    summary=dict(source_state=manifest['source_state'],units=len(manifest['units']),skipped=manifest['skipped'],
                 baseline_scored=sum(r['status']=='scored' for r in baseline.values()),versions=stats,candidates=len(candidates),best_units=len(winners),
                 baseline_failures=[dict(unit=r['unit'],status=r['status'],diagnostic=r.get('diagnostic','')) for r in baseline.values() if r['status']!='scored'],
                 complete=len(baseline)==len(manifest['units']) and all(s['attempted']==len(manifest['units']) for s in stats),
                 scoring_config=manifest['scoring_config'],source_mutations=False)
    write_json(out/'summary.json',summary)
    lines=['# Game compiler trial','',f"Frozen source: `{manifest['source_state']['revision']}`. {len(manifest['units'])} Game units, {len(manifest['skipped'])} skipped entries. Fresh default-compiler control; compiler flags are identical across versions. No source or config edits applied.",'',
           '| Version | Attempted | Scored | Strict gain units | Units gaining matched code | Median compile seconds |','|---|---:|---:|---:|---:|---:|']
    for stat in stats:lines.append(f"| {stat['version']} | {stat['attempted']} | {stat['scored']} | {stat['strict_gains']} | {stat['code_gain_units']} | {stat['median_compile_seconds']} |")
    lines+=['','Strict gains have zero per-function and data-section score drops, and no matched-code/data byte drop. Fuzzy-only gains are included and identified by zero matched-code gain. Scores use report mode with `functionRelocDiffs=none`. Baseline/variant JSONL files retain all individual function/section scores and rejection evidence.','',
            '`best-config.patch` chooses the best strict survivor per unit. It changes only `mw_version` keywords, applies nothing, and requires the lander to validate against its current source/target snapshot. Whole-unit private compiler outputs are deleted after each score. Frozen target copies may be removed after this trial is complete.']
    (out/'gametrial-summary.md').write_text('\n'.join(lines)+'\n')
    args.publish.resolve().mkdir(parents=True,exist_ok=True)
    for name in ('gametrial.tsv','gametrial-summary.md'):shutil.copyfile(out/name,args.publish.resolve()/name)
    if summary['complete'] and (out/'targets').is_dir():
        targets=out/'targets'
        if targets.is_symlink() or any(Path(item['target']).parent!=targets for item in manifest['units']):
            raise ValueError('refusing cleanup outside the private target snapshot')
        shutil.rmtree(targets);manifest['targets_removed']=True;write_json(out/'manifest.json',manifest)
    print(json.dumps(summary,indent=2),flush=True)


def run(args):
    out=args.out.resolve();manifest=json.loads((out/'manifest.json').read_text())
    for version in ['baseline',*manifest['versions']]:
        while len(stage_rows(out,version))<len(manifest['units']):
            check_inputs(manifest)
            command=['lockf','-k',str(LOCK),sys.executable,str(Path(__file__).resolve()),'stage','--out',str(out),'--version',version]
            print('Waiting for server-heavy.lock:',version,flush=True)
            result=subprocess.run(command,env={**os.environ,'GAMETRIAL_HEAVY_LOCKED':str(LOCK)})
            if result.returncode:raise ValueError('stage failed: '+version)
            time.sleep(manifest['pause_seconds'])
            manifest=json.loads((out/'manifest.json').read_text())
    summarize(args)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action',choices=['prepare','run','stage','summarize'])
    parser.add_argument('--root',type=Path,default=Path.cwd())
    parser.add_argument('--edge-root',type=Path,default=Path.home()/'MacioGalaxy2-wt/decomp-land')
    parser.add_argument('--out',type=Path,default=SCRATCH/'gametrial')
    parser.add_argument('--publish',type=Path,default=SCRATCH)
    parser.add_argument('--versions',nargs='+',default=['Wii/1.1','Wii/1.3','Wii/0x4201_127'])
    parser.add_argument('--workers',type=int,default=14)
    parser.add_argument('--hold-seconds',type=float,default=480)
    parser.add_argument('--pause-seconds',type=float,default=20)
    parser.add_argument('--version')
    args=parser.parse_args()
    if not 1<=args.workers<=14 or not 0<args.hold_seconds<=540 or args.pause_seconds<10:parser.error('workers 1..14, holds >0..540s, pauses >=10s')
    if any(v not in ('Wii/1.1','Wii/1.3','Wii/0x4201_127') for v in args.versions):parser.error('unsupported trial compiler')
    if args.action=='stage' and args.version not in ('baseline','Wii/1.1','Wii/1.3','Wii/0x4201_127'):parser.error('invalid stage version')
    return {'prepare':prepare,'run':run,'stage':run_stage,'summarize':summarize}[args.action](args)


if __name__=='__main__':
    try:main()
    except (ValueError,OSError) as error:raise SystemExit('gametrial: '+str(error))
