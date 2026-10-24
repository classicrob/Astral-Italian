#!/usr/bin/env python3
"""Item-3 tier diagnostics/sweep on an explicit build; never replace gate arms."""
import argparse,json,re,subprocess,sys
from common import *

def analyze(out):
 lines=(out/'app.log').read_text(errors='replace').splitlines();frames=rows(out/'frames.csv');compiles={};coverage={};stall_lines={};blocking=[];fallback_waits=[]
 for n,line in enumerate(lines):
  if line.startswith('[gx pipeline compile]'):
   info=dict(re.findall(r'(\w+)=([^\s]+)',line));compiles[info['key']]=info
  elif line.startswith('[gx uber config]'):
   info=dict(re.findall(r'(\w+)=([^\s]+)',line));coverage[info['config']]=info
  match=re.search(r'\[gx pipeline\] blocking resolve ([\d.]+) ms key=([0-9a-f]+)',line)
  if match:blocking.append({'line':n+1,'milliseconds':float(match[1]),'key':match[2]})
  if line.startswith('[gx uber wait]'):
   fields=dict(re.findall(r'(\w+)=([^\s]+)',line))
   fallback_waits.append({'line':n+1,'renderer_frame':int(fields['frame']),'fields':fields,
    'fallback_ready_at_wait':fields.get('fallback_ready')=='1','raw':line})
  match=re.match(r'\[stall\] frame (\d+) ',line)
  if match:stall_lines[int(match[1])]=n+1
 selected=[]
 for row in frames:
  if float(row['interval_ms'])<=100:continue
  lanes={k:float(row.get(k) or 0) for k in ('pipeline_wait_ms','stage_prep_wait_ms','token_barrier_wait_ms','efb_capture_max_ms','efb_staging_wait_ms','efb_submit_wait_ms','texture_hash_ms','texture_upload_ms','frame_submit_ms','drawable_acquire_ms','present_call_ms','draw_done_wait_ms','seam_compose_ms','seam_end_frame_ms','seam_begin_frame_ms','seam_reacquire_ms','retrace_wait_ms','game_work_ms')}
  where=stall_lines.get(int(row['frame']));near=[]
  if where:
   for wait in blocking:
    if not 0<where-wait['line']<=40:continue
    item={**wait,'compile':compiles.get(wait['key'])};config=(item['compile'] or {}).get('config');item['coverage']=coverage.get(config)
    if (item['coverage'] or {}).get('covered')=='1':item['fallback_inference']='Covered config waited for a specialized pipeline: the ready-fallback fast path did not resolve this draw. Exact preparation status at request is not logged.'
    else:item['fallback_inference']='No matched covered config; do not infer fallback readiness.'
    near.append(item)
  interval=float(row['interval_ms']);largest=max(lanes,key=lanes.get)
  # Prefer the instrumented cause inside a containing seam/game-thread wait.
  fine=('pipeline_wait_ms','stage_prep_wait_ms','token_barrier_wait_ms','efb_capture_max_ms','efb_staging_wait_ms','efb_submit_wait_ms','texture_hash_ms','texture_upload_ms','frame_submit_ms','drawable_acquire_ms','present_call_ms')
  dominant=next((k for k in fine if lanes[k]>=.5*interval),largest);fraction=lanes[dominant]/interval
  exact_context=[w for w in fallback_waits if where and 0<where-w['line']<=40]
  selected.append({'frame':int(row['frame']),'phase':row['phase'],'interval_ms':interval,'attribution':dominant if fraction>=.5 else 'unattributed_or_uninstrumented','attribution_lane_fraction':fraction,'largest_measured_lane':largest,'largest_lane_fraction':lanes[largest]/interval,'lanes_ms':lanes,'blocking_resolves_in_preceding_40_log_lines':near,'fallback_readiness_in_preceding_40_log_lines':exact_context,'raw_frame':row})
 return {'stall_frames_over100':selected,'gameplay_over100':sum(r['phase']=='gameplay' for r in selected),'max_gameplay_ms':max((float(r['interval_ms']) for r in frames if r['phase']=='gameplay'),default=None),
  'worker_policy_lines':[l for l in lines if l.startswith(('[gx pipeline] policy=','[gx pipeline pool]'))],
  'fallback_readiness_summary':[l for l in lines if l.startswith('[gx uber readiness]')],
  'fallback_readiness_at_each_complete_scene_wait':fallback_waits,
  'limitations':['Telemetry lanes overlap; do not sum them.','Nearby log lines provide key/config context, not a synchronized per-draw timestamp. Renderer-frame IDs are retained separately from game frame CSV IDs. Exact readiness is available only at logged complete-scene waits; other fallback readiness remains unknown.']}

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('label');p.add_argument('--only',choices=('diag','sweep','both'),default='both');p.add_argument('--candidate-dir',type=Path);a=p.parse_args();assert re.fullmatch('[A-Za-z0-9_-]+',a.label)
 assert os.getpriority(os.PRIO_PROCESS,0)==0,'tier sweep must run at recorded nice0'
 app,meta=campaign_build(a.candidate_dir);build_kind='server-candidate' if a.candidate_dir else 'frozen54';base=ROOT/'campaigns'/a.label;base.mkdir(parents=True,exist_ok=False);plan=[]
 hardware={}
 for name in ('hw.physicalcpu','hw.logicalcpu','hw.perflevel0.physicalcpu','hw.perflevel1.physicalcpu'):
  probe=subprocess.run(['/usr/sbin/sysctl','-n',name],capture_output=True,text=True)
  hardware[name]=int(probe.stdout.strip()) if probe.returncode==0 else None
 if a.only in ('diag','both'):
  plan.extend(('diag-'+stage,stage,None,1) for stage in ('YosshiHomeGalaxy','MokumokuValleyGalaxy','MarioFaceShipGalaxy'))
 if a.only in ('sweep','both'):
  plan.extend((f'workers{workers}-r{repeat}','YosshiHomeGalaxy',workers,0) for repeat in (1,2) for workers in (2,4,8))
 results=[]
 for name,stage,workers,diagnostics in plan:
  out=ROOT/'runs'/(a.label+'-'+name);env={'PETARI_PIPELINE_DIAG':str(diagnostics)}
  if workers is not None:env['PETARI_PIPELINE_PREWARM_WORKERS']=str(workers)
  print('START',name,flush=True)
  proc=run(app,out,stage=stage,script=REPO/'native/tools/perf_sweep_move.script',extra=env,cold=True,seconds=240)
  parsed=analyze(out);gate=subprocess.run([sys.executable,'-B',str(HERE/'gate533.py'),str(out)],capture_output=True,text=True)
  record={'case':name,'stage':stage,'requested_prewarm_workers':workers,'pipeline_diag':diagnostics,'item':'item3tier','gate_arm':False,'functional_acceptance_claimed':False,'build_kind':build_kind,('candidate_build' if a.candidate_dir else 'frozen_build'):{k:v for k,v in meta.items() if k!='resource_tree'},'process':proc,'measurements':parsed,'legacy_gate_observation':json.loads(gate.stdout),'legacy_gate_exit':gate.returncode,'load_and_compiler_evidence':str(out/'quiet.json'),'launch':str(out/'launch.json')}
  write(out/'TIER.json',record);results.append(record);trim(out)
  write(base/'TIER.json',{'item':'item3tier','functional_acceptance_claimed':False,'build_kind':build_kind,'source':meta['source'],'renderer_key':meta['renderer_key'],'source_exe_sha256':meta['source_exe_sha256'],'nice':0,'hardware_cpu_counts':hardware,'runs':results,'completed_all_runs':len(results)==len(plan) and all(r['process']['completed'] and r['process']['no_faults'] for r in results),'target':'0 gameplay frames >100ms in true-cold Yoshi/Fluffy/Starship; original legacy gate results remain unchanged.'})
  print('END',name,'gameplay>100',parsed['gameplay_over100'],'max',parsed['max_gameplay_ms'],flush=True)
 return 0 if all(r['process']['completed'] and r['process']['no_faults'] for r in results) else 1

if __name__=='__main__':sys.exit(main())
