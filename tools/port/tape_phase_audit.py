#!/usr/bin/env python3
"""Read-only schema2 and delivered-KPAD audit. Never repairs or relabels a tape."""
import csv
import hashlib
import json
import math
from pathlib import Path
import struct
from replay_tape import parse_actions, parse_kpd

PROTOCOL = 'game-scene-pre-read-v1'
POLICY = 'game-defined-scene-input-v2'


def need(value, why):
    if not value:
        raise ValueError(why)


def integer(v, low=0, high=10_000_000):
    return type(v) is int and low <= v <= high


def state(s):
    need(isinstance(s, dict), 'missing marker state')
    need(integer(s.get('input_frame')), 'invalid marker input frame')
    need(all(type(s.get(k)) is str for k in ('kind', 'scene', 'stage')), 'invalid marker strings')
    need(integer(s.get('scenario'), -1, 10000), 'invalid marker scenario')
    for k in ('first_normal_update', 'valid', 'grounded', 'dead', 'rng_observed', 'map_wait', 'galaxy_map'):
        need(type(s.get(k)) is bool, 'missing marker boolean ' + k)
    bits = s.get('position_bits')
    need(isinstance(bits, list) and len(bits) == 3, 'missing marker position bits')
    need(all(integer(b, 0, 0xffffffff) and math.isfinite(struct.unpack('>f', struct.pack('>I', b))[0]) for b in bits), 'nonfinite/invalid marker bits')
    need(integer(s.get('rng_seed'), 0, 0xffffffff), 'invalid marker RNG')


def key(s):
    return tuple(s[k] for k in ('kind', 'scene', 'stage', 'scenario'))


def has_control(sample):
    def word(at): return struct.unpack_from('>I', sample, at)[0]
    if any(word(at) for at in (0, 4, 8)) or 0 < sample[94] < 128:
        return True
    if sample[92] in (1, 6):
        return any(word(at) & 0x7fffffff for at in (96, 100))
    if sample[92] in (2, 7):
        return any(word(at) for at in (96, 100, 104)) or any(word(at) & 0x7fffffff for at in range(108, 129, 4))
    return False


def validate(meta, route, reads, actions=None):
    need(meta.get('schema') == 2 and meta.get('marker_protocol') == PROTOCOL, 'unsupported schema/marker protocol')
    need(meta.get('marker_error') == '', 'recorder rejected marker path')
    need(integer(meta.get('seams'), 2) and len(route) >= meta['seams'], 'route prefix is truncated')
    need(all(int(r['frame']) == i for i, r in enumerate(route[:meta['seams']])), 'route row gap')
    launch = meta.get('launch', {})
    need(type(launch.get('stage')) is str and integer(launch.get('scenario'), 1, 10000), 'missing exact launch metadata')
    phases = meta.get('input_phases')
    need(isinstance(phases, list) and phases, 'no game-defined phases; no initEnd fallback')
    byframe = {}
    for r in reads:
        byframe.setdefault(r['frame'], []).append(r)
    previous = None
    for i, p in enumerate(phases):
        need(p.get('id') == i and type(p.get('id')) is int, 'unordered phase IDs')
        first, end = p.get('first_input_frame'), p.get('end_input_frame_exclusive')
        need(integer(first, 1) and integer(end) and first < end <= meta['seams'], 'phase outside complete prefix')
        need(p.get('end_kind') in ('phase-transition', 'recording-prefix'), 'unknown phase exit')
        s = p.get('marker'); state(s)
        need(s['input_frame'] == first and s['kind'] in ('controller-normal', 'scenario-foreground'), 'wrong phase marker')
        if previous:
            need(previous['end_kind'] == 'phase-transition' and first >= previous['end_input_frame_exclusive'], 'overlapping/open intermediate phase')
        if s['kind'] == 'controller-normal' and not s['first_normal_update']:
            need(previous and previous['marker']['kind'] == 'scenario-foreground', 'Normal is not first update/overlay resume')
        for f in range(first, end):
            group = byframe.get(f, [])
            need(group and group[0]['channel'] == 0 and group[0]['ordinal'] == 0, 'phase frame has no first channel0 read')
        points = p.get('checkpoints'); need(isinstance(points, list), 'checkpoint list missing')
        last, seen = first, set()
        for point in points:
            s2 = point.get('state'); state(s2)
            need(key(s2) == key(s) and last <= s2['input_frame'] < end, 'checkpoint order/phase mismatch')
            last = s2['input_frame']
            need(point.get('kind') in ('map-wait', 'galaxy-map'), 'unknown map checkpoint')
            need(s2['map_wait' if point['kind'] == 'map-wait' else 'galaxy_map'], 'checkpoint predicate is false')
            item = (last, point['kind']); need(item not in seen, 'duplicate map checkpoint'); seen.add(item)
        previous = p
    for r in reads:
        if r['frame'] < meta['seams'] and not any(p['first_input_frame'] <= r['frame'] < p['end_input_frame_exclusive'] for p in phases):
            need(not any(has_control(s) for s in r['samples']), 'non-neutral control outside executable input phases')
    for (f, _), buttons in (actions or {}).items():
        if f < meta['seams'] and any(buttons):
            need(any(p['first_input_frame'] <= f < p['end_input_frame_exclusive'] for p in phases),
                 'mod action outside executable input phases')
    return {'phases': len(phases), 'active_input_loops': sum(p['end_input_frame_exclusive'] - p['first_input_frame'] for p in phases)}


def bootstrap(channel, capacity):
    connected = channel == 0
    s = bytearray(240)
    def word(at, value): struct.pack_into('>I', s, at, value)
    word(16, 0xbf800000)
    for at in (24, 52, 84): word(at, 0x3f800000)
    s[92], s[93], s[95] = (1, 0, 5) if connected else (253, 255, 0)
    if connected:
        word(108, 0xbf800000); word(116, 0x3f800000)
    return dict(channel=channel, capacity=capacity, returned=1 if connected else 0,
                error=0 if connected else -2, probe=0 if connected else -1,
                device=1 if connected else 253, samples=[bytes(s)])


def payload(r):
    return tuple(r[k] for k in ('channel', 'capacity', 'returned', 'error', 'probe', 'device')) + (tuple(r['samples']),)


def audit_inputs(canonical, actual, mapping, report):
    need(report.get('finished') is True and not report.get('stopped'), 'source replay did not finish strictly')
    need(report.get('anchor_policy') in ('interactive-and-play-ready-v1', POLICY), 'unsupported source replay policy')
    originals = {(r['frame'], r['ordinal']): r for r in canonical}
    delivered = {(r['frame'], r['ordinal']): r for r in actual}
    need(len(originals) == len(canonical) and len(delivered) == len(actual), 'duplicate raw read identity')
    ledger = {(r['actual_frame'], r['actual_ordinal']): r for r in mapping}
    need(len(ledger) == len(mapping), 'duplicate mapping identity')
    preopen = set(ledger) - set(delivered)
    need(all(f == 0 and ledger[f, o]['source'] == 'neutral' for f, o in preopen), 'missing actually captured input')
    need(not (set(delivered) - set(ledger)), 'actual raw read has no mapping')
    recorded_count = neutral_count = 0
    for identity, r in delivered.items():
        m = ledger[identity]
        need(all(m.get(k) == r[k] for k in ('channel', 'capacity', 'returned', 'error')), 'mapping differs from actual output')
        need(m.get('samples') == len(r['samples']), 'actual sample count differs')
        if m['source'] == 'recorded':
            idx = m.get('phase'); need(integer(idx, 0, len(report['segments'])-1), 'mapping phase missing')
            segment = report['segments'][idx]
            cp = segment.get('play_ready_state_check')
            if cp and cp.get('checked') and r['frame'] > cp['replay_frame']:
                expected_frame = r['frame'] - cp['frame_offset']
            else:
                expected_frame = r['frame'] - segment['scene_anchor_frame_offset']
            need(m.get('canonical_frame') == expected_frame, 'input offset differs from native state anchor')
            need(m.get('canonical_ordinal') == r['ordinal'], 'per-frame read ordinal shifted')
            expected = originals.get((expected_frame, r['ordinal']))
            need(expected is not None, 'mapping refers to absent canonical read')
            need(payload(r) == payload(expected), 'actual returned raw KPAD bytes/count/error/connection differ')
            recorded_count += 1
        else:
            need(m['source'] == 'neutral' and m.get('canonical_frame') is None, 'unknown input provenance')
            need(payload(r) == payload(bootstrap(r['channel'], r['capacity'])), 'bootstrap/post-roll input is not the declared neutral sample')
            neutral_count += 1
    return dict(actual_reads=len(actual), exact_recorded_reads=recorded_count, exact_neutral_reads=neutral_count,
                pre_recorder_open_neutral_reads=len(preopen), actual_payloads_independently_checked=True)


def audit_play_ready(source_meta, source_rows, actual_meta, actual_rows, report):
    expected, actual = source_meta.get('play_ready_seams', []), actual_meta.get('play_ready_seams', [])
    need(len(expected) == len(actual), 'missing/extra play-ready checkpoint (including resume)')
    evidence = []
    def value(row):
        return tuple(row[k] for k in ('stage', 'scenario', 'valid', 'grounded', 'dead')) + tuple(
            struct.pack('>f', float(row[k])).hex() for k in ('x', 'y', 'z'))
    for point, seen in zip(expected, actual):
        row = point['row']
        segment = next((s for s in report['segments'] if s['stage'] == point['stage'] and s['scenario'] == point['scenario']
                        and s['scene_anchor_recorded_frame'] <= row <= s['route_start_row'] + s['tape_frames']), None)
        need(segment is not None, 'play-ready checkpoint outside native segments')
        cp = segment.get('play_ready_state_check')
        offset = cp['frame_offset'] if cp and cp.get('checked') else segment['scene_anchor_frame_offset']
        need((point['stage'], point['scenario'], point['rng_seed']) == (seen['stage'], seen['scenario'], seen['rng_seed']) and
             seen['row'] == row + offset, 'play-ready state/RNG clock mismatch')
        need(value(source_rows[row]) == value(actual_rows[seen['row']]) and source_rows[row]['valid'] == '1',
             'play-ready position/state bits differ')
        evidence.append(dict(recorded=point, actual=seen, position_bit_exact=True, rng_verified=True,
                             restored_at_this_checkpoint=bool(cp and cp.get('recorded_frame') == row)))
    return evidence


def audit_actions(canonical, actual, mapping, source_seams, actual_seams):
    frames = {}
    byframe = {}
    for (f, mod), buttons in canonical.items():
        if any(buttons): byframe.setdefault(f, []).append((mod, buttons))
    for read in mapping:
        item = (read['source'], read.get('canonical_frame'))
        need(read['actual_frame'] not in frames or frames[read['actual_frame']] == item, 'mixed input anchors in one loop')
        frames[read['actual_frame']] = item
    expected = {}
    for frame, (kind, source) in frames.items():
        if frame >= actual_seams or kind != 'recorded': continue
        for mod, buttons in byframe.get(source, []): expected[frame, mod] = buttons
    seen = {k: v for k, v in actual.items() if k[0] < actual_seams and any(v)}
    need(seen == expected, 'actual consumed mod actions differ from canonical game-frame mapping')
    used = {source for kind, source in frames.values() if kind == 'recorded'}
    need(all(not any(v) or f in used or f >= source_seams for (f, _), v in canonical.items()), 'canonical mod action was omitted')
    return len(expected)


def input_phase_evidence(meta, reads):
    result = []
    for phase in meta['input_phases']:
        first, end = phase['first_input_frame'], phase['end_input_frame_exclusive']
        rows = [r for r in reads if first <= r['frame'] < end]
        item = dict(id=phase['id'], marker=phase['marker'], reads=len(rows),
                    first_pointer=None, first_stick=None)
        for read in rows:
            if read['channel'] != 0: continue
            for ordinal, sample in enumerate(read['samples'][:max(read['returned'], 0)]):
                event = dict(input_frame=read['frame'], phase_relative_loop=read['frame']-first,
                             read_ordinal=read['ordinal'], sample=ordinal)
                if item['first_pointer'] is None and 0 < sample[94] < 128:
                    item['first_pointer'] = dict(event, position_bits=list(struct.unpack_from('>II', sample, 32)))
                if sample[92] in (1, 6) and item['first_stick'] is None:
                    sticks = list(struct.unpack_from('>II', sample, 96))
                    if any(v & 0x7fffffff for v in sticks): item['first_stick'] = dict(event, stick_bits=sticks)
        result.append(item)
    return result


def input_gap_evidence(meta, reads):
    result, cursor = [], 0
    for phase in meta['input_phases'] + [dict(first_input_frame=meta['seams'], end_input_frame_exclusive=meta['seams'])]:
        end = phase['first_input_frame']
        if cursor < end:
            result.append(dict(first_input_frame=cursor, end_input_frame_exclusive=end,
                               input='neutral-no-executable-scene',
                               reads=sum(cursor <= r['frame'] < end for r in reads)))
        cursor = phase['end_input_frame_exclusive']
    return result


def audit_native(report, meta):
    need(report.get('finished') is True and not report.get('stopped') and
         report.get('mode') == 'whole-session' and report.get('interactive_position_bit_exact') is True,
         'native whole-session exact replay missing')
    segments = report.get('segments')
    need(isinstance(segments, list) and segments, 'native segments missing')
    for s in segments:
        frames, compared, skipped = (s.get(k) for k in ('tape_frames', 'rows_compared', 'identity_rows_skipped'))
        need(integer(frames) and integer(compared) and integer(skipped, 0, 30) and compared + skipped == frames + 1,
             'native state row accounting differs')
        need(s.get('pointer_rows_compared') == frames, 'native pointer row accounting differs')
        need(s.get('identity_offset_checked') is True and s.get('identity_offset_passed') is True and
             s.get('identity_offset_limit') == 30 and integer(s.get('identity_frame_offset'), -30, 30), 'native identity bound missing')
    if meta.get('schema', 1) == 1:
        need(report.get('anchor_policy') == 'interactive-and-play-ready-v1', 'legacy source policy changed')
        return
    need(report.get('anchor_policy') == POLICY and report.get('marker_protocol') == PROTOCOL and
         report.get('tape_schema') == 2 and report.get('legacy_policy') is False, 'schema2 report label missing')
    phases = report.get('input_phases')
    need(isinstance(phases, list) and len(phases) == len(meta['input_phases']) == len(segments), 'native phase accounting differs')
    for want, seen, segment in zip(meta['input_phases'], phases, segments):
        need(seen.get('id') == want['id'] and seen.get('recorded_marker') == want['marker'], 'native marker is not bound to canonical phase')
        actual = seen.get('actual_marker'); state(actual)
        need(all(seen.get(k) is True for k in ('marker_checked', 'marker_passed', 'rng_verified')), 'native marker check missing')
        need(all(actual[k] == v for k, v in want['marker'].items() if k not in ('input_frame', 'rng_seed')),
             'native marker state differs')
        need(seen.get('frame_offset') == actual['input_frame'] - want['first_input_frame'] == segment['scene_anchor_frame_offset'],
             'native marker feeding/state offset differs')
        need(segment.get('identity_rows_skipped') == 0, 'schema2 skipped a strict state row')
        ready = next((p for p in meta.get('play_ready_seams', []) if p['stage'] == want['marker']['stage'] and
                      p['scenario'] == want['marker']['scenario'] and want['first_input_frame'] <= p['row'] < want['end_input_frame_exclusive']), None)
        need(segment.get('play_ready_required') is (ready is not None), 'native required play-ready contract differs')
        if ready:
            need((segment.get('play_ready_state_check') or {}).get('recorded_frame') == ready['row'], 'native play-ready anchor differs')
        if want['marker']['rng_observed']:
            need(seen.get('rng_readback') == want['marker']['rng_seed'], 'native marker RNG differs')
        required = want['end_kind'] == 'phase-transition'
        need(seen.get('exit_required') is required and seen.get('end_input_frame_exclusive') == want['end_input_frame_exclusive'], 'native exit contract differs')
        need(not required or seen.get('exit_checked') is True and seen.get('exit_passed') is True, 'native phase exit not checked')
        need(seen.get('checkpoints_required') == seen.get('checkpoints_checked') == len(want['checkpoints']), 'native map checkpoint accounting differs')


def qualify(source, capture, report_path, map_path):
    source, capture = Path(source), Path(capture)
    original_meta = json.loads((source/'route.tape.json').read_text())
    meta = json.loads((capture/'route.tape.json').read_text())
    report = json.loads(Path(report_path).read_text())
    audit_native(report, original_meta)
    mapping = [json.loads(x) for x in Path(map_path).read_text().splitlines() if x]
    _, canonical = parse_kpd((source/'route.kpd').read_bytes(), allow_trailing=not original_meta.get('complete'))
    _, actual = parse_kpd((capture/'route.kpd').read_bytes())
    rows = list(csv.DictReader((capture/'route.csv').open()))
    source_rows = list(csv.DictReader((source/'route.csv').open()))
    need(meta.get('complete') is True and meta.get('lossy_samples') == meta.get('unencodable_samples') == 0, 'capture incomplete/lossy')
    actual_actions = parse_actions((capture/'route.actions').read_text())
    summary = validate(meta, rows, actual, actual_actions)
    proof = audit_inputs(canonical, actual, mapping, report)
    proof['exact_mod_action_frames'] = audit_actions(parse_actions((source/'route.actions').read_text()),
        actual_actions, mapping, original_meta['seams'], meta['seams'])
    checkpoints = audit_play_ready(original_meta, source_rows, meta, rows, report)
    need(proof['exact_recorded_reads'] > 0, 'no source inputs captured')
    need(meta['kpd_reads'] == len(actual), 'capture metadata read count differs')
    for seg in report['segments']:
        need(seg.get('completed') is True and not seg.get('diverged') and seg.get('max_position_error') == 0, 'strict source segment did not match')
        if seg.get('play_ready_required'):
            cp = seg.get('play_ready_state_check') or {}
            need(all(cp.get(k) is True for k in ('checked', 'passed', 'position_bit_exact')) and cp.get('rng', {}).get('verified') is True, 'play-ready/RNG checkpoint missing')
    files = [(source/n) for n in ('route.csv', 'route.kpd', 'route.tape.json', 'route.actions')]
    files += [(capture/n) for n in ('route.csv', 'route.kpd', 'route.tape.json', 'route.actions')]
    files += [Path(report_path), Path(map_path)]
    return dict(passed=True, source_schema=original_meta['schema'], captured_schema=2,
                source_policy=report['anchor_policy'], policy=POLICY, **summary, **proof, play_ready_checkpoints=checkpoints,
                captured_phase_inputs=input_phase_evidence(meta, actual),
                captured_non_interactive_gaps=input_gap_evidence(meta, actual),
                hashes={str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in files},
                scope='Actual post-replay output capture; no source failure replaced, no guessed alignment; neutral gaps explicit')
