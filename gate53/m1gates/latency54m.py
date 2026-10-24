#!/usr/bin/env python3
"""Approved finite WALL-TIME latency harness on an unchanged candidate perf copy."""
import argparse,json,math,re,subprocess,sys
from pathlib import Path
from common import ROOT,HERE,REPO,prep_candidate,native_lock,wait_quiet,run,sha,write,trim

SYMBOLS={
 'key':'__ZN12PetariNative5Input8keyEventEtbb',
 'quit':'__ZN12PetariNative3App4Host11requestQuitEv',
 'gameplay':'__ZN12PetariNative3App13LatencyReport12_GLOBAL__N_19gGameplayE.0',
 'stage':'__ZN12PetariNative12LatencyProbe5stageE',
 **{name:'__ZN12PetariNative12LatencyProbe'+str(len(name+'Ns'))+name+'NsE' for name in ('press','report','read','copy','shown','present')},
 'read_frame':'__ZN12PetariNative12LatencyProbe9readFrameE',
 'shown_frame':'__ZN12PetariNative12LatencyProbe10shownFrameE',
}

def script_without_space(source):
 lines=[];before=[];after=[]
 for line in source.splitlines():
  words=line.split('#')[0].split()
  if not words:continue
  count=int(words[0]);assert count>0
  keys=words[1:];new=[k for k in keys if k!='Space']
  assert not any(k in ('Return','Escape') for k in new),'another A/Start stimulus'
  before.extend([keys]*count);after.extend([new]*count)
  lines.append(' '.join([str(count),*new]))
 assert len(before)==len(after)==970 and sum('Space' in x for x in before)==40
 assert all(b==[k for k in a if k!='Space'] for a,b in zip(before,after))
 return '# Original movement timeline; only Space removed to avoid concurrent latency-key stimuli.\n'+'\n'.join(lines)+'\n'

def ledger_checks(events,text):
 checks={};checks['no_driver_errors_or_busy_skips']=not any(e['event'] in ('error','skip_busy') for e in events)
 starts=[e for e in events if e['event']=='start'];quits=[e for e in events if e['event']=='quit']
 sends=[e for e in events if e['event']=='send'];releases=[e for e in events if e['event']=='release'];matches=[e for e in events if e['event']=='match']
 checks['finite27_wall_time_config']=len(starts)==1 and all(starts[0].get(k)==v for k,v in {'clock':'CLOCK_UPTIME_RAW','planned_presses':27,'seed':12345,'gap_min_ms':1000,'gap_max_ms':1600,'hold_ms':60,'symbol_exports_verified':True}.items())
 ids=list(range(1,28))
 checks['exact27_sends_releases_matches']=all([e['id'] for e in es]==ids for es in (sends,releases,matches))
 checks['no_skipped_in_window']=not any(e['event']=='skip_not_gameplay' and e['id']!=1 for e in events)
 checks['event_order']=[e['event'] for e in events if e['event']!='skip_not_gameplay']==['start']+['send','release','match']*27+['quit']
 checks['absolute_wall_schedule']=len(sends)==27 and all(1000<=e['gap_ms']<=1600 and e['ns']>=e['planned_ns'] for e in sends) and all(b['planned_ns']-a['planned_ns']==60_000_000+b['gap_ms']*1_000_000 for a,b in zip(sends,sends[1:]))
 checks['send_release_read_correspondence']=len(sends)==len(releases)==len(matches)==27 and all(
  s['ns']<=s['native_press_ns']<=s['return_ns']<=r['ns']<=r['return_ns']<=m['ns'] and
  r['ns']-s['return_ns']>=60_000_000 and m['matched'] is True and m['stage']==0 and
  m['native_press_ns']==s['native_press_ns'] and m['native_press_ns']<=m['report_ns']<=m['read_ns']<=m['copy_ns']<=m['shown_ns']<=m['present_ns']<=m['ns']
  for s,r,m in zip(sends,releases,matches))
 reports=[(int(a),int(b),float(c)) for a,b,c in re.findall(r'\[latency\] press:.*game read [\d.]+ ms \(frame (\d+)\).*\(frame \d+, (\d+) frames after the read\).*on screen ([\d.-]+) ms',text)]
 checks['native27_at2frames']=len(reports)==27 and all(delta==2 and math.isfinite(ms) for _,delta,ms in reports)
 checks['ledger_matches_native_reports']=len(matches)==len(reports)==27 and all(m['read_frame']==frame and m['shown_frame']==frame+2 for m,(frame,_,_) in zip(matches,reports))
 checks['no_explicit_dropped_presses']='[latency] press dropped' not in text
 checks['driver_finished_before_quit']=len(quits)==1 and quits[0]['sent']==27 and quits[0]['failed'] is False and quits[0]['drain_ms']==250 and len(matches)==27 and quits[0]['ns']-matches[-1]['ns']>=250_000_000
 unmatched=[s['id'] for s in sends if not any(m['id']==s['id'] and m.get('matched') is True and m.get('read_ns',0)>=s['native_press_ns'] for m in matches)]
 return {'passed':all(checks.values()),'checks':checks,'unmatched_send_ids':unmatched,'sent':len(sends),'native_reports':len(reports),'frames_after_read':sorted(set(b for _,b,_ in reports)),'read_frames':[a for a,_,_ in reports],'send_ns':[e['ns'] for e in sends],'release_ns':[e['ns'] for e in releases],'quit_ns':quits[0]['ns'] if len(quits)==1 else None}

def inspect_arm(path):
 path=Path(path);launch=json.loads((path/'launch.json').read_text());proc=json.loads((path/'process.json').read_text());text=(path/'app.log').read_text(errors='replace')
 events=[json.loads(l) for l in (path/'driver.jsonl').read_text().splitlines()]
 result=ledger_checks(events,text)
 result['checks']['healthy_process']=all(proc[k] for k in ('completed','no_faults','binary_unchanged','seed_unchanged')) and not proc['timed_out']
 env=launch['environment'];result['checks']['only_finite_wall_driver']=env['PETARI_LATENCY_TEST']=='0' and env['PETARI_INPUT_SCRIPT_QUIT']=='0' and env['PETARI_LATENCY_PROBE']=='1' and env['PETARI_PIPELINE_DIAG']=='0'
 result['checks']['normal_priority_warm_private_cache']=launch['nice']==0 and bool(launch['warm_from']) and not launch['cold_metal_cleared']
 mode='immediate' if 'immediate' in path.name else 'fifo'
 result['checks']['present_mode']=('present mode Immediate' in text and env.get('PETARI_PRESENT_MODE')=='immediate') if mode=='immediate' else ('present mode Fifo' in text and 'PETARI_PRESENT_MODE' not in env and '[display] vsync auto: starting with VSync (Fifo)' in text)
 result['passed']=all(result['checks'].values());result['case']=path.name;result['path']=str(path)
 result['display_policy_lines']=[l for l in text.splitlines() if '[display]' in l and 'vsync' in l]
 result['evidence_sha256']={str(path/n):sha(path/n) for n in ('driver.jsonl','app.log','launch.json','process.json','frames.csv','route.kpd','route.tape.json','input.csv','quiet.json')}
 return result

def validate_result(path,meta):
 path=Path(path);d=json.loads(path.read_text());assert d['passed'] is True and d['build_kind']=='server-candidate'
 assert d['source']==meta['source'] and d['perf_exe_sha256']==meta['perf_exe_sha256'] and d['source_exe_sha256']==meta['source_exe_sha256']
 for p,h in d['files_sha256'].items():assert sha(p)==h,p
 assert sha(d['app_executable'])==meta['perf_exe_sha256']
 script=Path(d['movement_script']);assert script.read_text()==script_without_space((REPO/'native/tools/perf_sweep_move.script').read_text())
 assert d['prior_finding_sha256']==sha(d['prior_finding']) and d['prior_failed_arms_retained'] is True
 finding=json.loads(Path(d['prior_finding']).read_text());assert finding['counts']==[26,25,25,25]
 for arm in finding['arms']:
  for p,h in arm['evidence_sha256'].items():assert sha(p)==h,p
 results=[]
 for old in d['runs']:
  actual=inspect_arm(old['path']);assert actual==old and actual['passed'] is True
  launch=json.loads((Path(old['path'])/'launch.json').read_text());assert launch['app_sha256']==meta['perf_exe_sha256']
  assert launch['warm_from']==d['prime']
  for k,v in {'PETARI_M1_LATENCY_EXE':d['app_executable'],'PETARI_M1_LATENCY_SYMBOLS':d['symbol_map'],'DYLD_INSERT_LIBRARIES':d['driver_dylib'],'PETARI_INPUT_SCRIPT':d['movement_script']}.items():assert launch['environment'][k]==v
  assert launch['environment']['PETARI_M1_LATENCY_LEDGER']==str(Path(old['path'])/'driver.jsonl')
  results.append(actual)
 assert len(results)==4 and {x['case'].rsplit('-',2)[-2]+'-'+x['case'].rsplit('-',1)[-1] for x in results}=={'fifo-r1','immediate-r1','fifo-r2','immediate-r2'}
 return {'passed':True,'runs':results,'harness_fix':'Finite27 independent wall-time driver, absolute send/release ledger, no scripted Space collision; original failing arms retained.','prior_finding':d['prior_finding'],'prior_finding_sha256':d['prior_finding_sha256'],'receipt':str(path),'receipt_sha256':sha(path)}

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('label');p.add_argument('--candidate-dir',type=Path,required=True);p.add_argument('--prior-campaign',type=Path,required=True);a=p.parse_args();assert re.fullmatch('[A-Za-z0-9_-]+',a.label)
 app,meta=prep_candidate(a.candidate_dir);exe=app/'Contents/MacOS/Petari';base=ROOT/'campaigns'/a.label;base.mkdir(parents=True,exist_ok=False)
 movement=base/'move-no-space.script';movement.write_text(script_without_space((REPO/'native/tools/perf_sweep_move.script').read_text()))
 nm=subprocess.check_output(['nm','-an',str(exe)],text=True);(base/'nm.txt').write_text(nm)
 symbols={m[2]:int(m[1],16) for m in re.finditer(r'^([0-9a-f]{16}) [TDb] (\S+)$',nm,re.M)}
 symbol_map=base/'symbols.map';symbol_map.write_text(''.join(k+' '+format(symbols[v],'x')+'\n' for k,v in SYMBOLS.items()))
 dylib=base/'latency-driver.dylib';build=['clang++','-dynamiclib','-std=c++20','-O2',str(HERE/'latency_driver.cpp'),'-o',str(dylib)]
 with native_lock():
  wait_quiet(base/'build-quiet.json');r=subprocess.run(build,capture_output=True,text=True);(base/'build.log').write_text(r.stdout+r.stderr);assert r.returncode==0
 finding=ROOT/'LATENCY-07223918-EXISTING-LOG-FINDING.json';assert finding.is_file()
 old=[a.prior_campaign/'VALIDATION.json',a.prior_campaign/'COMPARISON.md']
 for path in old:
  dest=path.with_name(path.stem+'-initial-input-latency-failure'+path.suffix)
  assert not dest.exists();dest.write_bytes(path.read_bytes())
 prior=json.loads((a.prior_campaign/'VALIDATION.json').read_text());assert prior['failed_gates']==['input-latency']
 write(base/'command.json',{'argv':sys.argv,'build':build,'app':str(app),'source':meta['source'],'build_kind':'server-candidate','harness_fix_authority':'PORT: finite27 WALL-TIME driver and ledger approved; original27/27-at2frames unchanged','no_native_executable_changes':True})
 prime=ROOT/'runs'/(a.label+'-prime');proc=run(app,prime,stage='YosshiHomeGalaxy',script=movement,cold=True);assert proc['completed'] and proc['no_faults']
 results=[]
 for n in (1,2):
  for mode in ('fifo','immediate'):
   out=ROOT/'runs'/f'{a.label}-{mode}-r{n}';env={'PETARI_INPUT_SCRIPT_QUIT':'0','PETARI_LATENCY_TEST':'0','PETARI_LATENCY_PROBE':'1','PETARI_PIPELINE_DIAG':'0','DYLD_INSERT_LIBRARIES':str(dylib),'PETARI_M1_LATENCY_EXE':str(exe),'PETARI_M1_LATENCY_SYMBOLS':str(symbol_map),'PETARI_M1_LATENCY_LEDGER':str(out/'driver.jsonl')}
   if mode=='immediate':env['PETARI_PRESENT_MODE']='immediate'
   print('START',mode,n,flush=True);run(app,out,stage='YosshiHomeGalaxy',script=movement,warm=prime,extra=env,seconds=180)
   actual=inspect_arm(out);results.append(actual);write(out/'LATENCY.json',actual);trim(out)
   result={'passed':len(results)==4 and all(x['passed'] for x in results),'build_kind':'server-candidate','source':meta['source'],'perf_exe_sha256':meta['perf_exe_sha256'],'source_exe_sha256':meta['source_exe_sha256'],'app_executable':str(exe),'prime':str(prime),'movement_script':str(movement),'symbol_map':str(symbol_map),'driver_dylib':str(dylib),'prior_finding':str(finding),'prior_finding_sha256':sha(finding),'prior_failed_arms_retained':True,'runs':results,'files_sha256':{str(q):sha(q) for q in [movement,symbol_map,dylib,HERE/'latency_driver.cpp',HERE/'latency54m.py',base/'nm.txt',prime/'launch.json',prime/'process.json',*old,*[q.with_name(q.stem+'-initial-input-latency-failure'+q.suffix) for q in old]]}}
   # Bind immutable original arm logs; VALIDATION/COMPARISON can be regenerated by the collector.
   for q in old:result['files_sha256'].pop(str(q))
   write(base/'RESULT.json',result);print('END',mode,n,'PASS' if actual['passed'] else 'FAIL',actual['sent'],actual['native_reports'],actual['unmatched_send_ids'],flush=True)
   if not actual['passed']:trim(prime);return 1
 trim(prime);return 0

if __name__=='__main__':sys.exit(main())
