import ctypes as C,json,plistlib,sys,time
CG=C.CDLL('/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics');CF=C.CDLL('/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation');AX=C.CDLL('/System/Library/Frameworks/ApplicationServices.framework/ApplicationServices')
class Point(C.Structure):_fields_=[('x',C.c_double),('y',C.c_double)]
class Size(C.Structure):_fields_=[('width',C.c_double),('height',C.c_double)]
class Rect(C.Structure):_fields_=[('origin',Point),('size',Size)]
def bind(lib,name,restype,args):
 f=getattr(lib,name);f.restype=restype;f.argtypes=args;return f
release=bind(CF,'CFRelease',None,[C.c_void_p]);string=bind(CF,'CFStringCreateWithCString',C.c_void_p,[C.c_void_p,C.c_char_p,C.c_uint32]);plist=bind(CF,'CFPropertyListCreateData',C.c_void_p,[C.c_void_p,C.c_void_p,C.c_long,C.c_ulong,C.c_void_p]);dlen=bind(CF,'CFDataGetLength',C.c_long,[C.c_void_p]);dptr=bind(CF,'CFDataGetBytePtr',C.POINTER(C.c_ubyte),[C.c_void_p]);getwindows=bind(CG,'CGWindowListCopyWindowInfo',C.c_void_p,[C.c_uint32,C.c_uint32]);main=bind(CG,'CGMainDisplayID',C.c_uint32,[]);dbounds=bind(CG,'CGDisplayBounds',Rect,[C.c_uint32]);pixw=bind(CG,'CGDisplayPixelsWide',C.c_size_t,[C.c_uint32]);pixh=bind(CG,'CGDisplayPixelsHigh',C.c_size_t,[C.c_uint32]);trusted=bind(AX,'AXIsProcessTrusted',C.c_bool,[]);postaccess=bind(CG,'CGPreflightPostEventAccess',C.c_bool,[]);screenaccess=bind(CG,'CGPreflightScreenCaptureAccess',C.c_bool,[])
mouse=bind(CG,'CGEventCreateMouseEvent',C.c_void_p,[C.c_void_p,C.c_uint32,Point,C.c_uint32]);post=bind(CG,'CGEventPost',None,[C.c_uint32,C.c_void_p]);event=bind(CG,'CGEventCreate',C.c_void_p,[C.c_void_p]);location=bind(CG,'CGEventGetLocation',Point,[C.c_void_p]);key=bind(CG,'CGEventCreateKeyboardEvent',C.c_void_p,[C.c_void_p,C.c_uint16,C.c_bool]);app=bind(AX,'AXUIElementCreateApplication',C.c_void_p,[C.c_int32]);attr=bind(AX,'AXUIElementCopyAttributeValue',C.c_int32,[C.c_void_p,C.c_void_p,C.POINTER(C.c_void_p)]);setattr=bind(AX,'AXUIElementSetAttributeValue',C.c_int32,[C.c_void_p,C.c_void_p,C.c_void_p]);action=bind(AX,'AXUIElementPerformAction',C.c_int32,[C.c_void_p,C.c_void_p]);alen=bind(CF,'CFArrayGetCount',C.c_long,[C.c_void_p]);aget=bind(CF,'CFArrayGetValueAtIndex',C.c_void_p,[C.c_void_p,C.c_long]);axvalue=bind(AX,'AXValueCreate',C.c_void_p,[C.c_int32,C.c_void_p]);axget=bind(AX,'AXValueGetValue',C.c_bool,[C.c_void_p,C.c_int32,C.c_void_p])
def text(s):return string(None,s.encode(),0x08000100)
def cursor():
 e=event(None);p=location(e);release(e);return p

def windows(pid=None):
 raw=getwindows(1|16,0);data=plist(None,raw,100,0,None)
 if not data:raise RuntimeError('window metadata serialization failed')
 value=plistlib.loads(C.string_at(dptr(data),dlen(data)));release(data);release(raw)
 return [dict(pid=r.get('kCGWindowOwnerPID'),window_id=r.get('kCGWindowNumber'),owner=r.get('kCGWindowOwnerName'),title=r.get('kCGWindowName',''),bounds=r.get('kCGWindowBounds')) for r in value if (r.get('kCGWindowLayer')==0 and ((pid is not None and r.get('kCGWindowOwnerPID')==pid) or (pid is None and ('Petari' in r.get('kCGWindowOwnerName','') or 'Astral' in r.get('kCGWindowName','')))))]
def query(pid=None):
 d=main();b=dbounds(d);p=cursor();return dict(accessibility_trusted=trusted(),post_event_access=postaccess(),screen_capture_access=screenaccess(),display_points=[b.size.width,b.size.height],display_pixels=[pixw(d),pixh(d)],cursor_global_points=[p.x,p.y],windows=windows(pid))
def firstwin(pid):
 a=app(pid);v=C.c_void_p();name=text('AXWindows');rc=attr(a,name,C.byref(v));release(name)
 if rc or not v.value or not alen(v):release(a);raise RuntimeError('AXWindows status '+str(rc))
 w=aget(v,0);return a,v,w
args=sys.argv[1:];op=args[0] if args else 'query';result={}
if op=='query':result=query(int(args[1]) if len(args)>1 else None)
elif op=='activate':
 pid=int(args[1]);a,v,w=firstwin(pid);name=text('AXFrontmost');rc=setattr(a,name,C.c_void_p.in_dll(CF,'kCFBooleanTrue'));release(name);name=text('AXRaise');raised=action(w,name);release(name);release(v);release(a);result=dict(activate_status=rc,raise_status=raised,windows=windows(pid))
elif op=='move':
 p=Point(float(args[1]),float(args[2]));e=mouse(None,5,p,0);post(0,e);release(e);time.sleep(.08);now=cursor();result=dict(move_global_points=[p.x,p.y],actual_cursor=[now.x,now.y])
elif op=='click':
 right=args[1]=='right';p=cursor()
 for down in (True,False):
  e=mouse(None,(3 if down else 4) if right else (1 if down else 2),p,1 if right else 0);post(0,e);release(e);time.sleep(.12)
 result=dict(click=args[1],global_points=[p.x,p.y])
elif op=='key':
 code=int(args[1]);ms=int(args[2]) if len(args)>2 else 120
 for down in (True,False):
  e=key(None,code,down);post(0,e);release(e);time.sleep(ms/1000 if down else .02)
 result=dict(key_code=code,hold_ms=ms)
elif op=='size':
 pid=int(args[1]);a,v,w=firstwin(pid);name=text('AXSize');size=Size(float(args[2]),float(args[3]));encoded=axvalue(2,C.byref(size));rc=setattr(w,name,encoded);release(encoded);release(name);release(v);release(a);time.sleep(.2);result=dict(resize_ax_status=rc,requested_outer_points=[size.width,size.height],windows=windows(pid))
else:raise SystemExit('unknown operation')
print(json.dumps(result,sort_keys=True))
