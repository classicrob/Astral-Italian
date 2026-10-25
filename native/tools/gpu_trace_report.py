#!/usr/bin/env python3
"""Join diagnostic boundaries to frames.csv. Pure file analysis; never runs apps.

A queue completion is a CPU observation including callback dispatch. Independent
and overlapping waits are reported separately, never added into a GPU total.
"""
import argparse
import bisect
import csv
import hashlib
import importlib.util
import json
import math
import re
from pathlib import Path


def scheduling_audit(events):
    spec = importlib.util.spec_from_file_location('petari_trace_attribution', Path(__file__).with_name('gpu_trace_attribution.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.audit(events)


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def csv_rows(path):
    with Path(path).open() as stream:
        return list(csv.DictReader(stream))


def distributions(rows, seconds=300, global100=None):
    if not math.isfinite(seconds) or seconds <= 0: raise ValueError("positive finite window required")
    selected, selected_rows, elapsed = [], [], 0.0
    for row in rows:
        duration = float(row['interval_ms'])
        if not math.isfinite(duration) or duration <= 0:
            raise ValueError('non-positive/non-finite frame interval')
        # Fixed elapsed-time window, with no late-frame removal.
        if elapsed >= seconds * 1000:
            break
        selected.append(duration)
        selected_rows.append(row)
        elapsed += duration
    if elapsed < seconds * 1000:
        raise ValueError('CSV does not cover the requested fixed window')
    def quantile(values, q):
        values = sorted(values)
        return values[max(0, math.ceil(q * len(values)) - 1)]
    post = [float(r["interval_ms"]) for r in selected_rows if global100 is not None and int(r["frame"]) > global100]
    return dict(frames=len(selected), covered_ms=elapsed,
                actual_global100_csv_frame=global100, post100_p99=quantile(post, .99) if post else None,
                pipeline_resolves=sum(int(r.get("pipeline_resolves", 0)) for r in selected_rows),
                gt50=sum(x > 50 for x in selected), maximum=max(selected),
                p50=quantile(selected, .5), p95=quantile(selected, .95),
                p99=quantile(selected, .99), samples_ms=selected)


def compare(off_rows, on_rows, seconds=300, off_global100=None, on_global100=None):
    off, on = distributions(off_rows, seconds, off_global100), distributions(on_rows, seconds, on_global100)
    delta = {key: on[key] - off[key] for key in ('gt50', 'maximum', 'p50', 'p95', 'p99')}
    delta['post100_p99'] = on['post100_p99'] - off['post100_p99'] if on['post100_p99'] is not None and off['post100_p99'] is not None else None
    return dict(off=off, on=on, delta=delta, seconds=seconds,
                acceptance=False, materiality='review paired distributions and retained individual arms; no automatic acceptance')


def join(records, rows):
    errors = []
    def require(test, reason):
        if not test: errors.append(reason)
    require(len([x for x in records if x.get('type') == 'header']) == 1, 'missing/duplicate header')
    headers = [x for x in records if x.get('type') == 'header']
    if headers:
        require(headers[0].get('clock') == 'steady_clock_ns', 'wrong clock')
        require(headers[0].get('schema') in (1, 2), 'wrong schema')
    footers = [x for x in records if x.get('type') == 'footer']
    require(len(footers) == 1, 'missing/duplicate footer')
    if footers:
        require(footers[0].get('drops') == 0, 'trace dropped records; window invalid')
        require(footers[0].get('io_errors') == 0, 'trace I/O failure')
    events = [dict(x) for x in records if x.get('type') == 'boundary']
    sequences = {}
    for e in events:
        thread, sequence = e['thread'], e['sequence']
        require(sequence == sequences.get(thread, 0) + 1, f'producer sequence gap {thread}/{sequence}')
        sequences[thread] = sequence
    events.sort(key=lambda x: (x['ns'], x['thread'], x['sequence']))
    opens, closes, packets, tickets, submissions = {}, {}, {}, {}, {}
    for e in events:
        name = e['event']
        if name in ('csv_interval_open', 'csv_interval_close'):
            destination = opens if name.endswith('open') else closes
            require(e['csv'] not in destination, f'duplicate {name} {e["csv"]}')
            destination[e['csv']] = e['ns']
        elif name == 'packet_open':
            require(e['packet'] not in packets, f'duplicate packet {e["packet"]}')
            packets[e['packet']] = e
        elif name == 'capture_request':
            require(e['ticket'] not in tickets, f'duplicate capture ticket {e["ticket"]}')
            tickets[e['ticket']] = e
        elif name == 'submit_begin':
            require(e['submission'] not in submissions, f'duplicate submit {e["submission"]}')
            submissions[e['submission']] = e
        elif name in ('map_callback_enter', 'gpu_queue_done_observed'):
            require(e['value'] == 0, f'callback failed {name}')
    intervals = []
    for row in rows:
        frame = int(row['frame'])
        require(frame in opens and frame in closes, f'CSV interval missing {frame}')
        if frame not in opens or frame not in closes: continue
        start, stop = opens[frame], closes[frame]
        require(stop > start, f'CSV time order {frame}')
        require(abs((stop - start) / 1e6 - float(row['interval_ms'])) <= .000501,
                f'CSV duration mismatch {frame}')
        intervals.append((start, stop, frame))
    intervals.sort()
    for a, b in zip(intervals, intervals[1:]):
        require(a[1] == b[0] and a[2] + 1 == b[2], 'CSV intervals discontinuous')
    starts = [x[0] for x in intervals]
    def observed(ns):
        i = bisect.bisect_right(starts, ns) - 1
        return intervals[i][2] if i >= 0 and ns < intervals[i][1] else None
    # Stable origin identity is bound at packet-open, independently of callback
    # observation time. Late callback -> old origin, current observed interval.
    for e in events:
        packet = e['packet']
        if e['ticket'] and not packet:
            request = tickets.get(e['ticket'])
            require(request is not None, f'unbound ticket {e["ticket"]}')
            if request: packet = request['packet']
        if e['submission'] and not packet:
            submit = submissions.get(e['submission'])
            require(submit is not None, f'unbound submission {e["submission"]}')
            if submit: packet = submit['packet']
        origin = packets.get(packet)
        if packet: require(origin is not None, f'unbound packet {packet}')
        e['origin_packet'] = packet or None
        e['origin_csv_frame'] = observed(origin['ns']) if origin else None
        e['observed_csv_frame'] = observed(e['ns'])
    births = {}
    for e in events:
        if e['event'] == 'thread_birth': births.setdefault(e['subject'], []).append(e)
    for e in events:
        is_thread = e['event'].startswith('thread_') or e['event'].startswith(('gx_draw_done_wait_', 'gx_token_wait_', 'vi_wait_', 'host_blocking_')) or e['event'] in ('baton_grant', 'holder_first_os_observation')
        if not is_thread or not e['subject']: continue
        known = births.get(e['subject'], [])
        i = bisect.bisect_right([b['ns'] for b in known], e['ns']) - 1
        require(i >= 0, f'unbound thread lifetime {e["subject"]}')
        e['thread_lifetime'] = known[i]['span'] if i >= 0 else None
    # Duplicate or missing completion callbacks invalidate the relevant chain.
    # Footer pending callbacks from the unobserved tail stay explicitly separate.
    callback_counts = {}
    for e in events:
        if e['event'] == 'gpu_queue_done_observed':
            callback_counts[e['submission']] = callback_counts.get(e['submission'], 0) + 1
    for submission, count in callback_counts.items():
        require(submission in submissions and count == 1, f'unbound/duplicate completion {submission}')
    return events, intervals, errors, footers[0] if len(footers) == 1 else None


PAIRS = {
    'capture_request': ('readback_cpu_complete', 'ticket'),
    'capture_staging_wait_begin': ('capture_staging_wait_end', 'ticket'),
    'capture_enqueue': ('capture_worker_begin', 'ticket'),
    'capture_encode_begin': ('capture_encode_end', 'ticket'),
    'submit_begin': ('submit_return', 'submission'),
    'submit_return': ('gpu_queue_done_observed', 'submission'),
    'map_request': ('map_callback_enter', 'ticket'),
    'map_callback_enter': ('readback_cpu_complete', 'ticket'),
    'gx_snapshot_marked_ready': ('gx_delivery_begin', 'ticket'),
    'gx_draw_done_wait_begin': ('gx_draw_done_wait_return', 'span'),
    'gx_token_wait_begin': ('gx_token_wait_return', 'span'),
    'vi_wait_begin': ('vi_wait_return', 'span'),
    'vi_timer_due': ('vi_timer_woke', 'span'),
    'vi_interrupt_lock_request': ('vi_interrupt_lock_acquired', 'span'),
    'vi_interrupt_enter': ('vi_interrupt_exit', 'span'),
    'gx_interrupt_lock_request': ('gx_interrupt_lock_acquired', 'span'),
    'gx_delivery_begin': ('gx_delivery_end', 'span'),
    'worker_begin': ('worker_end', 'span'),
}


def wait_chains(events):
    """Follow actual frontiers and resumed threads with indexed event lookups."""
    by_name, by_span, captures, grants, vi = {}, {}, {}, {}, {}
    for e in events:
        by_name.setdefault(e['event'], []).append(e)
        by_span.setdefault((e['event'], e['span']), []).append(e)
        if e['ticket']: captures.setdefault(e['ticket'], []).append(e)
        if e['event'] == 'thread_baton_assigned': grants.setdefault(e['subject'], []).append(e)
        if e['event'] == 'vi_interrupt_enter': vi.setdefault(e['detail'], []).append(e)
    grant_times = {key: [e['ns'] for e in value] for key, value in grants.items()}
    names = {'gx_draw_done_wait_begin': ('gx_draw_done_wait_return', 'gx_draw_done_frontier'),
             'gx_token_wait_begin': ('gx_token_wait_return', 'gx_token_frontier'),
             'vi_wait_begin': ('vi_wait_return', None)}
    frontiers = {name: by_name.get(name, []) for _, name in names.values() if name}
    frontier_times = {name: [e['ns'] for e in value] for name, value in frontiers.items()}
    result = []
    for begin in events:
        if begin['event'] not in names: continue
        end_name, frontier_name = names[begin['event']]
        ends = by_span.get((end_name, begin['span']), [])
        end = ends[0] if len(ends) == 1 else None
        chain = dict(wait=begin, returned=end, complete=False)
        if end:
            if frontier_name:
                values = frontiers[frontier_name]
                offset = bisect.bisect_left(frontier_times[frontier_name], begin['ns'])
                frontier = next((e for e in values[offset:offset + 16] if
                                 e['ns'] <= end['ns'] and e['value'] >= begin['detail']), None)
                # More than 16 intervening events is left unresolved, never
                # silently inferred as the same wake/frontier.
            else:
                frontier = next((e for e in vi.get(end['value'], []) if
                                 begin['ns'] <= e['ns'] <= end['ns']), None)
            resume = next((e for e in by_span.get(('thread_cpu_resumed', begin['span']), []) if
                           e['subject'] == begin['subject'] and begin['ns'] <= e['ns'] <= end['ns']), None)
            wake = next((e for e in by_span.get(('thread_wake_ready', frontier['span']), []) if
                         e['subject'] == begin['subject']), None) if frontier else None
            grant = None
            if resume and begin['subject'] in grants:
                offset = bisect.bisect_right(grant_times[begin['subject']], resume['ns']) - 1
                if offset >= 0:
                    candidate = grants[begin['subject']][offset]
                    if candidate['ns'] >= begin['ns']: grant = candidate
            chain.update(frontier=frontier, ready=wake, baton=grant, resumed=resume)
            chain['complete'] = all(x is not None for x in (frontier, wake, resume, grant))
            if frontier and frontier['ticket']:
                chain['capture_boundaries'] = captures.get(frontier['ticket'], [])
        result.append(chain)
    return result


def worst_rows(events, intervals, rows, count=10):
    by_event = {}
    for e in events: by_event.setdefault(e['event'], []).append(e)
    spans, missing = [], []
    for begin, (end, key) in PAIRS.items():
        ends = {}
        for e in by_event.get(end, []): ends.setdefault(e[key], []).append(e)
        for b in by_event.get(begin, []):
            if not b[key]: continue
            found = [e for e in ends.get(b[key], []) if e['ns'] >= b['ns']]
            if not found:
                missing.append(dict(begin=begin, key=key, identity=b[key], ns=b['ns']))
                continue
            e = found[0]
            spans.append(dict(begin=begin, end=end, key=key, identity=b[key],
                              start_ns=b['ns'], end_ns=e['ns'], duration_ns=e['ns'] - b['ns']))
    frame_intervals = {frame: (a, b) for a, b, frame in intervals}
    worst = []
    chains = wait_chains(events)
    for row in sorted(rows, key=lambda x: float(x['interval_ms']), reverse=True)[:count]:
        frame = int(row['frame'])
        if frame not in frame_intervals: continue
        a, b = frame_intervals[frame]
        overlaps = [dict(s, overlap_ns=min(b, s['end_ns']) - max(a, s['start_ns']))
                    for s in spans if s['start_ns'] < b and s['end_ns'] > a]
        overlaps.sort(key=lambda x: x['overlap_ns'], reverse=True)
        worst.append(dict(csv=row, start_ns=a, end_ns=b, overlapping_spans=overlaps,
                          wait_dependency_chains=[c for c in chains if c['wait']['ns'] < b and
                              (c['returned'] is None or c['returned']['ns'] > a)],
                          scheduler_and_present_events=[e for e in events if a <= e['ns'] < b and
                              (e['event'].startswith('thread_') or e['event'].startswith('present') or
                               e['event'].startswith('drawable_'))],
                          attribution='dependency evidence only; overlapping spans must not be summed'))
    return dict(worst=worst, missing_ends=missing,
                completion_limit='submit-to-queue-done-observed includes queue/GPU and callback dispatch')


def global100_frame(path):
    if path is None: return None
    matches = re.findall(r'\[graphics preparation\] event=actual-global-100 [^\n]*?frame=(\d+)', path.read_text())
    if len(matches) > 1: raise ValueError('multiple actual-global-100 events')
    return int(matches[0]) if matches else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--trace', type=Path)
    parser.add_argument('--frames', type=Path)
    parser.add_argument('--off-frames', type=Path)
    parser.add_argument('--on-frames', type=Path)
    parser.add_argument('--seconds', type=float, default=300)
    parser.add_argument('--off-log', type=Path)
    parser.add_argument('--on-log', type=Path)
    parser.add_argument('--binding', type=Path, required=True, help='preserved command/PREP provenance; SHA bound, not inferred')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    result = {'acceptance': False, 'binding': str(args.binding), 'binding_sha256': sha(args.binding)}
    if args.trace and args.frames:
        records = [json.loads(line) for line in args.trace.read_text().splitlines()]
        rows = csv_rows(args.frames)
        header = next((x for x in records if x.get('type') == 'header'), {})
        joined, intervals, errors, footer = join(records, rows)
        if header.get('binding_sha256') != sha(args.binding):
            errors.append('trace binding SHA does not match supplied provenance')
        attribution = worst_rows(joined, intervals, rows)
        if header.get('schema') == 2:
            scheduling = scheduling_audit(joined)
            errors.extend(scheduling['errors'])
            result['scheduling_attribution'] = scheduling
            for worst in attribution['worst']:
                start, end = worst['start_ns'], worst['end_ns']
                worst['baton_assignments'] = [x for x in scheduling['assignments'] if x['holder'] and
                    x['start_ns'] < end and x['end_ns'] > start]
                worst['primitive_waits'] = [x for x in scheduling['waits'] if x['start_ns'] < end and x['end_ns'] > start]
                worst['host_and_phase_events'] = [e for e in joined if start <= e['ns'] < end and
                    ('host' in e or e['event'].startswith(('startup_', 'packet_open_phase_', 'frame_slot_', 'frame_staging_', 'queue_callback_')))]
        result.update(trace_sha256=sha(args.trace), frames_sha256=sha(args.frames),
                      mapping_errors=errors, mapping_valid=not errors, footer=footer,
                      attribution_valid=not errors and bool(footer and footer.get('complete')) and
                          not attribution['missing_ends'], **attribution)
        with (args.output / 'JOINED.jsonl').open('w') as stream:
            for event in joined: stream.write(json.dumps(event) + '\n')
    if args.off_frames and args.on_frames:
        result.update(overhead=compare(csv_rows(args.off_frames), csv_rows(args.on_frames), args.seconds,
                                      global100_frame(args.off_log), global100_frame(args.on_log)),
                      off_log_sha256=sha(args.off_log) if args.off_log else None,
                      on_log_sha256=sha(args.on_log) if args.on_log else None,
                      off_sha256=sha(args.off_frames), on_sha256=sha(args.on_frames))
    if 'mapping_valid' not in result and 'overhead' not in result:
        parser.error('provide trace+frames and/or off-frames+on-frames')
    (args.output / 'REPORT.json').write_text(json.dumps(result, indent=2) + '\n')
    return 1 if result.get('mapping_errors') or result.get('attribution_valid') is False else 0

if __name__ == '__main__':
    raise SystemExit(main())
