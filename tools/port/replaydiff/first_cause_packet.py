#!/usr/bin/env python3
"""Seal an offline trace window without discarding its input ancestry."""
import argparse
import csv
import gzip
import hashlib
import json
import subprocess
from pathlib import Path


def sha(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def pin(path):
    return {'path': str(path.resolve()), 'bytes': path.stat().st_size, 'sha256': sha(path)}


def traces(native, dolphin, excluded, start, stop):
    first = {}
    window = []
    changes = []
    previous = None
    state_fields = ('scene_epoch', 'scene', 'stage', 'scenario', 'init_state',
                    'movement_lo', 'movement_hi', 'flags14_lo', 'flags14_hi',
                    'draw', 'flags20', 'flags24_lo', 'flags24_hi', 'state_flags3',
                    'state_ids', 'mode', 'anim_name', 'hp', 'dead')
    with native.open() as nf, dolphin.open() as df:
        nr, dr = csv.DictReader(nf), csv.DictReader(df)
        if nr.fieldnames != dr.fieldnames:
            raise ValueError('trace headers differ')
        count = 0
        while True:
            n, d = next(nr, None), next(dr, None)
            if n is None or d is None:
                if n is not None or d is not None:
                    raise ValueError('trace lengths differ')
                break
            if n['frame'] != str(count) or d['frame'] != str(count):
                raise ValueError('trace frames must be dense from zero')
            different = [k for k in nr.fieldnames if k not in excluded and n[k] != d[k]]
            for k in different:
                first.setdefault(k, {'frame': count, 'native': n[k], 'dolphin': d[k]})
            if start <= count <= stop:
                window.append({'frame': count, 'native': n, 'dolphin': d,
                               'different_fields': different})
                current = {side: {k: row[k] for k in state_fields if k in row}
                           for side, row in [('native', n), ('dolphin', d)]}
                if current != previous:
                    changes.append({'frame': count, 'state': current})
                previous = current
            count += 1
    return {'frames': count, 'first_by_field': first, 'window': window,
            'state_changes_in_window': changes}


def read_history(path, start, stop, source_origin=0):
    """Offsets point into the COMPLETE retained file, including all sample arrays."""
    groups, window, prefix = [], [], hashlib.sha256()
    group = None
    previous_key = None
    offset = 0
    with path.open('rb') as f:
        for raw in f:
            row = json.loads(raw)
            frame, ordinal, channel = (row[k] for k in ('frame', 'read_ordinal', 'channel'))
            key = (frame, ordinal)
            if previous_key is not None and key <= previous_key:
                raise ValueError('input read order is not strictly increasing')
            if len(row['samples']) != max(row['returned_count'], 1):
                raise ValueError('input history count mismatch')
            if group is None or group['frame'] != frame:
                if group is not None:
                    groups.append(group)
                if ordinal != 0:
                    raise ValueError('read group does not begin at ordinal zero')
                group = {'frame': frame, 'byte_start': offset, 'byte_end': offset,
                         'reads': 0, 'channels': [], 'samples': 0}
            elif ordinal != group['reads']:
                raise ValueError('read ordinal gap')
            group['byte_end'] = offset + len(raw)
            group['reads'] += 1
            group['channels'].append(channel)
            group['samples'] += len(row['samples'])
            if frame < start:
                prefix.update(raw)
            if start <= frame <= stop:
                window.append({'source_frame': frame, 'input_frame': frame-source_origin-1,
                               'affected_trace_frame': frame-source_origin,
                               'byte_start': offset, 'byte_end': offset+len(raw), 'record': row})
            offset += len(raw)
            previous_key = key
    if group is not None:
        groups.append(group)
    return {'records': sum(g['reads'] for g in groups), 'groups': groups,
            'window': window, 'preceding_bytes_sha256': prefix.hexdigest(),
            'preceding_bytes_end': next((g['byte_start'] for g in groups if g['frame'] >= start), offset)}


def source_inventory(repo, old, current, out):
    def git(*args):
        return subprocess.check_output(['git', '-C', str(repo), *args])
    old = git('rev-parse', old+'^{commit}').decode().strip()
    current = git('rev-parse', current+'^{commit}').decode().strip()
    scope = ['native', 'src', 'include', 'libs']
    diff = git('diff', '--no-ext-diff', '--binary', old, current, '--', *scope)
    (out/'source-difference.patch.gz').write_bytes(gzip.compress(diff, mtime=0))
    files = ['native/app/headless/observe.cpp', 'native/src/mtx.cpp',
             'native/src/intrinsics.cpp', 'native/include/petari/ppc_float.hpp',
             'native/include/petari/ppc_math.hpp', 'src/Game/Player/Mario.cpp',
             'src/Game/Player/MarioActor.cpp', 'src/Game/Player/MarioSMG2_A.inc',
             'src/Game/Util/MathUtil.cpp', 'include/Game/Util/MathUtil.hpp',
             'libs/JSystem/include/JSystem/JGeometry/TVec.hpp']
    blobs = {}
    for rev in (old, current):
        available = set(git('ls-tree', '-r', '--name-only', rev, '--', *files).decode().splitlines())
        blobs[rev] = {p: hashlib.sha256(git('show', rev+':'+p)).hexdigest()
                      if p in available else None for p in files}
    return {'original': old, 'current': current, 'scope': scope, 'selected_blob_sha256': blobs,
            'diff': pin(out/'source-difference.patch.gz'),
            'changes': git('diff', '--name-status', old, current, '--', *scope).decode().splitlines(),
            'interpretation': 'Exact source delta; no attribution of runtime cause or original compiler/FP mode.'}


def seal(root, out, acceptance_sha, start=880, stop=910, source_repo=None, original_source=None):
    if not 1 <= start <= stop:
        raise ValueError('window must include positive trace frames in ascending order')
    ap = root/'acceptance.json'
    if sha(ap) != acceptance_sha:
        raise ValueError('acceptance hash mismatch')
    acceptance = json.loads(ap.read_text())
    before = [pin(Path(e['path'])) for e in acceptance['evidence']]
    for actual, expected in zip(before, acceptance['evidence']):
        if actual['sha256'] != expected['sha256'] or actual['bytes'] != expected['bytes']:
            raise ValueError('evidence changed: '+actual['path'])
    native, dolphin = root/'native-observed/native.csv', root/'full-30000/capture/oracle.csv'
    result = traces(native, dolphin, acceptance['primary']['unavailable_excluded_fields'], start, stop)
    earliest = min((v['frame'] for v in result['first_by_field'].values()), default=None)
    if earliest != acceptance['primary']['raw_first_difference']['frame']:
        raise ValueError('independent raw chronology disagrees with acceptance')
    if result['frames'] != acceptance['frames']['trace_rows_per_producer']:
        raise ValueError('trace count disagrees with acceptance')
    origin = acceptance['dolphin']['sync_origin']['source_frame']
    actual = root/'full-30000/capture/observed-inputs.jsonl'
    full = root/'full-30000/capture/inputs.jsonl'
    # A trace row consumes the immediately preceding suffix input frame.
    normalized = read_history(actual, start-1, stop-1, -1)
    exported = read_history(full, origin+start, origin+stop, origin)
    if [x['record'] for x in normalized['window']] != [dict(x['record'], frame=x['input_frame']) for x in exported['window']]:
        raise ValueError('normalized suffix differs from full exported records')
    if before != [pin(Path(e['path'])) for e in acceptance['evidence']]:
        raise ValueError('evidence changed during sealing')
    out.mkdir(parents=True, exist_ok=False)
    def write(name, data):
        (out/name).write_text(json.dumps(data, indent=2)+'\n')
    write('chronology.json', result)
    write('input-window.json', {'normalized': normalized['window'], 'full_export': exported['window']})
    groups = {
        'normalized': {k:v for k,v in normalized.items() if k != 'window'},
        'full_export': {k:v for k,v in exported.items() if k != 'window'}}
    (out/'input-groups.json.gz').write_bytes(gzip.compress(json.dumps(groups, separators=(',', ':')).encode(), mtime=0))
    sources = source_inventory(source_repo, original_source, acceptance['native']['source'], out) if source_repo else None
    if sha(ap) != acceptance_sha:
        raise ValueError('acceptance changed during sealing')
    receipt = {'schema_version': 1, 'qualification': 'BLOCKED', 'measured_qualified_frames': None,
               'acceptance': pin(ap), 'trace_window': [start, stop], 'raw_first_frame': earliest,
               'default_first_from_pinned_acceptance': acceptance['primary']['default_tolerance_first_difference'],
               'recording_fidelity': acceptance['recording_fidelity'],
               'producer_policy': {k:acceptance[k] for k in ('native','dolphin','input_delivery')},
               'retained_complete_evidence': before,
               'history_policy': 'Complete original files retained in place and hash checked before/after. All read groups indexed, full samples retained. Window is not a standalone replay or equivalent initial state.',
               'source_inventory': sources,
               'source_cause': 'UNESTABLISHED; first recorded discrepancy is not necessarily first internal divergence',
               'outputs': [pin(out/n) for n in ('chronology.json','input-window.json','input-groups.json.gz')],
               'tool': pin(Path(__file__))}
    write('receipt.json', receipt)
    return receipt


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--acceptance-sha256', required=True)
    p.add_argument('--start', type=int, default=880)
    p.add_argument('--stop', type=int, default=910)
    p.add_argument('--source-repo', type=Path)
    p.add_argument('--original-source')
    args = p.parse_args()
    if bool(args.source_repo) != bool(args.original_source):
        p.error('--source-repo and --original-source must be supplied together')
    r = seal(args.root, args.out, args.acceptance_sha256, args.start, args.stop, args.source_repo, args.original_source)
    print(json.dumps({'receipt': str(args.out/'receipt.json'), 'raw_first_frame': r['raw_first_frame'], 'qualification': r['qualification']}))
