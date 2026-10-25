#!/usr/bin/env python3
"""Native schema2 producer. No legacy input, OS input injection, or receipt editing."""
import csv
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import time
from replay_tape import parse_kpd,parse_actions
from tape_phase_audit import need,validate,power_events,has_control,POWER_PROTOCOL,bootstrap,payload

POLICY='native-neutral-controls-v1'
LEGACY_POLICY='single-bootstrap-v1'


def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def audit_tail(meta,reads,policy=LEGACY_POLICY):
    events=power_events(meta)
    need(len(events)==2,'tail needs explicit begin and power')
    first,last=events[0]['before_input_frame'],events[1]['after_route_row']
    selected=[r for r in reads if first<=r['frame']<=last]
    need(selected,'empty neutral tail')
    if policy==LEGACY_POLICY:
        for read in selected:need(payload(read)==payload(bootstrap(read['channel'],read['capacity'])),'non-bootstrap payload in legacy neutral tail')
        return dict(policy=policy,reads=len(selected))
    need(policy==POLICY,'unknown neutral-tail policy')
    samples_by_channel={};connections={};samples=0
    for read in selected:
        channel=read['channel'];connection=tuple(read[k] for k in ('probe','device','error','capacity'))
        need(channel not in connections or connections[channel]==connection,'native tail connection/control envelope changed')
        connections[channel]=connection
        for sample in read['samples']:
            need(len(sample)==240 and not has_control(sample),'native tail has button/stick/pointer control')
            def zero(at):return struct.unpack_from('>I',sample,at)[0]&0x7fffffff==0
            need(zero(28) and all(zero(at) for at in (176,180,184)),'native tail has accel/gyro event')
            if sample[92] in (1,6):need(zero(120),'native extension accel event')
            need(channel not in samples_by_channel or sample==samples_by_channel[channel],
                 'native tail sample changed from first recorded rest sample')
            samples_by_channel[channel]=sample;samples+=1
    need(set(samples_by_channel)==set(connections),'native tail channel missing retained sample')
    return dict(policy=policy,reads=len(selected),samples=samples,first_sample_sha256={str(c):hashlib.sha256(v).hexdigest() for c,v in samples_by_channel.items()},
        sample_exemptions=[],advancing_read_envelope_fields=['frame','ordinal','returned','sample_count'],connections={str(c):list(v) for c,v in connections.items()})


def audit_recording(out):
    from capture_schema2_tape import PRODUCTION_PIPELINE_ENV
    from tape_phase_audit import load_meta
    out=Path(out);meta=load_meta(out/'route.tape.json');launch=json.loads((out/'launch.json').read_text());process=json.loads((out/'process.json').read_text())
    need(launch.get('child_nice_at_spawn')==0,'native producer child not nice0')
    need(launch.get('kind')=='native-script-recording' and launch.get('neutral_tail_policy')==POLICY,'native origin/policy binding missing')
    need(launch.get('neutral_tail_policy_sha256')==sha(Path(__file__).with_name('native-neutral-controls-v1.json')),'native policy file binding differs')
    env=launch['environment'];need(not any(k.startswith('PETARI_TAPE_PLAY') for k in env),'native recording must not replay a source')
    need(all(env.get(k)==v for k,v in PRODUCTION_PIPELINE_ENV.items()),'native producer diagnostics differ')
    need(env.get('PETARI_SMOKE_IGNORE_PHYSICAL')=='1' and env.get('PETARI_INPUT_SCRIPT_RECORD_TAIL_SECONDS')=='180','native isolation/tail settings missing')
    need(process['exit']==0 and process['timed_out'] is False and process['seconds']>=90,'native recording did not exit normally after unchanged wall gate')
    need(meta.get('complete') is True and meta.get('session_control_protocol')==POWER_PROTOCOL and meta.get('marker_error')=='' and meta.get('session_control_error')=='','native recording incomplete/rejected')
    need(meta.get('lossy_samples')==meta.get('unencodable_samples')==0,'native recording lossy')
    frames,reads=parse_kpd((out/'route.kpd').read_bytes());rows=list(csv.DictReader((out/'route.csv').open()))
    need(frames==meta['seams']+1 and len(rows)==meta['seams'] and meta['kpd_reads']==len(reads) and meta['kpd_samples']==sum(len(r['samples']) for r in reads),'native KPD/CSV counters disagree')
    phase=validate(meta,rows,reads,parse_actions((out/'route.actions').read_text()))
    events=power_events(meta);need(len(events)==2 and events[-1]['neutral_elapsed_seconds']>=180,'native tail shorter than180 wall seconds')
    need(meta['seams']>events[-1]['before_input_frame'],'native power has no strict shutdown rows')
    rest=audit_tail(meta,reads,POLICY)
    script=out/'input.script';need(sha(script)==launch['script_sha256'],'native script changed')
    log=(out/'app.log').read_text(errors='replace');need('[input-script] done after 1450 frames' in log and '[input-script] stopped' not in log,'native script did not finish all input')
    for n,x,y in ((150,400,300),(258,900,450),(365,700,600)):
        need(f'[input-script pointer] map_valid_rows={n} x={x} y={y} off=0' in log,'missing native pointer schedule event')
    pointer_rows=set();positions=set()
    for read in reads:
        if read['channel']!=0 or read['returned']<=0 or not read['samples']:continue
        sample=read['samples'][0]
        if 0<sample[94]<128:pointer_rows.add(read['frame']);positions.add(sample[32:40])
    need(len(pointer_rows)>=30 and len(positions)>=3,'native actual SDK pointer coverage missing')
    chain=[]
    for row in rows:
        if row['stage'] and (not chain or chain[-1]!=row['stage']):chain.append(row['stage'])
    need('WorldMap01Galaxy' in chain,'native producer did not reach target map')
    filenames=['route.csv','route.kpd','route.tape.json','route.actions','input.script','input.csv','launch.json','process.json','app.log']
    return dict(passed=True,source_kind='native-input-script-v1',captured_schema=2,actual_payloads_independently_checked=True,
        neutral_tail_policy=POLICY,tail=rest,phases=phase,actual_reads=len(reads),actual_samples=sum(len(r['samples']) for r in reads),
        pointer_valid_rows=len(pointer_rows),pointer_distinct_positions=len(positions),session_events=events,scene_chain=chain,
        shutdown_rows=meta['seams']-events[-1]['before_input_frame'],hashes={**{str(out/n):sha(out/n) for n in filenames},str(Path(__file__).with_name('native-neutral-controls-v1.json')):sha(Path(__file__).with_name('native-neutral-controls-v1.json'))},
        replay_equality_claimed=False,scope='Literal native recording; every retained SDK byte becomes canonical. Strict replay is a separate required matrix.')


def qualify_origin(out):
    out=Path(out);result=json.loads((out/'RESULT.json').read_text())
    need(result.get('kind')=='native-script-recording' and result.get('passed') is True and result.get('checks') and all(result['checks'].values()),'native origin failed admission')
    proof=audit_recording(out)
    need(proof==result.get('native_recording_proof'),'native origin audit/bytes changed')
    return dict(path=str(out),result_sha256=sha(out/'RESULT.json'),policy=POLICY,hashes=proof['hashes'])


def record(a):
    from capture_schema2_tape import inventory,tree,PRODUCTION_PIPELINE_ENV
    out=a.out.resolve();app=a.app.resolve();need(a.allowed_root.resolve() in out.parents and not out.exists(),'fresh owned native output required')
    need(os.getpriority(os.PRIO_PROCESS,0)==0 and a.post_roll_seconds==180 and a.neutral_tail_policy==POLICY,'native producer nice/tail/policy invalid')
    need(a.stage=='WorldMap01Galaxy' and a.scenario==1 and a.user_seed and a.fixture_receipt,'native map requires exact fixture/launch')
    prep=json.loads(a.prep.read_text());exe=app/'Contents/MacOS/Petari';need(sha(exe)==prep['perf_sha256'],'native exe/PREP differs')
    fixture=json.loads(a.fixture_receipt.read_text());expected={k:fixture[k] for k in ('files','xattrs')};need(inventory(a.user_seed)==expected,'native fixture differs')
    before_app=tree(app);before_seed=inventory(a.user_seed)
    out.mkdir(parents=True,mode=0o700);shutil.copy2(a.native_script,out/'input.script')
    user=out/'user';subprocess.run(['/usr/bin/ditto','--rsrc','--extattr',str(a.user_seed),str(user)],check=True);need(inventory(user)==expected,'native copied fixture differs')
    env={k:v for k,v in os.environ.items() if not k.startswith(('PETARI_','ASTRAL_','DYLD_','THRPROBE_'))}
    env.update(ASTRAL_TAPE='off',PETARI_MODS='OdysseyJumpDive',SDL_AUDIO_DRIVER='dummy',PETARI_SMOKE_IGNORE_PHYSICAL='1',
        PETARI_CACHE_DIR=str(out/'cache'),PETARI_FRAME_CSV=str(out/'frames.csv'),PETARI_FRAME_STATS='1',PETARI_TAPE_RECORD=str(out),
        PETARI_INPUT_SCRIPT=str(out/'input.script'),PETARI_INPUT_SCRIPT_LOG=str(out/'input.csv'),PETARI_INPUT_SCRIPT_RECORD_TAIL_SECONDS='180',**PRODUCTION_PIPELINE_ENV)
    command=[str(exe),'--disc',str(a.disc.resolve()),'--user',str(user),'--stage',a.stage,'--scenario',str(a.scenario)]
    launch=dict(kind='native-script-recording',neutral_tail_policy=POLICY,command=command,environment={k:v for k,v in env.items() if k.startswith(('PETARI_','ASTRAL_','SDL_AUDIO'))},
        exe_sha256=sha(exe),prep=str(a.prep),prep_sha256=sha(a.prep),prep_contents=prep,script_sha256=sha(out/'input.script'),
        neutral_tail_policy_sha256=sha(Path(__file__).with_name('native-neutral-controls-v1.json')),host_input_used=False,OS_input_injection_used=False,nice=0,bound_seconds=a.bound,locks='caller heavy then native',warm_from=None)
    start=time.monotonic();process=dict(exit=None,timed_out=False)
    with (out/'app.log').open('xb') as log:
        child=subprocess.Popen(command,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
        launch.update(pid=child.pid,child_nice_at_spawn=os.getpriority(os.PRIO_PROCESS,child.pid),start_monotonic=start)
        (out/'launch.json').write_text(json.dumps(launch,indent=2)+'\n')
        try:process['exit']=child.wait(timeout=a.bound)
        except subprocess.TimeoutExpired:
            process['timed_out']=True;child.terminate()
            try:process['exit']=child.wait(timeout=15)
            except subprocess.TimeoutExpired:child.kill();process['exit']=child.wait()
    process['seconds']=time.monotonic()-start;(out/'process.json').write_text(json.dumps(process,indent=2)+'\n')
    result=dict(passed=False,kind='native-script-recording',checks={},error=None,acceptance_claimed=False)
    try:
        need(before_app==tree(app) and before_seed==inventory(a.user_seed),'native original app/fixture changed')
        proof=audit_recording(out);result.update(native_recording_proof=proof,checks=dict(native_recording_strict_audit=True,originals_unchanged=True),passed=True)
    except (OSError,ValueError,KeyError,TypeError) as e:result['error']=str(e)
    (out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print(out/'RESULT.json','PASS' if result['passed'] else 'FAIL',flush=True)
    return 0 if result['passed'] else 1
