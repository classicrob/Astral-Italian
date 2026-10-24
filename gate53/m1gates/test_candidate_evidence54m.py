#!/usr/bin/env python3
"""Candidate identity and authorized tier boundaries; no game launch."""
import copy,json,tempfile,unittest,subprocess
from pathlib import Path
from unittest.mock import patch
import candidate_evidence54m as ce
from common import sha,write
from receipts54m import requires_refusal

class CandidateEvidence(unittest.TestCase):
 def normal_fixture(self,root):
  directory=root/'candidates/candidate';directory.mkdir(parents=True)
  source='c'*40;original=directory/'Petari.app/Contents/MacOS/Petari';original.parent.mkdir(parents=True);original.write_bytes(b'original')
  (directory/'Petari.app/Contents/Resources').mkdir()
  write(directory/'BUILD.json',{'source':source,'binary_sha256':sha(original)})
  app=root/'solb/candidate/Petari.app';exe=app/'Contents/MacOS/Petari';exe.parent.mkdir(parents=True);exe.write_bytes(b'signed copy');(app/'Contents/Resources').mkdir()
  prep={'source_commit':source,'original_exe_sha256':sha(original),'perf_sha256':sha(exe),'build_kind':'server-candidate','source_unchanged_after_prep':True};write(app.parent/'PREP.json',prep)
  paths=[]
  for n in range(3):
   path=root/f'normal-r{n}/RESULT.json';command={'command':[str(exe)],'binary_sha256':sha(exe)};write(path.parent/'command.json',command)
   write(path.parent/'replay/divergence.json',{'mode':'whole-session','finished':True,'stopped':'','anchor_policy':'interactive-and-play-ready-v1','segments':[{'stage':'FileSelect','started':True,'completed':True,'diverged':False,'play_ready_required':False,'rows_compared':2,'tape_frames':1}]})
   write(path,{'build_kind':'server-candidate','binary_sha256':sha(exe),'passed':True,'acceptance_passed':True,'checks':dict.fromkeys(ce.NORMAL_CHECKS,True),'priority':{'nice':0},'host_input_used':False,'anchor_policy':{'id':'interactive-and-play-ready-v1'},'scene_anchors':[{'stage':''},{'stage':'FileSelect'}]});paths.append(path)
  return directory,paths,app.parent/'PREP.json'
 def test_normal_joins_three_real_identity_records(self):
  with tempfile.TemporaryDirectory() as raw,patch.object(ce,'ROOT',Path(raw).resolve()):
   directory,paths,_=self.normal_fixture(Path(raw).resolve());d=ce.normal_evidence(paths,directory)
   self.assertTrue(d['passed']);self.assertEqual(d['acceptance_runs'],3)
   with self.assertRaises(AssertionError):ce.normal_evidence([paths[0]]*3,directory)
 def test_normal_refuses_cross_build_and_missing_checks(self):
  with tempfile.TemporaryDirectory() as raw,patch.object(ce,'ROOT',Path(raw).resolve()):
   directory,paths,prep_path=self.normal_fixture(Path(raw).resolve());original=json.loads(prep_path.read_text());bad=copy.deepcopy(original);bad['original_exe_sha256']='0'*64;write(prep_path,bad)
   with self.assertRaises(AssertionError):ce.normal_evidence(paths,directory)
   write(prep_path,original);d=json.loads(paths[0].read_text());del d['checks']['play_ready_state_checks'];write(paths[0],d)
   with self.assertRaises(AssertionError):ce.normal_evidence(paths,directory)
 def test_failed_functional_or_missing_tier_measurement_refuses(self):
  for name in ('flipswap-cappy','intro-skip','gx-fresh-warm','input-latency','normal-route-grandmap','wii-proof'):
   self.assertTrue(requires_refusal(name,False,True,True),name)
  for name in ('v2-cold-stall','fifo-default-cadence-latency'):
   self.assertFalse(requires_refusal(name,False,True,True),name)
   self.assertTrue(requires_refusal(name,False,False,True),name)
   self.assertTrue(requires_refusal(name,False,True,False),name)
 def test_promotion_join_retains_failed_pointer_verdict(self):
  with tempfile.TemporaryDirectory() as raw:
   root=Path(raw);meta={'source':'c'*40,'perf_exe_sha256':'a'*64,'source_exe_sha256':'b'*64,'renderer_key':'d'*64}
   direct=[]
   cases=[(n,mode) for n in ('yoshi','flip','fluffy','unlocked-stage-adapter','return') for mode in ('fresh','warm')]+[(n,'functional') for n in ('cappy-fluffy','cappy-panel','cappy-spin-after-throw')]
   for name,mode in cases:
    r={'case':name,'mode':mode,'passed':True,'checks':{'no_faults':True},'source':meta['source'],'binary_sha256':meta['perf_exe_sha256'],'build_kind':'server-candidate','nice':0}
    write(root/'direct'/(name+'-'+mode)/'receipt.json',r);direct.append(r)
   write(root/'direct/results.json',direct);runs=[]
   for arm in ('fresh','warm'):
    checks=dict.fromkeys(ce.POINTER_CHECKS+('menu_frames','gameplay_frames','matching_key_cache_written','cache_arm_verified','real_audio_zero_underruns'),True)
    if arm=='warm':checks['no_divergence']=False
    r={'arm':arm,'app_sha256':meta['perf_exe_sha256'],'nice':0,'passed':all(checks.values()),'checks':checks,'limitation':'fixture'}
    write(root/'pointer'/arm/'RESULT.json',r);runs.append(r)
   pointer={'build_kind':'server-candidate','build':meta,'passed':False,'checks':{'fresh_warm_pointer_replays_passed':False},'runs':runs};write(root/'pointer/RESULT.json',pointer)
   combined=ce.combine_promotion(root/'direct/results.json',root/'pointer/RESULT.json',meta,root/'combined/results.json')
   self.assertEqual(len(combined),15);self.assertEqual(sum(r['passed'] for r in combined),14)
   self.assertFalse(next(r for r in combined if r['case']=='worldmap' and r['mode']=='warm')['checks']['no_divergence'])
   missing=copy.deepcopy(pointer);del missing['runs'][1]['checks']['real_audio_zero_underruns'];write(root/'pointer/warm/RESULT.json',missing['runs'][1]);write(root/'pointer/RESULT.json',missing)
   with self.assertRaises(AssertionError):ce.combine_promotion(root/'direct/results.json',root/'pointer/RESULT.json',meta,root/'missing-audio/results.json')
   write(root/'pointer/warm/RESULT.json',pointer['runs'][1])
   pointer['build']['source']='f'*40;write(root/'pointer/RESULT.json',pointer)
   with self.assertRaises(AssertionError):ce.combine_promotion(root/'direct/results.json',root/'pointer/RESULT.json',{**meta,'source':'c'*40},root/'bad/results.json')
 def test_wii_requires_fresh_build_and_strict_dispatch_success(self):
  import candidate_wii54m as cw
  with tempfile.TemporaryDirectory() as raw:
   root=Path(raw).resolve();directory=root/'candidates/candidate';exe=directory/'Petari.app/Contents/MacOS/Petari';exe.parent.mkdir(parents=True);exe.write_bytes(b'candidate')
   write(directory/'BUILD.json',{'source':'c'*40,'binary_sha256':sha(exe)})
   write(root/'MacioGalaxy2-team/main-report'/(cw.MAIN+'.json'),{'version':2,'units':[{}]})
   log=root/'candidates/wii-proof-cccccccc.log'
   def git(command,**kwargs):
    text=''
    if command[1:2]==['show']:
     text='tools/team/nj.sh dispatchcheck.py --strict if [ "$gate_mode" = full ]; then main-report' if 'land.sh' in command[2] else 'checked allow list'
    return subprocess.CompletedProcess(command,0,text,'')
   with patch.object(cw,'ROOT',root),patch.object(cw,'HOME',root),patch.object(cw.subprocess,'run',side_effect=git):
    for status,want in (('rc=0\ndispatch_rc=0\n',True),('rc=1\ndispatch_rc=0\n',False),('rc=0\ndispatch_rc=1\n',False),('rc=0\n',False)):
     log.write_text(status);self.assertEqual(cw.evaluate(directory,log)['passed'],want,status)

if __name__=='__main__':unittest.main()
