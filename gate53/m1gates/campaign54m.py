#!/usr/bin/env python3
"""Sequential M1 gate campaign based on server gates54f.sh and validate541.sh.
Failures are preserved and do not skip independent gates. Missing replay gates fail closed.
"""
import argparse, csv, json, os, re, shutil, subprocess, sys, time
from common import *
STAGES=('YosshiHomeGalaxy','MokumokuValleyGalaxy','MarioFaceShipGalaxy')

def main():
 p=argparse.ArgumentParser();p.add_argument('label');p.add_argument('--only');p.add_argument('--quiet-seconds',type=int,default=900);p.add_argument('--tape-result',type=Path);p.add_argument('--candidate-app',type=Path);p.add_argument('--candidate-sha');p.add_argument('--candidate-source');p.add_argument('--candidate-original-app',type=Path);p.add_argument('--candidate-original-sha');p.add_argument('--tape',type=Path,default=ROOT/'tapes/tape-normal-54');p.add_argument('--allow-incomplete-tape',action='store_true');p.add_argument('--server-candidate-dir',type=Path);p.add_argument('--normal-results',type=Path,nargs=3);a=p.parse_args()
 assert re.fullmatch(r'[A-Za-z0-9_-]+',a.label),'safe unique label required'
 assert os.getpriority(os.PRIO_PROCESS,0)==0,'run campaign at nice 0'
 candidate_mode=a.server_candidate_dir is not None
 if candidate_mode:assert not any((a.candidate_app,a.candidate_sha,a.candidate_source,a.candidate_original_app,a.candidate_original_sha,a.tape_result)),'do not mix legacy tape-only selectors with a full candidate campaign'
 if candidate_mode:
  from candidate_evidence54m import normal_evidence,combine_promotion
  assert a.normal_results,'candidate campaign requires all three completed SOL B normal acceptance receipts'
  normal=normal_evidence(a.normal_results,a.server_candidate_dir)
 app,meta=campaign_build(a.server_candidate_dir)
 base=ROOT/'campaigns'/a.label;base.mkdir(parents=True,exist_ok=False)
 manifest={'schema':1,'label':a.label,'host':platform.node(),'platform':platform.platform(),'nice':0,'build_kind':'server-candidate' if candidate_mode else 'frozen54','priority_reason':'Normal priority approved by PORT; server promotion/normal/intro/Cappy ran nice 19, a documented load-sensitive harness difference.',('candidate_build' if candidate_mode else 'frozen'):{k:v for k,v in meta.items() if k!='resource_tree'},'steps':[],'started_utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'reference_root':str(HOME/'MacioGalaxy2-team/scratch-port/gate53/m1gates/srv-ref')}
 if candidate_mode:manifest.update(app=str(app),normal_results=[str(q.resolve()) for q in a.normal_results],tier_authority='PORT/head final candidate ruling: cold stall and Fifo cadence are M1 TIER rows; all functional gates required')
 write(base/'MANIFEST.json',manifest)
 def quiet(name):
  wait_quiet(base/(name+'-quiet.json'),seconds=a.quiet_seconds)
 def command(name,cmd):
  disk();print('START',name,time.strftime('%H:%M:%S'),flush=True)
  with (base/(name+'.out')).open('w') as f:
   r=subprocess.run([str(x) for x in cmd],stdout=f,stderr=subprocess.STDOUT,env={**os.environ,'PYTHONDONTWRITEBYTECODE':'1','PORT53_WINDOW':'PORT_APPROVED'})
  entry={'name':name,'exit':r.returncode,'log':str(base/(name+'.out'))}
  manifest['steps'].append(entry);write(base/'MANIFEST.json',manifest)
  print('END',name,'exit',r.returncode,flush=True);return r.returncode
 def stage(name,st,*,warm=None,extra=None,script=None):
  label=a.label+'-'+name;args=[sys.executable,'-B',HERE/'stage54m.py',label,st]
  if warm:args+=['--warm',a.label+'-'+warm]
  if name.startswith('cadence-') and name!='cadence-prime':args+=['--cadence-check']
  if extra:args+=['--extra',json.dumps(extra)]
  if script:args+=['--script',script]
  if candidate_mode:args+=['--candidate-dir',a.server_candidate_dir]
  command(name,args);return ROOT/'runs'/label
 selected=(a.only or ('platform,tape,promotion,intro,cappy,cold,image,warm,cadence,audio' if candidate_mode else 'platform,promotion,intro,cappy,cold,image,warm,cadence,audio,tape')).split(',')
 for gate in selected:
  try:
   if gate=='platform':command('platform',[sys.executable,'-B',HERE/'platform54m.py',base/'platform']+(['--candidate-dir',a.server_candidate_dir] if candidate_mode else []))
   elif gate=='promotion':
    quiet('promotion');command('promotion',[sys.executable,'-B',HERE/'promotion54m.py',app,meta['perf_exe_sha256'],a.label+('-promotion-direct' if candidate_mode else '-promotion'),'--isolated-cache','--source',meta['source'],'--renderer-key',meta['renderer_key']]+(['--candidate-dir',a.server_candidate_dir,'--cases','yoshi,flip,fluffy,unlocked-stage-adapter,return,cappy-fluffy,cappy-panel,cappy-spin-after-throw'] if candidate_mode else []))
    if candidate_mode:combine_promotion(ROOT/'runs'/(a.label+'-promotion-direct')/'results.json',ROOT/'runs'/(a.label+'-pointer')/'RESULT.json',meta,ROOT/'runs'/(a.label+'-promotion')/'results.json')
   elif gate=='intro':
    if candidate_mode:command('intro',[sys.executable,'-B',HERE/'replay_cases54m.py','intro',a.label+'-intro','--app',meta['source_app'],'--sha',meta['source_exe_sha256'],'--source',meta['source'],'--tape',ROOT/'tapes/tape-intro-54-49','--none-tape',ROOT/'tapes/tape-intro-54-none','--allow-incomplete'])
    else:command('intro',[sys.executable,'-B',HERE/'introskip54m.py',a.label+'-intro'])
   elif gate=='cappy':command('cappy',[sys.executable,'-B',HERE/'cappy54m.py',a.label+'-cappy']+(['--app',app,'--sha',meta['perf_exe_sha256'],'--source',meta['source'],'--build-kind','server-candidate'] if candidate_mode else []))
   elif gate=='cold':
    quiet('cold')
    for st in STAGES:
     for n in (1,2,3):
      d=stage(f'cold-{st}-r{n}',st);boot=[r for r in rows(d/'frames.csv') if r['phase']=='menu']
      worst=max(boot,key=lambda r:float(r['interval_ms'])) if boot else None
      write(d/'boot-screen.json',{'worst_menu_frame':worst,'nice':0});trim(d)
   elif gate=='image':
    quiet('image');ds=[]
    for mode in ('0','all'):
     name='image-'+mode;d=ROOT/'runs'/(a.label+'-'+name);dump=d/'dump';dump.mkdir(parents=True,exist_ok=False)
     # stage runner requires a new case directory, dump sits adjacent until moved after launch.
     outside=ROOT/'runs'/(a.label+'-'+name+'-dump');dump.rename(outside);d.rmdir()
     d=stage(name,STAGES[0],extra={'PETARI_UBERSHADER':mode,'PETARI_SURFACE_DUMP':str(outside),'PETARI_SURFACE_DUMP_LABELS':'input-script','PETARI_SURFACE_DUMP_EVERY':'60','PETARI_SURFACE_DUMP_SPAN':'180'},script=HERE/'stationary210.script')
     ds.append((d,outside));trim(d)
    command('image-diff',[sys.executable,'-B',HERE/'imgdiff54.py',ds[0][1],ds[1][1]])
    t=(ds[0][0]/'app.log').read_text(errors='replace');write(base/'image-offguard.json',{'uber_lines':sum('[gx uber]' in l for l in t.splitlines()),'dump_files':[{str(p):sha(p) for p in sorted(dump.glob('*.png'))} for _,dump in ds]})
    for _,dump in ds:shutil.rmtree(dump)
   elif gate=='warm':
    quiet('warm')
    for st in STAGES[:2]:
     cold='wp-'+st+'-prime-cold';prime='wp-'+st+'-prime';d=stage(cold,st);stage(prime,st,warm=cold);trim(d)
     for n in (1,2,3):
      for arm in (('hybrid','off') if n%2 else ('off','hybrid')):
       d=stage(f'wp-{st}-r{n}-{arm}',st,warm=prime,extra={'PETARI_UBERSHADER':'0' if arm=='off' else '1','PETARI_PIPELINE_DIAG':'0'});trim(d)
     trim(ROOT/'runs'/(a.label+'-'+prime))
    command('warm-pairs',[sys.executable,'-B',HERE/'warmpairs54.py',ROOT/'runs',a.label+'-wp',*STAGES[:2]])
   elif gate=='cadence':
    quiet('cadence');prime='cadence-prime';stage(prime,STAGES[0])
    for n in (1,2):
     for mode in ('fifo','immediate'):
      env={'PETARI_LATENCY_PROBE':'1','PETARI_LATENCY_TEST':'1','PETARI_PIPELINE_DIAG':'0'}
      if mode=='immediate':env['PETARI_PRESENT_MODE']='immediate'
      d=stage(f'cadence-{mode}-r{n}',STAGES[0],warm=prime,extra=env);trim(d)
    trim(ROOT/'runs'/(a.label+'-'+prime))
   elif gate=='audio':
    quiet('audio');dylib=base/'thrprobe.dylib';command('audio-probe-build',['clang','-dynamiclib','-O2',HERE/'thrprobe.c','-o',dylib,'-lpthread'])
    cold='audio-cold'
    for mode in ('cold','warm'):
     name='audio-'+mode
     d=stage(name,STAGES[1],warm=cold if mode=='warm' else None,extra={'SDL_AUDIO_DRIVER':'coreaudio','PETARI_SMOKE_REAL_AUDIO':'1','PETARI_AUDIO_DIAG':'1','PETARI_PIPELINE_DIAG':'1','DYLD_INSERT_LIBRARIES':str(dylib),'THRPROBE_SECONDS':'12','THRPROBE_OUT':str(base/('thr-'+mode+'.tsv'))})
     if mode=='warm':trim(d);trim(ROOT/'runs'/(a.label+'-'+cold))
   elif gate=='tape':
    if candidate_mode:
     write(base/'tape-status.json',normal)
     write(base/'candidate-normal-boot-ui.json',{**normal,'gate':'normal-boot-ui','frozen54_acceptance':False})
     command('candidate-pointer',[sys.executable,'-B',HERE/'replay_cases54m.py','pointer',a.label+'-pointer','--app',meta['source_app'],'--sha',meta['source_exe_sha256'],'--source',meta['source'],'--tape',ROOT/'tapes/tape-worldmap-54','--allow-incomplete'])
     continue
    # Candidate replay receipts cannot satisfy frozen54 identity. SOL B owns its runner.
    if a.candidate_app:
     assert a.candidate_sha and a.candidate_source, 'candidate app requires explicit SHA and source'
     command('candidate-normal-route',[sys.executable,'-B',HERE/'normal_tape_gate.py',a.candidate_app,a.candidate_sha,a.candidate_source,a.label+'-candidate-normal','--build-kind','server-candidate','--tape',a.tape,'--bound','600']+(['--allow-incomplete'] if a.allow_incomplete_tape else []))
     a.tape_result=ROOT/(a.label+'-candidate-normal')/'RESULT.json'
    if a.candidate_original_app:
     assert a.candidate_original_sha and a.candidate_source,'candidate original app requires its SHA and source'
     replay_base=[sys.executable,'-B',HERE/'replay_cases54m.py']
     identity_args=['--app',a.candidate_original_app,'--sha',a.candidate_original_sha,'--source',a.candidate_source]+(['--allow-incomplete'] if a.allow_incomplete_tape else [])
     command('candidate-intro',replay_base+['intro',a.label+'-candidate-intro',*identity_args,'--tape',ROOT/'tapes/tape-intro-54-49','--none-tape',ROOT/'tapes/tape-intro-54-none'])
     command('candidate-pointer',replay_base+['pointer',a.label+'-candidate-pointer',*identity_args,'--tape',ROOT/'tapes/tape-worldmap-54'])
    write(base/'tape-status.json',{'passed':False,'reason':'Frozen54 lacks full-session replay. PORT approved separate server-built candidate tape runs; never merge candidate evidence into frozen54 receipts.','candidate_result':str(a.tape_result) if a.tape_result else None})
    if a.tape_result:
     normal=json.loads(a.tape_result.read_text());write(base/'candidate-normal-boot-ui.json',{'passed':normal.get('acceptance_passed') is True,'build_kind':normal.get('build_kind'),'binary_sha256':normal.get('binary_sha256'),'source':normal.get('source'),'gate':'normal-boot-ui','receipt':str(a.tape_result),'checks':{k:normal.get('checks',{}).get(k) for k in ('scene_chain','existing_unlocked_file1','no_faults','no_divergence','whole_session_replay','replay_completed')},'limits':['Canonical normal-route tape includes ordinary title/FileSelect boot; same actual run proves this prefix. Candidate evidence only.']})
   else:raise ValueError('unknown gate '+gate)
  except Exception as e:
   write(base/(gate+'-error.json'),{'passed':False,'error':repr(e)});print('ERROR',gate,repr(e),flush=True)
  finally:
   # The gate command has reaped its app; no subsequent gate depends on its caches.
   for owned in (ROOT/'runs').glob(a.label+'-*'):
    if owned.is_dir():
     for child in sorted(owned.rglob('*'),key=lambda q:len(q.parts),reverse=True):
      if child.is_dir() and (child.name in ('cache','user','captures','tmp','dump') or child.name.endswith('-cache')):shutil.rmtree(child)
     if owned.name.endswith('-dump'):shutil.rmtree(owned)
 manifest['finished_utc']=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime());write(base/'MANIFEST.json',manifest)
 command('receipts',[sys.executable,'-B',HERE/'receipts54m.py',base,base/'receipts'])
 print('CAMPAIGN EVIDENCE',base,flush=True)
 return manifest['steps'][-1]['exit']
if __name__=='__main__':sys.exit(main())
