#!/usr/bin/env python3
"""Private, resumable report-parity MWCC configuration archaeology.

prepare freezes configured edges and targets; probe checks compiler acceptance;
run scores fresh controls and each accepted variant. Serial is the default and
locks every compiler invocation with decomp-quiet.lock. Only an explicit
parallel-enabled marker allows four workers. No configure, Ninja, or split.
"""
from __future__ import annotations

import argparse
import ast
from collections import Counter, defaultdict
from concurrent.futures import ThreadPoolExecutor
import csv
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time

import flagsearch
import fnsearch
import gametrial as g

OUT = Path.home() / 'MacioGalaxy2-team/scratch-decomp3/cfgtrial'
QUIET_LOCK = Path.home() / 'MacioGalaxy2-team/slots/decomp-quiet.lock'


def variants():
    result = []
    def add(name, family, argv=(), pragma=None):
        result.append(dict(name=name, family=family, argv=list(argv), pragma=pragma))
    for value in ('nobottomup', 'noauto', 'smart', 'deferred', 'level=0', 'level=2', 'level=4', 'level=8'):
        add('inline-' + value.replace('=', '-'), 'inline', ['-inline', value])
    for value in ('function', 'file', 'program'):
        add('ipa-' + value, 'ipa', ['-ipa', value])
    for value in ('p', 's'):
        add('O4-' + value, 'optimizer', ['-O4,' + value])
    add('O3', 'optimizer', ['-O3'])
    for value in (0, 1, 2, 3, 4):
        add('opt-level-' + str(value), 'optimizer', ['-opt', 'level=' + str(value)])
    for value in ('nopeephole', 'noschedule', 'nocse', 'noloop', 'nopropagation', 'nolifetimes', 'nocse,nopropagation', 'noloop,nocse'):
        add('opt-' + value.replace(',', '-'), 'optimizer', ['-opt', value])
    add('schedule-off', 'optimizer', ['-schedule', 'off'])
    add('nopeephole-schedule-off', 'optimizer', ['-opt', 'nopeephole', '-schedule', 'off'])
    for value in ('on', 'off'):
        add('fp-contract-' + value, 'fp', ['-fp_contract', value])
    for value in ('hard', 'fmadd'):
        add('fp-' + value, 'fp', ['-fp', value])
    for value in ('reuse', 'pool', 'readonly', 'noreuse'):
        add('str-' + value, 'misc', ['-str', value])
    for flag in ('sdata', 'sdata2'):
        for value in (0, 8):
            add(flag + '-' + str(value), 'misc', ['-' + flag, str(value)])
    add('enum-min', 'misc', ['-enum', 'min'])
    add('exceptions-on', 'misc', ['-Cpp_exceptions', 'on'])
    add('RTTI-on', 'misc', ['-RTTI', 'on'])
    for value in (4, 8, 16, 32):
        add('func-align-' + str(value), 'misc', ['-func_align', str(value)])
    for value in (0, 2, 4, 8):
        add('pragma-inline-depth-' + str(value), 'inline', pragma='inline_depth(' + str(value) + ')')
    for value in (0, 8, 16, 32, 64, 128, 256):
        add('pragma-inline-max-size-' + str(value), 'inline', pragma='inline_max_size(' + str(value) + ')')
    for name in ('auto_inline', 'optimize_for_size', 'peephole', 'scheduling', 'fp_contract'):
        for value in ('on', 'off'):
            add('pragma-' + name.replace('_', '-') + '-' + value,
                'fp' if name == 'fp_contract' else 'inline' if name == 'auto_inline' else 'optimizer',
                pragma=name + ' ' + value)
    for value in (2, 3, 4):
        add('pragma-optimization-level-' + str(value), 'optimizer', pragma='optimization_level ' + str(value))
    order = {'inline': 0, 'ipa': 1, 'optimizer': 2, 'fp': 3, 'misc': 4}
    return sorted(result, key=lambda v: order[v['family']])


def report_scores(report):
    """Keep repeated names separate using target address and occurrence."""
    if len(report.get('units', [])) != 1:
        raise ValueError('expected exactly one unit')
    unit = report['units'][0]
    def entries(values):
        counts, result = Counter(), {}
        for value in values:
            address = value.get('metadata', {}).get('virtual_address', value.get('address', ''))
            identity = value['name'] + '@' + str(address)
            counts[identity] += 1
            key = identity + '#' + str(counts[identity])
            result[key] = dict(name=value['name'], score=float(value.get('fuzzy_match_percent') or 0), size=int(value['size']))
        return result
    functions = entries(unit.get('functions', []))
    sections = entries([s for s in unit.get('sections', []) if s['name'] not in g.CODE])
    measures = unit.get('measures', {})
    return dict(functions=functions, sections=sections,
                matched_code=int(measures.get('matched_code', sum(f['size'] for f in functions.values() if f['score'] == 100))),
                matched_data=int(measures.get('matched_data', sum(s['size'] for s in sections.values() if s['score'] == 100))),
                unit_fuzzy=float(measures.get('fuzzy_match_percent') or 0))


def compare(before, after):
    result = g.compare(before, after)
    pairs = [(key, value, after['functions'].get(key, {}).get('score', 0)) for key, value in before['functions'].items()]
    result.update(functions_better=sum(v > f['score'] for _, f, v in pairs),
                  functions_worse=sum(v < f['score'] for _, f, v in pairs),
                  functions_equal=sum(v == f['score'] for _, f, v in pairs),
                  exact_functions_lost=[key for key, f, v in pairs if f['score'] == 100 and v < 100],
                  unit_fuzzy_before=before['unit_fuzzy'], unit_fuzzy_after=after['unit_fuzzy'],
                  fuzzy_net_bytes=sum(f['size'] * (v - f['score']) / 100 for _, f, v in pairs))
    result['zero_drops'] &= after['unit_fuzzy'] >= before['unit_fuzzy']
    result['strict_gain'] &= result['zero_drops']
    return result


def read_rows(out, name):
    path = out / (name + '.jsonl')
    return [json.loads(line) for line in path.read_text().splitlines()] if path.exists() else []


def check(manifest):
    g.check_inputs(manifest)
    for item in manifest['units']:
        if g.sha(item['target']) != item['target_sha256']:
            raise ValueError('frozen target changed: ' + item['target'])


def prepare(args):
    root, out = args.root.resolve(), args.out.resolve()
    state = g.source_state(root)
    if (out / 'manifest.json').exists():
        raise ValueError('manifest already exists; use a fresh directory')
    out.mkdir(parents=True, exist_ok=True)
    inputs = out / 'inputs'; inputs.mkdir()
    for name in ('configure.py', 'build.ninja', 'objdiff.json'):
        shutil.copyfile(root / name, inputs / name)
    requested = {line.split('\t')[0] for line in args.residue.read_text().splitlines() if line.strip()}
    tree = ast.parse((root / 'configure.py').read_text())
    pins10 = next(ast.literal_eval(n.value) for n in tree.body if isinstance(n, ast.Assign)
                  and any(isinstance(t, ast.Name) and t.id == 'GAME_MW10_EXCEPTIONS' for t in n.targets))
    requested.update('main/' + name.rsplit('.', 1)[0] for name in pins10)
    targets = out / 'targets'; targets.mkdir()
    units, skipped, cache = [], [], {}
    cfg = json.loads((root / 'objdiff.json').read_text())
    for unit in cfg['units']:
        if unit['name'] not in requested:
            continue
        try:
            if not unit.get('base_path'):
                raise ValueError('no configured source object')
            edge = fnsearch.compiler_edge(inputs, unit['base_path'], cache)
            source = Path(edge['source'])
            if source.is_absolute() or '..' in source.parts or not (root / source).is_file():
                raise ValueError('source missing or not repository relative')
            target = targets / (g.slug(unit['name']) + '.o')
            shutil.copyfile(root / unit['target_path'], target)
            units.append(dict(unit=unit, edge=edge, source_sha256=g.sha(root / source),
                              target=str(target), target_sha256=g.sha(target),
                              pinned10=source.relative_to('src').as_posix() in pins10))
        except ValueError as error:
            skipped.append(dict(unit=unit['name'], reason=str(error)))
    seen = {item['unit']['name'] for item in units} | {item['unit'] for item in skipped}
    skipped.extend(dict(unit=n, reason='unit absent from objdiff configuration') for n in sorted(requested - seen))
    pins = {str(path.resolve()): g.sha(path) for path in [Path(__file__), Path(g.__file__), Path(fnsearch.__file__),
            Path(flagsearch.__file__), root / 'configure.py', *inputs.iterdir()]}
    tools = {'build/tools/wibo', 'build/tools/sjiswrap.exe', 'build/tools/objdiff-cli', 'build/tools/dtk',
             'build/compilers/Wii/1.3/mwcceppc.exe'}
    tools.update(i['edge']['argv'][i['edge']['compiler_index']] for i in units)
    for name in tools:
        path = (root / name).resolve(); pins[str(path)] = g.sha(path)
    manifest = dict(root=str(root), out=str(out), source_state=state, pins=pins, units=units, skipped=skipped,
                    variants=variants(), scoring_config={'functionRelocDiffs': 'none'}, workers=4)
    g.write_json(out / 'manifest.json', manifest)
    print(json.dumps(dict(units=len(units), skipped=skipped, pinned10=sum(i['pinned10'] for i in units), variants=len(variants()))), flush=True)


def variant_argv(edge, variant, output, prefix=None, force13=False):
    argv = g.compiler_argv(edge, 'Wii/1.3' if force13 else 'baseline', output)
    extra = list(variant.get('argv', []))
    if variant.get('pragma'):
        if prefix is None:
            raise ValueError('pragma variant requires prefix file')
        path = str(prefix)
        if prefix.is_absolute():
            path = 'Z:' + path.replace('/', '\\')
        extra += ['-prefix', path]
    index = argv.index('-c')
    argv[index:index] = [*extra, '-w', 'illpragmas,cmdline']
    return argv


def compile_one(manifest, item, variant, serial=True, force13=False):
    root, out = Path(manifest['root']), Path(manifest['out'])
    if shutil.disk_usage(out).free < 50_000_000_000:
        raise ValueError('less than 50 GB free')
    if g.sha(root / item['edge']['source']) != item['source_sha256']:
        raise ValueError('source changed: ' + item['edge']['source'])
    with tempfile.TemporaryDirectory(prefix='cfg-', dir='/tmp') as directory:
        work = Path(directory); dest = work / 'obj'; dest.mkdir()
        prefix = work / 'prefix.h'
        if variant.get('pragma'):
            prefix.write_text('#pragma ' + variant['pragma'] + '\n')
        argv = variant_argv(item['edge'], variant, dest, prefix, force13)
        if serial:
            argv = ['lockf', '-k', str(QUIET_LOCK), *argv]
        result = fnsearch.run(argv, root, 180)
        row = dict(unit=item['unit']['name'], variant=variant['name'], family=variant.get('family', 'control'),
                   compiler='Wii/1.3' if force13 else item['edge']['argv'][item['edge']['compiler_index']],
                   source_sha256=item['source_sha256'], compile_seconds=result['seconds'],
                   status='compile_failed', command=argv)
        diagnostic = result['stdout'] + result['stderr']
        if result['returncode'] or flagsearch.UNSUPPORTED.search(diagnostic):
            row['diagnostic'] = diagnostic[-4000:]; return row
        obj = dest / (Path(item['edge']['source']).stem + '.o')
        if not obj.is_file():
            row['diagnostic'] = 'compiler object missing'; return row
        for tail in item['edge']['tails']:
            if len(tail) == 7:
                post = fnsearch.run([str(root / tail[0]), *tail[1:5], str(obj), str(obj)], root, 30)
                if post['returncode']:
                    row.update(status='postprocess_failed', diagnostic=post['stderr'][-4000:]); return row
        row['object_sha256'] = g.sha(obj)
        project = work / 'project'; project.mkdir()
        (project / 'objdiff.json').write_text(json.dumps({'units': [{**item['unit'], 'target_path': item['target'], 'base_path': str(obj)}]}))
        result = fnsearch.run([str(root / 'build/tools/objdiff-cli'), '-L', 'error', 'report', 'generate',
                              '-c', 'functionRelocDiffs=none', '-p', str(project), '-o', '-'], root, 60)
        if result['returncode']:
            row.update(status='report_failed', diagnostic=result['stderr'][-4000:]); return row
        row.update(status='scored', scores=report_scores(json.loads(result['stdout'])), report_seconds=result['seconds'])
        if diagnostic.strip():
            row['diagnostic'] = diagnostic[-4000:]
        if g.sha(root / item['edge']['source']) != item['source_sha256']:
            raise ValueError('source changed during compilation')
        return row


def probe(args):
    out = args.out.resolve(); manifest = json.loads((out / 'manifest.json').read_text()); check(manifest)
    with tempfile.TemporaryDirectory(prefix='cfg-probe-', dir='/tmp') as directory:
        work = Path(directory); source = work / 'probe.cpp'
        source.write_text('inline int leaf(int a) { return a + 1; }\nint probe(int x) { return leaf(x); }\n')
        edge = dict(argv=['build/tools/wibo', 'build/compilers/Wii/1.3/mwcceppc.exe', '-nodefaults', '-proc', 'gekko',
                         '-O4,s', '-inline', 'auto', '-ipa', 'file', '-c', str(source), '-o', str(work)], compiler_index=1)
        all_variants = [dict(name='negative-unknown-pragma', pragma='cfgtrial_unknown_pragma on', argv=[]), *manifest['variants']]
        records = []
        for variant in all_variants:
            prefix = work / 'prefix.h'
            if variant.get('pragma'):
                prefix.write_text('#pragma ' + variant['pragma'] + '\n')
            argv = ['lockf', '-k', str(QUIET_LOCK), *variant_argv(edge, variant, work, prefix)]
            result = fnsearch.run(argv, Path(manifest['root']), 180)
            diagnostic = result['stdout'] + result['stderr']
            obj = work / 'probe.o'
            accepted = not result['returncode'] and not flagsearch.UNSUPPORTED.search(diagnostic) and obj.is_file()
            elf = accepted and obj.read_bytes().startswith(b'\x7fELF')
            records.append(dict(variant=variant['name'], accepted=bool(accepted), reportable=bool(elf), diagnostic=diagnostic, command=argv))
            obj.unlink(missing_ok=True)
        if records[0]['accepted'] or not re.search(r'(?:unrecognized|unknown|illegal).*pragma|pragma.*(?:unrecognized|unknown|illegal)', records[0]['diagnostic'], re.I):
            raise ValueError('unknown-pragma negative control did not produce an unknown-pragma diagnostic: ' + records[0]['diagnostic'])
        if not next(r for r in records if r['variant'] == 'pragma-auto-inline-off')['reportable']:
            raise ValueError('known auto_inline pragma positive control failed')
        g.write_json(out / 'acceptance.json', records)
    print(json.dumps(dict(accepted=sum(r['reportable'] for r in records[1:]), total=len(records)-1,
                         rejected=[r['variant'] for r in records[1:] if not r['reportable']])), flush=True)


def run(args):
    out = args.out.resolve(); manifest = json.loads((out / 'manifest.json').read_text()); check(manifest)
    accepted = {r['variant'] for r in json.loads((out / 'acceptance.json').read_text()) if r['reportable']}
    controls = [dict(name='baseline', argv=[])]
    selected = [v for v in manifest['variants'] if v['name'] in accepted and (not args.variants or v['name'] in args.variants)]
    for variant in controls + selected:
        name = variant['name']; existing = {r['unit']: r for r in read_rows(out, name)}
        baseline = {r['unit']: r for r in read_rows(out, 'baseline')}
        pending = [i for i in manifest['units'] if i['unit']['name'] not in existing]
        if args.limit:
            pending = pending[:args.limit]
        with (out / (name + '.jsonl')).open('a') as stream:
            for start in range(0, len(pending), manifest['workers']):
                check(manifest)
                parallel = (out / 'parallel-enabled').exists()
                batch = pending[start:start + manifest['workers']]
                def work(item):
                    row = compile_one(manifest, item, variant, serial=not parallel)
                    before = baseline.get(row['unit'])
                    if name != 'baseline' and row['status'] == 'scored':
                        if before and before['status'] == 'scored':
                            row['comparison'] = compare(before['scores'], row['scores'])
                            row['identical_to_control'] = row['object_sha256'] == before['object_sha256']
                        else:
                            row['status'] = 'baseline_unavailable'
                    return row
                if parallel:
                    with tempfile.TemporaryDirectory(prefix='cfg-batch-') as directory:
                        taskfile = Path(directory) / 'tasks.json'
                        taskfile.write_text(json.dumps(dict(items=batch, variant=variant, baseline=baseline)))
                        command = ['lockf', '-k', str(g.LOCK), sys.executable, str(Path(__file__).resolve()),
                                   'batch', '--out', str(out), '--tasks', str(taskfile)]
                        completed = subprocess.run(command, text=True, stdout=subprocess.PIPE, check=True)
                        rows = [json.loads(line) for line in completed.stdout.splitlines()]
                else:
                    rows = [work(item) for item in batch]
                for row in rows:
                    stream.write(json.dumps(row) + '\n'); stream.flush()
                count = len(existing) + start + len(batch)
                if count % 40 < len(batch) or count == len(existing) + len(pending):
                    print(name, count, '/', len(manifest['units']), 'parallel' if parallel else 'quiet-serial', flush=True)
                if (out / 'stop').exists():
                    summarize(args); return
        summarize(args)
    retrial_pins(args, manifest, selected)
    summarize(args)
    expected = ['baseline', *accepted]
    if not args.limit and not args.variants and all(len(read_rows(out, name)) == len(manifest['units']) for name in expected):
        (out / 'complete').write_text(time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()) + '\n')


def batch(args):
    manifest = json.loads((args.out / 'manifest.json').read_text()); check(manifest)
    tasks = json.loads(args.tasks.read_text())
    def work(item):
        row = compile_one(manifest, item, tasks['variant'], serial=False)
        before = tasks['baseline'].get(row['unit'])
        if row['variant'] != 'baseline' and row['status'] == 'scored':
            if before and before['status'] == 'scored':
                row['comparison'] = compare(before['scores'], row['scores'])
                row['identical_to_control'] = row['object_sha256'] == before['object_sha256']
            else:
                row['status'] = 'baseline_unavailable'
        return row
    with ThreadPoolExecutor(manifest['workers']) as executor:
        for row in executor.map(work, tasks['items']):
            print(json.dumps(row), flush=True)


def retrial_pins(args, manifest, selected):
    out = args.out.resolve()
    baseline = {r['unit']: r for r in read_rows(out, 'baseline')}
    existing = {(r['unit'], r['variant']) for r in read_rows(out, 'pinned13')}
    with (out / 'pinned13.jsonl').open('a') as stream:
        for variant in [dict(name='wii13-control', argv=[]), *selected]:
            for item in manifest['units']:
                if not item['pinned10'] or (item['unit']['name'], variant['name']) in existing:
                    continue
                row = compile_one(manifest, item, variant, serial=True, force13=True)
                before = baseline.get(row['unit'])
                if row['status'] == 'scored' and before and before['status'] == 'scored':
                    row['comparison'] = compare(before['scores'], row['scores'])
                stream.write(json.dumps(row) + '\n'); stream.flush()
    print('Pinned Wii/1.3 retrials complete', flush=True)


def summarize(args):
    out = args.out.resolve(); manifest = json.loads((out / 'manifest.json').read_text())
    baseline = read_rows(out, 'baseline'); stats, survivors, failures = [], [], []
    for variant in manifest['variants']:
        rows = read_rows(out, variant['name'])
        scored = [r for r in rows if r['status'] == 'scored' and 'comparison' in r]
        strict = [r for r in scored if r['comparison']['strict_gain']]
        stats.append(dict(variant=variant['name'], family=variant['family'], attempted=len(rows), scored=len(scored),
                          identical=sum(r.get('identical_to_control', False) for r in scored), strict=len(strict),
                          code_gain=sum(r['comparison']['matched_code_gain'] for r in strict),
                          data_gain=sum(r['comparison']['matched_data_gain'] for r in strict),
                          drop_units=sum(not r['comparison']['zero_drops'] for r in scored)))
        survivors.extend(dict(unit=r['unit'], variant=r['variant'], **r['comparison']) for r in strict)
        failures.extend(dict(unit=r['unit'], variant=r['variant'], status=r['status'], diagnostic=r.get('diagnostic', ''))
                        for r in rows if r['status'] != 'scored')
    survivors.sort(key=lambda r: (r['matched_code_gain'], r['matched_data_gain'], r['fuzzy_code_gain']), reverse=True)
    winners = {}
    for row in survivors:
        winners.setdefault(row['unit'], row)
    patterns = defaultdict(lambda: dict(units=set(), variants=set(), code_gain=0, fuzzy_gain=0))
    for unit, row in winners.items():
        directory = '/'.join(unit.split('/')[:3])
        pattern = patterns[directory]; pattern['units'].add(unit); pattern['variants'].add(row['variant'])
        pattern['code_gain'] += row['matched_code_gain']; pattern['fuzzy_gain'] += row['fuzzy_net_bytes']
    pattern_rows = [dict(directory=k, units=len(v['units']), variants=sorted(v['variants']), code_gain=v['code_gain'],
                         fuzzy_gain=v['fuzzy_gain']) for k, v in patterns.items()]
    summary = dict(units=len(manifest['units']), baseline_attempted=len(baseline), baseline_scored=sum(r['status'] == 'scored' for r in baseline),
                   variants=stats, survivors=survivors, winners=list(winners.values()), failures=failures, skipped=manifest['skipped'],
                   directory_patterns=pattern_rows,
                   best_code_gain=sum(r['matched_code_gain'] for r in winners.values()),
                   best_data_gain=sum(r['matched_data_gain'] for r in winners.values()))
    g.write_json(out / 'summary.json', summary)
    columns = ['unit', 'variant', 'functions_better', 'functions_worse', 'functions_equal', 'matched_code_gain',
               'matched_data_gain', 'new_exact_bytes', 'fuzzy_net_bytes', 'zero_drops', 'strict_gain',
               'exact_functions_lost', 'function_drops', 'data_section_drops', 'improved_functions', 'improved_data_sections']
    with (out / 'all-trials.tsv').open('w') as stream:
        writer = csv.DictWriter(stream, fieldnames=columns, delimiter='\t', lineterminator='\n', extrasaction='ignore'); writer.writeheader()
        for variant in manifest['variants']:
            for row in read_rows(out, variant['name']):
                if 'comparison' in row:
                    writer.writerow(dict(unit=row['unit'], variant=row['variant'], **row['comparison']))
    lines = ['# Configuration trial (report parity)', '',
             f"Source `{manifest['source_state']['revision']}`. {len(manifest['units'])} units, {summary['baseline_scored']} fresh controls. "
             'Actual configured edges are the baseline; Game uses -O4,s. All function occurrences, every noncode section, unit fuzzy score and matched code/data bytes must not decrease.', '',
             '| Variant | Attempted | Scored | Identical objects | No-drop gains | Code gain | Data gain | Units with drops |',
             '|---|---:|---:|---:|---:|---:|---:|---:|']
    for stat in stats:
        if stat['attempted']:
            lines.append('| ' + ' | '.join(str(stat[k]) for k in ('variant', 'attempted', 'scored', 'identical', 'strict', 'code_gain', 'data_gain', 'drop_units')) + ' |')
    lines += ['', f"Best per unit: {len(winners)} units, +{summary['best_code_gain']} exact code bytes, +{summary['best_data_gain']} exact data bytes.", '',
              '| Unit | Variant | Exact code gain | Exact data gain | Improved functions |', '|---|---|---:|---:|---:|']
    for row in winners.values():
        lines.append(f"| {row['unit']} | {row['variant']} | {row['matched_code_gain']} | {row['matched_data_gain']} | {row['functions_better']} |")
    lines += ['', '## Directory patterns', '']
    for row in sorted(pattern_rows, key=lambda r: r['code_gain'], reverse=True):
        lines.append(f"- {row['directory']}: {row['units']} survivor units, +{row['code_gain']} exact bytes; settings: {', '.join(row['variants'])}.")
    lines += ['', 'JSONL retains every function/section score, command, diagnostic, source/object hash and comparison. '
              'Private objects are removed immediately. No production settings have been applied. '
              'Acceptance probes and unknown-pragma negative control are in acceptance.json. '
              'A missing complete marker means this is a partial matrix. Source variants from miners require separate owner review.', '']
    (out / 'summary.md').write_text('\n'.join(lines))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['prepare', 'probe', 'run', 'batch', 'summary'])
    parser.add_argument('--root', type=Path, default=Path.cwd())
    parser.add_argument('--out', type=Path, default=OUT)
    parser.add_argument('--residue', type=Path, default=OUT.parent / 'lists/residue.tsv')
    parser.add_argument('--variants', nargs='+')
    parser.add_argument('--limit', type=int)
    parser.add_argument('--tasks', type=Path)
    args = parser.parse_args()
    if args.variants:
        unknown = set(args.variants) - {v['name'] for v in variants()}
        if unknown:
            parser.error('unknown variants: ' + ', '.join(sorted(unknown)))
    {'prepare': prepare, 'probe': probe, 'run': run, 'batch': batch, 'summary': summarize}[args.command](args)


if __name__ == '__main__':
    main()
