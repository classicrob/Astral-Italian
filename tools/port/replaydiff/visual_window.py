"""Read-only macOS window images, explicitly unqualified for rendered-frame lag."""
import ctypes as C
import hashlib
import json
from pathlib import Path
import plistlib
import struct
import subprocess
import sys
import time

from visual_pairs import private_path


class MacWindows:
    def __init__(self):
        self.cg=C.CDLL('/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics')
        self.cf=C.CDLL('/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation')
        self.proc=C.CDLL('/usr/lib/libproc.dylib')
        def bind(lib,name,result,args):
            fn=getattr(lib,name); fn.restype=result; fn.argtypes=args; return fn
        self.release=bind(self.cf,'CFRelease',None,[C.c_void_p])
        self.plist=bind(self.cf,'CFPropertyListCreateData',C.c_void_p,[C.c_void_p,C.c_void_p,C.c_long,C.c_ulong,C.c_void_p])
        self.length=bind(self.cf,'CFDataGetLength',C.c_long,[C.c_void_p])
        self.bytes=bind(self.cf,'CFDataGetBytePtr',C.POINTER(C.c_ubyte),[C.c_void_p])
        self.list=bind(self.cg,'CGWindowListCopyWindowInfo',C.c_void_p,[C.c_uint32,C.c_uint32])
        self.permission=bind(self.cg,'CGPreflightScreenCaptureAccess',C.c_bool,[])
        self.pidpath=bind(self.proc,'proc_pidpath',C.c_int,[C.c_int,C.c_void_p,C.c_uint32])

    def identity(self,pid):
        buf=C.create_string_buffer(4096)
        if self.pidpath(pid,buf,len(buf)) <= 0: raise RuntimeError('owned application PID is no longer readable')
        start=subprocess.check_output(['ps','-p',str(pid),'-o','lstart='],text=True).strip()
        if not start: raise RuntimeError('owned application process start time unavailable')
        return dict(executable=str(Path(buf.value.decode()).resolve()),start_time=start)

    def windows(self,pid):
        raw=self.list(1|16,0)
        if not raw: raise RuntimeError('window metadata unavailable')
        data=None
        try:
            data=self.plist(None,raw,100,0,None)
            if not data: raise RuntimeError('window metadata serialization failed')
            rows=plistlib.loads(C.string_at(self.bytes(data),self.length(data)))
        finally:
            if data: self.release(data)
            self.release(raw)
        return [dict(pid=pid,window_id=row['kCGWindowNumber'],bounds=row.get('kCGWindowBounds'),
                     owner=row.get('kCGWindowOwnerName'),title=row.get('kCGWindowName',''))
                for row in rows if row.get('kCGWindowOwnerPID')==pid and row.get('kCGWindowLayer')==0]


def capture_window(pid,executable,expected_sha256,image,request,window_id=None,backend=None):
    image=private_path(image)
    if image.exists(): raise RuntimeError('window image output already exists')
    backend=backend or MacWindows()
    identity=backend.identity(pid)
    if identity['executable'] != str(Path(executable).resolve()): raise RuntimeError('owned PID executable differs')
    if hashlib.sha256(Path(executable).read_bytes()).hexdigest() != expected_sha256: raise RuntimeError('application executable hash changed')
    if not backend.permission(): raise RuntimeError('screen capture permission unavailable; launch driver through the granted Python.app with LaunchServices')
    windows=backend.windows(pid)
    if window_id is not None: windows=[row for row in windows if row['window_id']==window_id]
    if len(windows)!=1: raise RuntimeError('owned game window is missing or ambiguous; do not capture another process')
    command=['/usr/sbin/screencapture','-x','-o','-l',str(windows[0]['window_id']),str(image)]
    started=time.time()
    result=subprocess.run(command,capture_output=True,text=True,timeout=15)
    if result.returncode or not image.exists(): raise RuntimeError('window capture failed: '+result.stderr.strip())
    if backend.identity(pid)!=identity: raise RuntimeError('application identity changed during capture')
    raw=image.read_bytes()
    if len(raw)<24 or raw[:8]!=b'\x89PNG\r\n\x1a\n': raise RuntimeError('window capture is not a PNG')
    return dict(schema_version=1,status='UNQUALIFIED_WINDOW_CAPTURE',request=request,owner_pid=pid,
                process_identity=identity,python_executable=sys.executable,window=windows[0],command=command,
                started_epoch=started,completed_epoch=time.time(),image=dict(path=str(image),sha256=hashlib.sha256(raw).hexdigest()),
                dimensions=list(struct.unpack('>II',raw[16:24])),image_scope='whole owned window; viewport/chrome not separated',
                guest_state='caller must bind its actual stopped post-endFrame seam',
                rendered_frame_id=None,readback_frame_id=None,render_fence_complete=None,
                presentation_binding='unknown; no screenshot-frame equivalence claim')
