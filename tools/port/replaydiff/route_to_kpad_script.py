#!/usr/bin/env python3
"""Convert closed GX BoundState rows into explicitly reconstructed KPAD intent."""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path

from dolphin_collect import game_private_path
from kpad_script import load_script

BUTTONS = {'jump': 'A', 'crouch': 'Z', 'camera_left': 'LEFT', 'camera_right': 'RIGHT',
           'camera_center': 'C', 'camera_up': 'UP', 'camera_down': 'DOWN'}
BOOLS = list(BUTTONS) + ['spin', 'valid', 'dead']
REQUIRED = {'frame', 'stage', 'scenario', 'move_x', 'move_y', 'x', 'y', 'z', *BOOLS}
SPIN_POLICY = 'held-remote-x3-v1'


def convert(rows, stage='RedBlueExGalaxy', scenario=1, limit=None, anchor_override=None):
    if not rows or not REQUIRED <= rows[0].keys():
        raise ValueError('missing required CSV columns or rows')
    for frame, row in enumerate(rows):
        if None in row or None in row.values() or row['frame'] != str(frame):
            raise ValueError(f'malformed or noncontiguous source row {frame}')
        if any(row[key] not in ('0', '1') for key in BOOLS):
            raise ValueError(f'nonboolean source row {frame}')
        for key in ('move_x', 'move_y', 'x', 'y', 'z'):
            value = float(row[key])
            if not math.isfinite(value) or (key.startswith('move_') and not -1 <= value <= 1):
                raise ValueError(f'invalid {key} at source row {frame}')
    anchor = next((i for i, row in enumerate(rows) if row['stage'] == stage and
                   row['scenario'] == str(scenario) and row['valid'] == '1'), None)
    if anchor is None:
        raise ValueError('no valid target-stage anchor')
    first_valid_anchor = anchor
    if anchor_override is not None:
        if type(anchor_override) is not int or not anchor <= anchor_override < len(rows):
            raise ValueError('explicit anchor precedes first valid target or is out of range')
        candidate = rows[anchor_override]
        if candidate['stage'] != stage or candidate['scenario'] != str(scenario) or candidate['valid'] != '1':
            raise ValueError('explicit anchor must be valid in exact target stage/scenario')
        anchor = anchor_override
    end = next((i for i in range(anchor + 1, len(rows)) if rows[i]['stage'] != stage or
                rows[i]['scenario'] != str(scenario)), len(rows))
    if limit is not None:
        if limit < 1:
            raise ValueError('prefix must contain at least one input')
        end = min(end, anchor + 1 + limit)
    selected = rows[anchor + 1:end]
    if not selected:
        raise ValueError('no inputs after target anchor')
    segments = []
    spin_edges, previous_spin = [], False
    for frame, row in enumerate(selected):
        spin = row['spin'] == '1'
        if spin and not previous_spin:
            spin_edges.append(frame)
        previous_spin = spin
        intent = dict(buttons=[button for key, button in BUTTONS.items() if row[key] == '1'],
                      stick=[float(row['move_x']), float(row['move_y'])],
                      acc=[3, -1, 0] if spin else [0, -1, 0])
        if segments and all(segments[-1][key] == value for key, value in intent.items()):
            segments[-1]['end'] = frame + 1
        else:
            segments.append(dict(start=frame, end=frame + 1, **intent))
    script = dict(version=1, frames=len(selected), segments=segments)
    receipt = dict(schema_version=1, status='reconstructed-intent', qualification_status='BLOCKED',
                   qualified_divergences=None, source_rows=len(rows), anchor_source_frame=anchor,
                   anchor_reason=('explicit measured-phase candidate, separately bound evidence; not proof of exact ready seam'
                                  if anchor_override is not None else
                                  'first valid row matching exact stage and placed scenario; first-any-valid may be FileSelect'),
                   first_target_valid_source_frame=first_valid_anchor,
                   explicit_anchor_offset=anchor - first_valid_anchor,
                   anchor_method='explicit measured-phase candidate; not automatic alignment' if anchor_override is not None else 'first-target-valid proxy',
                   anchor_stage=stage, anchor_scenario=scenario,
                   anchor_position={axis: rows[anchor][axis] for axis in 'xyz'},
                   anchor_is_exact_ready_seam=False, input_source_start=anchor + 1,
                   input_source_end=end - 1, input_frames=len(selected), trace_frames=len(selected) + 1,
                   invalid_frames_retained=sum(row['valid'] == '0' for row in selected),
                   dead_frames_retained=sum(row['dead'] == '1' for row in selected),
                   selection_end_reason='EOF' if end == len(rows) else 'prefix limit or stage/scenario change',
                   input_origin='source anchor+1 maps to suffix0 and paired trace1; host consumed-input phase is a proxy',
                   spin_policy=SPIN_POLICY, spin_frames=sum(row['spin'] == '1' for row in selected),
                   spin_rising_frames=spin_edges, raw_gx_acceleration='UNKNOWN', raw_gx_rng='UNKNOWN',
                   synthetic_policy='neutral pointer absent, remote gravity [0,-1,0], held spin x acceleration +3; not native shake queue',
                   known_gaps=['no pointer or shoot/B columns: BOTH reconstructed replays omit unrecorded aiming/shooting',
                               'no raw KPAD read ordinals/channels/history/connection, GX RNG, exact init receipt or timing',
                               'stick rounded to 5 decimals; positions rounded to 3 decimals',
                               'BoundState sampled separately from consumed KPAD; host/GX input phase unproved'],
                   user_note="Oh bc I shot at an enemy at one place so that might be a divergence point then",
                   user_note_interpretation='Shot omission alone is not a Dolphin/native mismatch; first GX path departure needs separate evidence')
    return script, receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--source-sha256', required=True)
    parser.add_argument('--prefix', type=int)
    parser.add_argument('--anchor-source-frame', type=int)
    parser.add_argument('--anchor-evidence', type=Path)
    parser.add_argument('--spin-policy', choices=[SPIN_POLICY], required=True)
    args = parser.parse_args()
    source = game_private_path(args.source)
    raw = source.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != args.source_sha256:
        raise ValueError('closed source hash differs from pinned recording')
    rows = list(csv.DictReader(raw.decode('utf-8').splitlines()))
    if (args.anchor_source_frame is None) != (args.anchor_evidence is None):
        raise ValueError('explicit anchor requires a declared evidence file and vice versa')
    script, receipt = convert(rows, limit=args.prefix, anchor_override=args.anchor_source_frame)
    if args.anchor_evidence:
        evidence_path = game_private_path(args.anchor_evidence)
        evidence = json.loads(evidence_path.read_text())
        if evidence.get('candidate_anchor_source_frame') != args.anchor_source_frame or evidence.get('source_sha256') != digest:
            raise ValueError('anchor evidence does not bind candidate and source recording')
        receipt['anchor_evidence'] = dict(path=str(evidence_path), sha256=hashlib.sha256(evidence_path.read_bytes()).hexdigest(), evidence=evidence)
    if source.read_bytes() != raw:
        raise ValueError('source changed while converting')
    output = game_private_path(args.output)
    output.mkdir(parents=True, exist_ok=False)
    intent = output / 'intent.json'
    intent.write_text(json.dumps(script, separators=(',', ':')) + '\n')
    load_script(intent, script['frames'])
    receipt.update(source_path=str(source), source_sha256=digest,
                   converter_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                   script_path=intent.name, script_sha256=hashlib.sha256(intent.read_bytes()).hexdigest())
    (output / 'conversion.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print(json.dumps(receipt))


if __name__ == '__main__':
    main()
