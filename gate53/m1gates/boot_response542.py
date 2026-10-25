"""Read-only PID-addressed AppleEvent replies and owned-window prep screenshot.

Opt-in on the NEW candidate only. Missing/timed-out/denied replies fail closed;
this never sends input, activates a window, or requests termination.
"""
import ctypes as C
import ctypes.util
import json
import os
from pathlib import Path
import re
import subprocess
import threading
import time

def fourcc(s): return int.from_bytes(s.encode('ascii'), 'big')

class Desc(C.Structure):
    _fields_ = [('descriptorType', C.c_uint32), ('dataHandle', C.c_void_p)]

def ping(pid, executable):
    libproc = C.CDLL('/usr/lib/libproc.dylib')
    buf = C.create_string_buffer(4096)
    libproc.proc_pidpath.argtypes = [C.c_int, C.c_void_p, C.c_uint32]
    if libproc.proc_pidpath(pid, buf, len(buf)) <= 0 or Path(os.fsdecode(buf.value)).resolve() != executable.resolve():
        raise ValueError('heartbeat refuses a different executable/PID')
    ae = C.CDLL('/System/Library/Frameworks/ApplicationServices.framework/ApplicationServices')
    ae.AECreateDesc.argtypes = [C.c_uint32,C.c_void_p,C.c_long,C.POINTER(Desc)]
    ae.AECreateAppleEvent.argtypes = [C.c_uint32,C.c_uint32,C.POINTER(Desc),C.c_int16,C.c_int32,C.POINTER(Desc)]
    ae.AESendMessage.argtypes = [C.POINTER(Desc),C.POINTER(Desc),C.c_int32,C.c_int32]
    ae.AEGetParamPtr.argtypes = [C.POINTER(Desc),C.c_uint32,C.c_uint32,C.POINTER(C.c_uint32),C.c_void_p,C.c_long,C.POINTER(C.c_long)]
    ae.AEDisposeDesc.argtypes = [C.POINTER(Desc)]
    target,event,reply = Desc(),Desc(),Desc()
    pid_value = C.c_int32(pid)
    started=time.monotonic_ns()
    status=-1; cookie=C.c_int32(0); actual_type=C.c_uint32(); actual_size=C.c_long()
    try:
        status=ae.AECreateDesc(fourcc('kpid'),C.byref(pid_value),4,C.byref(target))
        if not status: status=ae.AECreateAppleEvent(fourcc('Ptra'),fourcc('ping'),C.byref(target),-1,0,C.byref(event))
        # kAEWaitReply | kAENeverInteract, 30/60s timeout. Never retry a failure.
        if not status: status=ae.AESendMessage(C.byref(event),C.byref(reply),0x13,30)
        if not status: status=ae.AEGetParamPtr(C.byref(reply),fourcc('----'),fourcc('long'),C.byref(actual_type),C.byref(cookie),4,C.byref(actual_size))
        return {'status':status,'cookie':cookie.value,'reply_bytes':actual_size.value,'type':actual_type.value,
                'passed':status==0 and cookie.value==0x50455441 and actual_size.value==4,
                'start_monotonic_ns':started,'stop_monotonic_ns':time.monotonic_ns(), 'wall_ns':time.time_ns()}
    finally:
        for d in (reply,event,target):
            if d.dataHandle: ae.AEDisposeDesc(C.byref(d))

class Probe:
    def __init__(self,folder,pid,app,*,m1_screenshot=False):
        self.folder,self.pid,self.app=Path(folder),pid,Path(app)
        self.stop=threading.Event();self.samples=[];self.error=None;self.screenshot=None
        self.shown=False;self.ended=False;self.frames=None;self.m1_screenshot=m1_screenshot
        self.thread=threading.Thread(target=self.run,daemon=True);self.thread.start()
    def run(self):
        try:
            while not self.stop.is_set():
                text=(self.folder/'app.log').read_text(errors='replace')
                if re.search(r'^\[gx boot splash\] event=shown .*mode=normal-frames$',text,re.M):
                    # Only the new native protocol uses non-TCC proof. Keep
                    # historical v1 behavior/results intact, including failures.
                    from boot_frames542 import Probe as FrameProbe
                    self.frames=FrameProbe(self.folder,self.pid,self.app,m1_screenshot=self.m1_screenshot)
                    return
                self.shown='[gx boot splash] event=shown' in text
                self.ended='[gx boot splash] event=handed_off' in text
                if self.shown and not self.ended:
                    self.samples.append(ping(self.pid,self.app/'Contents/MacOS/Petari'))
                    # One actual OS-window image, after at least one successful reply.
                    window=re.search(r'\[gx boot splash window\] id=(\d+) pid=(\d+)',text)
                    if self.screenshot is None and window and int(window[2])==self.pid:
                        command=['/usr/sbin/screencapture','-x','-l',window[1],str(self.folder/'boot-splash.png')]
                        result=subprocess.run(command,capture_output=True,text=True,timeout=5)
                        self.screenshot={'command':command,'exit':result.returncode,'stderr':result.stderr,'wall_ns':time.time_ns()}
                if self.ended: break
                self.stop.wait(.25 if self.shown else .01)
        except Exception as exc:self.error=repr(exc)
    def finish(self):
        self.stop.set();self.thread.join(timeout=7)
        if getattr(self,"frames",None) is not None:return self.frames.finish()
        passed=(not self.thread.is_alive() and self.error is None and self.shown and self.ended and
                bool(self.samples) and all(s['passed'] for s in self.samples) and
                self.screenshot is not None and self.screenshot['exit']==0 and (self.folder/'boot-splash.png').is_file())
        result={'passed':passed,'method':'kernel-PID AppleEvent Ptra/ping main-thread reply, never-interact',
                'timeout_seconds':.5,'sample_period_seconds':.25,'pid':self.pid,'app':str(self.app),
                'shown':self.shown,'handed_off':self.ended,'samples':self.samples,'error':self.error,'screenshot':self.screenshot,
                'scope':'OS event-loop responsiveness + actual owned-window image; no claim from pump counts alone'}
        (self.folder/'boot-response.json').write_text(json.dumps(result,indent=2)+'\n')
        return result
