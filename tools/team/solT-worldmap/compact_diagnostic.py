import csv,hashlib,json,re,sys
from pathlib import Path
p=Path(sys.argv[1]);c=json.loads((p/'command.json').read_text());lines=(p/'app.log').read_text(errors='replace').splitlines();rows=list(csv.DictReader((p/'route.csv').open()));maprows=[r for r in rows if r.get('stage')=='WorldMap01Galaxy' and r.get('valid')=='1'];first=int(maprows[0]['frame']) if maprows else 0;traces=[]
for l in lines:
 m=re.search(r'Petari trace: frame (\d+).*? at ([0-9.]+) s',l)
 if m:traces.append((int(m[1]),float(m[2])))
lo=max((t for t in traces if t[0]<=first),default=(0,0));hi=min((t for t in traces if t[0]>=first),default=lo);ready=(lo[1]+hi[1])/2;files=list((p/'captures/surface').glob('*.png'));keep=set();pairs=[]
for origin,ts in [('map-ready-estimate',[0,10,45,80,100,125,150]),('app-start',[20,40,90,120,140])]:
 for t in ts:
  want=c['start_epoch']+t+(ready if origin=='map-ready-estimate' else 0);s=min(files,key=lambda q:abs(q.stat().st_mtime-want));x=p/'captures/xfb'/s.name.replace('surface-','xfb-',1);keep.update([s,x]);pairs.append(dict(origin=origin,requested_s=t,actual_from_app_start_s=round(s.stat().st_mtime-c['start_epoch'],3),surface=str(s),xfb=str(x)))
# Retain nearest internal pair for each timestamped host capture/action.
for meta in p.glob('host-*.json'):
 try: want=json.loads(meta.read_text()).get('capture_epoch')
 except (ValueError,OSError):continue
 if want is None:continue
 q=min(files,key=lambda q:abs(q.stat().st_mtime-want));keep.update([q,p/'captures/xfb'/q.name.replace('surface-','xfb-',1)])
# Preserve newest internal pair.

s=max(files,key=lambda q:q.stat().st_mtime);keep.update([s,p/'captures/xfb'/s.name.replace('surface-','xfb-',1)])
freed=removed=0
for f in (p/'captures').rglob('*.png'):
 if f not in keep:freed+=f.stat().st_size;f.unlink();removed+=1
ir=[r for r in csv.DictReader((p/'input.csv').open()) if r.get('grav_z') is not None];samples={}
for f in [254,854,2954,5054,5894]:
 r=min(ir,key=lambda x:abs(int(x['frame'])-f));samples[str(f)]=r
r=json.loads((p/'result.json').read_text());r.update(status='NORMAL ROUTE DIAGNOSTIC; review fault/functional receipt separately',pointer_click='See timestamped host events; full consumed KPAD tuple absent',alternate_window_size='See host metadata and trace image rectangle',worldmap_valid_rows=len(maprows),script_rows=len(ir),map_ready_app_trace_bracket_s=[lo[1],hi[1]],map_ready_frame=first,script_samples=samples,scene_trace=[l for l in lines if '[input-script] scene-ready' in l or '[input-script] transition:' in l or '[input-script] matched scene' in l],script_done=any('[input-script] done' in l for l in lines),selected_pairs=pairs,capture_limit='same label/index is not exact game-frame binding; map readiness estimated from route runtime frame and neighboring trace frames; capture wall timestamps retained',cleanup=dict(removed_png=removed,freed_bytes=freed,retained_pairs=len(keep)//2))
(p/'summary.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(dict(case=str(p),first_map_runtime_frame=first,trace_bracket=[lo,hi],script_rows=len(ir),freed_bytes=freed,retained_pairs=len(keep)//2)))
