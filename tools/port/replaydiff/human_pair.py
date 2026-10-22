#!/usr/bin/env python3
"""Replay actual Dolphin suffix records natively and retain unqualified diagnostics."""
import argparse
import csv
import hashlib
import importlib.util
import json
import os
import math
from pathlib import Path
import subprocess
import sys

import tracediff
from dolphin_collect import game_private_path


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def raw_first(a, b):
    for key in sorted(set(a.rows) | set(b.rows)):
        ar, br = a.rows.get(key, {}), b.rows.get(key, {})
        differences = [{'field': field, 'native': ar.get(field), 'dolphin': br.get(field)}
                       for field in sorted(set(a.columns) | set(b.columns)) if ar.get(field) != br.get(field)]
        if differences:
            return dict(frame=key[0], fields=differences)
    return None


def gx_departure(trace, gx, anchor, tolerance):
    for (frame,), row in sorted(trace.rows.items()):
        source = anchor + frame
        if source >= len(gx) or gx[source]['valid'] != '1' or row.get('player_valid') != '1':
            continue
        differences = [{'field': axis, 'replay': row[axis], 'gx': gx[source][axis],
                        'absolute_error': abs(float(row[axis]) - float(gx[source][axis]))}
                       for axis in 'xyz' if abs(float(row[axis]) - float(gx[source][axis])) > tolerance]
        if differences:
            return dict(trace_frame=frame, gx_source_frame=source, absolute_tolerance=tolerance,
                        phase='provisional GX anchor+trace frame, not proven consumed-input alignment', fields=differences)
    return None


def tracking_window(trace, gx, anchor, source_start, count):
    maximum = {axis: 0.0 for axis in 'xyz'}
    euclidean = 0.0
    first_failure = None
    compared = 0
    missing = []
    for source in range(source_start, source_start + count):
        frame = source - anchor
        row = trace.rows.get((frame,))
        if source >= len(gx) or gx[source]['valid'] != '1' or not row or row.get('player_valid') != '1':
            missing.append(dict(source_frame=source, trace_frame=frame))
            continue
        error = {axis: abs(float(row[axis]) - float(gx[source][axis])) for axis in 'xyz'}
        compared += 1
        maximum = {axis: max(maximum[axis], error[axis]) for axis in 'xyz'}
        euclidean = max(euclidean, math.sqrt(sum(value * value for value in error.values())))
        if first_failure is None and any(value > 0.01 for value in error.values()):
            first_failure = dict(source_frame=source, trace_frame=frame, absolute_axis_error=error,
                                 replay={axis: row[axis] for axis in 'xyz'}, gx={axis: gx[source][axis] for axis in 'xyz'})
    return dict(status='TRACKED' if compared == count and not first_failure else 'NOT_TRACKED',
                qualification='unqualified proxy-anchor tracking metric only', source_start=source_start,
                source_end=source_start + count - 1, trace_start=source_start - anchor,
                trace_end=source_start + count - 1 - anchor, required_frames=count, compared_frames=compared,
                absolute_tolerance_per_axis=0.01, tolerance_declared_before_comparison=True,
                max_axis_error=maximum, max_euclidean_error=euclidean, first_failure=first_failure, missing=missing,
                route_budget_units=50, route_budget_satisfied=compared == count and euclidean <= 50,
                ground_zone_comparison='UNKNOWN: not observed by common86 frozen providers; no link-zone substitution')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture', type=Path, required=True)
    parser.add_argument('--conversion', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native-checkout', type=Path, default=Path.home() / 'MacioGalaxy2-wt/port')
    parser.add_argument('--executable', type=Path)
    parser.add_argument('--frozen-build-receipt', type=Path)
    parser.add_argument('--tracking-source-start', type=int)
    parser.add_argument('--tracking-frames', type=int)
    parser.add_argument('--tracking-policy', type=Path)
    args = parser.parse_args()
    if (args.tracking_source_start is None) != (args.tracking_frames is None):
        raise ValueError('tracking source-start and frame count must be declared together')
    policy = json.loads(args.tracking_policy.read_text()) if args.tracking_policy else None
    if args.tracking_source_start is not None:
        if not policy or policy['source_window'] != [args.tracking_source_start, args.tracking_source_start + args.tracking_frames - 1]:
            raise ValueError('tracking window must match predeclared policy')
    capture, conversion = game_private_path(args.capture), game_private_path(args.conversion)
    receipt = json.loads(conversion.read_text())
    manifest = json.loads((capture / 'manifest.json').read_text())
    if manifest['status'] != 'captured; repeat qualification required':
        raise ValueError('Dolphin capture is not finalized; do not replay a partial export')
    source = Path(receipt['source_path'])
    if sha(source) != receipt['source_sha256']:
        raise ValueError('human recording hash changed')
    export = capture / 'inputs.jsonl'
    if manifest.get('input_export', {}).get('sha256') != sha(export):
        raise ValueError('Dolphin export receipt hash mismatch')
    intent = conversion.parent / receipt['script_path']
    if sha(intent) != receipt['script_sha256'] or manifest['sync_input_script_sha256'] != sha(intent):
        raise ValueError('conversion and actual injected suffix differ')
    frames = receipt['input_frames']
    if manifest['frames'] != frames + 1:
        raise ValueError('Dolphin trace count differs from suffix plus anchor')
    origin = manifest['sync']['source_frame']
    if manifest['sync_input_source_start'] != origin + 1:
        raise ValueError('Dolphin suffix mapping is not first loop after anchor')
    output = game_private_path(args.output)
    output.mkdir(parents=True, exist_ok=False)
    rows = []
    with export.open() as stream:
        for line in stream:
            row = json.loads(line)
            if origin + 1 <= row['frame'] <= origin + frames:
                row['frame'] -= origin + 1
                rows.append(row)
    if {row['frame'] for row in rows} != set(range(frames)):
        raise ValueError('actual export suffix has a missing frame')
    inputs = output / 'inputs.jsonl'
    with inputs.open('x') as stream:
        for row in rows:
            stream.write(json.dumps(row, separators=(',', ':')) + '\n')
    repo = args.native_checkout.expanduser().resolve()
    spec = importlib.util.spec_from_file_location('human_headless', repo / 'native/tools/headless.py')
    contract = importlib.util.module_from_spec(spec)
    sys.path.insert(0, str(repo / 'native/tools'))
    spec.loader.exec_module(contract)
    with (output / 'inputs.tape').open('xb') as stream:
        contract.encode_rows(rows, frames, stream)
    executable = args.executable.expanduser().resolve() if args.executable else repo / 'build/macos-arm64-headless/native/app/headless/petari_headless'
    executable_hash = sha(executable)
    frozen = json.loads(args.frozen_build_receipt.read_text()) if args.frozen_build_receipt else None
    if frozen:
        if executable_hash != frozen['executable_sha256']:
            raise ValueError('frozen executable hash differs')
        for item in frozen['files']:
            if sha(Path(item['path'])) != item['sha256']:
                raise ValueError('frozen runtime or launcher hash differs')
    trace = output / 'native.csv'
    command = ['nice', '-n', '9', sys.executable, str(repo / 'native/tools/headless.py'), 'run',
               '--executable', str(executable), '--inputs', str(inputs), '--frames', str(frames),
               '--disc', str(Path.home() / 'MacioGalaxy2-gamedata/SB4E01-extracted'),
               '--user', str(output / 'user'), '--trace', str(trace), '--epoch-ticks', '0',
               '--stage', 'RedBlueExGalaxy', '--scenario', '1', '--sync-stage', 'RedBlueExGalaxy',
               '--sync-scenario', '1', '--inputs-from-sync']
    env = dict(os.environ, ASAN_OPTIONS='detect_odr_violation=0')
    with (output / 'native.log').open('x') as log:
        status = subprocess.call(command, stdout=log, stderr=subprocess.STDOUT, env=env)
    if status or sha(executable) != executable_hash:
        raise RuntimeError('native run failed or binary changed; retain failed artifacts')
    if frozen and any(sha(Path(item['path'])) != item['sha256'] for item in frozen['files']):
        raise ValueError('frozen runtime/launcher changed during native run')
    native_manifest = json.loads(Path(str(trace) + '.manifest.json').read_text())
    if native_manifest['status'] != 'complete' or native_manifest['trace_frames'] != frames + 1:
        raise ValueError('native run did not finalize expected rows')
    a, b = tracediff.read_trace(trace), tracediff.read_trace(capture / 'oracle.csv')
    excluded = {'pending_state_id': 'unavailable (callback-context only)',
                'prev_draw': 'A exclusion: pre-first-stride initialization/previous-frame semantics unproved'}
    for current, producer in [(a, native_manifest), (b, manifest)]:
        if (producer.get('observer_contract_incomplete') is not True or
                producer.get('field_availability', {}).get('pending_state_id') != excluded['pending_state_id'] or
                any(row['pending_state_id'] for row in current.rows.values())):
            raise ValueError('observer contract metadata/blank cells differ')
        current.columns = [k for k in current.columns if k not in excluded]
        current.rows = {key: {k: v for k, v in row.items() if k not in excluded} for key, row in current.rows.items()}
    gx = list(csv.DictReader(source.open()))
    report = dict(schema_version=1, status='DIAGNOSTIC_ONLY', qualification_status='BLOCKED',
                  qualified_divergences=None, conversion=receipt, excluded_fields=excluded,
                  input_source_origin=origin, native_source_origin=native_manifest['sync_origin'],
                  natural_ready_rng=dict(native=a.rows[(0,)]['rng_seed'], dolphin=b.rows[(0,)]['rng_seed']),
                  rng_override=False, raw_first_difference=raw_first(a, b),
                  native_producer_host=os.uname().nodename, dolphin_producer_host=manifest.get('producer_host'),
                  report_driver_sha256=sha(Path(__file__)),
                  default_tolerance=tracediff.compare(a, b, expected_frames=frames + 1),
                  zero_tolerance=tracediff.compare(a, b, policy=tracediff.Policy(abs_tol=0, rel_tol=0), expected_frames=frames + 1),
                  gx_position_departure={side: {str(tol): gx_departure(current, gx, receipt['anchor_source_frame'], tol)
                                                for tol in [0, 0.0005, 0.01]} for side, current in [('native', a), ('dolphin', b)]},
                  actual_inputs=dict(source_sha256=sha(export), source_start=origin + 1, input_frames=frames,
                                     read_count=len(rows), rebased_sha256=sha(inputs)),
                  executable_sha256=executable_hash, native_trace_sha256=sha(trace),
                  frozen_build_receipt=frozen,
                  dolphin_trace_sha256=sha(capture / 'oracle.csv'),
                  command=command, ASAN_OPTIONS=env['ASAN_OPTIONS'],
                  native_log_sha256=sha(output / 'native.log'),
                  limitations=['No raw GX input/RNG; synthetic shake, missing pointer/shoot, proxy phase, rounded GX positions.',
                               'No actors requested for bounded long diagnostic capture; no per-actor equivalence claim.',
                               'Pending-state observer unavailable, timing/FP policy incomplete; no admission or qualification.',
                               'ODR detector override and existing UBSan diagnostics retained.'])
    if args.tracking_source_start is not None:
        report['movement_tracking'] = {side: tracking_window(current, gx, receipt['anchor_source_frame'],
                                                             args.tracking_source_start, args.tracking_frames)
                                       for side, current in [('native', a), ('dolphin', b)]}
        report['tracking_policy'] = dict(path=str(args.tracking_policy), sha256=sha(args.tracking_policy), policy=policy)
    result = output / 'diagnostic.json'
    result.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(dict(report=str(result), raw_first=report['raw_first_difference'],
                          natural_ready_rng=report['natural_ready_rng'], gx_position_departure=report['gx_position_departure'])))


if __name__ == '__main__':
    main()
