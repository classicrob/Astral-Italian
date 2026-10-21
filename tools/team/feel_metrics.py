#!/usr/bin/env python3
"""Extract unqualified GX feel diagnostics from existing receipts; never run an app."""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
import re
import statistics

TIMINGS = ('interval_ms', 'present_ms', 'present_call_ms', 'pipeline_wait_ms',
           'game_work_ms', 'retrace_wait_ms', 'draw_done_wait_ms')
BOUNDS = (1000 / 60, 25, 1000 / 30, 50, 100)
DIAGNOSTICS = ('PETARI_FRAME_STATS', 'PETARI_PIPELINE_DIAG', 'PETARI_AUDIO_DIAG',
               'PETARI_CAPPY_TRACE', 'PETARI_ODYSSEY_TRACE', 'PETARI_TRACE_BOOT',
               'PETARI_XFB_DUMP', 'PETARI_SURFACE_DUMP', 'PETARI_FRAME_CSV',
               'PETARI_ROUTE_RECORD', 'PETARI_EDITOR_MOUSE_TRACE', 'PETARI_INPUT_SCRIPT_LOG',
               'PETARI_CAPTURE_DIR')


def require(ok, message):
    if not ok:
        raise ValueError('feel metrics: ' + message)


def missing(reason):
    return {'status': 'NOTMEASURED', 'reason': reason, 'value': None}


def number(value):
    result = float(value)
    require(math.isfinite(result) and result >= 0, 'invalid negative/nonfinite timing')
    return result


def percentile(values, fraction):
    ordered = sorted(values)
    return ordered[min(len(ordered) - 1, math.ceil(len(ordered) * fraction) - 1)]


def distribution(values):
    if not values:
        return missing('no numeric samples')
    edges = (0, *BOUNDS, None)
    counts = []
    for i, (lower, upper) in enumerate(zip(edges, edges[1:])):
        counts.append({'lower_ms': lower, 'lower_inclusive': i == 0,
                       'upper_ms': upper, 'upper_inclusive': upper is not None,
                       'count': sum((v >= lower if i == 0 else v > lower)
                                    and (upper is None or v <= upper) for v in values)})
    return {'status': 'MEASURED', 'unit': 'ms', 'samples': len(values),
            'mean_ms': statistics.fmean(values), 'p50_ms': percentile(values, .5),
            'p95_ms': percentile(values, .95), 'p99_ms': percentile(values, .99),
            'max_ms': max(values), 'over_25ms': sum(v > 25 for v in values), 'histogram': counts}


def frames(path):
    if path is None or not path.is_file():
        return missing('frame CSV absent')
    phases = {}
    previous = None
    with path.open(newline='') as source:
        reader = csv.DictReader(source)
        require(reader.fieldnames is not None and len(reader.fieldnames) == len(set(reader.fieldnames)), 'bad/duplicate frame CSV header')
        require({'frame', 'phase'} <= set(reader.fieldnames), 'frame/phase columns required')
        for row in reader:
            require(None not in row and all(v is not None for v in row.values()), 'malformed frame row')
            frame = int(row['frame'])
            require(frame >= 0 and (previous is None or frame > previous), 'duplicate/nonmonotonic frame')
            previous = frame
            phase = row['phase'] or 'UNKNOWN'
            data = phases.setdefault(phase, {'rows': 0, 'samples': {k: [] for k in TIMINGS},
                                           'unfocused': [], 'occluded': [], 'efb_captures': []})
            data['rows'] += 1
            for column in TIMINGS:
                if row.get(column, '') != '':
                    data['samples'][column].append(number(row[column]))
            for column in ('unfocused', 'occluded', 'efb_captures'):
                if row.get(column, '') != '':
                    value = int(row[column]); require(value >= 0, 'negative frame counter')
                    if column != 'efb_captures':
                        require(value in (0, 1), 'invalid frame boolean')
                    data[column].append(value)
    if not phases:
        return missing('frame CSV contains no rows')
    output = {}
    for phase, data in phases.items():
        output[phase] = {'rows': data['rows'], 'timings': {k: distribution(v) for k, v in data['samples'].items()}}
        for column in ('unfocused', 'occluded', 'efb_captures'):
            values = data[column]
            output[phase][column] = {'status': 'MEASURED', 'samples': len(values), 'sum': sum(values)} if values else missing('column absent/empty')
    return {'status': 'MEASURED', 'phase_source': 'literal CSV phase; no inferred ready seam', 'phases': output}


def log_metrics(text, environment, context):
    lines = text.splitlines()
    compiles = [line for line in lines if line.startswith('[gx pipeline compile] ')]
    blocking = [number(v) for v in re.findall(r'^\[gx pipeline\] blocking resolve ([^ ]+) ms', text, re.M)]
    pipeline_marker = any(line.startswith('[gx pipeline] policy=') for line in lines)
    complete = context.get('log_complete') is True
    pipeline_complete = complete and environment.get('PETARI_PIPELINE_DIAG') == '1' and pipeline_marker
    if compiles or blocking or pipeline_complete:
        compile_metrics = {}
        for key in ('queue_ms', 'build_ms', 'pipeline_api_ms'):
            values = []
            for line in compiles:
                found = re.search(r'(?:^| )' + key + r'=([^ ]+)', line)
                if found:
                    values.append(number(found[1]))
            compile_metrics[key] = distribution(values)
        statuses = [re.search(r'(?:^| )status=([^ ]+)', line) for line in compiles]
        statuses = [match[1] for match in statuses if match]
        shaders = {'status': 'MEASURED', 'coverage': 'complete diagnostic log' if pipeline_complete else 'observed events only',
                   'compile_events': len(compiles), 'compile_status_reports': len(statuses),
                   'non_ready_events': sum(status != 'ready' for status in statuses),
                   'compile_timings': compile_metrics, 'blocking_resolve_ms': distribution(blocking),
                   'blocking_resolve_events': len(blocking),
                   'phase': 'NOTMEASURED: log events lack bound frame/phase IDs',
                   'interpretation': 'async queue/build latency is not a measured gameplay stall'}
    else:
        shaders = missing('pipeline diagnostics absent or coverage unproved')
    cache = context.get('cache', {})
    if cache.get('classification') == 'fresh' and cache.get('verified') is True and re.fullmatch(r'[0-9a-f]{64}', cache.get('inventory_sha256', '')):
        if blocking or pipeline_complete:
            fresh = {'status': 'MEASURED', 'blocking_resolve_events': len(blocking),
                     'blocking_ms_total': sum(blocking), 'over_25ms': sum(v > 25 for v in blocking),
                     'cache_inventory_sha256': cache['inventory_sha256'],
                     'phase': 'NOTMEASURED: no bound event frame IDs'}
        else:
            fresh = missing('fresh cache bound but no measured blocking channel')
    else:
        fresh = missing('fresh cache inventory not verified/bound; warm or unknown cannot count as fresh')
    levels = re.findall(r'^\[audio\] level .*?underrun (\d+) frames, AI replayed (\d+) blocks \(since previous report, ([0-9.]+) s', text, re.M)
    holds = [int(v) for v in re.findall(r'^\[audio-dma\] .*DSP holds (\d+)', text, re.M)]
    warnings = [i + 1 for i, line in enumerate(lines) if re.search(r'\[(?:audio|audio-dma)[^]]*\].*\b(?:warning|warn|error|failed)\b', line, re.I)]
    if levels or holds or warnings:
        audio = {'status': 'MEASURED', 'reports': len(levels),
                 'reported_seconds': sum(number(row[2]) for row in levels) if levels else None,
                 'underrun_frames': sum(int(row[0]) for row in levels) if levels else None,
                 'replayed_blocks': sum(int(row[1]) for row in levels) if levels else None,
                 'dsp_holds': sum(holds) if holds else None,
                 'underrun_counter_status': 'MEASURED' if levels else 'NOTMEASURED',
                 'dsp_counter_status': 'MEASURED' if holds else 'NOTMEASURED',
                 'coverage': 'observed interval reports only; no unlogged time inferred',
                 'warning_lines': warnings, 'warning_count': len(warnings),
                 'real_output': 'VERIFIED_BY_CONTEXT' if context.get('real_audio_verified') is True else 'NOTMEASURED',
                 'phase': 'NOTMEASURED: no bound frame IDs', 'audible_quality': 'NOTMEASURED'}
    else:
        audio = missing('audio diagnostic counters/warnings absent; silence is not zero underruns')
    return {'pipeline': shaders, 'fresh_shader_stalls': fresh, 'audio': audio}


def extract(launch, context, frame_path=None, log_path=None):
    source, binary = launch.get('source'), launch.get('binary_sha256')
    require(re.fullmatch('[0-9a-f]{40}', source or '') is not None, 'source identity required')
    require(re.fullmatch('[0-9a-f]{64}', binary or '') is not None, 'binary identity required')
    for key, value in (('source', source), ('binary_sha256', binary)):
        require(context.get(key) == value, 'context identity mismatch')
    require(isinstance(context.get('case_id'), str) and context['case_id'], 'case_id required')
    environment = launch.get('environment', {})
    frame_data = frames(frame_path)
    text = log_path.read_text(errors='replace') if log_path is not None and log_path.is_file() else ''
    channels = {'frames': frame_data, **log_metrics(text, environment, context)}
    enabled = [key for key in DIAGNOSTICS if environment.get(key) not in (None, '', '0', 'off', 'false')]
    contamination = context.get('contamination', {})
    require(isinstance(contamination, dict), 'contamination must be structured')
    return {'schema_version': 1, 'status': 'UNQUALIFIED_FEEL_DIAGNOSTIC', 'qualification': 'BLOCKED',
            'measured_qualified_frames': None,
            'provenance': {'source_revision': source, 'executable_sha256': binary, 'case_id': context['case_id']},
            'metric_policy': {'frame_intervals': 'CPU/main-loop interval, not inferred image FPS or GPU latency',
                              'present_ms': 'only literal nonempty CSV samples', 'hitch_threshold_ms': 25,
                              'hitch_comparison': 'strictly greater than', 'phase_alignment': 'CSV phases only',
                              'image_metrics': 'separate schema; SSIM never implies feel/performance'},
            'conditions': {'diagnostic_overhead': {'status': 'NOTQUANTIFIED', 'enabled': enabled,
                                                  'reason': 'no matched diagnostics-off measurement'},
                           'contamination': contamination or {'status': 'UNKNOWN', 'reason': 'no owned-job/window receipt'},
                           'cache': context.get('cache', {'classification': 'UNKNOWN'}),
                           'real_audio_requested': environment.get('PETARI_SMOKE_REAL_AUDIO') == '1',
                           'audio_driver_requested': environment.get('SDL_AUDIO_DRIVER', environment.get('SDL_AUDIODRIVER', 'DEFAULT'))},
            'channels': channels}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--launch', type=Path, required=True)
    parser.add_argument('--context', type=Path, required=True)
    parser.add_argument('--frames', type=Path)
    parser.add_argument('--log', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    require(not args.output.exists(), 'output exists; preserve prior extraction')
    def identities():
        output = {}
        for key in ('launch', 'context', 'frames', 'log'):
            path = getattr(args, key)
            output[key] = {'sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'bytes': path.stat().st_size} if path is not None and path.is_file() else None
        return output
    artifacts = identities()
    result = extract(json.loads(args.launch.read_text()), json.loads(args.context.read_text()), args.frames, args.log)
    require(identities() == artifacts, 'input changed during extraction; use quiescent completed artifacts')
    result['provenance']['artifacts'] = artifacts
    result['provenance']['extractor_sha256'] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    with args.output.open('x') as out:
        out.write(json.dumps(result, indent=2, allow_nan=False) + '\n')
    print('Extracted unqualified feel diagnostics; missing channels remain NOTMEASURED.')


if __name__ == '__main__':
    main()
