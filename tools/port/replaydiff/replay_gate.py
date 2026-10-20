#!/usr/bin/env python3
"""Admission and orchestration for private native/original-game replay evidence.

This does not capture, repair, align, or qualify a producer by itself. Admission
is an independently reviewed, hash-bound prerequisite, not a producer role flag.
See README.md for the v1 fixture and its deliberately narrow observation scope.
"""
from __future__ import annotations

import argparse
from collections import defaultdict
import csv
import importlib.util
import hashlib
import json
from pathlib import Path
import re
import tempfile

import tracediff
from sync_validation import validate_sync

REPO = Path(__file__).resolve().parents[3]
CONTEXT = 'frame scene_epoch scene stage scenario init_state'.split()
ACTOR_FLOATS = 'x y z rot_x rot_y rot_z scale_x scale_y scale_z'.split()
ACTOR_COLUMNS = ('schema_version frame scene_epoch scene stage scenario init_state '
                 'producer_handle actor_id identity_kind identity_status name_valid name_hex '
                 'link_valid link_id link_zone link_context dead x x_bits y y_bits z z_bits '
                 'rot_x rot_x_bits rot_y rot_y_bits rot_z rot_z_bits '
                 'scale_x scale_x_bits scale_y scale_y_bits scale_z scale_z_bits').split()
CENSUS_COLUMNS = ('schema_version frame scene_epoch scene stage scenario init_state ready '
                  'registered living emitted identity_unresolved identity_ambiguous').split()
SIDES = ('native', 'dolphin', 'dolphin_repeat')
COMMON_ARTIFACTS = {'frames', 'actors', 'census', 'sync', 'manifest', 'executable', 'producer', 'input'}
COVERAGE = {'initialization', 'input_delivery', 'actor_identity', 'timing', 'floating_point', 'state_rng'}
CALLBACK_ONLY_NOTICE = 'unavailable (callback-context only)'
DIGEST = re.compile(r'[0-9a-f]{64}\Z')
HEX_NAME = re.compile(r'(?:[0-9a-f]{2}){0,1023}\Z')


class Blocked(ValueError):
    """An input is unavailable, malformed, or outside the admitted contract."""


def require(condition, message):
    if not condition:
        raise Blocked(message)


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, 'duplicate JSON key: ' + key)
        result[key] = value
    return result


def json_value(text):
    def nonfinite(token):
        raise Blocked('nonfinite JSON: ' + token)
    return json.loads(text, object_pairs_hook=unique_object, parse_constant=nonfinite)


def load_json(path):
    value = json_value(Path(path).read_text(encoding='utf-8'))
    require(isinstance(value, dict), str(path) + ': expected JSON object')
    return value


def number(value, low=0, high=0xffffffff):
    result = tracediff.integer(value)
    require(low <= result <= high, f'integer out of range: {value!r}')
    return result


def uint(value, low=0, high=0xffffffff):
    require(type(value) is int and low <= value <= high, f'expected integer {low}..{high}: {value!r}')
    return value


def table(path, columns):
    """Strict raw sidecar reader; zero actor rows are allowed only with census."""
    with Path(path).open(newline='', encoding='utf-8-sig') as source:
        reader = csv.DictReader(source, strict=True)
        require(reader.fieldnames == columns, str(path) + ': unexpected CSV schema/order')
        rows = []
        for line, row in enumerate(reader, 2):
            require(None not in row and None not in row.values(), f'{path}:{line}: wrong cell count')
            rows.append(row)
    return rows


def frame_columns():
    return load_json(REPO / 'native/app/headless/trace_schema.json')['columns']


def validate_frames(trace, expected, stage, scenario):
    require(trace.columns == frame_columns(), 'main trace must contain the entire current schema in order')
    require(not trace.errors, '; '.join(trace.errors))
    require(list(trace.rows) == [(n,) for n in range(expected)], 'main frames must be exactly 0..expected_frames-1')
    policy = tracediff.Policy()
    words = ('movement_lo movement_hi flags14_lo flags14_hi draw flags20 flags24_lo '
             'flags24_hi prev_draw hp rng_seed actor_registered actor_living').split()
    for (frame,), row in trace.rows.items():
        prefix = f'main frame {frame}: '
        require(not tracediff.validate_row(row, policy), prefix + 'invalid values/float bits')
        for field in ('scene_valid', 'player_valid', 'actor_count_valid', 'rng_valid', 'animation_valid'):
            require(row[field] == '1', prefix + field + ' unavailable; origin is never moved')
        require(row['scene'] == 'Game' and row['stage'] == stage and number(row['scenario'], 1) == scenario
                and row['init_state'] == '6', prefix + 'outside declared ready gameplay stage')
        number(row['scene_epoch'], high=0xffffffffffffffff)
        for field in words:
            number(row[field])
        require(number(row['actor_living']) <= number(row['actor_registered'], high=32768), prefix + 'invalid counts')
        number(row['mode'], high=65535)
        number(row['dead'], high=1)
        require(row['vi_count'] == row['ticks'] == '', prefix + 'reserved timing columns must remain blank')
        require(re.fullmatch(r'[01];[01];[01]', row['state_flags3']), prefix + 'invalid state_flags3')
        states = json_value(row['state_ids'])
        require(isinstance(states, list) and len(states) <= 64, prefix + 'invalid ordered state stack')
        for state in states:
            uint(state)
        if row['pending_state_id']:
            number(row['pending_state_id'])
        for field in tracediff.FLOATS & set(trace.columns):
            require(row[field] != '' and row[field + '_bits'] != '', prefix + field + ' unavailable')


def actor_policy(columns):
    """Project existing tolerances, with rotation/scale exact finite f32 values."""
    rules = {}
    base = tracediff.Policy()
    for field in ACTOR_FLOATS:
        rules['actor.' + field] = base.rule(field) if field in tracediff.FLOATS else dict(type='float', abs=0, rel=0)
    # Census/identity integers are exact; the raw validators also enforce ranges.
    for column in columns:
        if column.startswith('census.') or column.startswith('actor.'):
            original = column.split('.', 1)[1].removesuffix('_value')
            if original in CENSUS_COLUMNS[6:] or original in {
                    'schema_version', 'name_valid', 'link_valid', 'link_id', 'link_zone', 'link_context', 'dead'}:
                rules[column] = dict(type='int')
    return tracediff.Policy(overrides=rules)


def projected_column(kind, column):
    # Blank cells on census records are intentional, not false actor validity.
    suffix = '_value' if column.endswith('_valid') else ''
    return kind + '.' + column + suffix


def validate_actors(main, actors, census, expected):
    """Validate all raw rows before making the explicit actor/census projection.

    producer_handle is the ONLY removed producer field. It is checked within a
    run below and retained in the original pinned CSV, never used cross-run.
    """
    require(len(census) == expected, 'one census receipt is required for EVERY main frame')
    by_frame = defaultdict(list)
    previous = -1
    for row in actors:
        frame = number(row['frame'], high=expected - 1)
        require(frame >= previous, 'actor frames out of order')
        previous = frame
        by_frame[frame].append(row)
    columns = list(dict.fromkeys(CONTEXT + ['record_kind', 'record_id'] + [
        projected_column(kind, c) for kind, fields in (('actor', ACTOR_COLUMNS), ('census', CENSUS_COLUMNS))
        for c in fields if c not in CONTEXT and c != 'producer_handle']))
    projected = {}
    handle_keys, key_handles, previous_handles, seen_handles = {}, {}, set(), set()
    max_handle = 0
    epoch = None
    for frame, receipt in enumerate(census):
        main_row = main.rows[(frame,)]
        require(number(receipt['frame']) == frame, 'missing/duplicate/out-of-order census frame')
        require(receipt['schema_version'] == '1', 'unsupported census schema')
        for field in CONTEXT:
            require(receipt[field] == main_row[field], f'frame {frame}: census/main context {field} differs')
        this_epoch = number(receipt['scene_epoch'], high=0xffffffffffffffff)
        if this_epoch != epoch:
            require(epoch is None or this_epoch > epoch, 'scene epoch went backwards')
            handle_keys, key_handles, previous_handles = {}, {}, set()
            epoch = this_epoch
        rows = by_frame[frame]
        ready = main_row['scene'] == 'Game' and main_row['init_state'] == '6' and main_row['actor_count_valid'] == '1'
        require(receipt['ready'] == str(int(ready)), 'census ready disagrees with main coverage')
        require(number(receipt['emitted'], high=32768) == len(rows), 'census emitted does not match actual actor rows')
        require(number(receipt['identity_unresolved'], high=32768) == 0
                and number(receipt['identity_ambiguous'], high=32768) == 0,
                'actor identity unresolved/ambiguous: whole actor gate HOLD')
        if not ready:
            require(receipt['registered'] == receipt['living'] == '' and not rows, 'nonready census must be blank with no actors')
        else:
            registered = number(receipt['registered'], high=32768)
            living = number(receipt['living'], high=registered)
            require(registered == len(rows), 'registered/emitted count differs; actors were lost')
            require(registered == number(main_row['actor_registered']) and living == number(main_row['actor_living']),
                    'census counts disagree with main counts')
            require(living == sum(row['dead'] == '0' for row in rows), 'census living disagrees with actual rows')
        frame_keys, frame_handles, players = set(), set(), []
        for row in rows:
            require(row['schema_version'] == '1', 'unsupported actor schema')
            for field in CONTEXT:
                require(row[field] == main_row[field], f'frame {frame}: actor/main context {field} differs')
            handle = number(row['producer_handle'], 1, 0xffffffffffffffff)
            require(handle not in frame_handles, 'duplicate run-local actor handle')
            require(handle not in seen_handles or handle in previous_handles, 'absent actor reused old handle')
            if handle not in seen_handles:
                require(handle > max_handle, 'new actor handle is not a monotonic serial')
            frame_handles.add(handle)
            require(row['identity_status'] == 'source-key', 'actor status is not source-key: whole actor gate HOLD')
            name_valid = number(row['name_valid'], high=1)
            require(HEX_NAME.fullmatch(row['name_hex']), 'invalid/lossy/unbounded actor name_hex')
            require('00' not in [row['name_hex'][i:i + 2] for i in range(0, len(row['name_hex']), 2)], 'embedded NUL actor name')
            require(name_valid or row['name_hex'] == '', 'null name has nonempty bytes')
            links = [number(row[field], -32768, 32767) for field in ('link_zone', 'link_context', 'link_id')]
            require(number(row['link_valid'], high=1) == int(all(n >= 0 for n in links)), 'link_valid disagrees with actual signed16 fields')
            if row['identity_kind'] == 'player':
                key = 'player'
                players.append(row)
            else:
                require(row['identity_kind'] == 'link' and row['link_valid'] == '1' and name_valid == 1,
                        'source-key requires typed player or valid named link')
                key = 'link:' + ':'.join(map(str, links)) + ':' + row['name_hex']
            require(row['actor_id'] == key and key not in frame_keys, 'invalid/duplicate candidate actor identity')
            frame_keys.add(key)
            require(handle not in handle_keys or handle_keys[handle] == key, 'actor key-change within epoch: HOLD')
            require(key not in key_handles or key_handles[key] == handle, 'actor key-reuse within epoch: HOLD')
            handle_keys[handle], key_handles[key] = key, handle
            number(row['dead'], high=1)
            float_policy = tracediff.Policy(overrides={field: dict(type='float', abs=0, rel=0) for field in ACTOR_FLOATS})
            for field in ACTOR_FLOATS:
                require(row[field] != '' and row[field + '_bits'] != '', 'unavailable actor scalar')
            require(not tracediff.validate_row({k: v for k, v in row.items() if k in ACTOR_FLOATS or k.endswith('_bits')}, float_policy),
                    'invalid/nonfinite actor scalar or mismatched decimal/bits')
        if main_row['player_valid'] == '1' and ready:
            require(len(players) == 1, 'main player requires exactly one typed player actor row')
            player = players[0]
            for field in ('x_bits', 'y_bits', 'z_bits'):
                require(tracediff.bits_value(player[field]) == tracediff.bits_value(main_row[field]), 'same-producer player ' + field + ' disagrees')
            require(player['dead'] == main_row['dead'], 'same-producer player dead disagrees')
        # Serial creation is global, not actor order. The first observed frame
        # may include pre-anchor handles in arbitrary order; later new handles
        # must exceed ALL prior frames, including prior scene epochs.
        max_handle = max([max_handle, *frame_handles])
        previous_handles = frame_handles
        seen_handles.update(frame_handles)
        for kind, row, key in [('census', receipt, 'census')] + [('actor', r, f'{epoch}:' + r['actor_id']) for r in rows]:
            output = dict.fromkeys(columns, '')
            output.update({c: row[c] for c in CONTEXT})
            output.update(record_kind=kind, record_id=key)
            for field, value in row.items():
                if field not in CONTEXT and field != 'producer_handle':
                    output[projected_column(kind, field)] = value
            projected[(frame, kind, key)] = output
    return tracediff.Trace(columns, projected, [])


def artifact(spec, root, label, pins):
    require(isinstance(spec, dict) and set(spec) == {'path', 'sha256'}, label + ': require path/sha256 artifact')
    require(isinstance(spec['path'], str) and spec['path'], label + ': missing path')
    require(isinstance(spec['sha256'], str) and DIGEST.fullmatch(spec['sha256']), label + ': invalid SHA-256')
    path = (root / spec['path']).resolve()
    require(path.is_file() and not path.name.endswith('.partial'), label + ': missing/final artifact required')
    actual = tracediff.sha256(path)
    require(actual == spec['sha256'], label + ': hash mismatch')
    pins[label] = dict(path=str(path), sha256=actual)
    return path


def producer(kind, paths, manifest, expected, columns):
    """Check real producer fields as written today; never synthesize a status."""
    additional_artifacts = {}
    require(type(manifest.get('schema_version')) is int and manifest['schema_version'] == 1, kind + ': manifest schema')
    require(uint(manifest.get('frames'), 1) == expected, kind + ': producer declared length mismatch')
    require(not manifest.get('error') and not manifest.get('cleanup_error') and not manifest.get('validation_error'), kind + ': producer error')
    require(manifest.get('diagnostic_placeholder') is not True, 'diagnostic binary cannot qualify gameplay')
    require(not Path(str(paths['executable']) + '.diagnostic.json').exists(), 'diagnostic executable marker')
    if kind == 'native':
        require(manifest.get('status') == 'complete' and type(manifest.get('exit_status')) is int and manifest['exit_status'] == 0,
                'native requires actual complete manifest and exit 0')
        require(uint(manifest.get('trace_frames'), 1) == expected, 'native trace_frames mismatch')
        require(paths['completion'] == Path(str(paths['frames']) + '.complete')
                and paths['timing'] == Path(str(paths['frames']) + '.timing.csv'), 'native completion/timing paths mismatch')
        require(paths['completion'].read_text().strip() == str(expected), 'native completion marker count mismatch')
        require(manifest.get('columns') == columns, 'native manifest schema coverage differs')
        require(manifest.get('executable_sha256') == tracediff.sha256(paths['executable']), 'native executable binding mismatch')
        require(manifest.get('input_sha256') == tracediff.sha256(paths['input']), 'native actual input binding mismatch')
        require(manifest.get('sync_origin') == load_json(paths['sync']), 'native origin/manifest mismatch')
        # Reuse the existing launcher validator; this is not a timing substitute.
        headless = input_contract()
        require(headless.valid_timing(paths['timing'], expected), 'native timing sidecar incomplete/invalid')
        origin = load_json(paths['sync'])
        with paths['timing'].open(newline='') as source:
            first_time = next(csv.DictReader(source), {})
        require(first_time == dict(frame='0', vi_count=str(origin.get('source_vi_count')), ticks=str(origin.get('source_ticks'))),
                'native timing row zero disagrees with actual origin')
    else:
        require(manifest.get('status') == 'captured; repeat qualification required', 'Dolphin capture not complete')
        require(manifest.get('requested_frames') == expected, 'Dolphin requested length mismatch')
        require(manifest.get('frame_seam') == '804be280' and manifest.get('caller_lr') == '804b6ac4', 'Dolphin accepted seam mismatch')
        require(manifest.get('collector_sha256') == tracediff.sha256(paths['producer']), 'Dolphin collector binding mismatch')
        require(manifest.get('dolphin_sha256') == tracediff.sha256(paths['executable']), 'Dolphin executable binding mismatch')
        require(manifest.get('columns_sha256') == hashlib.sha256(','.join(columns).encode()).hexdigest(), 'Dolphin manifest schema binding mismatch')
        require(manifest.get('sync') == load_json(paths['sync']), 'Dolphin origin/manifest mismatch')
        require(manifest.get('input_method') in ('scripted filtered KPAD injection', 'read-only original KPADReadEx and verified WPADProbe-equivalent snapshot'),
                'Dolphin requires actual recorded input export')
        observation = manifest.get('connection_observation', {})
        require(isinstance(observation, dict), 'invalid Dolphin connection observation')
        require(not observation.get('within_read_transitions'), 'unrepresented intra-read connection transition')
        revisions = manifest.get('live_input_revisions', [])
        require(isinstance(revisions, list), 'invalid input revision history')
        if revisions:
            require(manifest['input_method'] == 'scripted filtered KPAD injection'
                    and len(revisions) == 1 and isinstance(revisions[0], dict)
                    and set(revisions[0]) == {'suffix_source_start'},
                    'live-edited input provenance requires separate review/protocol')
            source_start = uint(load_json(paths['sync']).get('source_frame'), high=0xffffffffffffffff) + 1
            require(uint(revisions[0]['suffix_source_start'], 1, 0xffffffffffffffff) == source_start
                    and uint(manifest.get('sync_input_source_start'), 1, 0xffffffffffffffff) == source_start,
                    'automatic suffix marker disagrees with actual source clock')
            digest = manifest.get('sync_input_script_sha256')
            require(isinstance(digest, str) and DIGEST.fullmatch(digest), 'automatic suffix requires declared script SHA-256')
            # The collector writes this exact copy before capture. Bind it just
            # like every other actual artifact, including independent admission.
            additional_artifacts['sync_input_script'] = dict(
                path=str(paths['manifest'].parent / 'sync-input-script.json'), sha256=digest)
        else:
            require('sync_input_source_start' not in manifest and 'sync_input_script_sha256' not in manifest,
                    'declared automatic suffix is missing its exact revision marker')
        receipt = manifest.get('input_export')
        require(isinstance(receipt, dict) and type(receipt.get('schema_version')) is int and receipt['schema_version'] == 1
                and receipt.get('status') == 'captured', 'actual Dolphin exported-input receipt is unavailable/incomplete')
        require(receipt.get('path') == paths['input'].name and paths['input'].parent == paths['manifest'].parent
                and receipt.get('sha256') == tracediff.sha256(paths['input']), 'Dolphin actual exported input path/hash mismatch')
        require(uint(receipt.get('bytes'), high=0xffffffffffffffff) == paths['input'].stat().st_size, 'Dolphin input export byte count mismatch')
        require(uint(receipt.get('read_count'), high=0xffffffffffffffff) == manifest.get('input_reads'), 'Dolphin input export read count mismatch')
    require(isinstance(manifest.get('command'), list) and manifest['command'] and all(isinstance(s, str) for s in manifest['command']),
            kind + ': actual producer launch command missing')
    return additional_artifacts


def actor_receipt(paths, manifest, actors, census):
    """Require the opt-in producer receipt, not just plausible adjacent CSVs.

    C emits this schema. A/D agreed to the same native schema; until the native
    launcher actually emits it, that provider is unavailable and BLOCKED.
    """
    receipt = manifest.get('actors')
    require(isinstance(receipt, dict), 'actual producer actor receipt is unavailable')
    require(type(receipt.get('schema_version')) is int and receipt['schema_version'] == 1
            and receipt.get('status') == 'captured', 'actor receipt is not captured v1')
    holds = sum(number(row['identity_unresolved']) + number(row['identity_ambiguous']) for row in census)
    for field, actual in (('frames', len(census)), ('actor_rows', len(actors)), ('identity_holds', holds)):
        require(uint(receipt.get(field), high=0xffffffffffffffff) == actual, 'actor receipt ' + field + ' mismatch')
    expected_files = {paths[field].name: tracediff.sha256(paths[field]) for field in ('actors', 'census')}
    files = receipt.get('files')
    require(isinstance(files, list) and len(files) == 2, 'actor receipt needs exactly both final file hashes')
    observed = {}
    for item in files:
        require(isinstance(item, dict) and set(item) == {'path', 'sha256'}, 'invalid actor receipt file binding')
        require(isinstance(item['path'], str) and item['path'] in expected_files and item['path'] not in observed,
                'actor receipt file is not the actual unique sidecar')
        observed[item['path']] = item['sha256']
    require(observed == expected_files, 'actor producer final sidecar hashes differ')


def input_contract():
    spec = importlib.util.spec_from_file_location('gate_headless_contract', REPO / 'native/tools/headless.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def input_window(paths, manifest, expected, native=False):
    """Compare the exact common input loops without shifting the anchor.

    Native D's explicit suffix protocol: tape0 drives trace1, N inputs produce
    N+1 rows. Dolphin export retains boot-clock labels; source_frame+1 is its
    first post-anchor loop. Prefix bytes and both origin receipts remain pinned.
    """
    contract = input_contract()
    observed_export = not native and manifest.get('input_method') == 'read-only original KPADReadEx and verified WPADProbe-equivalent snapshot'
    total = uint(manifest.get('source_frames') if observed_export else manifest.get('input_frames'), 1, 10000000)
    with tempfile.TemporaryFile() as encoded:
        contract.encode_rows(contract.rows_from_jsonl(paths['input']), total, encoded)
    origin = load_json(paths['sync'])
    source_start = uint(origin.get('source_frame'), 0, 0xffffffffffffffff) + 1
    if native:
        require(manifest.get('inputs_from_sync') is True
                and manifest.get('input_origin') == 'first loop after ready anchor',
                'native requires explicit committed inputs-from-sync mapping')
        require(total == expected - 1, 'native suffix N inputs must produce N+1 trace rows')
        if 'input_source_start' in manifest:
            require(uint(manifest['input_source_start'], high=0xffffffffffffffff) == source_start, 'native input source start disagrees with actual origin')
        start = 0
    else:
        require(uint(manifest.get('source_frames'), 1, 10000000) == source_start + expected - 1, 'Dolphin actual source-loop length disagrees with origin/window')
        start = source_start
        if 'sync_input_source_start' in manifest:
            require(uint(manifest['sync_input_source_start'], high=0xffffffffffffffff) == start, 'Dolphin input suffix source start disagrees with origin')
    require(start + expected - 1 <= total, 'input export does not cover full post-anchor window')
    rows = []
    channels = defaultdict(set)
    for row in contract.rows_from_jsonl(paths['input']):
        require(row['frame'] < (total if native else manifest['source_frames']), 'input export row outside actual completed source loops')
        if start <= row['frame'] < start + expected - 1:
            item = dict(row, frame=row['frame'] - start)
            rows.append(item)
            channels[item['frame']].add(item['channel'])
    require(all(channels[n] == {0, 1, 2, 3} for n in range(expected - 1)), 'post-anchor inputs need all four channels in EVERY loop')
    if 'input_reads' in manifest:
        require(manifest['input_reads'] == sum(1 for _ in contract.rows_from_jsonl(paths['input'])), 'input read count mismatch')
    return rows


def admission(fixture, root, pins):
    path = artifact(fixture.get('admission'), root, 'admission', pins)
    record = load_json(path)
    require(type(record.get('schema_version')) is int and record['schema_version'] == 1
            and record.get('decision') == 'admit-gameplay', 'missing reviewed gameplay admission')
    require(record.get('synthetic') is False, 'synthetic evidence cannot qualify a gameplay gate')
    for field in ('reviewer', 'rationale'):
        require(isinstance(record.get(field), str) and record[field].strip(), 'admission needs ' + field)
    required = {label: pin['sha256'] for label, pin in pins.items() if label != 'admission'}
    require(record.get('bindings') == required, 'admission must bind EVERY actual artifact and gate/schema/compare/input-contract version')
    require(isinstance(record.get('coverage'), dict) and set(record['coverage']) == COVERAGE,
            'admission requires explicit coverage review, not a role label')
    for field, evidence in record['coverage'].items():
        evidence_path = artifact(evidence, path.parent, 'review.' + field, pins)
        require(evidence_path.stat().st_size > 0, 'empty admission evidence: ' + field)
    return record


def observer_contract(manifest):
    """Check actual D145 metadata; no complete-notice provider exists yet.

    An owner admission cannot manufacture availability. A future complete or
    not-applicable contract requires its own provider/parent review; no string
    below is accepted as a success label or inferred from a blank CSV cell.
    """
    errors = []
    incomplete = manifest.get('observer_contract_incomplete')
    if type(incomplete) is not bool:
        errors.append('required observer_contract_incomplete is missing/unknown')
    elif incomplete:
        errors.append('producer declares observer_contract_incomplete=true')
    availability = manifest.get('field_availability')
    if not isinstance(availability, dict) or 'pending_state_id' not in availability:
        errors.append('required pending_state_id availability is missing/unknown')
    elif availability['pending_state_id'] == CALLBACK_ONLY_NOTICE:
        errors.append('pending_state_id is unavailable (callback-context only), not absent')
    else:
        errors.append('pending_state_id availability has no implemented/approved provider contract')
    return errors


def compare_pair(a, b, expected):
    frames = tracediff.compare(a['frames'], b['frames'], expected_frames=expected, history=120)
    actors = tracediff.compare(a['projected'], b['projected'], actor_keys=('record_kind', 'record_id'),
                               policy=actor_policy(a['projected'].columns), expected_frames=expected, history=120)
    return dict(frames=frames, actors=actors)


def evaluate(fixture_path):
    """Return finishline output; unavailable measurements always remain null."""
    fixture_pin = None
    report = dict(schema_version=1, status='BLOCKED', exit_code=2, errors=[],
                  measured_diverging_frames=None, measured_diverging_actor_records=None, measured_diverging_records=None,
                  first_divergence=None, prehistory=[], inputpins={}, comparisons=None,
                  scope='Admitted recorded gameplay fields only; not a universal state, lifetime, or numerical proof')
    pins = report['inputpins']
    try:
        fixture_path = Path(fixture_path).resolve()
        root = fixture_path.parent
        fixture = load_json(fixture_path)
        pins['fixture'] = dict(path=str(fixture_path), sha256=tracediff.sha256(fixture_path))
        # Fixture hash is recorded, but excluded from admission to avoid a cycle.
        fixture_pin = pins.pop('fixture')
        for name, path in (('gate', Path(__file__)), ('compare', Path(tracediff.__file__)),
                           ('sync_validator', Path(__file__).with_name('sync_validation.py')),
                           ('schema', REPO / 'native/app/headless/trace_schema.json'),
                           ('input_contract', REPO / 'native/tools/headless.py')):
            pins[name] = dict(path=str(path), sha256=tracediff.sha256(path))
        require(type(fixture.get('schema_version')) is int and fixture['schema_version'] == 1
                and fixture.get('gate_kind') == 'native-vs-dolphin-gameplay', 'unsupported fixture/gate kind')
        require(fixture.get('synthetic') is False, 'synthetic/M0 evidence is not gameplay admission')
        expected = uint(fixture.get('expected_frames'), 2, 10000000)
        stage = fixture.get('stage')
        require(isinstance(stage, str) and re.fullmatch(r'[A-Za-z0-9_]+', stage), 'canonical stage required')
        scenario = uint(fixture.get('scenario'), 1)
        captures, manifests, paths_by_side = {}, {}, {}
        windows = []
        contract_errors = []
        for side in SIDES:
            required = COMMON_ARTIFACTS | ({'timing', 'completion'} if side == 'native' else set())
            require(isinstance(fixture.get(side), dict) and set(fixture[side]) == required, side + ': missing/unknown capture artifacts')
            paths = {key: artifact(value, root, side + '.' + key, pins) for key, value in fixture[side].items()}
            require(paths['sync'] == Path(str(paths['frames']) + '.sync.json'), side + ': sync must be the actual trace sidecar')
            require(paths['actors'] == Path(str(paths['frames']) + '.actors.csv')
                    and paths['census'] == Path(str(paths['frames']) + '.actor-frames.csv'), side + ': actor sidecar paths mismatch')
            require(not any(Path(str(path) + '.partial').exists() for path in paths.values()), side + ': partial capture artifact remains')
            manifest = load_json(paths['manifest'])
            gaps = observer_contract(manifest)
            report.setdefault('observer_contracts', {})[side] = dict(
                qualified=not gaps, errors=gaps,
                observer_contract_incomplete=manifest.get('observer_contract_incomplete'),
                field_availability=manifest.get('field_availability'))
            contract_errors.extend(side + ': ' + gap for gap in gaps)
            main = tracediff.read_trace(paths['frames'])
            validate_frames(main, expected, stage, scenario)
            if isinstance(manifest.get('field_availability'), dict) and manifest['field_availability'].get('pending_state_id') == CALLBACK_ONLY_NOTICE:
                require(all(row['pending_state_id'] == '' for row in main.rows.values()),
                        side + ': declared unavailable pending_state_id must remain blank, never a substitute value')
            extra = producer(side, paths, manifest, expected, main.columns)
            for label, spec in extra.items():
                artifact(spec, root, side + '.' + label, pins)
            actors = table(paths['actors'], ACTOR_COLUMNS)
            census = table(paths['census'], CENSUS_COLUMNS)
            actor_receipt(paths, manifest, actors, census)
            projected = validate_actors(main, actors, census, expected)
            captures[side] = dict(frames=main, projected=projected)
            paths_by_side[side], manifests[side] = paths, manifest
            windows.append(input_window(paths, manifest, expected, native=side == 'native'))
        require(windows[0] == windows[1] == windows[2], 'actual post-anchor input reads differ (prefixes retained, no realignment)')
        sync_errors = []
        for left, right in (('native', 'dolphin'), ('dolphin', 'dolphin_repeat')):
            sync = validate_sync([paths_by_side[s]['frames'] for s in (left, right)],
                                 [captures[s]['frames'] for s in (left, right)], require=True,
                                 manifests=[paths_by_side[s]['manifest'] for s in (left, right)])
            report.setdefault('diagnostics', {}).setdefault('synchronization', {})[left + '/' + right] = sync
            sync_errors.extend(sync['errors'])
        if contract_errors:
            report['diagnostics']['observer_contract_comparisons'] = dict(
                qualification='UNQUALIFIED_OBSERVER_CONTRACT',
                scope='Recorded fields only; blank pending_state_id is not evidence of absence or full observer coverage. No gameplay divergence or equivalence verdict.',
                native_vs_dolphin=compare_pair(captures['native'], captures['dolphin'], expected),
                dolphin_repeat=compare_pair(captures['dolphin'], captures['dolphin_repeat'], expected))
            for label, pin in dict(pins, fixture=fixture_pin).items():
                require(tracediff.sha256(pin['path']) == pin['sha256'], label + ': input changed during diagnostic comparison')
        require(not contract_errors, '; '.join(contract_errors))
        require(not sync_errors, '; '.join(sync_errors))
        admission_record = admission(fixture, root, pins)
        report['admission'] = dict(reviewer=admission_record['reviewer'], rationale=admission_record['rationale'])
        pins['fixture'] = fixture_pin
        repeat = compare_pair(captures['dolphin'], captures['dolphin_repeat'], expected)
        report.setdefault('diagnostics', {})['repeat_qualification'] = repeat
        require(all(item['exit_code'] == 0 and item['first_bit_difference'] is None for item in repeat.values()), 'Dolphin gameplay repeat did not qualify; diagnostic repeat retained')
        comparisons = compare_pair(captures['native'], captures['dolphin'], expected)
        report['comparisons'] = comparisons
        require(all(item['exit_code'] != 2 for item in comparisons.values()), 'comparison rejected invalid/incomplete records')
        # Recheck every bound file after parsing/comparison: no stale hashes if
        # a supposedly final capture or review artifact changed during the run.
        for label, pin in pins.items():
            require(tracediff.sha256(pin['path']) == pin['sha256'], label + ': input changed during gate')
        frames, actors = comparisons['frames'], comparisons['actors']
        report['measured_diverging_frames'] = frames['summary']['diverging_frames']
        report['measured_diverging_actor_records'] = actors['summary'].get('diverging_rows', 0)
        report['measured_diverging_records'] = frames['summary'].get('diverging_rows', 0) + report['measured_diverging_actor_records']
        firsts = [dict(kind=kind, **item['first_divergence']) for kind, item in comparisons.items() if item['first_divergence']]
        if firsts:
            first = min(firsts, key=lambda row: (row['frame'], row['kind']))
            if first['kind'] == 'actors':
                first['kind'] = first['record_kind']
            report['first_divergence'] = first
            for frame in range(max(0, first['frame'] - 120), first['frame']):
                report['prehistory'].append(dict(frame=frame, **{
                    side: dict(main=captures[side]['frames'].rows[(frame,)], records=[row for key, row in captures[side]['projected'].rows.items() if key[0] == frame])
                    for side in ('native', 'dolphin')}))
        report['status'] = 'DIVERGED' if firsts else 'PASS'
        report['exit_code'] = 1 if firsts else 0
    except (Blocked, tracediff.TraceError, OSError, ValueError, TypeError, KeyError, csv.Error) as exc:
        report['errors'].append(str(exc))
    if fixture_pin is not None:
        pins['fixture'] = fixture_pin
    # No partial measurement can escape as a finishline result on failure.
    if report['status'] == 'BLOCKED':
        report['measured_diverging_frames'] = report['measured_diverging_actor_records'] = report['measured_diverging_records'] = None
        report['first_divergence'] = None
    return report


def markdown(report):
    status = 'PASS' if report['status'] == 'PASS' else (str(report['measured_diverging_records']) + ' divergences' if report['status'] == 'DIVERGED' else 'BLOCKED')
    lines = ['# Replay gameplay gate', '', 'status: ' + status, '', '**' + report['status'] + '**', '', report['scope'], '',
             'Divergences count differing comparison records (main-frame rows plus tagged actor/census rows), not unique bugs.', '',
             'Measured frame divergences: ' + str(report['measured_diverging_frames']),
             'Measured actor/census divergences: ' + str(report['measured_diverging_actor_records'])]
    lines.extend('\n- ' + error for error in report['errors'])
    if any(not record['qualified'] for record in report.get('observer_contracts', {}).values()):
        lines.extend(['', 'Observer coverage is unavailable/unknown. Any retained comparisons are explicitly unqualified diagnostics; a blank pending cell does not establish absence.'])
    if report['first_divergence']:
        first = report['first_divergence']
        lines.extend(['', 'First divergence (' + first['kind'] + '):', '', '```json', json.dumps(first, indent=2), '```'])
    lines.extend(['', 'Full hashes, origins, comparison details and up to 120 preceding complete frames are in gate.json.',
                  'BLOCKED has no measured divergence count. A synthetic test or Dolphin repeat alone cannot pass this gate.'])
    return '\n'.join(lines) + '\n'


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fixture', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True, help='new output directory; never overwrites evidence')
    args = parser.parse_args(argv)
    try:
        args.out.mkdir(parents=True, exist_ok=False)
    except OSError as exc:
        parser.error(str(exc))
    report = evaluate(args.fixture)
    (args.out / 'gate.json').write_text(json.dumps(report, indent=2, allow_nan=False) + '\n')
    (args.out / 'gate.md').write_text(markdown(report))
    print(report['status'] + ': ' + str(args.out / 'gate.md'))
    return report['exit_code']


if __name__ == '__main__':
    raise SystemExit(main())
