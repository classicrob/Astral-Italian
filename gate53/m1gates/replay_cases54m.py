#!/usr/bin/env python3
"""Candidate-only adapters for canonical intro-skip twins and promotion pointer tapes.

Copies a server candidate to SOL A's perfm1a namespace; records both source and
re-signed hashes. Never changes frozen54 results or invokes host input.
"""
import argparse,json,math,os,re,shutil,subprocess,sys
from common import *

ANCHOR_POLICY='interactive-and-play-ready-v1'

def identity_contract(report,required=False):
 return required or 'interactive_position_bit_exact' in report or any('identity_rows_skipped' in s for s in report.get('segments',[]))

def replay_accounting(report,require_identity=False):
 parts=report.get('segments',[])
 try:
  if not identity_contract(report,require_identity):return bool(parts) and all(s['rows_compared']==s['tape_frames']+1 for s in parts)
  return bool(parts) and report.get('interactive_position_bit_exact') is True and all(type(s[k]) is int and s[k]>=0 for s in parts for k in ('rows_compared','identity_rows_skipped','tape_frames','pointer_rows_compared')) and all(s['rows_compared']+s['identity_rows_skipped']==s['tape_frames']+1 and s['pointer_rows_compared']==s['tape_frames'] for s in parts)
 except (KeyError,TypeError):return False

def identity_offsets(parts):
 try:
  return bool(parts) and all(s.get('identity_offset_checked') is True and s.get('identity_offset_passed') is True and s['identity_offset_limit']==30 and all(type(s[k]) is int and s[k]>=0 for k in ('identity_recorded_frame','identity_replay_frame','identity_expected_replay_frame')) and type(s['identity_frame_offset']) is int and s['identity_replay_frame']-s['identity_expected_replay_frame']==s['identity_frame_offset'] and abs(s['identity_frame_offset'])<=30 for s in parts)
 except (KeyError,TypeError):return False

def game_pointer_coverage(report):
 """Coverage from successful actual-WPad seam checks, never host BoundState."""
 parts=[s for s in report.get('segments',[]) if s.get('stage')=='WorldMap01Galaxy']
 result={'passed':False,'valid_rows_compared':0,'distinct_positions_compared':0,'source':'successful recorded KPAD versus actual game WPad seam comparisons'}
 try:
  assert parts and all(type(s[k]) is int and s[k]>=0 for s in parts for k in ('pointer_valid_rows_compared','pointer_distinct_positions_compared','pointer_rows_compared'))
  assert all(s['pointer_distinct_positions_compared']<=s['pointer_valid_rows_compared']<=s['pointer_rows_compared'] for s in parts)
  result.update(valid_rows_compared=sum(s['pointer_valid_rows_compared'] for s in parts),distinct_positions_compared=max(s['pointer_distinct_positions_compared'] for s in parts))
  result['passed']=result['valid_rows_compared']>=30 and result['distinct_positions_compared']>=3
 except (AssertionError,KeyError,TypeError):pass
 return result

def pointer_observation_checks(report,process):
 """Retain the reference 90-second window after strict tape completion."""
 checks=dict.fromkeys(('original_observation_window','neutral_post_roll_recorded'),False)
 try:
  o=report['observation'];p=o['post_roll']
  def number(v):return type(v) in (int,float) and math.isfinite(v) and v>=0
  checks['original_observation_window']=o['completed'] is True and type(o['requested_seconds']) in (int,float) and o['requested_seconds']==90 and o['clock']=='steady_clock; quit scheduling only' and all(number(o[k]) for k in ('start_monotonic_seconds','stop_monotonic_seconds','elapsed_seconds')) and number(process['seconds']) and 90<=o['elapsed_seconds']<=process['seconds']+.05 and process['seconds']>=90 and abs(o['stop_monotonic_seconds']-o['start_monotonic_seconds']-o['elapsed_seconds'])<=.05
  checks['neutral_post_roll_recorded']=p['started'] is True and p['input']=='neutral' and p['input_source']=='Headless::Replay::neutral' and all(type(p[k]) is int and p[k]>=0 for k in ('start_route_row','end_route_row','loops','neutral_read_calls')) and p['end_route_row']>=p['start_route_row'] and p['loops']>0 and p['neutral_read_calls']>0 and all(number(p[k]) for k in ('start_elapsed_seconds','elapsed_seconds')) and p['start_elapsed_seconds']<=o['elapsed_seconds'] and abs(p['start_elapsed_seconds']+p['elapsed_seconds']-o['elapsed_seconds'])<=.05
 except (KeyError,TypeError):pass
 return checks

def anchor_checks(report,required_play_stage=None,require_identity=False):
 parts=report.get('segments',[])
 checks={'anchor_policy_applied':report.get('anchor_policy')==ANCHOR_POLICY,
         'play_ready_state_checks':bool(parts) and all(s.get('play_ready_required') is False or
          (s.get('play_ready_required') is True and
           all((s.get('play_ready_state_check') or {}).get(k) is True for k in ('checked','passed','position_bit_exact')) and
           ((s.get('play_ready_state_check') or {}).get('rng') or {}).get('verified') is True) for s in parts)}
 if required_play_stage:
  checks['requested_play_ready_anchor']=any(s.get('stage')==required_play_stage and s.get('play_ready_required') is True for s in parts)
 if identity_contract(report,require_identity):
  checks.update(interactive_position_bit_exact=report.get('interactive_position_bit_exact') is True,bounded_scene_identities=identity_offsets(parts),strict_pointer_and_state_row_accounting=replay_accounting(report,True),zero_interactive_position_error=bool(parts) and all(s.get('max_position_error')==0 for s in parts))
 return checks

def candidate(src,expected,source):
 src=src.resolve();assert src.is_relative_to(ROOT/'candidates')
 app,meta=prep_candidate(src.parent)
 assert Path(meta['source_app'])==src and meta['source']==source and meta['source_exe_sha256']==expected
 return app,meta

def replay(app,out,tape,stage,allow_incomplete,warm=None,renderer_key=None,promotion_pointer=False,require_identity=False):
 tape=tape.resolve();meta=json.loads((tape/'route.tape.json').read_text());before=tree(tape)
 assert meta.get('complete') or allow_incomplete,'incomplete recording requires explicit --allow-incomplete'
 assert not meta.get('lossy_samples',0) and not meta.get('unencodable_samples',0),'lossy tape refused'
 mods=meta.get('mods',[]);env={'PETARI_TAPE_PLAY':str(tape),'PETARI_TAPE_PLAY_SESSION':'1','PETARI_TAPE_PLAY_QUIT':'1','PETARI_TAPE_PLAY_OUT':str(out/'replay'),'PETARI_TRACE_BOOT':'1','PETARI_MODS':','.join(mods+(['-OdysseyJumpDive'] if 'OdysseyJumpDive' not in mods else []))}
 for k,v in (meta.get('replay_env') or {}).items():
  assert k=='PETARI_TEST_RETURN_TO_SHIP',f'unsupported tape hook {k}';env[k]=str(v)
 from promotion54m import prepare_m1_fixture,inspect_cache
 fixture=prepare_m1_fixture()/'fixture'
 def caches(path):return [inspect_cache(p,renderer_key) for p in sorted(path.glob('*.db')) if renderer_key in p.name]
 before_cache=caches(Path(warm)/'cache') if promotion_pointer and warm else []
 if promotion_pointer and warm:env.update(SDL_AUDIO_DRIVER='coreaudio',PETARI_SMOKE_REAL_AUDIO='1',PETARI_AUDIO_DIAG='1')
 if promotion_pointer:env['PETARI_TAPE_PLAY_OBSERVE_SECONDS']='90'
 proc=run(app,out,stage=stage,scenario=1,copied=True,fixture=fixture,verify_fixture_attrs=True,extra=env,warm=warm,cold=not warm,seconds=540)
 report=json.loads((out/'replay/divergence.json').read_text());parts=report.get('segments',[]);text=(out/'app.log').read_text(errors='replace');rs=rows(out/'route.csv')
 checks={'completed_process':proc['completed'],'no_faults':proc['no_faults'],'seed_unchanged':proc['seed_unchanged'],'binary_unchanged':proc['binary_unchanged'],'tape_unchanged':tree(tape)==before,'whole_session_replay':report.get('mode')=='whole-session' and 'jump cut' not in text,'replay_completed':report.get('finished') is True and bool(parts) and all(s.get('started') and s.get('completed') for s in parts) and replay_accounting(report,require_identity),'no_divergence':not report.get('stopped') and bool(parts) and not any(s.get('diverged') for s in parts)}
 checks.update(anchor_checks(report,stage if stage=='RedBlueExGalaxy' else None,require_identity))
 if promotion_pointer:
  frame_rows=rows(out/'frames.csv');after_cache=caches(out/'cache');audio=[int(n) for n in re.findall(r'underrun (\d+) frames',text)]
  checks.update(menu_frames=any(r['phase']=='menu' for r in frame_rows),gameplay_frames=sum(r['phase']=='gameplay' for r in frame_rows)>=120,matching_key_cache_written=any(r['rows'] for r in after_cache),cache_arm_verified=any(r['rows'] for r in before_cache) if warm else not before_cache)
  if warm:checks['real_audio_zero_underruns']=len(audio)>=50 and sum(audio)==0 and 'SDL audio underrun:' not in text
  if identity_contract(report,require_identity):checks.update(pointer_observation_checks(report,proc))
 result={'passed':all(checks.values()),'checks':checks,'process':proc,'replay':report,'build_kind':'server-candidate','tape':str(tape),'tape_exe_sha256':meta.get('exe_sha256'),'app_sha256':sha(app/'Contents/MacOS/Petari'),'tape_exe_hash_matches_app':meta.get('exe_sha256')==sha(app/'Contents/MacOS/Petari'),'hash_comparison_gated':False,'recording_complete':meta.get('complete'),'allow_incomplete':allow_incomplete,'launch_stage':stage,'nice':os.getpriority(os.PRIO_PROCESS,0),'host_input_used':False}
 result['first_divergence']=next((s for s in parts if s.get('diverged')),None)
 if promotion_pointer:result.update(before_cache=before_cache,after_cache=after_cache,real_audio=bool(warm),audio_reports=len(audio),audio_underrun_frames=sum(audio),frame_counts={phase:sum(r['phase']==phase for r in frame_rows) for phase in ('startup','loading','menu','gameplay')})
 write(out/'RESULT.json',result)
 # Replay output duplicates the input tape; retain only divergence report and logs.
 for name in ('route.csv','route.kpd','route.actions','route.tape.json'):(out/'replay'/name).unlink(missing_ok=True)
 return result,text,rs

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('kind',choices=('intro','pointer'));p.add_argument('label');p.add_argument('--app',type=Path,required=True);p.add_argument('--sha',required=True);p.add_argument('--source',required=True);p.add_argument('--tape',type=Path,required=True);p.add_argument('--none-tape',type=Path);p.add_argument('--allow-incomplete',action='store_true');p.add_argument('--require-bounded-identity',action='store_true');a=p.parse_args()
 assert re.fullmatch('[A-Za-z0-9_-]+',a.label);app,build=candidate(a.app,a.sha,a.source);base=ROOT/'runs'/a.label;base.mkdir(parents=True,exist_ok=False);records=[]
 if a.kind=='intro':
  assert a.none_tape,'intro requires no-press twin tape'
  for arm,tape in (('none',a.none_tape),('skip',a.tape)):
   out=base/arm;res,text,rs=replay(app,out,tape,'RedBlueExGalaxy',a.allow_incomplete,require_identity=a.require_bounded_identity)
   ready=[int(n) for n in re.findall(r'^\[play ready\] frame (\d+)',text,re.M)];first=next((int(r['frame']) for r in rs if r['stage']=='RedBlueExGalaxy'),None)
   res.update(arm=arm,play_ready_frame=ready[0] if ready else None,first_stage_frame=first,frames_from_stage_to_play=ready[0]-first if ready and first is not None else None)
   write(out/'RESULT.json',res);records.append(res);trim(out)
  no,skip=records;checks={'both_replays_passed':all(r['passed'] for r in records),'both_play_ready':all(r['play_ready_frame'] is not None for r in records),'skip_earlier_than_no_press':skip['frames_from_stage_to_play'] is not None and no['frames_from_stage_to_play'] is not None and skip['frames_from_stage_to_play']<no['frames_from_stage_to_play']}
 else:
  for arm in ('fresh','warm'):
   out=base/arm;res,text,rs=replay(app,out,a.tape,'WorldMap01Galaxy',a.allow_incomplete,warm=base/'fresh' if arm=='warm' else None,renderer_key=build['renderer_key'],promotion_pointer=True,require_identity=a.require_bounded_identity)
   valid=[r for r in rs if r['stage']=='WorldMap01Galaxy' and r['scenario']=='1' and r['valid']=='1' and r.get('dead','0')=='0'];pointer=[r for r in valid if r.get('pointer_valid')=='1'];positions={(r.get('pointer_x'),r.get('pointer_y')) for r in pointer}
   pointer_rows=len(pointer);pointer_positions=len(positions);pointer_pass=pointer_rows>=30 and pointer_positions>=3
   if identity_contract(res['replay'],a.require_bounded_identity):
    coverage=game_pointer_coverage(res['replay']);pointer_rows=coverage['valid_rows_compared'];pointer_positions=coverage['distinct_positions_compared'];pointer_pass=coverage['passed']
    res.update(pointer_coverage=coverage,host_route_pointer_rows=len(pointer),host_route_pointer_positions=len(positions))
   extra={'pointer_readback_compared':sum(s.get('pointer_rows_compared',0) for s in res['replay'].get('segments',[]) if s.get('stage')=='WorldMap01Galaxy')>=30,'valid_requested_rows':len(valid)>=120,'recorded_pointer_rows':pointer_pass,'existing_unlocked_file1':any(r.get('power_stars')=='242' for r in valid) and 'FileSelector.OpenFile' in text and 'FileSelector.Create' not in text and 'FileSelector.StartupCreate' not in text,'walk_and_idle':bool(valid) and any(abs(float(r[c])-float(valid[0][c]))>.01 for r in valid for c in ('x','y','z'))}
   res['checks'].update(extra);res.update(arm=arm,valid_requested_rows=len(valid),pointer_rows=pointer_rows,pointer_positions=pointer_positions,passed=all(res['checks'].values()),limitation='Recorded KPAD pointer replay verifies game pointer path, not physical host-coordinate mapping or screenshots.')
   write(out/'RESULT.json',res);records.append(res)
   if arm=='warm':trim(out);trim(base/'fresh')
  checks={'fresh_warm_pointer_replays_passed':len(records)==2 and all(r['passed'] for r in records)}
 result={'passed':all(checks.values()),'checks':checks,'build_kind':'server-candidate','build':build,'runs':records,'nice':os.getpriority(os.PRIO_PROCESS,0),'frozen54_acceptance':False,'limitation':'Candidate evidence is separate from frozen54 receipts.'}
 write(base/'RESULT.json',result);print(json.dumps(result,indent=2));return 0 if result['passed'] else 1
if __name__=='__main__':sys.exit(main())
