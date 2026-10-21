"""Shared capture scheduling from actual post-endFrame observations."""
import hashlib
from pathlib import Path


def integer(value, field):
    if type(value) is int: return value
    if isinstance(value, str) and value and (value.isdecimal() or value[0] == '-' and value[1:].isdecimal()): return int(value)
    raise ValueError(f'unknown/noninteger observation: {field}')


class ReadySchedule:
    def __init__(self, stage, scenario, offsets=(60, 300, 600)):
        if not stage or type(scenario) is not int or scenario <= 0:
            raise ValueError('explicit stage and placed scenario required')
        if tuple(offsets) != tuple(sorted(set(offsets))) or any(type(offset) is not int or offset <= 0 for offset in offsets):
            raise ValueError('offsets must be positive, unique and ordered')
        self.stage, self.scenario, self.offsets = stage, scenario, offsets
        self.next_source = 0
        self.origin = None
        self.cancelled = None
        self.requests = []

    def observe(self, row, source_frame):
        if type(source_frame) is not int or source_frame != self.next_source:
            raise ValueError('post-endFrame source seams must be observed contiguously from zero')
        self.next_source += 1
        if integer(row['frame'], 'frame') != source_frame:
            raise ValueError('observation is already rebased or has a different source frame')
        valid = integer(row['scene_valid'], 'scene_valid')
        player = integer(row['player_valid'], 'player_valid')
        state = integer(row['init_state'], 'init_state')
        epoch = integer(row['scene_epoch'], 'scene_epoch')
        scenario = integer(row['scenario'], 'scenario')
        if valid not in (0, 1) or player not in (0, 1) or epoch < 0:
            raise ValueError('invalid observation validity/epoch')
        match = valid == 1 and row['scene'] == 'Game' and row['stage'] == self.stage and scenario == self.scenario and state == 6
        if self.origin is None:
            if not match: return None
            self.origin = dict(schema_version=1, phase='post-endFrame', stage=self.stage, scenario=self.scenario,
                               source_frame=source_frame, source_scene_epoch=epoch, player_valid=player,
                               rng_seed=row.get('rng_seed'), ticks=row.get('ticks'), vi_count=row.get('vi_count'),
                               readiness='first exact Game/init6 target seam; player validity separately observed')
        if self.cancelled: return None
        if not match or epoch != self.origin['source_scene_epoch']:
            self.cancelled = dict(source_frame=source_frame, reason='ready context/epoch changed; no reanchor')
            return None
        offset = source_frame - self.origin['source_frame']
        if offset not in self.offsets: return None
        request = dict(schema_version=1, status='PENDING_PRESENTATION', stage=self.stage, scenario=self.scenario,
                       offset=offset, ready_source_frame=self.origin['source_frame'], capture_source_frame=source_frame,
                       source_scene_epoch=epoch, player_valid=player,
                       ready_player_valid=self.origin['player_valid'])
        if not player or not self.origin['player_valid']:
            request.update(status='BLOCKED', reason='player unavailable at actual first ready or capture seam; not shifted')
        self.requests.append(request)
        return request


def presentation_receipt(request, copy, readback, image, method):
    """Validate source tags supplied by real renderer copy/readback callbacks."""
    if request['status'] != 'PENDING_PRESENTATION':
        raise ValueError('capture readiness is blocked')
    if method not in ('native-renderer-readback', 'dolphin-xfb-present-readback'):
        raise ValueError('unknown renderer provider')
    frame_id = copy.get('frame_id')
    if type(frame_id) is not int or frame_id < 0 or readback.get('frame_id') != frame_id:
        raise ValueError('unknown/mismatched copy and readback frame IDs')
    if copy.get('source_frame') != request['capture_source_frame'] or copy.get('scene_epoch') != request['source_scene_epoch']:
        raise ValueError('presented copy is from another game frame or scene')
    if readback.get('fence_complete') is not True or readback.get('success') is not True:
        raise ValueError('renderer readback has not completed successfully')
    path = Path(image).resolve()
    if (Path.home() / 'MacioGalaxy2-gamedata') not in path.parents:
        raise ValueError('real rendered images must stay private')
    h = hashlib.sha256(path.read_bytes()).hexdigest()
    if readback.get('image_sha256') != h:
        raise ValueError('image bytes differ from the renderer readback receipt')
    return dict(schema_version=1, status='captured', method=method, stage=request['stage'], scenario=request['scenario'],
                ready_source_frame=request['ready_source_frame'], capture_source_frame=request['capture_source_frame'],
                rendered_source_frame=copy['source_frame'], rendered_frame_id=frame_id,
                readback_frame_id=readback['frame_id'], render_fence_complete=True, image_sha256=h,
                qualification='UNQUALIFIED_VISUAL_DIAGNOSTIC')
