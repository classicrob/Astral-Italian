"""One proposed Yoshi window reference at an observed stopped CPU seam."""
import argparse
import contextlib
import ctypes
import hashlib
import io
import json
import os
from pathlib import Path
import runpy
import struct
import sys
import time


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def require_current_ready(row):
    if not (row.get('frame') == 616 and row.get('scene_valid') == 1
            and row.get('scene') == 'Game' and row.get('stage') == 'YosshiHomeGalaxy'
            and row.get('scenario') == 1 and row.get('init_state') == 6
            and row.get('player_valid') == 1):
        raise ValueError('current row is not the requested ready Yoshi player observation')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--plan', required=True)
    parser.add_argument('--plan-sha256', required=True)
    args = parser.parse_args()
    plan_path = Path(args.plan)
    if sha(plan_path) != args.plan_sha256:
        raise ValueError('operator plan digest mismatch')
    plan = json.loads(plan_path.read_text())
    if plan.get('execution_enabled') is not True:
        raise ValueError('source proposal; no execution allocation')
    if plan['stage'] != 'YosshiHomeGalaxy' or plan['scenario'] != 1 or plan['inputs'] != 616:
        raise ValueError('only the exact one-case recipe is supported')
    if os.getpriority(os.PRIO_PROCESS, 0) < 19:
        raise ValueError('nice19 required')
    for record in plan['pins']:
        if sha(record['path']) != record['sha256']:
            raise ValueError('source/context drift: ' + record['path'])
    output = Path(plan['output'])
    if output.exists() or output.is_symlink():
        raise ValueError('fresh private output required')
    tools = Path(plan['snapshot']) / 'tools/port/replaydiff'
    sys.path.insert(0, str(tools))
    import dolphin_stage_entry as entry
    collector = entry.collector
    original_popen = collector.subprocess.Popen
    original_take = collector.Snapshot.take
    original_select = collector.StageSync.select
    state = {'process': None, 'remote': None, 'captured': False}

    def popen(argv, *a, **kw):
        if argv[0] == plan['dolphin'] and state['process'] is not None:
            raise ValueError('second emulator launch forbidden')
        child = original_popen(argv, *a, **kw)
        if argv[0] == plan['dolphin']:
            state['process'] = child
        return child

    def take(snapshot, frame):
        row = original_take(snapshot, frame)
        state['remote'] = snapshot.m.remote
        return row

    def select(sync, source):
        row = original_select(sync, source)
        if row is None or row['frame'] != 616:
            return row
        require_current_ready(row)
        if state['captured']:
            raise ValueError('second screenshot forbidden')
        remote, process = state['remote'], state['process']
        if process is None or process.poll() is not None:
            raise ValueError('registered emulator child absent')
        if remote.reg(64) != collector.SEAM or remote.reg(67) != collector.CALLER:
            raise ValueError('not stopped at accepted CPU seam')
        libproc = ctypes.CDLL('/usr/lib/libproc.dylib')
        libproc.proc_pidpath.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint32]
        libproc.proc_pidpath.restype = ctypes.c_int
        buf = ctypes.create_string_buffer(4096)
        if libproc.proc_pidpath(process.pid, buf, len(buf)) <= 0:
            raise ValueError('owned executable identity unavailable')
        if Path(os.fsdecode(buf.value)).resolve() != Path(plan['dolphin']).resolve():
            raise ValueError('owned executable identity mismatch')
        before = {'pc': remote.reg(64), 'lr': remote.reg(67)}
        saved_argv = sys.argv
        stream = io.StringIO()
        try:
            sys.argv = ['host_input.py', 'query', str(process.pid)]
            with contextlib.redirect_stdout(stream):
                runpy.run_path(plan['host_query'], run_name='__main__')
        finally:
            sys.argv = saved_argv
        metadata = json.loads(stream.getvalue())
        windows = metadata['windows']
        if not metadata['screen_capture_access'] or len(windows) != 1:
            raise ValueError('permission unavailable or ambiguous owned window; no guessed selection')
        window = windows[0]
        if window['pid'] != process.pid or window['bounds']['Width'] <= 0 or window['bounds']['Height'] <= 0:
            raise ValueError('invalid owned window')
        png = output / 'host-stopped-ready-plus616.png'
        if png.exists():
            raise ValueError('capture collision')
        argv = ['/usr/sbin/screencapture', '-x', '-l', str(window['window_id']), str(png)]
        began = time.time()
        child = original_popen(argv, stdout=collector.subprocess.PIPE, stderr=collector.subprocess.PIPE)
        try:
            stdout, stderr = child.communicate(timeout=10)
        except BaseException:
            child.kill()
            child.wait(timeout=3)
            raise
        if child.returncode != 0:
            raise ValueError('ordinary screenshot failed: ' + stderr.decode(errors='replace'))
        data = png.read_bytes()
        if len(data) < 24 or data[:8] != b'\x89PNG\r\n\x1a\n':
            raise ValueError('screenshot PNG header invalid; decode remains separate')
        after = {'pc': remote.reg(64), 'lr': remote.reg(67)}
        if before != after or process.poll() is not None:
            raise ValueError('stopped seam/process changed during screenshot')
        receipt = dict(status='DIAGNOSTIC_HOST_WINDOW_ONLY', qualification='BLOCKED', qualified_frames=None,
                       source_row=source, selected_row=row, sync_origin=sync.origin,
                       stop_before=before, stop_after=after, owned_pid=process.pid,
                       window_metadata=metadata, screenshot_argv=argv, capture_epoch=began,
                       screenshot_sha256=hashlib.sha256(data).hexdigest(), dimensions=list(struct.unpack('>II', data[16:24])),
                       screenshot_child_reaped=True, displayed_vi_draw_age='UNKNOWN', color_encoding='UNKNOWN',
                       native_pair_eligible=False, no_guest_continue_during_screenshot=True,
                       later_collector_behavior='unchanged end_frame/final seam instruction/cleanup; no no-advance claim for finalization')
        (output / 'host-stopped-reference.json').write_text(json.dumps(receipt, indent=2) + '\n')
        state['captured'] = True
        return row

    collector.subprocess.Popen = popen
    collector.Snapshot.take = take
    collector.StageSync.select = select
    sys.argv = [str(tools / 'dolphin_stage_entry.py'), *plan['collector_argv']]
    try:
        result = entry.main()
        if not state['captured']:
            raise ValueError('requested screenshot not produced')
        return result
    finally:
        collector.subprocess.Popen = original_popen
        collector.Snapshot.take = original_take
        collector.StageSync.select = original_select


if __name__ == '__main__':
    raise SystemExit(main())
