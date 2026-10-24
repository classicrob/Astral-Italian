#!/usr/bin/env python3
"""Candidate-only adapters for canonical intro-skip twins and promotion pointer tapes.

Copies a server candidate to SOL A's perfm1a namespace; records both source and
re-signed hashes. Never changes frozen54 results or invokes host input.
"""
import argparse,json,os,re,shutil,subprocess,sys
from common import *

ANCHOR_POLICY='interactive-and-play-ready-v1'

def anchor_checks(report,required_play_stage=None):
 parts=report.get('segments',[])
 checks={'anchor_policy_applied':report.get('anchor_policy')==ANCHOR_POLICY,
         'play_ready_state_checks':bool(parts) and all(s.get('play_ready_required') is False or
          (s.get('play_ready_required') is True and
           all((s.get('play_ready_state_check') or {}).get(k) is True for k in ('checked','passed','position_bit_exact')) and
           ((s.get('play_ready_state_check') or {}).get('rng') or {}).get('verified') is True) for s in parts)}
 if required_play_stage:
  checks['requested_play_ready_anchor']=any(s.get('stage')==required_play_stage and s.get('play_ready_required') is True for s in parts)
 return checks

def candidate(src,expected,source):
 src=src.resolve();assert src.is_relative_to(ROOT/'candidates'), 'use PORT server-built candidate under m1g/candidates'
 assert re.fullmatch('[0-9a-f]{40}',source)
 assert sha(src/'Contents/MacOS/Petari')==expected
 dest=ROOT/'apps'/('candidate-'+expected[:12]+'-perfm1a');app=dest/'Petari.app'
 if not dest.exists():
  disk();dest.mkdir(parents=True);subprocess.run(['/usr/bin/ditto',str(src),str(app)],check=True)
  subprocess.run(['/usr/libexec/PlistBuddy','-c',f'Set :CFBundleIdentifier {ID}',str(app/'Contents/Info.plist')],check=True)
  subprocess.run(['codesign','--force','--deep','-s','-',str(app)],check=True,capture_output=True)
  write(dest/'PREP.json',{'build_kind':'server-candidate','source':source,'source_app':str(src),'source_exe_sha256':expected,'perf_exe_sha256':sha(app/'Contents/MacOS/Petari'),'bundle_id':ID,'resources_identical':tree(app/'Contents/Resources')==tree(src/'Contents/Resources')})
 meta=json.loads((dest/'PREP.json').read_text());assert meta['source_exe_sha256']==expected and meta['source']==source and meta['resources_identical'] and sha(app/'Contents/MacOS/Petari')==meta['perf_exe_sha256']
 return app,meta

def replay(app,out,tape,stage,allow_incomplete,warm=None):
 tape=tape.resolve();meta=json.loads((tape/'route.tape.json').read_text());before=tree(tape)
 assert meta.get('complete') or allow_incomplete,'incomplete recording requires explicit --allow-incomplete'
 assert not meta.get('lossy_samples',0) and not meta.get('unencodable_samples',0),'lossy tape refused'
 mods=meta.get('mods',[]);env={'PETARI_TAPE_PLAY':str(tape),'PETARI_TAPE_PLAY_SESSION':'1','PETARI_TAPE_PLAY_QUIT':'1','PETARI_TAPE_PLAY_OUT':str(out/'replay'),'PETARI_TRACE_BOOT':'1','PETARI_MODS':','.join(mods+(['-OdysseyJumpDive'] if 'OdysseyJumpDive' not in mods else []))}
 for k,v in (meta.get('replay_env') or {}).items():
  assert k=='PETARI_TEST_RETURN_TO_SHIP',f'unsupported tape hook {k}';env[k]=str(v)
 proc=run(app,out,stage=stage,scenario=1,copied=True,extra=env,warm=warm,cold=not warm,seconds=540)
 report=json.loads((out/'replay/divergence.json').read_text());parts=report.get('segments',[]);text=(out/'app.log').read_text(errors='replace');rs=rows(out/'route.csv')
 checks={'completed_process':proc['completed'],'no_faults':proc['no_faults'],'seed_unchanged':proc['seed_unchanged'],'binary_unchanged':proc['binary_unchanged'],'tape_unchanged':tree(tape)==before,'whole_session_replay':report.get('mode')=='whole-session' and 'jump cut' not in text,'replay_completed':report.get('finished') is True and bool(parts) and all(s.get('started') and s.get('completed') and s.get('rows_compared')==s.get('tape_frames',-2)+1 for s in parts),'no_divergence':not report.get('stopped') and bool(parts) and not any(s.get('diverged') for s in parts)}
 checks.update(anchor_checks(report,stage if stage=='RedBlueExGalaxy' else None))
 result={'passed':all(checks.values()),'checks':checks,'process':proc,'replay':report,'build_kind':'server-candidate','tape':str(tape),'tape_exe_sha256':meta.get('exe_sha256'),'app_sha256':sha(app/'Contents/MacOS/Petari'),'tape_exe_hash_matches_app':meta.get('exe_sha256')==sha(app/'Contents/MacOS/Petari'),'hash_comparison_gated':False,'recording_complete':meta.get('complete'),'allow_incomplete':allow_incomplete,'launch_stage':stage,'nice':os.getpriority(os.PRIO_PROCESS,0),'host_input_used':False}
 result['first_divergence']=next((s for s in parts if s.get('diverged')),None)
 write(out/'RESULT.json',result)
 # Replay output duplicates the input tape; retain only divergence report and logs.
 for name in ('route.csv','route.kpd','route.actions','route.tape.json'):(out/'replay'/name).unlink(missing_ok=True)
 return result,text,rs

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('kind',choices=('intro','pointer'));p.add_argument('label');p.add_argument('--app',type=Path,required=True);p.add_argument('--sha',required=True);p.add_argument('--source',required=True);p.add_argument('--tape',type=Path,required=True);p.add_argument('--none-tape',type=Path);p.add_argument('--allow-incomplete',action='store_true');a=p.parse_args()
 assert re.fullmatch('[A-Za-z0-9_-]+',a.label);app,build=candidate(a.app,a.sha,a.source);base=ROOT/'runs'/a.label;base.mkdir(parents=True,exist_ok=False);records=[]
 if a.kind=='intro':
  assert a.none_tape,'intro requires no-press twin tape'
  for arm,tape in (('none',a.none_tape),('skip',a.tape)):
   out=base/arm;res,text,rs=replay(app,out,tape,'RedBlueExGalaxy',a.allow_incomplete)
   ready=[int(n) for n in re.findall(r'^\[play ready\] frame (\d+)',text,re.M)];first=next((int(r['frame']) for r in rs if r['stage']=='RedBlueExGalaxy'),None)
   res.update(arm=arm,play_ready_frame=ready[0] if ready else None,first_stage_frame=first,frames_from_stage_to_play=ready[0]-first if ready and first is not None else None)
   write(out/'RESULT.json',res);records.append(res);trim(out)
  no,skip=records;checks={'both_replays_passed':all(r['passed'] for r in records),'both_play_ready':all(r['play_ready_frame'] is not None for r in records),'skip_earlier_than_no_press':skip['frames_from_stage_to_play'] is not None and no['frames_from_stage_to_play'] is not None and skip['frames_from_stage_to_play']<no['frames_from_stage_to_play']}
 else:
  for arm in ('fresh','warm'):
   out=base/arm;res,text,rs=replay(app,out,a.tape,'WorldMap01Galaxy',a.allow_incomplete,warm=base/'fresh' if arm=='warm' else None)
   valid=[r for r in rs if r['stage']=='WorldMap01Galaxy' and r['scenario']=='1' and r['valid']=='1' and r.get('dead','0')=='0'];pointer=[r for r in valid if r.get('pointer_valid')=='1'];positions={(r.get('pointer_x'),r.get('pointer_y')) for r in pointer}
   extra={'pointer_readback_compared':sum(s.get('pointer_rows_compared',0) for s in res['replay'].get('segments',[]) if s.get('stage')=='WorldMap01Galaxy')>=30,'valid_requested_rows':len(valid)>=120,'recorded_pointer_rows':len(pointer)>=30 and len(positions)>=3,'existing_unlocked_file1':any(r.get('power_stars')=='242' for r in valid) and 'FileSelector.OpenFile' in text and 'FileSelector.Create' not in text and 'FileSelector.StartupCreate' not in text,'walk_and_idle':bool(valid) and any(abs(float(r[c])-float(valid[0][c]))>.01 for r in valid for c in ('x','y','z'))}
   res['checks'].update(extra);res.update(arm=arm,valid_requested_rows=len(valid),pointer_rows=len(pointer),pointer_positions=len(positions),passed=all(res['checks'].values()),limitation='Recorded KPAD pointer replay verifies game pointer path, not physical host-coordinate mapping or screenshots.')
   write(out/'RESULT.json',res);records.append(res)
   if arm=='warm':trim(out);trim(base/'fresh')
  checks={'fresh_warm_pointer_replays_passed':len(records)==2 and all(r['passed'] for r in records)}
 result={'passed':all(checks.values()),'checks':checks,'build_kind':'server-candidate','build':build,'runs':records,'nice':os.getpriority(os.PRIO_PROCESS,0),'frozen54_acceptance':False,'limitation':'Candidate evidence is separate from frozen54 receipts.'}
 write(base/'RESULT.json',result);print(json.dumps(result,indent=2));return 0 if result['passed'] else 1
if __name__=='__main__':sys.exit(main())
