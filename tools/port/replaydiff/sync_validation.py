"""Validate stage-sync provenance against frame traces without changing their rows."""
import hashlib
import json
from pathlib import Path


PROVENANCE = {'tick_hz_basis', 'rate_basis', 'fp_policy'}
REQUIRED_POLICY = {'tick_hz', 'rate_numerator', 'rate_denominator', 'fpcr_fz', 'audio_output', 'audio_processing'}


def validate_sync(paths, traces, require=False, manifests=(None, None)):
    sidecars = [Path(str(path) + '.sync.json') for path in paths]
    enabled = require or any(path.exists() for path in sidecars) or any(manifests)
    result = dict(enabled=enabled, compatible=False, errors=[], sides={},
                  qualification='Metadata checks only; no proof of initialization, scheduling, or replay equivalence')
    if not enabled:
        result['status'] = 'not-requested'
        return result
    errors = result['errors']

    def reject(message):
        errors.append('synchronization: ' + message)

    def load(path, side, kind):
        def unique(pairs):
            obj = {}
            for key, value in pairs:
                if key in obj:
                    raise ValueError('duplicate JSON key ' + key)
                obj[key] = value
            return obj
        try:
            data = path.read_bytes()
            obj = json.loads(data, object_pairs_hook=unique,
                             parse_constant=lambda token: (_ for _ in ()).throw(ValueError('nonfinite JSON ' + token)))
            if not isinstance(obj, dict):
                raise ValueError('expected JSON object')
            return dict(path=str(path), sha256=hashlib.sha256(data).hexdigest(), data=obj)
        except (OSError, ValueError) as exc:
            reject(f'{side} {kind}: {exc}')
            return dict(path=str(path), data={})

    def uint(value, bits):
        return type(value) is int and 0 <= value < 1 << bits

    def csv_integer(value):
        try:
            if not isinstance(value, str) or not value:
                return None
            return int(value, 16 if value.lstrip('+-').lower().startswith('0x') else 10)
        except ValueError:
            return None

    policies = []
    origins = []
    for index, (path, trace, sidecar) in enumerate(zip(paths, traces, sidecars)):
        side = 'ab'[index]
        record = load(sidecar, side, 'sidecar')
        origin = record['data']
        result['sides'][side] = dict(sync=record)
        origins.append(origin)
        if type(origin.get('schema_version')) is not int or origin.get('schema_version') != 1:
            reject(side + ' sidecar schema_version must be 1')
        if origin.get('phase') != 'post-endFrame':
            reject(side + ' phase must be post-endFrame')
        stage = origin.get('stage')
        if not isinstance(stage, str) or not stage or not all(c.isascii() and (c.isalnum() or c == '_') for c in stage):
            reject(side + ' stage must be a canonical internal stage name')
        if not uint(origin.get('scenario'), 32) or origin.get('scenario') == 0:
            reject(side + ' scenario must be a positive placed ScenarioNo')
        for field in ('source_frame', 'source_scene_epoch'):
            if not uint(origin.get(field), 64):
                reject(side + ' ' + field + ' must be u64')
        for field in ('source_ticks', 'source_vi_count'):
            if field not in origin or (origin[field] is not None and not uint(origin[field], 64)):
                reject(side + ' ' + field + ' must be u64 or null')
        if origin.get('rng_valid') is not True or not uint(origin.get('rng_seed'), 32):
            reject(side + ' requires an observed valid u32 RNG seed at the first qualifying seam')
        if type(origin.get('rng_forced')) is not bool:
            reject(side + ' rng_forced must be boolean')
        if any(len(key) != 1 for key in trace.rows):
            reject(side + ' sync validation requires the normal frame trace; use it as the actor-table gate')
        row = trace.rows.get((0,))
        if row is None:
            reject(side + ' has no frame-zero origin row')
        else:
            expected = dict(schema_version=1, frame=0, scene_epoch=0, scene_valid=1,
                            scenario=origin.get('scenario'), init_state=6, rng_valid=1,
                            rng_seed=origin.get('rng_seed'))
            for field, value in expected.items():
                if csv_integer(row.get(field)) != value:
                    reject(f'{side} row 0 {field} disagrees with the sync contract')
            if row.get('scene') != 'Game' or row.get('stage') != stage:
                reject(side + ' row 0 must be Game at the declared stage')
        manifest_path = manifests[index]
        if manifest_path is None:
            candidate = Path(str(path) + '.manifest.json')
            manifest_path = candidate if candidate.exists() else Path(path).parent / 'manifest.json'
        manifest = load(Path(manifest_path), side, 'manifest')
        result['sides'][side]['manifest'] = {key: value for key, value in manifest.items() if key != 'data'}
        metadata = manifest['data']
        if str(metadata.get('status', '')).lower().startswith(('incomplete', 'failed', 'error')):
            reject(side + ' manifest declares an incomplete/failed capture')
        policy = metadata.get('timing_policy')
        result['sides'][side]['timing_policy'] = policy
        if not isinstance(policy, dict):
            reject(side + ' manifest requires explicit timing_policy; defaults are not inferred')
            policies.append(None)
            continue
        missing = REQUIRED_POLICY - policy.keys()
        if missing:
            reject(side + ' timing_policy lacks ' + ', '.join(sorted(missing)))
        for field in ('tick_hz', 'rate_numerator', 'rate_denominator'):
            if not uint(policy.get(field), 64) or policy.get(field) == 0:
                reject(side + ' timing_policy.' + field + ' must be an observed positive integer')
        if type(policy.get('fpcr_fz')) is not bool:
            reject(side + ' timing_policy.fpcr_fz must be an observed boolean')
        for field in ('audio_output', 'audio_processing'):
            if not isinstance(policy.get(field), str) or not policy.get(field):
                reject(side + ' timing_policy.' + field + ' must be declared')
        substantive = {key: value for key, value in policy.items() if key not in PROVENANCE}
        if any(value is None for value in substantive.values()):
            reject(side + ' timing_policy contains unobserved values')
        if 'epoch_ticks' in policy and not uint(policy['epoch_ticks'], 64):
            reject(side + ' timing_policy.epoch_ticks must be u64')
        if 'rtc_epoch' in metadata:
            substantive['rtc_epoch'] = metadata['rtc_epoch']
        policies.append(substantive)
    for field in ('phase', 'stage', 'scenario', 'rng_seed'):
        if origins[0].get(field) != origins[1].get(field):
            reject('incompatible ' + field + ' between traces')
    if policies[0] is not None and policies[1] is not None:
        for key in sorted(policies[0].keys() | policies[1].keys()):
            if key not in policies[0] or key not in policies[1] or policies[0][key] != policies[1][key] or type(policies[0][key]) is not type(policies[1][key]):
                reject('incompatible timing_policy.' + key)
    result['compatible'] = not errors
    result['status'] = 'compatible' if not errors else 'unqualified'
    return result
