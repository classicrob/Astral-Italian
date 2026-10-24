#!/usr/bin/env python3
"""Create validated sidecars from TWO identical warm recordings, preserving seed rows/ranks.

--seed-dir V13_EXPANSION --source-pack V13.xz --provenance BATCH/PROVENANCE.json [...] --out NEW_DIR
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
import sqlite3

import pipeline_deadlines as deadlines
import record_pipeline_deadlines as recording
import seedpack


def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def validate_source(seeds, pack):
    """Bind every input row/rank to the exact pack used by the recording build."""
    errors = seedpack.check(str(pack))
    if errors: raise ValueError('source pack check failed: ' + str(errors))
    meta, blobs, stages, sidecars = seedpack.parse_full(pack)
    if sidecars: raise ValueError('source must be the original seed pack without deadline metadata')
    selected = {'__global__': [(i, row[4]) for i, row in enumerate(meta)], **dict(stages)}
    if {p.stem for p in seeds.glob('*.db')} != set(selected):
        raise ValueError('source seed database set differs from recording pack')
    if list(seeds.glob('*.first-use.bin')):
        raise ValueError('source seeds already contain deadline metadata')
    for stage, entries in selected.items():
        expected = {(meta[i][0], meta[i][1], meta[i][2], meta[i][3], blobs[i], rank) for i, rank in entries}
        rows = seedpack.database_rows(str(seeds / (stage + '.db')))
        if len(rows) != len(expected) or set(rows) != expected:
            raise ValueError('source seed rows/ranks differ from recording pack: ' + stage)
    return {'path': str(pack.resolve()), 'sha256': sha(pack), 'databases': len(selected),
            'all_source_rows_and_ranks_match_recording_pack': True}


def all_groups(path):
    stages = {r['stage'] for r in map(json.loads, Path(path).read_text().splitlines()) if r.get('event') == 'first_use'}
    result = {}
    for stage in stages:
        try: result[stage] = recording.observations(path, stage)
        except ValueError: result[stage] = None
    return result


def close_manifest(seeds, output, stage, frames):
    """Only matching warm observations can name additions, copied verbatim from global."""
    target = output / (stage + '.db')
    existed = target.exists()
    existing = seedpack.database_rows(str(target)) if existed else []
    known = {(r[0], r[1] & deadlines.MASK): r for r in existing}
    global_rows = {(r[0], r[1] & deadlines.MASK): r
                   for r in seedpack.database_rows(str(seeds / '__global__.db'))}
    missing = sorted(key for key in frames if (1, key) not in known)
    unknown = [key for key in missing if (1, key) not in global_rows]
    if unknown:
        raise ValueError('observed configs absent from global: ' + ','.join(f'{key:016x}' for key in unknown))
    additions = [global_rows[(1, key)] for key in missing]
    with sqlite3.connect(target) as db:
        if not existed:
            db.executescript(seedpack.SCHEMA)
            db.execute('INSERT INTO aurora_schema VALUES(1)')
        db.executemany('INSERT INTO pipeline_cache VALUES(?,?,?,?,?,?)', additions)
    after = seedpack.database_rows(str(target))
    actual = {(r[0], r[1] & deadlines.MASK): r for r in after}
    assert all(actual[key] == row for key, row in known.items()), 'existing stage row/rank changed'
    assert all(actual[(1, key)] == global_rows[(1, key)] for key in missing), 'added global row differs'
    return after, {'added_count': len(additions), 'added_hashes': [f'{key:016x}' for key in missing],
                   'existing_rows_byte_identical': True, 'added_rows_byte_identical_to_global': True,
                   'source_global_sha256': sha(seeds / '__global__.db')}


def build(seeds, provenances, output, report, expected_stages=None, *, source_pack, require_cold_confirmation=False,
          fallback_stages=()):
    seeds, output, report = Path(seeds).resolve(), Path(output).resolve(), Path(report).resolve()
    if output.exists() or report.exists() or output == seeds or report.is_relative_to(output):
        raise ValueError('output/report must be new; report must stay outside the packed seed directory')
    source_binding = validate_source(seeds, Path(source_pack))
    output.mkdir(parents=True)
    for path in seeds.glob('*.db'): shutil.copyfile(path, output / path.name)
    results, frontend, planned, admitted, attempts = [], {}, set(), set(), {}
    expected = set(expected_stages) if expected_stages is not None else None
    dedicated_title = False
    build_identity = None
    provenance_bindings = []
    for path in map(Path, provenances):
        data = json.loads(path.read_text())
        if data.get('version') != 1 or data.get('recording_only') is not True:
            raise ValueError('unsupported/non-recording provenance')
        dedicated = data.get('mode') == 'no-input-title'
        tape_driven = data.get('mode') == 'tape-driven'
        dedicated_title |= dedicated
        identity = data['recording_build_metadata']
        if identity.get('seed_pack_sha256') != source_binding['sha256']:
            raise ValueError('source seed pack differs from recording build')
        identity = tuple(identity.get(k) for k in ('source', 'binary_sha256', 'renderer_key'))
        if not re.fullmatch(r'[0-9a-f]{40}', identity[0] or '') or any(not re.fullmatch(r'[0-9a-f]{64}', x or '') for x in identity[1:]):
            raise ValueError('invalid recording source/executable/renderer identity')
        if build_identity is not None and identity != build_identity: raise ValueError('recording builds differ across batches')
        build_identity = identity
        if admitted & set(data['stages_planned']):
            raise ValueError('cannot replace an already admitted recording stage')
        planned.update(data['stages_planned'])
        provenance_bindings.append({'path': str(path.resolve()), 'sha256': sha(path),
                                    'recording_build_metadata': data['recording_build_metadata'],
                                    'recording_build_metadata_sha256': data['recording_build_metadata_sha256']})
        for result in data['stage_results']:
            stage = result['stage']
            item = {'stage': stage, 'admitted': False, 'provenance': str(path.resolve())}
            item['previous_failed_attempts'] = attempts.get(stage, []).copy()
            attempts.setdefault(stage, []).append(str(path.resolve()))
            results.append(item)
            try:
                if stage == 'Unknown':
                    raise ValueError('protocol3 bootstrap scope unqualified; Unknown keeps first-use fallback')
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
                    if not re.fullmatch(r'[0-9a-f]{64}', arm['recording_exe_sha256']) or arm['renderer_key'] != identity[2]:
                        raise ValueError('perf executable hash invalid or renderer differs from recording build')
                    if any(arm['checks'].get(k) is not True for k in ('valid_capture', 'no_pipeline_waits', 'no_pop_in')):
                        raise ValueError('record was not fully warm for drawn configs')
                    text = (folder / 'app.log').read_text(errors='replace')
                    with (folder / 'frames.csv').open() as file: timing = list(csv.DictReader(file))
                    if not timing or any(float(r['pipeline_wait_ms']) != 0 for r in timing) or re.search(r'^\[gx uber wait\]|^\[pop-in\] frame \d+ skipped_draws [1-9]', text, re.M):
                        raise ValueError('raw log/timing proves pipeline wait or pop-in in warm record')
                    if not dedicated and not tape_driven and '[input-script] done after 970 frames' not in text:
                        raise ValueError('perf input script did not complete')
                    if tape_driven:
                        if launch.get('mode') != 'tape-driven' or 'PETARI_INPUT_SCRIPT' in launch['environment'] or launch['environment'].get('PETARI_TAPE_PLAY_SESSION') != '1':
                            raise ValueError('recording did not use the declared whole-session tape')
                        native_path = folder / 'replay/divergence.json'
                        if sha(native_path) != arm['replay_report_sha256'] or not all(recording.replay_checks(json.loads(native_path.read_text())).values()):
                            raise ValueError('raw native tape replay did not pass strictly')
                        if 'jump cut' in text: raise ValueError('tape-driven recording used a jump cut')
                    if dedicated and ('PETARI_INPUT_SCRIPT' in launch['environment'] or launch.get('mode') != 'no-input-title'):
                        raise ValueError('title observation was not the no-input recording arm')
                    maps.append(all_groups(folder / 'first-use.jsonl'))
                if not maps[0].get(stage) or not maps[1].get(stage): raise ValueError('invalid/multiple target visits')
                differences = recording.disagreements(maps[0][stage], maps[1][stage])
                if differences:
                    item['disagreements'] = differences; raise ValueError('raw warm maps differ')
                cold_checks = []
                for arm in result['arms']:
                    if not arm.get('cold_confirmation'): continue
                    folder = path.parent / stage / arm['name']
                    if not arm['passed'] or arm['checks'].get('valid_capture') is not True:
                        raise ValueError('cold confirmation did not complete successfully')
                    for name, field in [('first-use.jsonl', 'capture_sha256'), ('app.log', 'app_log_sha256'), ('launch.json', 'launch_sha256')]:
                        if sha(folder / name) != arm[field]: raise ValueError('cold raw artifact hash mismatch')
                    launch = json.loads((folder / 'launch.json').read_text())
                    if launch['exe_sha256'] != arm['recording_exe_sha256'] or launch['renderer_key'] != identity[2] or launch.get('cold_namespace') is not True:
                        raise ValueError('cold confirmation build/namespace binding mismatch')
                    cold = recording.observations(folder / 'first-use.jsonl', stage)
                    diff = recording.disagreements(maps[0][stage], cold)
                    cold_checks.append({'capture_sha256': arm['capture_sha256'], 'disagreements': diff})
                    if diff: raise ValueError('raw cold confirmation frame groups differ')
                item['cold_confirmation_checks'] = cold_checks
                values = {int(k, 16): v for k, v in maps[0][stage].items()}
                rows, closure = close_manifest(seeds, output, stage, values)
                item['manifest_closure'] = closure
                target = output / (stage + '.first-use.bin')
                target.write_bytes(deadlines.encode(stage, rows, values))
                assert deadlines.read(target, stage, rows) == values
                item.update(admitted=True, configs=len(values), groups=len(set(values.values())),
                            sidecar_sha256=sha(target), source_database_sha256=
                            sha(seeds / (stage + '.db')) if (seeds / (stage + '.db')).exists() else None)
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
        if stage == 'Unknown':
            # Protocol3 deliberately pins requests to the next whole packet.
            # Its initial bootstrap packet has no named scene, so equality of
            # Unknown's later map cannot prove first use across the whole boot.
            item.update(reason='protocol3 does not cover pre-scene bootstrap GX lookups; whole-boot deadlines unqualified',
                        bootstrap_scope_qualified=False)
            continue
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
        try:
            values = {int(k, 16): v for k, v in first.items()}
            rows, closure = close_manifest(seeds, output, stage, values)
            item['manifest_closure'] = closure
            target = output / (stage + '.first-use.bin'); target.write_bytes(deadlines.encode(stage, rows, values))
            item.update(admitted=True, configs=len(values), groups=len(set(values.values())), sidecar_sha256=sha(target))
        except ValueError as exc:
            item['reason'] = str(exc)
    unchanged = all(seedpack.database_rows(str(p)) == seedpack.database_rows(str(output / p.name)) for p in seeds.glob('*.db'))
    existing_unchanged = all(set(seedpack.database_rows(str(p))) <= set(seedpack.database_rows(str(output / p.name)))
                             for p in seeds.glob('*.db'))
    global_unchanged = seedpack.database_rows(str(seeds / '__global__.db')) == seedpack.database_rows(str(output / '__global__.db'))
    required = expected if expected is not None else planned - {'Unknown', 'FileSelect'}
    missing = sorted(required - admitted)
    permitted = set(fallback_stages)
    if permitted - planned or permitted & admitted or permitted - required:
        raise ValueError('explicit fallback must name an attempted, unqualified required stage')
    cold_stages = sorted(r['stage'] for r in results if r['admitted'] and r.get('cold_confirmation_checks'))
    result = {'version': 1, 'passed': not (set(missing) - permitted) and existing_unchanged and global_unchanged
              and (not require_cold_confirmation or bool(cold_stages)),
              'all_requested_deadlines_qualified': not missing,
              'explicit_whole_stage_fallbacks': sorted(permitted),
              'unapproved_missing_stages': sorted(set(missing) - permitted),
              'cold_confirmation_required': require_cold_confirmation, 'cold_confirmed_stages': cold_stages,
              'all_database_rows_unchanged': unchanged, 'all_existing_rows_and_ranks_byte_identical': existing_unchanged,
              'global_rows_unchanged': global_unchanged,
              'planned_stages': sorted(planned), 'admitted_stages': sorted(admitted), 'missing_or_failed_stages': missing,
              'expected_stages': sorted(required), 'unrecorded_stages': sorted(required - planned),
              'frontend_recording_failures': sorted((planned - admitted) & {'Unknown', 'FileSelect'}),
              'stage_results': results, 'frontend_results': front_results, 'provenance': provenance_bindings,
              'source_seed_dir': str(seeds), 'output_seed_dir': str(output),
              'source_pack_binding': source_binding,
              'policy': 'explicit matching warm frames; additive stage closure only from exact global rows; unknown global configs fail; no existing ranks rewritten; mismatched frontend falls back'}
    report.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--seed-dir', required=True, type=Path)
    parser.add_argument('--source-pack', required=True, type=Path, help='Exact original pack named by recording BUILD.json')
    parser.add_argument('--disc', required=True, type=Path, help='Enumerate the complete scenario-1 sweep; partial batches cannot pass')
    parser.add_argument('--provenance', required=True, nargs='+', type=Path)
    parser.add_argument('--fallback-stage', action='append', default=[], help='Explicit authorised whole-stage first-use fallback; original failures remain reported')
    parser.add_argument('--out', required=True, type=Path)
    parser.add_argument('--report', required=True, type=Path)
    args = parser.parse_args()
    result = build(args.seed_dir, args.provenance, args.out, args.report, recording.select(args.disc),
                   source_pack=args.source_pack, require_cold_confirmation=True, fallback_stages=args.fallback_stage)
    print(json.dumps({'passed': result['passed'], 'admitted': len(result['admitted_stages']),
                      'missing': result['missing_or_failed_stages'], 'frontend': result['frontend_results']}, indent=2))
    return 0 if result['passed'] else 1


if __name__ == '__main__': raise SystemExit(main())
