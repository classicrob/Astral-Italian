"""Explicit candidate evidence joins; never substitutes for frozen54 results."""
import json
from pathlib import Path
from common import ROOT,sha,tree,write
NORMAL_CHECKS=('scene_chain','destination_300_consecutive_valid','no_faults','no_invalid_worldmap_request','existing_unlocked_file1','seed_unchanged','binary_unchanged','no_divergence','whole_session_replay','replay_completed','lossless_tape','tape_unchanged','game_pointer_buffers_compared','anchor_policy_applied','play_ready_state_checks')
POINTER_CHECKS=('completed_process','no_faults','seed_unchanged','binary_unchanged','tape_unchanged','whole_session_replay','replay_completed','no_divergence','anchor_policy_applied','play_ready_state_checks','pointer_readback_compared','valid_requested_rows','recorded_pointer_rows','existing_unlocked_file1','walk_and_idle')

def normal_evidence(paths,directory):
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
  assert d['priority']['nice']==0 and d['passed'] is True and d['acceptance_passed'] is True and all(d['checks'].get(k) is True for k in NORMAL_CHECKS) and all(v is True for v in d['checks'].values())
  stages=[s['stage'] for s in d['scene_anchors']]
  assert '' in stages and 'FileSelect' in stages
  records.append({'receipt':str(path),'sha256':sha(path),'command':str(command_path),'command_sha256':sha(command_path),'prep':str(prep_path),'prep_sha256':sha(prep_path),'normal_perf_sha256':d['binary_sha256'],'anchor_policy':d['anchor_policy'],'checks':d['checks']})
 assert len({r['normal_perf_sha256'] for r in records})==1
 return {'passed':True,'build_kind':'server-candidate','source':build['source'],'source_exe_sha256':build['binary_sha256'],'normal_perf_sha256':records[0]['normal_perf_sha256'],'acceptance_runs':3,'records':records,'reason':'Three whole-session normal-route passes also prove the title/FileSelect normal-boot-ui prefix, per PORT ruling.'}

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
  records.append({'case':'worldmap','mode':r['arm'],'passed':r['passed'],'checks':r['checks'],'source':meta['source'],'binary_sha256':meta['perf_exe_sha256'],'renderer_key':meta['renderer_key'],'nice':0,'build_kind':'server-candidate','input_driver':'canonical whole-session KPAD pointer tape','equivalence_authority':'PORT final candidate ruling: promotion world map via pointer tape adapter','actual_receipt':str(receipt),'actual_receipt_sha256':sha(receipt),'limitation':r['limitation']})
  evidence.append({'case':'worldmap','mode':r['arm'],'receipt':str(receipt),'sha256':sha(receipt)})
 write(out,records);write(Path(out).with_name('promotion-evidence.json'),{'build_kind':'server-candidate','source':meta['source'],'source_exe_sha256':meta['source_exe_sha256'],'perf_exe_sha256':meta['perf_exe_sha256'],'cases':evidence,'failed_verdicts_preserved':True,'frozen54_acceptance':False})
 return records
