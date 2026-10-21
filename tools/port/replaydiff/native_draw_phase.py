#!/usr/bin/env python3
"""Source-bound draw-phase proposal; no runtime hooks or image/frame admission."""
import argparse
import hashlib
import json
from pathlib import Path

PINS = json.loads(Path(__file__).with_name('native_draw_phase_pins.json').read_text())


def audit_sources(root):
    root = Path(root)
    verified = {}
    for name, expected in PINS['files'].items():
        actual = hashlib.sha256((root / name).read_bytes()).hexdigest()
        if actual != expected:
            raise ValueError('immutable source drift: ' + name)
        verified[name] = actual
    return dict(revision=PINS['revision'], files=verified)


def frame_order(mode, previous_render, counter=0, interval=1,
                drawn_equals_displaying=True, copy_drawing_valid=True, single_flag=2):
    """Symbolic local guards, not index-state simulation or observed EFB contents."""
    if mode not in (1, 2, 3) or type(mode) is not int:
        raise ValueError('unsupported buffer mode')
    if type(previous_render) is not bool or type(counter) is not int or type(interval) is not int or counter < 0 or interval < 1:
        raise ValueError('invalid symbolic render controls')
    if type(drawn_equals_displaying) is not bool or type(copy_drawing_valid) is not bool or type(single_flag) is not int or not 0 <= single_flag <= 255:
        raise ValueError('invalid symbolic branch guards')
    events = ['beginRender.entry']
    if previous_render:
        if mode == 1:
            events += ['single.set-drawing-flag-1']
            if single_flag != 2:
                events += ['clearEfb.single-branch']
            events += ['single.select-drawing-index']
        elif mode == 2:
            if drawn_equals_displaying:
                if copy_drawing_valid:
                    events += ['pre-copy-callback.if-installed', 'prepareCopyDisp', 'GXCopyDisp.double-before-current-draw']
                events += ['double.exchange-indices']
            else:
                events += ['clearEfb.double-no-exchange', 'double.ensure-drawing-index']
        else:
            events += ['triple.direct-print.if-drawing-valid', 'triple.exchange-indices']
    current_render = counter + 1 >= interval
    events += ['beginRender.recompute-render-flag']
    if current_render:
        events += ['clearEfb.current-render', 'preGX']
    events += ['proposed.draw-entry-observation', 'GameSystem.draw.call-unconditional',
               'proposed.draw-exit-observation', 'endRender.endGX']
    if current_render:
        if mode == 1 and copy_drawing_valid:
            events += ['prepareCopyDisp', 'single.wait-draw-done-arm', 'single.publish-drawn-index']
        elif mode == 3 and copy_drawing_valid:
            events += ['pre-copy-callback.if-installed', 'prepareCopyDisp', 'GXCopyDisp.triple-after-current-draw', 'GXPixModeSync']
    events += ['update', 'calcAnim', 'screen-preserver-capture', 'endFrame', 'post-endFrame.ready-observation', 'retrace-wait']
    return dict(buffer_mode=mode,previous_render=previous_render,current_render=current_render,
                draw_called=True,events=events,events_are_symbolic=True,
                single_copy=dict(site='JUTVideo::preRetraceProc',
                                 guards=['buffer==1','mSDrawingFlag==0','drawn-index>=0','black-countdown-passed'],
                                 thread='VI interrupt/retrace context',
                                 relative_to_current_draw='UNBOUND; not inserted into this linear trace'),
                runtime_token_producer=False,qualification='BLOCKED')


def typed_draw_eligibility(snapshot, stage, placed_scenario):
    """Contract preconditions only; no mutable object traversal is performed here."""
    if type(stage) is not str or not stage or type(placed_scenario) is not int or placed_scenario < 1:
        raise ValueError('invalid exact target')
    reasons = []
    expected = dict(scene_valid=1,scene='Game',stage=stage,placed_scenario=placed_scenario,
                    init_state=6,current_scene_nonnull=True,executed_scene_is_current=True,
                    canonical_player_valid=True,observation_phase='before-GameSystem::draw',
                    game_cpu_owned=True)
    for key, value in expected.items():
        if type(snapshot.get(key)) is not type(value) or snapshot.get(key) != value:
            reasons.append('missing/mismatched ' + key)
    epoch = snapshot.get('scene_epoch')
    if type(epoch) is not int or epoch < 0:
        reasons.append('scene_epoch unavailable')
    if snapshot.get('scene_epoch_kind') != 'sampled-identity':
        reasons.append('unreviewed scene epoch authority')
    return dict(typed_candidate=not reasons,reasons=reasons,scene_epoch=epoch,
                scene_epoch_authoritative=False,runtime_token_producer=False,
                qualification='BLOCKED',qualified_frames=None)


def proposal():
    return dict(schema_version=1,status='SOURCE_PROPOSAL_ONLY',qualification='BLOCKED',qualified_frames=None,
                runtime_token_producer=False,existing_uniform_hook=False,
                required_owner='GAME: native-gated draw-entry/draw-exit callsites; lifecycle authority separately reviewed',
                native_implementation='deferred: existing native hooks cannot observe this boundary',
                draw_entry='after beginRender returns, immediately before GameSystem::draw',
                draw_exit='after GameSystem::draw returns, immediately before endRender',
                anchor_phase='existing post-endFrame observation, separate from draw-entry',
                source_frame_is_drawn_state=False,fixed_frame_lag=None,
                copy_to_draw_binding='UNBOUND: no EFB token propagation or worker changes included',
                gpu_completion='UNBOUND',color_encoding='UNBOUND',
                modes=[frame_order(mode,True) for mode in (1,2,3)])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-root',required=True)
    parser.add_argument('--out',required=True)
    args = parser.parse_args()
    result = proposal()
    result['source_binding'] = audit_sources(args.source_root)
    result['tool_sha256'] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    output = Path(args.out).resolve()
    output.relative_to(Path.home()/'MacioGalaxy2-gamedata')
    with output.open('x') as stream:
        json.dump(result,stream,indent=2)
        stream.write('\n')
    print(json.dumps({k:result[k] for k in ('status','existing_uniform_hook','qualification')}))
