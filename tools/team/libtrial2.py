#!/usr/bin/env python3
"""Report-mode compiler matrix for configured non-Game library units.

Uses private compiler outputs and frozen controls. No configure, Ninja, split,
source edits or application of generated patches.
"""
from __future__ import annotations
import argparse
import ast
from concurrent.futures import ThreadPoolExecutor
import csv
import json
import os
from pathlib import Path
import shutil
import statistics
import subprocess
import sys
import tempfile
import time

import fnsearch
import flagsearch
import gametrial as g

OUT = g.SCRATCH / 'libtrial2'
GROUPS = {'JSystem', 'nw4r', 'RVL_SDK', 'RVLFaceLib', 'Runtime', 'MSL'}
WII = ['Wii/1.0', 'Wii/1.1', 'Wii/1.3', 'Wii/0x4201_127']
GC = ['GC/3.0a3', 'GC/2.7']
QUIET_LOCK = Path.home() / 'MacioGalaxy2-team/slots/decomp-quiet.lock'


def group_of(name):
    parts = name.split('/')
    group = parts[1] if len(parts) > 1 else ''
    return 'MSL' if group.startswith('MSL') else group


def version_of(edge):
    return '/'.join(Path(edge['argv'][edge['compiler_index']]).parts[-3:-1])


def library_config_key(source):
    tree = ast.parse(source)
    tree.body = [node for node in tree.body
                 if not (isinstance(node, ast.FunctionDef) and node.name == 'GameLib')
                 and not (isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'GAME_MW10_EXCEPTIONS' for t in node.targets))]
    return ast.dump(tree, include_attributes=False)


def prepare(args):
    root, edge_root, out = args.root.resolve(), args.edge_root.resolve(), args.out.resolve()
    if out.exists() and any(out.iterdir()):
        raise ValueError('prepare requires an empty output directory')
    if library_config_key((root/'configure.py').read_text()) != library_config_key((edge_root/'configure.py').read_text()):
        raise ValueError('library configuration differs between source and command graph')
    state = g.source_state(root)
    out.mkdir(parents=True, exist_ok=True)
    inputs = out / 'inputs'; inputs.mkdir()
    for name in ('configure.py', 'build.ninja', 'objdiff.json'):
        shutil.copyfile(edge_root / name, inputs / name)
    cfg = json.loads((inputs / 'objdiff.json').read_text())
    units, skipped, cache, group_compilers = [], [], {}, {}
    for unit in cfg['units']:
        group = group_of(unit['name'])
        if group not in GROUPS:
            continue
        try:
            if not unit.get('base_path'):
                raise ValueError('no configured source object')
            edge = fnsearch.compiler_edge(inputs, unit['base_path'], cache)
            relative = Path(edge['source'])
            if relative.is_absolute() or '..' in relative.parts:
                raise ValueError('source is not repo-relative')
            if not (root / relative).is_file():
                raise ValueError('source absent: ' + str(relative))
            if any(str(edge_root) in arg for arg in edge['argv']):
                raise ValueError('absolute command-graph-owner path')
            if not (root / unit['target_path']).is_file():
                raise ValueError('target object absent')
            original = version_of(edge)
            group_compilers.setdefault(group, set()).add(original)
            units.append(dict(unit=unit, edge=edge, group=group, configured_version=original,
                              source_sha256=g.sha(root / relative)))
        except ValueError as error:
            skipped.append(dict(unit=unit['name'], reason=str(error)))
    for item in units:
        item['versions'] = WII + [version for version in GC if version in group_compilers[item['group']]]
    pins = {}
    for path in [Path(__file__), Path(g.__file__), Path(fnsearch.__file__), Path(flagsearch.__file__), root/'configure.py', *inputs.iterdir()]:
        pins[str(path.resolve())] = g.sha(path)
    tools = {'build/tools/wibo', 'build/tools/wibo.real', 'build/tools/sjiswrap.exe', 'build/tools/objdiff-cli', 'build/tools/dtk'}
    versions = set()
    for item in units:
        tools.add(item['edge']['argv'][item['edge']['compiler_index']])
        versions.update(item['versions'])
    tools.update(str(Path('build/compilers') / version / 'mwcceppc.exe') for version in versions)
    for tool in tools:
        path = (root / tool).resolve()
        if not path.is_file():
            raise ValueError('required executable absent: ' + str(path))
        pins[str(path)] = g.sha(path)
    manifest = dict(root=str(root), edge_root=str(edge_root), out=str(out), source_state=state,
                    edge_revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=edge_root, text=True).strip(),
                    units=units, skipped=skipped, pins=pins, targets_frozen=False,
                    versions=[v for v in WII + GC if v in versions],
                    group_compilers={k: sorted(v) for k, v in group_compilers.items()},
                    workers=args.workers, hold_seconds=args.hold_seconds, pause_seconds=20,
                    scoring_config={'functionRelocDiffs': 'none'})
    g.write_json(out / 'manifest.json', manifest)
    print(json.dumps(dict(units=len(units), skipped=skipped, group_compilers=manifest['group_compilers'], versions=manifest['versions']), indent=2), flush=True)


def expected(manifest, version):
    return [item for item in manifest['units'] if version == 'baseline' or version in item['versions']]


def compile_unit(manifest, item, version, deadline, serial):
    out, root = Path(manifest['out']), Path(manifest['root'])
    if time.monotonic() >= deadline - 2 or (not serial and (out / 'quiet').exists()):
        return None
    if shutil.disk_usage(out).free < 50_000_000_000:
        raise ValueError('less than 50 GB free')
    edge, unit = item['edge'], item['unit']
    if g.sha(root / edge['source']) != item['source_sha256']:
        raise ValueError('source changed: ' + edge['source'])
    with tempfile.TemporaryDirectory(prefix='decomp-libtrial2-') as directory:
        work = Path(directory); dest = work/'obj'; dest.mkdir()
        argv = g.compiler_argv(edge, version, dest)
        if serial:
            argv = ['lockf', '-k', str(QUIET_LOCK), *argv]
        result = fnsearch.run(argv, root, max(.05, min(45, deadline-time.monotonic())))
        if result['returncode'] == 124 and time.monotonic() >= deadline-.1:
            return None
        row = dict(unit=unit['name'], group=item['group'], version=version, configured_version=item['configured_version'],
                   compile_seconds=result['seconds'], status='compile_failed', source_sha256=item['source_sha256'])
        diagnostic = result['stdout']+result['stderr']
        if result['returncode'] or flagsearch.UNSUPPORTED.search(diagnostic):
            row['diagnostic'] = diagnostic[-4000:]; return row
        obj = dest/(Path(edge['source']).stem+'.o')
        if not obj.is_file():
            row['diagnostic']='compiler object missing'; return row
        for tail in edge['tails']:
            if len(tail) == 7:
                post = fnsearch.run([str(root/tail[0]), *tail[1:5], str(obj), str(obj)], root,
                                   max(.05, min(30, deadline-time.monotonic())))
                if post['returncode'] == 124 and time.monotonic() >= deadline-.1:
                    return None
                if post['returncode']:
                    row.update(status='postprocess_failed', diagnostic=post['stderr'][-4000:]); return row
        row['object_sha256'] = g.sha(obj)
        project = work/'project'; project.mkdir()
        (project/'objdiff.json').write_text(json.dumps({'units': [{**unit, 'target_path': item['target'], 'base_path': str(obj)}]}))
        report = fnsearch.run([str(root/'build/tools/objdiff-cli'), '-L', 'error', 'report', 'generate',
                              '-c', 'functionRelocDiffs=none', '-p', str(project), '-o', '-'], root,
                             max(.05, min(30, deadline-time.monotonic())))
        if report['returncode'] == 124 and time.monotonic() >= deadline-.1:
            return None
        if report['returncode']:
            row.update(status='report_failed', diagnostic=report['stderr'][-4000:]); return row
        row.update(status='scored', report_seconds=report['seconds'], scores=g.report_scores(json.loads(report['stdout'])))
        return row


def stage(args):
    out = args.out.resolve(); manifest = json.loads((out/'manifest.json').read_text()); g.check_inputs(manifest)
    serial = args.serial or (out/'quiet').exists()
    if not serial and os.environ.get('LIBTRIAL2_HEAVY_LOCKED') != str(g.LOCK):
        raise ValueError('parallel stage requires shared heavy lock')
    start = time.monotonic(); g.freeze_targets(manifest)
    existing = {row['unit'] for row in g.stage_rows(out, args.version)}
    pending = [item for item in expected(manifest, args.version) if item['unit']['name'] not in existing]
    baseline = {row['unit']:row for row in g.stage_rows(out, 'baseline')}
    deadline = start+manifest['hold_seconds']; count=0
    workers = 1 if serial else manifest['workers']
    print(f'Start {args.version}: {workers} workers, {len(pending)} pending', flush=True)
    with (out/(g.slug(args.version)+'.jsonl')).open('a') as log, ThreadPoolExecutor(workers) as executor:
        for row in executor.map(lambda item:compile_unit(manifest, item, args.version, deadline, serial), pending):
            if row is None: continue
            if args.version != 'baseline' and row['status']=='scored':
                prior = baseline.get(row['unit'])
                if prior and prior['status']=='scored': row['comparison']=g.compare(prior['scores'], row['scores'])
                else: row['status']='baseline_unavailable'
            log.write(json.dumps(row)+'\n'); log.flush(); count+=1
            if count%100==0: print(args.version,len(existing)+count,'/',len(expected(manifest,args.version)),flush=True)
    g.check_inputs(manifest)
    print(f'Released {args.version}: {count} units, {time.monotonic()-start:.2f}s',flush=True)


def summarize(args):
    out=args.out.resolve(); manifest=json.loads((out/'manifest.json').read_text())
    base={r['unit']:r for r in g.stage_rows(out,'baseline')}; all_rows=[]; survivors=[]; stats=[]
    for version in manifest['versions']:
        rows=g.stage_rows(out,version); all_rows.extend(rows)
        good=[r for r in rows if r.get('comparison',{}).get('strict_gain')]
        for row in good:
            c=row['comparison']
            survivors.append(dict(unit=row['unit'],group=row['group'],configured_version=row['configured_version'],version=version,
                matched_code_gain=c['matched_code_gain'],matched_data_gain=c['matched_data_gain'],new_exact_bytes=c['new_exact_bytes'],
                fuzzy_code_gain=c['fuzzy_code_gain'],improved_functions=len(c['improved_functions']),improved_data_sections=len(c['improved_data_sections']),
                function_drops=0,data_section_drops=0))
        times=[r['compile_seconds'] for r in rows]
        stats.append(dict(version=version,expected=len(expected(manifest,version)),attempted=len(rows),scored=sum(r['status']=='scored' for r in rows),
            strict_survivors=len(good),survivor_code_gain=sum(r['comparison']['matched_code_gain'] for r in good),
            survivor_data_gain=sum(r['comparison']['matched_data_gain'] for r in good),
            drop_units=sum(bool(r.get('comparison',{}).get('function_drops') or r.get('comparison',{}).get('data_section_drops')) for r in rows),
            mean_compile_seconds=statistics.mean(times) if times else None,median_compile_seconds=statistics.median(times) if times else None))
    survivors.sort(key=lambda r:(-r['matched_code_gain'],-r['matched_data_gain'],-r['fuzzy_code_gain'],r['unit'],r['version']))
    columns=['unit','group','configured_version','version','matched_code_gain','matched_data_gain','new_exact_bytes','fuzzy_code_gain','improved_functions','improved_data_sections','function_drops','data_section_drops']
    with (out/'libtrial2.tsv').open('w') as stream:
        writer=csv.DictWriter(stream,fieldnames=columns,delimiter='\t',lineterminator='\n');writer.writeheader();writer.writerows(survivors)
    winners={}
    for row in survivors: winners.setdefault(row['unit'],row['version'])
    (out/'best-config.patch').write_text(g.config_patch((out/'inputs/configure.py').read_text(),winners))
    failures=[dict(unit=r['unit'],version=r['version'],status=r['status'],diagnostic=r.get('diagnostic','')) for r in list(base.values())+all_rows if r['status']!='scored']
    summary=dict(units=len(manifest['units']),baseline_scored=sum(r['status']=='scored' for r in base.values()),skipped=manifest['skipped'],
        versions=stats,candidates=len(survivors),best_units=len(winners),failures=failures,source_state=manifest['source_state'],
        complete=len(base)==len(manifest['units']) and all(s['attempted']==s['expected'] for s in stats),scoring_config=manifest['scoring_config'])
    g.write_json(out/'summary.json',summary)
    lines=['# Library compiler trial, report mode','',f"{summary['units']} configured units; {summary['baseline_scored']} fresh current-compiler controls. Source `{manifest['source_state']['revision']}`. Exact configured flags are retained; only compiler and private output operands change.",'',
        '| Compiler | Expected | Scored | Strict survivor units | Survivor code gain | Survivor data gain | Drop units |',
        '|---|---:|---:|---:|---:|---:|---:|']
    for s in stats: lines.append(f"| {s['version']} | {s['expected']} | {s['scored']} | {s['strict_survivors']} | {s['survivor_code_gain']} | {s['survivor_data_gain']} | {s['drop_units']} |")
    lines+=['',f"{len(winners)} best units. Zero function/data-section drops and nondecreasing matched bytes are required. The TSV includes fuzzy-only survivors. Per-unit/function/section scores and diagnostics are preserved in JSONL. `best-config.patch` is unapplied and requires current-branch validation. MSL includes MSL_C. GC versions are trialled only in groups already using that compiler version.",'',f"Failures: {len(failures)}. Skipped source entries: {len(manifest['skipped'])}. Each scored object/dependency directory is deleted immediately."]
    (out/'summary.md').write_text('\n'.join(lines)+'\n')
    if summary['complete'] and (out/'targets').is_dir():
        target=out/'targets'
        if target.is_symlink() or any(Path(i['target']).parent!=target for i in manifest['units']): raise ValueError('unsafe target cleanup')
        shutil.rmtree(target);manifest['targets_removed']=True;g.write_json(out/'manifest.json',manifest)
    print(json.dumps(summary,indent=2),flush=True)


def run(args):
    out=args.out.resolve();manifest=json.loads((out/'manifest.json').read_text())
    for version in ['baseline',*manifest['versions']]:
        while len(g.stage_rows(out,version))<len(expected(manifest,version)):
            g.check_inputs(manifest)
            serial=args.serial or (out/'quiet').exists()
            command=[sys.executable,str(Path(__file__).resolve()),'stage','--out',str(out),'--version',version]
            if serial: command.append('--serial')
            else: command=['lockf','-k',str(g.LOCK),*command]
            print('Waiting/staging',version,'serial' if serial else 'server-heavy.lock',flush=True)
            result=subprocess.run(command,env={**os.environ,'LIBTRIAL2_HEAVY_LOCKED':str(g.LOCK)})
            if result.returncode: raise ValueError('stage failed: '+version)
            time.sleep(manifest['pause_seconds']);manifest=json.loads((out/'manifest.json').read_text())
    summarize(args)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action',choices=['prepare','run','stage','summarize'])
    parser.add_argument('--root',type=Path,default=Path.cwd())
    parser.add_argument('--edge-root',type=Path,default=Path.home()/'MacioGalaxy2-wt/decomp-land')
    parser.add_argument('--out',type=Path,default=OUT)
    parser.add_argument('--workers',type=int,default=14)
    parser.add_argument('--hold-seconds',type=float,default=480)
    parser.add_argument('--serial',action='store_true',help='one worker; every compiler takes decomp-quiet.lock; no parallel heavy hold')
    parser.add_argument('--version')
    args=parser.parse_args()
    if not 1<=args.workers<=14 or not 0<args.hold_seconds<=540: parser.error('workers 1..14; holds >0..540s')
    if args.action=='stage' and args.version not in ['baseline',*WII,*GC]: parser.error('unsupported stage compiler')
    {'prepare':prepare,'run':run,'stage':stage,'summarize':summarize}[args.action](args)


if __name__=='__main__':
    try: main()
    except (ValueError,OSError) as error: raise SystemExit('libtrial2: '+str(error))
