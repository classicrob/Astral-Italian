#!/usr/bin/env python3
"""Create validated sidecars from TWO identical warm recordings, preserving seed rows/ranks.

--seed-dir V13_EXPANSION --provenance BATCH/PROVENANCE.json [...] --out NEW_DIR
--report OUTSIDE_PACK.json

Creates a new DB+sidecar directory, never edits the input. No recording hashes,
paths or raw JSON enter a sidecar or the resulting seedpack. The report retains
all missing stages, unknown configs and warm disagreements; these are not passes.
Frontend metadata is optional and admitted only if ALL observed warm pairs agree.
"""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import shutil
import re

import pipeline_deadlines as deadlines
import record_pipeline_deadlines as recording
import seedpack


def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def all_groups(path):
    stages = {r['stage'] for r in map(json.loads, Path(path).read_text().splitlines()) if r.get('event') == 'first_use'}
    result = {}
    for stage in stages:
        try: result[stage] = recording.observations(path, stage)
        except ValueError: result[stage] = None
    return result


def build(seeds, provenances, output, report):
    seeds, output, report = Path(seeds).resolve(), Path(output).resolve(), Path(report).resolve()
    if output.exists() or report.exists() or output == seeds or report.is_relative_to(output):
        raise ValueError('output/report must be new; report must stay outside the packed seed directory')
    output.mkdir(parents=True)
    for path in seeds.glob('*.db'): shutil.copyfile(path, output / path.name)
    results, frontend, planned, admitted = [], {}, set(), set()
    dedicated_title = False
    build_identity = None
    provenance_bindings = []
    for path in map(Path, provenances):
        data = json.loads(path.read_text())
        if data.get('version') != 1 or data.get('recording_only') is not True:
            raise ValueError('unsupported/non-recording provenance')
        dedicated = data.get('mode') == 'no-input-title'
        dedicated_title |= dedicated
        identity = data['recording_build_metadata']
        identity = tuple(identity.get(k) for k in ('source', 'binary_sha256', 'renderer_key'))
        if not re.fullmatch(r'[0-9a-f]{40}', identity[0] or '') or any(not re.fullmatch(r'[0-9a-f]{64}', x or '') for x in identity[1:]):
            raise ValueError('invalid recording source/executable/renderer identity')
        if build_identity is not None and identity != build_identity: raise ValueError('recording builds differ across batches')
        build_identity = identity
        if planned & set(data['stages_planned']): raise ValueError('overlapping recording batches')
        planned.update(data['stages_planned'])
        provenance_bindings.append({'path': str(path.resolve()), 'sha256': sha(path),
                                    'recording_build_metadata': data['recording_build_metadata'],
                                    'recording_build_metadata_sha256': data['recording_build_metadata_sha256']})
        for result in data['stage_results']:
            stage = result['stage']
            item = {'stage': stage, 'admitted': False, 'provenance': str(path.resolve())}
            results.append(item)
            try:
                if not result['passed'] or result['warm_disagreements'] or result['cold_disagreements']:
                    raise ValueError('recording stage failed or warm/cold groups disagreed')
                warm = [r for r in result['arms'] if r['name'] in ('warm-1', 'warm-2')]
                if len(warm) != 2 or any(not r['passed'] for r in warm): raise ValueError('two valid warm captures required')
                maps = []
                for arm in warm:
                    folder = path.parent / stage / arm['name']
                    for name, field in [('first-use.jsonl', 'capture_sha256'), ('app.log', 'app_log_sha256'), ('launch.json', 'launch_sha256')]:
                        if sha(folder / name) != arm[field]: raise ValueError('recording raw artifact hash mismatch')
                    launch = json.loads((folder / 'launch.json').read_text())
                    if launch['exe_sha256'] != arm['recording_exe_sha256'] or launch['renderer_key'] != arm['renderer_key']:
                        raise ValueError('recording build/renderer binding mismatch')
                    if any(arm['checks'].get(k) is not True for k in ('valid_capture', 'no_pipeline_waits', 'no_pop_in')):
                        raise ValueError('record was not fully warm for drawn configs')
                    text = (folder / 'app.log').read_text(errors='replace')
                    with (folder / 'frames.csv').open() as file: timing = list(csv.DictReader(file))
                    if not timing or any(float(r['pipeline_wait_ms']) != 0 for r in timing) or re.search(r'^\[gx uber wait\]|^\[pop-in\] frame \d+ skipped_draws [1-9]', text, re.M):
                        raise ValueError('raw log/timing proves pipeline wait or pop-in in warm record')
                    if not dedicated and '[input-script] done after 970 frames' not in text:
                        raise ValueError('perf input script did not complete')
                    if dedicated and ('PETARI_INPUT_SCRIPT' in launch['environment'] or launch.get('mode') != 'no-input-title'):
                        raise ValueError('title observation was not the no-input recording arm')
                    maps.append(all_groups(folder / 'first-use.jsonl'))
                if not maps[0].get(stage) or not maps[1].get(stage): raise ValueError('invalid/multiple target visits')
                differences = recording.disagreements(maps[0][stage], maps[1][stage])
                if differences:
                    item['disagreements'] = differences; raise ValueError('raw warm maps differ')
                rows = seedpack.database_rows(str(seeds / (stage + '.db')))
                values = {int(k, 16): v for k, v in maps[0][stage].items()}
                known = {r[1] & deadlines.MASK for r in rows if r[0] == 1}
                unknown = sorted(k for k in values if k not in known)
                if unknown:
                    item['unknown_configs'] = [f'{k:016x}' for k in unknown]
                    raise ValueError('recorded configurations absent from stage manifest; explicit closure required')
                target = output / (stage + '.first-use.bin')
                target.write_bytes(deadlines.encode(stage, rows, values))
                assert deadlines.read(target, stage, rows) == values
                item.update(admitted=True, configs=len(values), groups=len(set(values.values())),
                            sidecar_sha256=sha(target), source_database_sha256=sha(seeds / (stage + '.db')))
                admitted.add(stage)
                for front in ('Unknown', 'FileSelect'):
                    if maps[0].get(front) and maps[1].get(front):
                        frontend.setdefault(front, []).append((recording.disagreements(maps[0][front], maps[1][front]), maps[0][front], stage, dedicated))
                    else:
                        frontend.setdefault(front, []).append(({'capture': 'missing or multiple visits'}, {}, stage, dedicated))
            except (OSError, ValueError, KeyError) as exc:
                item['reason'] = str(exc)
    front_results = []
    for stage in ('Unknown', 'FileSelect'):
        observations = frontend.get(stage, [])
        item = {'stage': stage, 'admitted': False, 'policy': 'whole-stage c198 first-use fallback unless every warm pair agrees'}
        front_results.append(item)
        if dedicated_title:
            observations = [o for o in observations if o[3]]
            item['policy'] = 'dedicated no-input title warm pair; incidental shorter direct visits retained in raw provenance'
        if not observations:
            item['reason'] = 'no validated frontend warm pairs'; continue
        first = observations[0][1]
        differences = [{'launch_stage': s, 'within_pair': diff, 'across_pairs': recording.disagreements(first, values)}
                       for diff, values, s, _ in observations if diff or values != first]
        if differences:
            item.update(reason='frontend exact frame groups disagree', disagreements=differences); continue
        rows = seedpack.database_rows(str(seeds / (stage + '.db')))
        try:
            values = {int(k, 16): v for k, v in first.items()}
            target = output / (stage + '.first-use.bin'); target.write_bytes(deadlines.encode(stage, rows, values))
            item.update(admitted=True, configs=len(values), groups=len(set(values.values())), sidecar_sha256=sha(target))
        except ValueError as exc:
            item['reason'] = str(exc)
    unchanged = all(seedpack.database_rows(str(p)) == seedpack.database_rows(str(output / p.name)) for p in seeds.glob('*.db'))
    missing = sorted(planned - admitted - {'Unknown', 'FileSelect'})
    result = {'version': 1, 'passed': not missing and unchanged, 'all_database_rows_unchanged': unchanged,
              'planned_stages': sorted(planned), 'admitted_stages': sorted(admitted), 'missing_or_failed_stages': missing,
              'frontend_recording_failures': sorted((planned - admitted) & {'Unknown', 'FileSelect'}),
              'stage_results': results, 'frontend_results': front_results, 'provenance': provenance_bindings,
              'source_seed_dir': str(seeds), 'output_seed_dir': str(output),
              'policy': 'explicit frame deadlines only; unknown/mismatched frontend falls back; no ranks rewritten'}
    report.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--seed-dir', required=True, type=Path)
    parser.add_argument('--provenance', required=True, nargs='+', type=Path)
    parser.add_argument('--out', required=True, type=Path)
    parser.add_argument('--report', required=True, type=Path)
    args = parser.parse_args()
    result = build(args.seed_dir, args.provenance, args.out, args.report)
    print(json.dumps({'passed': result['passed'], 'admitted': len(result['admitted_stages']),
                      'missing': result['missing_or_failed_stages'], 'frontend': result['frontend_results']}, indent=2))
    return 0 if result['passed'] else 1


if __name__ == '__main__': raise SystemExit(main())
