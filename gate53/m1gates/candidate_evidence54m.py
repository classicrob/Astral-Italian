"""Explicit candidate evidence joins; never substitutes for frozen54 results."""
import json
from pathlib import Path
from common import ROOT,HOME,FIXTURE,FROZEN,sha,tree,write
NORMAL_CHECKS=('scene_chain','destination_300_consecutive_valid','no_faults','no_invalid_worldmap_request','existing_unlocked_file1','seed_unchanged','binary_unchanged','no_divergence','whole_session_replay','replay_completed','lossless_tape','tape_unchanged','game_pointer_buffers_compared','anchor_policy_applied','play_ready_state_checks')
POINTER_CHECKS=('completed_process','no_faults','seed_unchanged','binary_unchanged','tape_unchanged','whole_session_replay','replay_completed','no_divergence','anchor_policy_applied','play_ready_state_checks','pointer_readback_compared','valid_requested_rows','recorded_pointer_rows','existing_unlocked_file1','walk_and_idle')

def protected_inputs(directory):
 from promotion54m import inventory
 package=ROOT/'fixtures/unlocked-242';receipt=json.loads((package/'receipt.json').read_text())
 return {'candidate_app':tree(Path(directory)/'Petari.app'),'frozen_exe_sha256':sha(FROZEN/'Petari.app/Contents/MacOS/Petari'),'canonical_fixture':inventory(FIXTURE),'local_fixture':inventory(package/'fixture'),'live_save':inventory(Path(receipt['source'])),'user_cache_files':tree(HOME/'Library/Caches/AstralItalian')}

def _candidate_replays(paths,directory,require_identity,required_checks):
 directory=Path(directory).resolve();build=json.loads((directory/'BUILD.json').read_text())
 assert directory.is_relative_to(ROOT/'candidates') and len(paths)==3 and len({str(Path(p).resolve()) for p in paths})==3
 records=[]
 for path in paths:
  path=Path(path).resolve();assert path.is_relative_to(ROOT)
  d=json.loads(path.read_text());command_path=path.parent/'command.json';command=json.loads(command_path.read_text())
  exe=Path(command['command'][0]).resolve();assert exe.is_relative_to(ROOT/'solb')
  app=exe.parents[2];prep_path=app.parent/'PREP.json';prep=json.loads(prep_path.read_text())
  assert prep['source_commit']==build['source'] and prep['original_exe_sha256']==build['binary_sha256']
  assert prep['build_kind']==d['build_kind']=='server-candidate' and prep['source_unchanged_after_prep'] is True
  assert prep['perf_sha256']==d['binary_sha256']==command['binary_sha256']==sha(exe)
  assert tree(app/'Contents/Resources')==tree(directory/'Petari.app/Contents/Resources')
  assert d['anchor_policy']['id']=='interactive-and-play-ready-v1' and d['host_input_used'] is False
  assert d['priority']['nice']==0 and d['passed'] is True and d['acceptance_passed'] is True and all(d['checks'].get(k) is True for k in required_checks) and all(v is True for v in d['checks'].values())
  from replay_cases54m import anchor_checks,replay_accounting
  native=json.loads((path.parent/'replay/divergence.json').read_text())
  assert native.get('finished') is True and not native.get('stopped') and native.get('mode')=='whole-session'
  assert all(s.get('started') and s.get('completed') and not s.get('diverged') for s in native['segments'])
  assert all(anchor_checks(native,require_identity=require_identity).values()) and replay_accounting(native,require_identity)
  record={'receipt':str(path),'sha256':sha(path),'command':str(command_path),'command_sha256':sha(command_path),'prep':str(prep_path),'prep_sha256':sha(prep_path),'normal_perf_sha256':d['binary_sha256'],'anchor_policy':d['anchor_policy'],'checks':d['checks']}
  records.append((d,native,record))
 assert len({r['normal_perf_sha256'] for _,_,r in records})==1
 return build,records

def normal_evidence(paths,directory,require_identity=False):
 build,loaded=_candidate_replays(paths,directory,require_identity,NORMAL_CHECKS)
 for d,_,_ in loaded:
  stages=[s['stage'] for s in d['scene_anchors']]
  assert '' in stages and 'FileSelect' in stages
 records=[r for _,_,r in loaded]
 return {'passed':True,'build_kind':'server-candidate','source':build['source'],'source_exe_sha256':build['binary_sha256'],'normal_perf_sha256':records[0]['normal_perf_sha256'],'acceptance_runs':3,'records':records,'reason':'Three whole-session normal-route passes also prove the title/FileSelect normal-boot-ui prefix, per PORT ruling.'}

FULL_STAR_CHECKS=tuple(k for k in NORMAL_CHECKS if k not in ('destination_300_consecutive_valid','existing_unlocked_file1','seed_unchanged'))+('identity_offsets_bounded','app_unchanged','fresh_user_zero_stars','no_deaths','star_get','star_row_aligned','star_state_bit_exact','zero_interactive_position_error','all_play_ready_state_rng_checks','actual_recording_complete','route_rows_contiguous')

def full_star_evidence(paths,directory):
 import csv
 import full_star_tape_gate as full
 build,loaded=_candidate_replays(paths,directory,True,FULL_STAR_CHECKS);records=[]
 for d,native,record in loaded:
  case=Path(record['receipt']).parent;command=json.loads(Path(record['command']).read_text())
  assert d['gate']=='full_star_tape_gate' and d['save_mode']=='fresh private empty user'
  assert d['launch_stage']=='RedBlueExGalaxy' and d['launch_scenario']==1 and command['save_seed'] is None and command['save_before']=={}
  argv=command['command'];assert argv[argv.index('--stage')+1]=='RedBlueExGalaxy' and argv[argv.index('--scenario')+1]=='1'
  assert not any(k in command['environment'] for k in ('PETARI_INPUT_SCRIPT','PETARI_TEST_RETURN_TO_SHIP','PETARI_LATENCY_TEST'))
  tape=Path(d['tape']).resolve();assert tape.is_relative_to(ROOT/'tapes') and argv[argv.index('--replay-tape')+1]==str(tape)
  assert command['environment']['PETARI_ROUTE_RECORD']==str(case/'route.csv') and d['replay']==native
  meta=json.loads((tape/'route.tape.json').read_text())
  with (case/'route.csv').open() as stream:rows=list(csv.DictReader(stream))
  checks,proof=full.evaluate(rows,(case/'app.log').read_text(errors='replace'),native,meta,case,tape)
  assert checks and all(v is True for v in checks.values()) and all(d['checks'].get(k)==v for k,v in checks.items())
  for key in ('recorded_star_event','star_event','star_anchor_offset','scene_chain','recorded_scene_chain','recorded_play_ready_count','actual_play_ready_count','all_play_ready_checkpoints','tape_prefix'):
   assert d[key]==proof[key]
  expected=proof['recorded_star_event'];assert expected=={'row':8426,'stage':'MarioFaceShipGalaxy','scenario':2,'previous':0,'power_stars':1}
  points=proof['all_play_ready_checkpoints'];assert proof['recorded_play_ready_count']==proof['actual_play_ready_count']==len(points)==4
  assert [(p['recorded']['stage'],p['recorded']['row']) for p in points]==[('RedBlueExGalaxy',1255),('RedBlueExGalaxy',4222),('RedBlueExGalaxy',5464),('MarioFaceShipGalaxy',10750)]
  assert all(p['rng']['restored_at_this_checkpoint'] is value for p,value in zip(points,(True,False,False,True)))
  assert all(p['passed'] is True and p['position_bit_exact'] is True and p['rng']['verified'] is True and p['rng']['recorded']==p['rng']['readback'] for p in points)
  record={**record,'full_star_perf_sha256':record['normal_perf_sha256'],'star_event':proof['star_event'],'recorded_star_event':expected,'all_play_ready_checkpoints':points,'tape_prefix':proof['tape_prefix'],'actual_artifacts_sha256':{str(p):sha(p) for p in (case/'route.csv',case/'route.tape.json',case/'replay/divergence.json',case/'app.log')},'source_tape':str(tape),'source_tape_files':tree(tape)}
  del record['normal_perf_sha256'];records.append(record)
 return {'passed':True,'build_kind':'server-candidate','source':build['source'],'source_exe_sha256':build['binary_sha256'],'full_star_perf_sha256':records[0]['full_star_perf_sha256'],'acceptance_runs':3,'records':records,'reason':'Three strict full-star replays from fresh private users; zero-to-one star transition and all four exact state/RNG checkpoints revalidated.','limits':['Each tape_prefix records the unchanged native min(declared seams, complete CSV rows) policy; original input tapes remain unchanged.']}

def combine_promotion(direct_path,pointer_path,meta,out):
 direct_path=Path(direct_path);pointer_path=Path(pointer_path)
 direct=json.loads(direct_path.read_text());pointer=json.loads(pointer_path.read_text())
 expected={(n,mode) for n in ('yoshi','flip','fluffy','unlocked-stage-adapter','return') for mode in ('fresh','warm')}|{(n,'functional') for n in ('cappy-fluffy','cappy-panel','cappy-spin-after-throw')}
 assert len(direct)==13 and {(r['case'],r['mode']) for r in direct}==expected
 records=[];evidence=[]
 for r in direct:
  receipt=direct_path.parent/(r['case']+'-'+r['mode'])/'receipt.json'
  assert json.loads(receipt.read_text())==r and r['source']==meta['source'] and r['binary_sha256']==meta['perf_exe_sha256'] and r['build_kind']=='server-candidate'
  assert r['passed']==all(r['checks'].values())
  records.append(r);evidence.append({'case':r['case'],'mode':r['mode'],'receipt':str(receipt),'sha256':sha(receipt)})
 assert pointer['build_kind']=='server-candidate' and pointer['build']['source']==meta['source'] and pointer['build']['perf_exe_sha256']==meta['perf_exe_sha256']
 assert len(pointer['runs'])==2 and {r['arm'] for r in pointer['runs']}=={'fresh','warm'}
 assert pointer['passed']==all(r['passed'] for r in pointer['runs'])==all(pointer['checks'].values())
 for r in pointer['runs']:
  receipt=pointer_path.parent/r['arm']/'RESULT.json';assert json.loads(receipt.read_text())==r
  assert r['app_sha256']==meta['perf_exe_sha256'] and r['nice']==0 and r['passed']==all(r['checks'].values())
  assert set(POINTER_CHECKS).issubset(r['checks'])
  assert all(k in r['checks'] for k in ('menu_frames','gameplay_frames','matching_key_cache_written','cache_arm_verified'))
  if r['arm']=='warm':assert 'real_audio_zero_underruns' in r['checks']
  records.append({'case':'worldmap','mode':r['arm'],'passed':r['passed'],'checks':r['checks'],'source':meta['source'],'binary_sha256':meta['perf_exe_sha256'],'renderer_key':meta['renderer_key'],'nice':0,'build_kind':'server-candidate','input_driver':'canonical whole-session KPAD pointer tape','equivalence_authority':'PORT final candidate ruling: promotion world map via pointer tape adapter','actual_receipt':str(receipt),'actual_receipt_sha256':sha(receipt),'limitation':r['limitation']})
  evidence.append({'case':'worldmap','mode':r['arm'],'receipt':str(receipt),'sha256':sha(receipt)})
 write(out,records);write(Path(out).with_name('promotion-evidence.json'),{'build_kind':'server-candidate','source':meta['source'],'source_exe_sha256':meta['source_exe_sha256'],'perf_exe_sha256':meta['perf_exe_sha256'],'cases':evidence,'failed_verdicts_preserved':True,'frozen54_acceptance':False})
 return records
