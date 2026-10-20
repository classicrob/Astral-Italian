#!/usr/bin/env python3
"""Reconstruct a recording's host intent as strict KPAD JSONL; never raw-input proof."""
import argparse
import csv
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import struct
import sys

_spec = importlib.util.spec_from_file_location('petari_kpad_tape', Path(__file__).with_name('kpad_tape.py'))
kpad = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(kpad)
BUTTONS = {'jump': 2048, 'crouch': 8192, 'camera_left': 1, 'camera_right': 2,
           'camera_up': 8, 'camera_down': 4, 'camera_center': 16384, 'shoot': 1024}
REQUIRED = {'frame', 'stage', 'scenario', 'valid', 'dead', 'move_x', 'move_y', 'spin',
            'x', 'y', 'z', *(set(BUTTONS) - {'shoot'})}
POINTER = {'shoot', 'pointer_valid', 'pointer_x', 'pointer_y'}
POLICY = 'host-intent-kpad-v1-preserve-pointer-held-remote-x3'


def real(value):
    # The shared encoder defines the representable finite f32 range. Never clamp.
    return kpad.floating(float(value))


def vector(*values):
    return {axis: real(value) for axis, value in zip('xyz', values)}


def checked_rows(path):
    with Path(path).open(newline='') as source:
        reader = csv.DictReader(source)
        if not reader.fieldnames or len(reader.fieldnames) != len(set(reader.fieldnames)):
            raise ValueError('missing/duplicate recording header')
        if not REQUIRED <= set(reader.fieldnames):
            raise ValueError('missing required host-intent columns')
        coverage = POINTER & set(reader.fieldnames)
        if coverage and coverage != POINTER:
            raise ValueError('partial pointer/shoot header; cannot invent the missing columns')
        rows = list(reader)
    if not rows:
        raise ValueError('empty recording')
    for frame, row in enumerate(rows):
        if None in row or None in row.values() or row['frame'] != str(frame):
            raise ValueError(f'malformed/noncontiguous source row {frame}')
        for key in [*(set(BUTTONS) - {'shoot'}), 'valid', 'dead', 'spin'] + (['shoot', 'pointer_valid'] if coverage else []):
            if row[key] not in ('0', '1'):
                raise ValueError(f'nonboolean {key} at source row {frame}')
        for key in ['move_x', 'move_y', 'x', 'y', 'z'] + (['pointer_x', 'pointer_y'] if coverage else []):
            value = float(row[key])
            if not math.isfinite(value) or (key in ('move_x', 'move_y') and not -1 <= value <= 1):
                raise ValueError(f'invalid {key} at source row {frame}')
    return rows, bool(coverage)


def intent(row, pointer_columns):
    sample = kpad.idle_sample(True)
    sample['hold'] = sum(bit for field, bit in BUTTONS.items() if row.get(field, '0') == '1')
    sample['ex_status']['fs']['stick'] = vector(row['move_x'], row['move_y'])
    acc = (3, -1, 0) if row['spin'] == '1' else (0, -1, 0)
    sample.update(acc=vector(*acc), acc_value=real(math.sqrt(sum(v*v for v in acc))))
    if pointer_columns and row['pointer_valid'] == '1':
        sample.update(pos=vector(row['pointer_x'], row['pointer_y']), dpd_valid_fg=2, dist=real(2))
    return sample


def sample_for(row, previous, pointer_columns):
    sample = intent(row, pointer_columns)
    before = intent(previous, pointer_columns) if previous is not None else kpad.idle_sample(True)
    hold, old = sample['hold'], before['hold']
    sample.update(trig=hold & ~old, release=old & ~hold)
    delta_acc = [sample['acc'][a]['value'] - before['acc'][a]['value'] for a in 'xyz']
    sample['acc_speed'] = real(math.sqrt(sum(v*v for v in delta_acc)))
    if sample['dpd_valid_fg'] and before['dpd_valid_fg']:
        delta = [sample['pos'][a]['value'] - before['pos'][a]['value'] for a in 'xy']
        sample.update(vec=vector(*delta), speed=real(math.hypot(*delta)))
    kpad.encode_sample(sample)  # Verify representability and the shared complete schema.
    return sample


def input_rows(rows, anchor, pointer_columns, frames=None):
    suffix = rows[anchor + 1:]
    if not suffix:
        raise ValueError('anchor has no following input')
    total = len(suffix) if frames is None else frames
    if total < len(suffix):
        raise ValueError('frame budget truncates the recording; choose an explicit earlier input file')
    previous = rows[anchor]
    for frame in range(total):
        if frame < len(suffix):
            row = suffix[frame]
        else:
            # Explicit postroll, never presented as recorded user input.
            row = dict(previous, move_x='0', move_y='0', spin='0', pointer_valid='0',
                       **{key: '0' for key in BUTTONS})
        sample = sample_for(row, previous, pointer_columns)
        previous = row
        for channel in range(4):
            yield dict(frame=frame, read_ordinal=channel, channel=channel, requested_length=120,
                       returned_count=1 if channel == 0 else 0, error=0 if channel == 0 else -2,
                       connection=dict(probe_error=0 if channel == 0 else -1, device_type=1 if channel == 0 else 253),
                       samples=[sample if channel == 0 else kpad.idle_sample(False)])


def provenance(rows, pointer_columns, anchor, stage, scenario, frames):
    source = rows[anchor + 1:]
    outside = [int(row['frame']) for row in rows if pointer_columns and row['pointer_valid'] == '1' and
               max(abs(float(row['pointer_x'])), abs(float(row['pointer_y']))) > 1]
    unrepresentable = []
    # Preserve invalid-pointer coordinates too as source provenance, but only a
    # valid sample is passed through to KPAD. Flag unsupported values, never clamp.
    for row in rows:
        if pointer_columns and row['pointer_valid'] == '1':
            try:
                real(row['pointer_x']); real(row['pointer_y'])
            except (ValueError, OverflowError, struct.error):
                unrepresentable.append(int(row['frame']))
    return dict(schema_version=1, status='reconstructed-host-intent', qualification_status='NOT_DOLPHIN_VALIDATED',
                policy=POLICY, source_rows=len(rows), pointer_shoot_columns_recorded=pointer_columns,
                source_shoot_frames=sum(row.get('shoot') == '1' for row in rows),
                source_pointer_valid_frames=sum(row.get('pointer_valid') == '1' for row in rows),
                pointer_outside_unit_square_source_frames=outside,
                unrepresentable_pointer_source_frames=unrepresentable,
                pointer_policy='preserve finite recorded x-right/y-down values as nearest f32, including outside +/-1; invalid pointer absent; no clamp',
                anchor_source_frame=anchor, anchor_stage=stage, anchor_placed_scenario=scenario,
                anchor_is_exact_ready_seam=False, anchor_provenance='explicit source row or first target/player-valid row; PROXY ONLY',
                input_source_start=anchor+1, input_source_end=len(rows)-1,
                recorded_input_frames=len(source), explicit_neutral_postroll_frames=frames-len(source), input_frames=frames,
                mapping='native exact ready seam=row0; tape0 next loop=row1; source proxy anchor+1=tape0; no fitted offset',
                boundary_history='source anchor retained for held edges, pointer/acceleration deltas; no neutral reset',
                captured=['host move/buttons/spin intent', 'host shoot and pointer intent' if pointer_columns else 'NO pointer/shoot recorded'],
                synthesized=['KPAD read schedule four channels/capacity120/one sample ch0', 'connection/error/device history',
                             'trig/release edges from predecessor hold', 'pointer delta/speed from predecessor when both valid',
                             'pointer distance2/horizon defaults; invalid-pointer position neutral',
                             'remote spin held acceleration [3,-1,0], rest [0,-1,0]', 'Nunchuk gravity and MotionPlus defaults',
                             'nearest-f32 encoding of rounded recording numbers'],
                unknown=['consumed KPAD filtering/history/latency', 'original raw RNG', 'exact original ready seam',
                         'initial-state equivalence and Dolphin equivalence'],
                recorded_stage_segments=stage_segments(rows),
                recorded_star_values=sorted(set(row.get('power_stars', '') for row in rows)))


def stage_segments(rows):
    segments = []
    for row in rows:
        key = (row['stage'], row['scenario'])
        if not segments or (segments[-1]['stage'], segments[-1]['scenario']) != key:
            segments.append(dict(stage=key[0], scenario=key[1], start=int(row['frame']), end=int(row['frame'])))
        else:
            segments[-1]['end'] = int(row['frame'])
    return segments


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recording', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True, help='fresh private directory')
    parser.add_argument('--stage', required=True)
    parser.add_argument('--scenario', type=int, required=True)
    parser.add_argument('--anchor', type=int)
    parser.add_argument('--frames', type=int, help='explicit total including labeled neutral postroll, never truncation')
    args = parser.parse_args(argv)
    root = Path(__file__).resolve().parents[2]
    for path in [args.recording, args.output]:
        resolved = path.resolve()
        if resolved == root or root in resolved.parents:
            raise ValueError('recording/output must remain private outside the checkout')
    rows, pointers = checked_rows(args.recording)
    anchor = args.anchor
    if anchor is None:
        anchor = next((i for i, row in enumerate(rows) if row['stage'] == args.stage and row['scenario'] == str(args.scenario) and row['valid'] == '1'), None)
    if anchor is None or not 0 <= anchor < len(rows)-1 or rows[anchor]['stage'] != args.stage or rows[anchor]['scenario'] != str(args.scenario):
        raise ValueError('invalid/missing exact stage/scenario source proxy anchor')
    frames = args.frames if args.frames is not None else len(rows)-anchor-1
    if not 1 <= frames <= 10000000 or frames < len(rows)-anchor-1:
        raise ValueError('invalid/truncating frame budget')
    manifest = provenance(rows, pointers, anchor, args.stage, args.scenario, frames)
    args.output.mkdir(mode=0o700, parents=True, exist_ok=False)
    manifest.update(source_sha256=hashlib.sha256(args.recording.read_bytes()).hexdigest(),
                    converter_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                    encoder_sha256=hashlib.sha256(Path(kpad.__file__).read_bytes()).hexdigest())
    try:
        if manifest['unrepresentable_pointer_source_frames']:
            raise ValueError('unrepresentable pointer rows listed in manifest; no tape emitted')
        jsonl = args.output/'inputs.jsonl'
        with jsonl.open('x') as out:
            for row in input_rows(rows, anchor, pointers, frames):
                out.write(json.dumps(row, separators=(',', ':'))+'\n')
        with (args.output/'inputs.kpd').open('xb') as out:
            kpad.encode_rows(kpad.rows_from_jsonl(jsonl), frames, out)
        manifest.update(jsonl_sha256=hashlib.sha256(jsonl.read_bytes()).hexdigest(),
                        tape_sha256=hashlib.sha256((args.output/'inputs.kpd').read_bytes()).hexdigest())
    except Exception as exc:
        manifest.update(status='conversion-failed', error=str(exc))
        (args.output/'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
        raise
    (args.output/'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
    print(json.dumps({key: manifest[key] for key in ['status', 'source_rows', 'input_frames', 'recorded_input_frames', 'explicit_neutral_postroll_frames', 'tape_sha256']}))
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (ValueError, OSError, OverflowError, struct.error) as exc:
        print(f'recording_to_kpad: {exc}', file=sys.stderr)
        sys.exit(2)
