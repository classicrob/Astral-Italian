#!/usr/bin/env python3
"""Freeze54-format receipts from actual M1 evidence; required failures refuse ALL receipts.

receipts54m.py CAMPAIGN_DIR OUT_DIR
Always writes VALIDATION.json and COMPARISON.md alongside the campaign. Only an
identity-consistent campaign with every functional requirement met creates OUT_DIR.
Only explicitly authorized candidate cold/cadence rows are TIER observations;
missing or invalid tier measurements still refuse. Candidate results cannot
satisfy frozen54 acceptance.
"""
import argparse,json,math,os,re,sys
from pathlib import Path
from common import HERE,ROOT,FROZEN,sha,write,rows
def is_tier_gate(name,candidate_mode):
 return candidate_mode and name in ('v2-cold-stall','fifo-default-cadence-latency')
def requires_refusal(name,passed,measurement_verified,candidate_mode):
 return not passed and (not is_tier_gate(name,candidate_mode) or not measurement_verified)
REQUIRED=('macos-15-compat','seed-pack','gx-fresh-warm','normal-route-grandmap','intro-skip','flipswap-cappy','v2-cold-stall','v2-image-pair','v2-warm-pairs','fifo-default-cadence-latency','audio-thread-priority','wii-proof')
SERVER={
 'macos-15-compat':'PASS minos 15.0, M1 macOS 15.1 weak imports 0, bundle clean',
 'seed-pack':'PASS v13 seed pack, 64 bundled stage DBs',
 'gx-fresh-warm':'PASS 15/15; nice 19; worldmap pointer used host input',
 'normal-route-grandmap':'PASS title → FileSelect → Starship → grand map → Sky Station; host input',
 'intro-skip':'PASS Flip-Swap play-ready frame 1255 → 558 using host Space',
 'flipswap-cappy':'PASS 23/23; nice 19',
 'v2-cold-stall':'PASS exact freeze exe 7/7 (Yoshi x1, Fluffy/Starship x3); menu p99 ≤21.2 ms; worst menu 46.8–51.2 ms',
 'v2-image-pair':'PASS forced-all vs off mean RGB <0.01; off guard 1 line',
 'v2-warm-pairs':'PASS 3 alternating hybrid/off pairs per stage; p99 not reliably >0.5 ms worse',
 'fifo-default-cadence-latency':'PASS 120 Hz Fifo late 0.25/0.20%; every input 2 frames after read',
 'audio-thread-priority':'PASS CoreAudio cold/warm nice 19: 0 underruns, 0 baton waits >50 ms, main priority 47',
 'wii-proof':'PASS release native/docs-only lineage from a23609ca; original full Wii proof retained by release',
}

def readj(path):
 text=Path(path).read_text();return json.loads(text.split('exit=')[0])

def latency(text):
 return [(int(n),float(ms)) for n,ms in re.findall(r'\[latency\] press:.*\(frame \d+, (\d+) frames after the read\).*on screen ([\d.]+) ms',text)]

def audio_observation(text, samples):
 und=[int(n) for n in re.findall(r'underrun (\d+) frames',text)]
 baton=[float(n) for n in re.findall(r'preemption pending ([\d.]+) ms',text)]
 raised='main thread priority: 37 -> 47 (raised)' in text
 # Unlike promotion's timed cases, the dedicated reference probe has no fixed report count.
 ok=bool(und) and sum(und)==0 and 'SDL audio underrun:' not in text and not any(n>50 for n in baton) and raised and samples>0
 return {'passed':ok,'underrun_reports':len(und),'underrun_frames':sum(und),'baton_over50':sum(n>50 for n in baton),'main_thread_raised':raised,'thread_samples':samples}

def summary(g):
 d=g['details'];name=g['gate']
 if 'error' in d:return d['error'][:220]
 if name=='macos-15-compat':return 'macOS '+d['macos']+'; '+', '.join(k+'='+str(v) for k,v in d['checks'].items())
 if name=='seed-pack':return '64 DBs; selected source resources identical; SQLite integrity checked'
 if name=='gx-fresh-warm':return str(d['passing'])+'/'+str(d['cases'])+'; failed: '+', '.join(d['failed_checks'])
 if name in ('normal-route-grandmap','normal-boot-ui'):return d.get('reason','Unavailable')
 if name=='full-star-flipswap':return str(d['acceptance_runs'])+' strict full-star replays; star 0 to 1; all four state/RNG checkpoints per run'
 if name=='intro-skip':return ('; '.join(r['arm']+': '+str(r.get('frames_from_stage_to_play'))+' frames to play' for r in d['runs']) if d.get('build_kind')=='server-candidate' else '; '.join(d.get('no_press_play_ready',[]))+'; skip arm unavailable without host input')
 if name=='flipswap-cappy':return str(d['passing'])+'/23; failed: '+', '.join(d.get('failed_cases',[]))
 if name=='v2-cold-stall':
  rs=d['runs'];stalls=[x for r in rs for x in r['gameplay_over100']]
  return str(sum(r['passed'] for r in rs))+'/9; menu p99 '+str(min(r['menu_p99'] for r in rs))+'–'+str(max(r['menu_p99'] for r in rs))+' ms; worst menu '+str(max(float(r['worst_menu_frame']['interval_ms']) for r in rs if r['worst_menu_frame']))+' ms; gameplay stalls '+str(len(stalls))+', worst '+str(max((x[1] for x in stalls),default=0))+' ms; holds >3s '+str(sum(len(r['holds_over3s']) for r in rs))
 if name=='v2-image-pair':return 'mean RGB '+str(d['diff']['mean_abs_rgb'])+'; '+str(len(d['diff']['pairs']))+' pairs; off guard '+str(d['off_guard_lines'])
 if name=='v2-warm-pairs':return '; '.join(st+': p99 delta '+('/'.join(f"{p["p99_delta_ms"]:+.2f}" for p in v['pairs']))+' ms' for st,v in d['stages'].items())
 if name=='input-latency' and d.get('harness_fix'):return '; '.join(r['case']+': '+str(r['native_reports'])+'/27 presses, frames '+str(r['frames_after_read']) for r in d['runs'])+'; finite wall-time driver; original failed arms retained'
 if name in ('fifo-default-cadence-latency','input-latency'):return '; '.join(r['case']+': late '+str(r['late_present_pct'])+'%, '+str(r['presses'])+' presses, frames '+str(r['frames_after_read']) for r in d['runs'])
 if name=='audio-thread-priority':return '; '.join(r['case']+': '+str(r['underrun_frames'])+' underrun frames / '+str(r['underrun_reports'])+' reports, baton >50ms '+str(r['baton_over50']) for r in d['runs'])
 if name=='wii-proof':return d['reason'] if d.get('proof_kind')=='main-lineage-and-fresh-candidate-wii' else 'Release source lineage rechecked; original Wii build/oracle evidence reused'
 return json.dumps(d,ensure_ascii=False,separators=(',',':'))[:220]

def main():
 p=argparse.ArgumentParser();p.add_argument('campaign',type=Path);p.add_argument('out',type=Path);p.add_argument('--promotion-result',type=Path,help='explicit separate rerun results.json; identity and all 15 cases still checked');p.add_argument('--latency-result',type=Path,help='approved finite WALL-TIME27 driver result; original cadence evidence remains unchanged');a=p.parse_args()
 base=a.campaign.resolve();m=readj(base/'MANIFEST.json');label=m['label'];candidate_mode=m.get('build_kind')=='server-candidate';meta=m['candidate_build'] if candidate_mode else m['frozen'];expected=meta['perf_exe_sha256'];frozen=meta['build_metadata'] if candidate_mode else readj(FROZEN/'BUILD.json');gates=[];issues=[]
 assert not a.latency_result or candidate_mode
 def gate(name,paths,fn,status,limits=()):
  try:
   detail=fn();ok=detail.pop('passed') is True
  except Exception as e:ok=False;detail={'error':repr(e)}
  tier=is_tier_gate(name,candidate_mode)
  record={'gate':name,'passed':ok,'classification':'tier' if tier else 'functional','status':('TIER ' if tier else 'PASS ' if ok else 'FAIL ')+status,'required_for_functional_acceptance':not tier,'measurement_verified':'error' not in detail,'receipt':str(paths[0]),'limits':list(limits),'nice':m['nice'],'build_kind':'server-candidate' if candidate_mode else 'frozen54','source':frozen['source'],'source_exe_sha256':frozen['binary_sha256'],'perf_exe_sha256':detail.get('normal_perf_sha256',detail.get('full_star_perf_sha256',expected)),'evidence_sha256':{str(q):sha(q) for q in paths if q.is_file()},'details':detail}
  if not candidate_mode:record.update(frozen_source=frozen['source'],frozen_exe_sha256=frozen['binary_sha256'])
  else:record.update(frozen54_acceptance=False,tier_authority=m['tier_authority'] if tier else None)
  gates.append(record)
  if requires_refusal(name,ok,record['measurement_verified'],candidate_mode):issues.append(name)
 def runpath(name):return ROOT/'runs'/(label+'-'+name)
 def actual(name):
  d=runpath(name);r=readj(d/'RESULT.json');launch=readj(d/'launch.json');proc=readj(d/'process.json')
  assert launch['app_sha256']==expected and launch['nice']==0 and proc['binary_unchanged'] and proc['seed_unchanged'] and proc['no_faults'] and proc['completed'],name
  assert (d/'frames.csv').is_file() and rows(d/'frames.csv'),name
  return r,launch,proc
 platform=base/'platform/RESULT.json'
 def pcheck(keys):
  d=readj(platform);return {'passed':d['perf_exe_sha256']==expected and d['build']['source']==meta['source'] and all(d['checks'].get(k) is True for k in keys),'checks':{k:d['checks'].get(k) for k in keys},'macos':d['macos']}
 gate('macos-15-compat',[platform],lambda:pcheck(('weak-imports','scan-bundle','check-bundle','minos')),'platform compatibility')
 gate('seed-pack',[platform,base/'platform/seed-pack.json'],lambda:pcheck(('seed-pack-count','seed-integrity','resource-copy-identical')),'64 seed databases identical to selected source resources')
 promo=a.promotion_result.resolve() if a.promotion_result else runpath('promotion')/'results.json'
 def promotion():
  rs=readj(promo);want={(n,mode) for n in ('yoshi','flip','fluffy','unlocked-stage-adapter','worldmap','return') for mode in ('fresh','warm')}|{(n,'functional') for n in ('cappy-fluffy','cappy-panel','cappy-spin-after-throw')}
  ok=len(rs)==15 and {(r['case'],r['mode']) for r in rs}==want and all(r['passed'] and all(r['checks'].values()) and r['binary_sha256']==expected and r['source']==meta['source'] and r['nice']==0 for r in rs)
  if candidate_mode:
   for r in rs:
    assert r['build_kind']=='server-candidate'
    if r['case']=='worldmap':
     original=Path(r['actual_receipt']);actual_pointer=readj(original)
     assert sha(original)==r['actual_receipt_sha256'] and actual_pointer['passed']==r['passed'] and actual_pointer['checks']==r['checks'] and actual_pointer['app_sha256']==expected
     from replay_cases54m import anchor_checks,replay_accounting,identity_contract,game_pointer_coverage,pointer_observation_checks
     native=readj(original.parent/'replay/divergence.json');assert native==actual_pointer['replay']
     ok &= all(anchor_checks(native,require_identity=m.get('require_bounded_identity',False)).values()) and replay_accounting(native,m.get('require_bounded_identity',False)) and sum(s.get('pointer_rows_compared',0) for s in native['segments'] if s['stage']=='WorldMap01Galaxy')>=30
     if identity_contract(native,m.get('require_bounded_identity',False)):
      coverage=game_pointer_coverage(native)
      ok &= coverage['passed'] and actual_pointer.get('pointer_coverage')==coverage and actual_pointer.get('pointer_rows')==coverage['valid_rows_compared'] and actual_pointer.get('pointer_positions')==coverage['distinct_positions_compared']
      launch=readj(original.parent/'launch.json');process=readj(original.parent/'process.json')
      assert launch['environment'].get('PETARI_TAPE_PLAY_OBSERVE_SECONDS')=='90' and process==actual_pointer['process']
      ok &= all(pointer_observation_checks(native,process).values())
  return {'passed':ok,'cases':len(rs),'passing':sum(r['passed'] for r in rs),'failed_checks':{r['case']+'/'+r['mode']:[k for k,v in r['checks'].items() if not v] for r in rs if not r['passed']}}
 gate('gx-fresh-warm',[promo],promotion,'15-case promotion',(('PORT authorized canonical KPAD pointer tape for the two world-map cases; physical host mapping is outside this check.',) if candidate_mode else ('Reference worldmap host-pointer evidence remains required; unavailable on frozen54 without host input.',)))
 tape=base/'tape-status.json'
 if candidate_mode:
  from candidate_evidence54m import normal_evidence
  def normalcheck():
   result=normal_evidence(m['normal_results'],meta['candidate_directory'],m.get('require_bounded_identity',False));assert result==readj(tape);return result
  gate('normal-route-grandmap',[tape]+[Path(q) for q in m['normal_results']],normalcheck,'three whole-session normal-route acceptance runs')
  def bootcheck():
   result=normalcheck();boot=readj(base/'candidate-normal-boot-ui.json');assert boot=={**result,'gate':'normal-boot-ui','frozen54_acceptance':False};return result
  gate('normal-boot-ui',[base/'candidate-normal-boot-ui.json'],bootcheck,'ordinary title/FileSelect prefix from canonical normal tape')
  if m.get('full_star_results') or m.get('require_bounded_identity'):
   def fullstarcheck():
    from candidate_evidence54m import full_star_evidence
    value=full_star_evidence(m['full_star_results'],meta['candidate_directory']);assert value==readj(base/'full-star-status.json');return value
   gate('full-star-flipswap',[base/'full-star-status.json']+[Path(q) for q in m.get('full_star_results') or []],fullstarcheck,'three Flip-Swap Star replays from fresh private users',('Each original tape_prefix retains the native min(declared seams, complete CSV rows) accounting; no input tape mutation.',))
 else:gate('normal-route-grandmap',[tape],lambda:{'passed':False,**readj(tape)},'whole normal route on frozen54',('Candidate tape runs are separate and cannot prove frozen54 whole-session replay.',))
 intro=runpath('intro')/'RESULT.json'
 def introcheck():
  d=readj(intro)
  if candidate_mode:
   from replay_cases54m import anchor_checks
   assert d['build_kind']=='server-candidate' and d['build']['source']==meta['source'] and d['build']['perf_exe_sha256']==expected and len(d['runs'])==2
   assert d['passed']==all(d['checks'].values())
   native_checks=[]
   for r in d['runs']:
    from replay_cases54m import replay_accounting
    assert r['app_sha256']==expected
    assert readj(intro.parent/r['arm']/'replay/divergence.json')==r['replay']
    native_checks.append(r['passed'] is True and all(r['checks'].values()) and all(anchor_checks(r['replay'],'RedBlueExGalaxy',m.get('require_bounded_identity',False)).values()) and replay_accounting(r['replay'],m.get('require_bounded_identity',False)))
   assert {r['arm'] for r in d['runs']}=={'none','skip'}
   arms={r['arm']:r for r in d['runs']};skipped=arms['skip']['frames_from_stage_to_play'];unskipped=arms['none']['frames_from_stage_to_play']
   d['collector_checks']={'native_reports_passed':all(native_checks),'skip_earlier_rechecked':type(skipped) is int and type(unskipped) is int and 0<=skipped<unskipped}
   d['passed']=d['passed'] is True and all(d['collector_checks'].values())
  return d
 gate('intro-skip',[intro],introcheck,'intro skip without host input')
 cappy=runpath('cappy')/'receipt.json'
 def cappycheck():
  d=readj(cappy)
  if candidate_mode:
   launch=readj(cappy.parent/'launch-m1.json');assert launch['build_kind']=='server-candidate' and launch['source']==meta['source'] and launch['app_sha256']==expected
  return {'passed':d['passed']==23 and d['failed']==0 and len(d['results'])==23 and all(r['passed'] for r in d['results']) and d['app_sha256']==expected,'passing':d['passed'],'failing':d['failed'],'failed_cases':[r['name'] for r in d['results'] if not r['passed']]}
 gate('flipswap-cappy',[cappy],cappycheck,'23 Cappy cases')
 cold=[runpath(f'cold-{st}-r{n}') for st in ('YosshiHomeGalaxy','MokumokuValleyGalaxy','MarioFaceShipGalaxy') for n in (1,2,3)]
 def coldcheck():
  rs=[]
  for d in cold:
   r,launch,proc=actual(d.name[len(label)+1:]);boot=readj(d/'boot-screen.json')
   assert launch['cold_metal_cleared'] and launch['warm_from'] is None
   assert r['gameplay_frames']>0 and r['menu_frames']>0
   rs.append({'case':d.name,'passed':r['passed'] and r['result']=='PASS' and r['gameplay_frames']>0 and r['menu_frames']>0 and r['menu_p99']<=25 and not r['gameplay_over100'] and not r['holds_over3s'],'menu_p99':r['menu_p99'],'gameplay_over100':r['gameplay_over100'],'holds_over3s':r['holds_over3s'],'worst_menu_frame':boot['worst_menu_frame']})
  return {'passed':len(rs)==9 and all(r['passed'] for r in rs),'runs':rs}
 gate('v2-cold-stall',[d/'RESULT.json' for d in cold]+[d/'boot-screen.json' for d in cold],coldcheck,'nine true-cold stalls and boot-screen worst frames')
 def imagecheck():
  d=readj(base/'image-diff.out');guard=readj(base/'image-offguard.json')
  for mode in ('0','all'):actual('image-'+mode)
  return {'passed':d['result']=='PASS' and bool(d['pairs']) and d['mean_abs_rgb']<.01 and guard['uber_lines']==1,'diff':d,'off_guard_lines':guard['uber_lines']}
 gate('v2-image-pair',[base/'image-diff.out',base/'image-offguard.json'],imagecheck,'mean absolute RGB <0.01')
 def warmcheck():
  data=readj(base/'warm-pairs.out');assert set(data)=={'YosshiHomeGalaxy','MokumokuValleyGalaxy'}
  for st in data:
   for n in (1,2,3):
    for arm in ('hybrid','off'):
     r,l,_=actual(f'wp-{st}-r{n}-{arm}');assert l['warm_from'] and l['environment']['PETARI_PIPELINE_DIAG']=='0' and l['environment']['PETARI_UBERSHADER']==('0' if arm=='off' else '1')
  ok=all(len(s['pairs'])==3 and not s['hybrid_p99_reliably_worse'] and all(p[a]['frames']>0 for p in s['pairs'] for a in ('hybrid','off')) for s in data.values())
  return {'passed':ok,'stages':data}
 gate('v2-warm-pairs',[base/'warm-pairs.out'],warmcheck,'three alternating warm hybrid/off pairs')
 def cadencecheck():
  data=[]
  for n in (1,2):
   for mode in ('fifo','immediate'):
    name=f'cadence-{mode}-r{n}';r,l,_=actual(name);warm_path=runpath(name)/'WARM-GATE.json';warm_result=readj(warm_path) if warm_path.exists() else r;text=(runpath(name)/'app.log').read_text(errors='replace');lat=latency(text)
    modeok='present mode Fifo' in text and 'PETARI_PRESENT_MODE' not in l['environment'] if mode=='fifo' else 'present mode Immediate' in text
    # Two Fifo arms must pass stall/menu/cadence thresholds; Immediate is a latency comparison.
    ok=bool(lat) and len(lat)==27 and all(fr==2 and math.isfinite(ms) for fr,ms in lat) and modeok and (mode!='fifo' or warm_result['result']=='PASS' and r['late_present_pct']<1)
    data.append({'case':name,'passed':ok,'latency_passed':len(lat)==27 and all(fr==2 and math.isfinite(ms) for fr,ms in lat) and modeok,'warm_assessment':str(warm_path) if warm_path.exists() else str(runpath(name)/'RESULT.json'),'late_present_pct':r['late_present_pct'],'presses':len(lat),'frames_after_read':sorted(set(fr for fr,_ in lat)),'latency_p50_ms':sorted(ms for _,ms in lat)[len(lat)//2] if lat else None,'latency_max_ms':max((ms for _,ms in lat),default=None),'display_policy_lines':[line for line in text.splitlines() if '[display]' in line and 'vsync' in line],'present_summary':[line for line in text.splitlines() if 'Petari present intervals' in line]})
  return {'passed':all(d['passed'] for d in data),'runs':data}
 gate('fifo-default-cadence-latency',[runpath(f'cadence-{mode}-r{n}')/'RESULT.json' for n in (1,2) for mode in ('fifo','immediate')],cadencecheck,'Fifo late <1%, 27/27 latency presses at 2 frames after read')
 if candidate_mode:
  def inputlatency():
   if a.latency_result:
    from latency54m import validate_result
    return validate_result(a.latency_result,meta)
   result=cadencecheck();return {'passed':all(r['latency_passed'] for r in result['runs']),'runs':result['runs']}
  gate('input-latency',[a.latency_result] if a.latency_result else [runpath(f'cadence-{mode}-r{n}')/'app.log' for n in (1,2) for mode in ('fifo','immediate')],inputlatency,'27/27 input latency presses at two frames after read; cadence threshold is a separate tier row')
 def audiocheck():
  data=[]
  for mode in ('cold','warm'):
   name='audio-'+mode;r,l,_=actual(name);text=(runpath(name)/'app.log').read_text(errors='replace');tsv=base/('thr-'+mode+'.tsv')
   item=audio_observation(text,len(tsv.read_text().splitlines()));item['case']=name
   item['passed'] &= l['environment']['SDL_AUDIO_DRIVER']=='coreaudio'
   data.append(item)
  return {'passed':all(d['passed'] for d in data),'runs':data}
 gate('audio-thread-priority',[runpath('audio-'+mode)/'RESULT.json' for mode in ('cold','warm')]+[base/('thr-'+mode+'.tsv') for mode in ('cold','warm')],audiocheck,'real CoreAudio thread probe, zero underruns')
 if candidate_mode:
  old=pcheck(('wii-source-unchanged',))
  gates.append({'gate':'wii-old-baseline-lineage','passed':old['passed'],'classification':'source-comparison','status':('PASS ' if old['passed'] else 'FAIL ')+'unchanged since a23609ca (preserved source comparison)','required_for_functional_acceptance':False,'measurement_verified':True,'receipt':str(platform),'build_kind':'server-candidate','source':meta['source'],'source_exe_sha256':meta['source_exe_sha256'],'perf_exe_sha256':expected,'nice':m['nice'],'limits':['PORT authorized main71353 lineage plus fresh candidate Wii/hash/oracle as the current functional proof; this older-source comparison remains unchanged.'],'evidence_sha256':{str(platform):sha(platform),str(base/'platform/wii-source-diff.log'):sha(base/'platform/wii-source-diff.log')},'details':old})
  from candidate_wii54m import validate as validate_wii
  gate('wii-proof',[base/'candidate-wii-proof.json'],lambda:validate_wii(base/'candidate-wii-proof.json',meta['candidate_directory']),'main71353 lineage plus fresh candidate Wii/hash and strict dispatch')
 else:gate('wii-proof',[platform,base/'platform/wii-source-diff.log'],lambda:pcheck(('wii-source-unchanged',)),'release source has no Wii changes relative to a23609ca',('Original Wii build/hash/oracle evidence belongs to release; this M1 run rechecks source lineage only.',))
 identity=sha(Path(meta['source_app'])/'Contents/MacOS/Petari')==frozen['binary_sha256'] and meta['source_exe_sha256']==frozen['binary_sha256'] and m['nice']==0
 if candidate_mode:
  identity &= sha(Path(m['app'])/'Contents/MacOS/Petari')==expected
  try:
   from candidate_evidence54m import protected_inputs
   protection=readj(base/'protected-inputs.json');before=Path(protection['before']);after=Path(protection['after'])
   assert protection['passed'] is True and sha(before)==protection['before_sha256'] and sha(after)==protection['after_sha256']
   assert readj(before)==readj(after)==protected_inputs(meta['candidate_directory'])
  except Exception as e:identity=False;issues.append('protected inputs: '+repr(e))
 if not identity:issues.append('selected build identity/priority')
 interrupted=readj(base/'INTERRUPTED.json') if (base/'INTERRUPTED.json').is_file() else None
 validation={'build_kind':'server-candidate' if candidate_mode else 'frozen54','campaign_interruption':interrupted,'frozen54_acceptance':not candidate_mode and not issues,'promotion_evidence_override':str(a.promotion_result) if a.promotion_result else None,'input_latency_evidence_override':str(a.latency_result) if a.latency_result else None,'passed':not issues,'failed_gates':issues,'tier_threshold_failures':[g['gate'] for g in gates if g['classification']=='tier' and not g['passed']],'gates':gates,'identity_verified':identity,'nice':0,'receipts_written':False}
 write(base/'VALIDATION.json',validation)
 lines=[('M1 server candidate '+frozen['source'] if candidate_mode else 'M1 frozen 5.4')+' versus server (actual evidence; nice 0 on M1)','',f"Source exe {frozen['binary_sha256']}; re-signed perfm1a exe {expected}.",'M1 Max / macOS 15.1 / built-in 120 Hz ProMotion. Panel cadence is recorded per run.','Server promotion/normal/intro/Cappy used nice 19; PORT approved nice 0 on M1 because the shared harness is load-sensitive.','', '| Gate | Server | M1 | Comparison |','| --- | --- | --- | --- |']
 if interrupted:lines[0:0]=['Superseded partial campaign: '+interrupted['authority']+'. Remaining measurements were not run; see INTERRUPTED.json.','']
 for g in gates:
  detail=summary(g)
  lines.append('| '+g['gate']+' | '+SERVER.get(g['gate'],'Derived functional check')+' | '+g['status']+': '+detail.replace('|','\\|').replace('\n',' ')+' | '+('Preserved older-source comparison' if g['classification']=='source-comparison' else 'M1 tier observation' if g['classification']=='tier' else 'Same acceptance met' if g['passed'] else 'Different: failed or unavailable')+' |')
 if a.promotion_result:lines+=['',f'Promotion evidence explicitly re-run after the initial harness table-parser failure: {a.promotion_result}. The initial campaign manifest/log remains unchanged.']
 if a.latency_result:lines+=['',f'PORT-approved latency harness fix: {a.latency_result}. Finite27 independent WALL-TIME Space presses with absolute send/release/read ledger; movement script Space removed, original host random gap distribution and60ms holds retained. Predicate remains exactly27/27 at2frames in all4arms. Original26/25/25/25 arms, cadence tier observations and initial refusal are preserved; see VALIDATION-initial-input-latency-failure.json and LATENCY-07223918-EXISTING-LOG-FINDING.json. Frozen5.4 also showed25/26; the collision predates5.4.1.']
 lines+=['','Each GX session holds heavy then native.lock. Only the owned perfm1a Metal cache is cleared. Private pipeline caches, user copies and raw captures are deleted after their dependent measurements; CSV/log summaries remain.','Candidate evidence is labelled separately; it does not change frozen54 gate status.']
 if candidate_mode:lines+=['The normal-route and normal-boot-ui rows reuse SOL B\'s actual perfm1b runs; their signed hashes are recorded separately and bound to the same original server candidate. Cold and cadence thresholds are tier observations; input latency remains functional.']
 (base/'COMPARISON.md').write_text('\n'.join(lines)+'\n')
 if issues:
  print('receipts54m: REFUSED; no receipt directory written: '+', '.join(issues));return 1
 if a.out.exists():raise SystemExit('refusing existing receipt directory '+str(a.out))
 a.out.mkdir(parents=True)
 for g in gates:write(a.out/(g['gate']+'.json'),g)
 validation['receipts_written']=True;write(base/'VALIDATION.json',validation)
 print(f'receipts54m: {len(gates)} verified gates written to {a.out}');return 0
if __name__=='__main__':sys.exit(main())
