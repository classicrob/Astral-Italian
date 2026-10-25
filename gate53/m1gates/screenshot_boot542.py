#!/usr/bin/env python3
"""Observation only, via the specifically authorized LaunchServices Python.app.

Never requests Screen Recording/Automation access. Its own read-only preflight
must be true before trying the owned-window screenshot. Refusal is retained.
"""
import argparse
import ctypes as C
import json
import os
from pathlib import Path
import subprocess
import time
from boot_frames542 import window_info

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--pid',type=int,required=True);p.add_argument('--window-id',type=int,required=True)
    p.add_argument('--app',type=Path,required=True);p.add_argument('--out',type=Path,required=True);p.add_argument('--result',type=Path,required=True)
    args=p.parse_args();root=(Path.home()/'MacioGalaxy2-gamedata/port53/m1g').resolve()
    for path in (args.out,args.result):
        if not path.resolve().is_relative_to(root) or path.exists():raise ValueError('refuses existing or unowned output')
    result=dict(gated=False,observer_pid=os.getpid(),target_pid=args.pid,window_id=args.window_id,started_wall_ns=time.time_ns(),
        method='already-authorized Python.app via LaunchServices; read-only preflight, owned-window screenshot; no permission request')
    try:
        libproc=C.CDLL('/usr/lib/libproc.dylib');libproc.proc_pidpath.argtypes=[C.c_int,C.c_void_p,C.c_uint32]
        buf=C.create_string_buffer(4096)
        if libproc.proc_pidpath(args.pid,buf,len(buf))<=0 or Path(os.fsdecode(buf.value)).resolve()!=(args.app/'Contents/MacOS/Petari').resolve():raise ValueError('different executable/PID')
        cg=C.CDLL('/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics')
        cg.CGPreflightScreenCaptureAccess.restype=C.c_bool
        permitted=bool(cg.CGPreflightScreenCaptureAccess());result['preflight_permitted']=permitted
        result['window_before']=window_info(args.pid,args.window_id)
        if not permitted:result['outcome']='refused-no-existing-screen-recording-permission'
        elif not result['window_before']['passed']:result['outcome']='window-no-longer-onscreen'
        else:
            command=['/usr/sbin/screencapture','-x','-l',str(args.window_id),str(args.out)]
            run=subprocess.run(command,capture_output=True,text=True,timeout=3)
            result.update(command=command,exit=run.returncode,stderr=run.stderr,outcome='captured' if run.returncode==0 and args.out.is_file() else 'capture-refused')
            result['window_after']=window_info(args.pid,args.window_id)
    except Exception as exc:result.update(outcome='observation-error',error=repr(exc))
    result['finished_wall_ns']=time.time_ns()
    args.result.write_text(json.dumps(result,indent=2)+'\n')
if __name__=='__main__':main()
