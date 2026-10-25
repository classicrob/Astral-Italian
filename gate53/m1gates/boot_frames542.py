"""Normal-frame boot proof: own drawable pixels + WindowServer + complete CSV.

No screencapture/Automation permission requests. A denied or timed-out PID
AppleEvent is a failure. Historical Cocoa-overlay receipts are not relabelled.
"""
import csv
import ctypes as C
import hashlib
import json
import math
import os
from pathlib import Path
import re
import struct
import subprocess
import threading
import time
import zlib

SEND_MODE = 0x00020013  # WaitReply | NeverInteract | DoNotPromptForUserConsent
COOKIE = 0x50455441

class Desc(C.Structure):
    _pack_ = 2  # AEDataModel.h pragma pack(push,2): 12 bytes, pointer at 4
    _fields_ = [('descriptorType', C.c_uint32), ('dataHandle', C.c_void_p)]

def fourcc(s): return int.from_bytes(s.encode('ascii'), 'big')

def ping(pid, executable):
    libproc = C.CDLL('/usr/lib/libproc.dylib')
    buf = C.create_string_buffer(4096)
    libproc.proc_pidpath.argtypes = [C.c_int,C.c_void_p,C.c_uint32]
    if libproc.proc_pidpath(pid,buf,len(buf)) <= 0 or Path(os.fsdecode(buf.value)).resolve() != Path(executable).resolve():
        raise ValueError('heartbeat refuses a different executable/PID')
    ae = C.CDLL('/System/Library/Frameworks/ApplicationServices.framework/ApplicationServices')
    ae.AECreateDesc.argtypes = [C.c_uint32,C.c_void_p,C.c_long,C.POINTER(Desc)]
    ae.AECreateAppleEvent.argtypes = [C.c_uint32,C.c_uint32,C.POINTER(Desc),C.c_int16,C.c_int32,C.POINTER(Desc)]
    ae.AESendMessage.argtypes = [C.POINTER(Desc),C.POINTER(Desc),C.c_int32,C.c_int32]
    ae.AEGetParamPtr.argtypes = [C.POINTER(Desc),C.c_uint32,C.c_uint32,C.POINTER(C.c_uint32),C.c_void_p,C.c_long,C.POINTER(C.c_long)]
    ae.AEDisposeDesc.argtypes = [C.POINTER(Desc)]
    target,event,reply=Desc(),Desc(),Desc();value=C.c_int32(pid)
    start=time.monotonic_ns();wall=time.time_ns();status=-1
    cookie=C.c_int32();actual_type=C.c_uint32();size=C.c_long();reply_error=C.c_int32();error_size=C.c_long()
    try:
        status=ae.AECreateDesc(fourcc('kpid'),C.byref(value),4,C.byref(target))
        if not status: status=ae.AECreateAppleEvent(fourcc('Ptra'),fourcc('ping'),C.byref(target),-1,0,C.byref(event))
        if not status: status=ae.AESendMessage(C.byref(event),C.byref(reply),SEND_MODE,30)
        error_status=ae.AEGetParamPtr(C.byref(reply),fourcc('errn'),fourcc('long'),C.byref(actual_type),C.byref(reply_error),4,C.byref(error_size)) if not status else None
        if not status: status=ae.AEGetParamPtr(C.byref(reply),fourcc('----'),fourcc('long'),C.byref(actual_type),C.byref(cookie),4,C.byref(size))
        stop=time.monotonic_ns()
        return dict(status=status,cookie=cookie.value,reply_bytes=size.value,type=actual_type.value,reply_descriptor_type=reply.descriptorType,reply_error_status=error_status,reply_error_number=reply_error.value if error_status==0 else None,
            passed=status==0 and cookie.value==COOKIE and size.value==4 and actual_type.value==fourcc('long') and stop-start<=500000000,
            start_monotonic_ns=start,stop_monotonic_ns=stop,start_wall_ns=wall,stop_wall_ns=time.time_ns(),send_mode=SEND_MODE)
    finally:
        for d in (reply,event,target):
            if d.dataHandle: ae.AEDisposeDesc(C.byref(d))

class Point(C.Structure): _fields_=[('x',C.c_double),('y',C.c_double)]
class Size(C.Structure): _fields_=[('width',C.c_double),('height',C.c_double)]
class Rect(C.Structure): _fields_=[('origin',Point),('size',Size)]

def window_info(pid, window_id):
    cg=C.CDLL('/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics')
    cf=C.CDLL('/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation')
    cg.CGWindowListCopyWindowInfo.argtypes=[C.c_uint32,C.c_uint32];cg.CGWindowListCopyWindowInfo.restype=C.c_void_p
    cg.CGRectMakeWithDictionaryRepresentation.argtypes=[C.c_void_p,C.POINTER(Rect)];cg.CGRectMakeWithDictionaryRepresentation.restype=C.c_bool
    cf.CFArrayGetCount.argtypes=[C.c_void_p];cf.CFArrayGetCount.restype=C.c_long
    cf.CFArrayGetValueAtIndex.argtypes=[C.c_void_p,C.c_long];cf.CFArrayGetValueAtIndex.restype=C.c_void_p
    cf.CFDictionaryGetValue.argtypes=[C.c_void_p,C.c_void_p];cf.CFDictionaryGetValue.restype=C.c_void_p
    cf.CFNumberGetValue.argtypes=[C.c_void_p,C.c_int,C.c_void_p];cf.CFNumberGetValue.restype=C.c_bool
    cf.CFBooleanGetValue.argtypes=[C.c_void_p];cf.CFBooleanGetValue.restype=C.c_bool
    cf.CFRelease.argtypes=[C.c_void_p]
    keys={k:C.c_void_p.in_dll(cg,k).value for k in ('kCGWindowOwnerPID','kCGWindowNumber','kCGWindowIsOnscreen','kCGWindowAlpha','kCGWindowBounds')}
    array=cg.CGWindowListCopyWindowInfo(0,0)
    if not array: raise ValueError('WindowServer returned no list')
    found=[]
    try:
        for i in range(cf.CFArrayGetCount(array)):
            d=cf.CFArrayGetValueAtIndex(array,i)
            def get(key):return cf.CFDictionaryGetValue(d,keys[key])
            def number(key,double=False):
                v=get(key);out=C.c_double() if double else C.c_int64()
                if not v or not cf.CFNumberGetValue(v,6 if double else 4,C.byref(out)):raise ValueError('missing window number')
                return out.value
            if number('kCGWindowOwnerPID')!=pid:continue
            if number('kCGWindowNumber')!=window_id:continue
            bounds=Rect();v=get('kCGWindowBounds')
            if not v or not cg.CGRectMakeWithDictionaryRepresentation(v,C.byref(bounds)):raise ValueError('missing bounds')
            onscreen=get('kCGWindowIsOnscreen')
            onscreen=bool(onscreen and cf.CFBooleanGetValue(onscreen))
            alpha=number('kCGWindowAlpha',True)
            size=[bounds.size.width,bounds.size.height]
            found.append(dict(pid=pid,window_id=window_id,onscreen=onscreen,alpha=alpha,bounds=[bounds.origin.x,bounds.origin.y,*size],
                passed=onscreen and alpha==1 and all(math.isfinite(n) and n>0 for n in size)))
    finally:cf.CFRelease(array)
    return dict(passed=len(found)==1 and found[0]['passed'],windows=found,wall_ns=time.time_ns())

def png_evidence(path):
    # Our in-app writer emits RGB8/noninterlaced PNG with filter 0. Check CRCs
    # and lengths rather than accepting a file name or native success string.
    path=Path(path);data=path.read_bytes()
    if data[:8]!=b'\x89PNG\r\n\x1a\n':raise ValueError('not PNG')
    at=8;idat=[];header=None;ended=False
    while at<len(data):
        n,kind=struct.unpack('>I4s',data[at:at+8]);body=data[at+8:at+8+n]
        if len(body)!=n or len(data)<at+n+12:raise ValueError('truncated PNG')
        crc=struct.unpack('>I',data[at+8+n:at+12+n])[0]
        if zlib.crc32(kind+body)&0xffffffff!=crc:raise ValueError('PNG CRC mismatch')
        at+=n+12
        if kind==b'IHDR':
            if header is not None:raise ValueError('duplicate header')
            header=struct.unpack('>IIBBBBB',body)
        elif kind==b'IDAT':idat.append(body)
        elif kind==b'IEND':ended=True;break
    if not ended or at!=len(data) or not header:raise ValueError('incomplete PNG')
    w,h,depth,kind,compression,filtering,interlace=header
    if not (64<=w<=8192 and 64<=h<=8192 and (depth,kind,compression,filtering,interlace)==(8,2,0,0,0)):raise ValueError('unexpected in-app PNG format')
    raw=zlib.decompress(b''.join(idat));stride=w*3
    if len(raw)!=(stride+1)*h or any(raw[y*(stride+1)] for y in range(h)):raise ValueError('invalid RGB rows')
    def count(region,threshold):
        x0,y0,x1,y1=region;n=0
        for y in range(int(y0*h),int(y1*h)):
            for x in range(int(x0*w),int(x1*w)):
                p=y*(stride+1)+1+x*3
                n+=max(raw[p:p+3])>=threshold
        return n
    # The SAME product pill's text and track, from ImageSpace in imgui_overlay.
    text=count((.75,.938,.975,.965),128)
    bar=count((.75,.965,.975,.970),16)
    return dict(passed=text>=8 and bar>=8,width=w,height=h,text_nonblack_pixels=text,bar_nonblack_pixels=bar,
        sha256=hashlib.sha256(data).hexdigest(),path=str(path),source='in-app presented surface; not a WindowServer screenshot')

def validate_frames(folder):
    folder=Path(folder);text=(folder/'app.log').read_text(errors='replace')
    result=dict(passed=False,checks={})
    try:
        def unique(pattern):
            found=re.findall(pattern,text,re.M)
            if len(found)!=1:raise ValueError('missing/ambiguous '+pattern)
            return found[0]
        receipt=unique(r'^\[gx frontend pre-present\] (version=[23] .+)$')
        fields=dict(re.findall(r'(\w+)=([^ ]+)',receipt))
        gap=float(fields['pump_max_gap_ms'])
        result['checks']['main_loop_turns_le100ms']=(fields['main_thread']=='1' and int(fields['pumps'])>0 and math.isfinite(gap) and 0<=gap<=100)
        result['main_loop']=dict(max_gap_ms=gap,turns=int(fields['pumps']),threshold_ms=100,source='actual main run-loop turn starts, same normal-frame prep receipt')
        release=unique(r'^\[gx frontend release\] finished_launching=1 window_onscreen=1 alpha=1 bounds_nonzero=1 first_present=1 presented_frames=(\d+)$')
        count,intervals,last=map(int,unique(r'^\[gx boot splash frames\] presented=(\d+) csv_intervals=(\d+) first_serial=0 last_serial=(\d+) .*game_advanced=0 route_advanced=0$'))
        first,end,offset=map(int,unique(r'^\[gx startup frame mapping\] prep_csv_first=(\d+) prep_csv_last=(\d+) game_csv_offset=(\d+) route_offset=0 policy=continuous-intervals$'))
        presents=[(int(f),int(ns)) for f,ns in re.findall(r'^\[gx boot splash present\] frame=(\d+) uptime_ns=(\d+) success=1$',text,re.M)]
        if fields['version']=='3':
            expected=('app_menu_setup_enter','app_menu_setup_exit','hidden_bootstrap_enter',
                'first_run_loop_enter','first_run_loop_exit','first_sdl_pump_enter','first_sdl_pump_exit',
                'first_aurora_update_enter','input_initialize_enter','sdl_input_init_enter','sdl_input_init_exit',
                'input_initialize_exit','first_aurora_update_exit','hidden_bootstrap_exit',
                'window_show_enter','window_shown','first_successful_present')
            calls={name:int(unique(r'^\[gx startup call\] event='+name+r' monotonic_ns=(\d+)$')) for name in expected}
            ordered=list(calls.values())
            shown_ns=int(unique(r'^\[gx boot splash\] event=shown .*monotonic_ns=(\d+) wall_ns=\d+ mode=normal-frames$'))
            result['startup_calls']=calls
            result['checks']['hidden_bootstrap_before_window_show']=(ordered==sorted(ordered) and ordered[0]>0 and
                calls['window_shown']<=shown_ns<=calls['first_successful_present']<=presents[0][1])
        with (folder/'frames.csv').open() as file: rows=list(csv.DictReader(file))
        prefix=rows[:intervals]
        result['checks']['complete_present_sequence']=(count>=3 and [f for f,_ in presents]==list(range(count)) and all(b[1]>a[1] for a,b in zip(presents,presents[1:])) and last==count-1 and 1<=int(release)<=count and text.index('[gx frontend release]')>text.index('[gx boot splash present]'))
        result['checks']['csv_interval_coverage']=(intervals==count-1==end and offset==count and first==1 and len(prefix)==intervals and len(rows)>intervals and
            all(int(r['frame'])==i and r['phase']=='startup' and abs(float(r['interval_ms'])-(presents[i][1]-presents[i-1][1])/1e6)<=.00051 for i,r in enumerate(prefix,1)) and
            int(rows[intervals]['frame'])==offset)
        capture_frame,w,h,path=unique(r'^\[gx boot splash capture\] result=written frame=(\d+) width=(\d+) height=(\d+) source=presented-surface after_present=1 path=(.+)$')
        path=Path(path)
        result['checks']['actual_presented_surface_bound']=(path.resolve()==(folder/'boot-splash.png').resolve() and 1<=int(capture_frame)<count)
        result['pixels']=png_evidence(path)
        result['checks']['text_and_bar_nonblack']=result['pixels']['passed'] and [int(w),int(h)]==[result['pixels']['width'],result['pixels']['height']]
        times=[float(r['interval_ms']) for r in prefix]
        result['cadence']=dict(target_hz=60,intervals=len(times),min_ms=min(times),max_ms=max(times),mean_ms=sum(times)/len(times),
            note='Actual ordinary-frame intervals; unchanged menu/startup thresholds score these same CSV rows.')
        result['passed']=all(result['checks'].values())
    except (OSError,ValueError,KeyError,IndexError,ZeroDivisionError,struct.error,zlib.error) as exc:result['error']=str(exc)
    return result

def launch_screen_observation(folder,pid,app,window_id):
    # Head explicitly authorized this interpreter's already-granted identity.
    # A read-only preflight inside that LaunchServices process still decides;
    # refusal/missing output is an observation, never a reason to request access.
    python_app=Path('/opt/homebrew/Cellar/python@3.15/3.15.0/Frameworks/Python.framework/Versions/3.15/Resources/Python.app')
    result=Path(folder)/'boot-splash-extra-screen.json'
    if not python_app.is_dir():return dict(attempted=False,reason='authorized Python.app not installed',gated=False)
    command=['/usr/bin/open','-n','-a',str(python_app),'--args',str(Path(__file__).with_name('screenshot_boot542.py')),
        '--pid',str(pid),'--window-id',str(window_id),'--app',str(app),'--out',str(Path(folder)/'boot-splash-os.png'),'--result',str(result)]
    try:
        opened=subprocess.run(command,capture_output=True,text=True,timeout=3)
        return dict(attempted=True,gated=False,command=command,exit=opened.returncode,stderr=opened.stderr,result=str(result))
    except Exception as exc:return dict(attempted=True,gated=False,command=command,error=repr(exc),result=str(result))

class Probe:
    def __init__(self,folder,pid,app,*,m1_screenshot=False):
        self.folder,self.pid,self.app=Path(folder),pid,Path(app)
        self.stop=threading.Event();self.samples=[];self.error=None;self.shown=False;self.ended=False
        self.appleevent=None;self.extra_screen=None;self.m1_screenshot=m1_screenshot
        self.thread=threading.Thread(target=self.run,daemon=True);self.thread.start()
    def run(self):
        try:
            while not self.stop.is_set():
                text=(self.folder/'app.log').read_text(errors='replace')
                self.shown='[gx boot splash] event=shown' in text
                self.ended='[gx boot splash] event=handed_off' in text
                if self.shown and not self.ended:
                    window=re.findall(r'^\[gx boot splash window\] id=(\d+) pid=(\d+)$',text,re.M)
                    if len(window)!=1 or int(window[0][1])!=self.pid:raise ValueError('missing/ambiguous owned window')
                    sample=dict(windowserver=window_info(self.pid,int(window[0][0])))
                    sample['passed']=sample['windowserver']['passed']
                    self.samples.append(sample)
                    if self.appleevent is None:
                        try:self.appleevent=ping(self.pid,self.app/'Contents/MacOS/Petari')
                        except Exception as exc:self.appleevent=dict(error=repr(exc))
                    if self.m1_screenshot and self.extra_screen is None:
                        self.extra_screen=launch_screen_observation(self.folder,self.pid,self.app,int(window[0][0]))
                if self.ended:break
                self.stop.wait(.1 if self.shown else .005)
        except Exception as exc:self.error=repr(exc)
    def finish(self):
        self.stop.set();self.thread.join(timeout=2)
        text=(self.folder/'app.log').read_text(errors='replace')
        shown=re.findall(r'^\[gx boot splash\] event=shown .*wall_ns=(\d+) mode=normal-frames$',text,re.M)
        ended=re.findall(r'^\[gx boot splash\] event=handed_off .*wall_ns=(\d+) mode=normal-frames$',text,re.M)
        self.shown=bool(shown);self.ended=bool(ended)
        frames=validate_frames(self.folder)
        scope_ok=len(shown)==len(ended)==1 and bool(self.samples) and all(
            int(shown[0])<=s['windowserver']['wall_ns']<=int(ended[0]) for s in self.samples)
        passed=not self.thread.is_alive() and self.error is None and scope_ok and all(s['passed'] for s in self.samples) and frames['passed']
        if self.extra_screen and self.extra_screen.get('result'):
            path=Path(self.extra_screen['result'])
            if path.is_file():
                try:self.extra_screen['observation']=json.loads(path.read_text())
                except Exception as exc:self.extra_screen['read_error']=repr(exc)
            else:self.extra_screen['observation_missing']=True
        result=dict(passed=passed,version=2,method='native main-loop gaps <=100ms plus own presented surface, WindowServer and same CSV',
            timeout_seconds=.5,sample_period_seconds=.1,pid=self.pid,app=str(self.app),send_mode=SEND_MODE,consent_prompt_allowed=False,
            shown=self.shown,handed_off=self.ended,samples=self.samples,error=self.error,within_prep=scope_ok,visibility=frames,
            appleevent_observation=self.appleevent,appleevent_gated=False,extra_screen_observation=self.extra_screen,
            scope='Surface submission/pixels and on-screen window, not physical scan-out. No TCC requests.')
        (self.folder/'boot-response.json').write_text(json.dumps(result,indent=2)+'\n');return result
