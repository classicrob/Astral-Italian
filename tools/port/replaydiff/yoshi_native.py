#!/usr/bin/env python3
"""Run the native Yoshi diagnostic against finalized, actually delivered inputs."""
import argparse
import csv
import json
import os
from pathlib import Path
import socket
import signal
import subprocess
import sys
import time

import tracediff
from dolphin_collect import game_private_path
from human_pair import raw_first, gx_departure
from kpad_records import digest
from yoshi_replay import write_json

EXCLUDED = {
    'pending_state_id': 'unavailable (callback-context only)',
    'prev_draw': 'pre-first-stride initialization/previous-frame semantics unproved',
    'ticks': 'Dolphin clock source not observed; native absolute ticks are separate timing evidence',
    'vi_count': 'Dolphin VI clock not observed; no invented clock equivalence',
}


def finalized_inputs(case):
    result = json.loads((case / 'result.json').read_text())
    manifest = json.loads((case / 'capture/manifest.json').read_text())
    if result.get('status') != 'verified diagnostic only' or manifest.get('status') != 'captured; repeat qualification required':
        raise ValueError('Dolphin input delivery is incomplete; no gameplay comparison')
    binding = result['delivered_input']
    frames = binding['frames']
    if type(frames) is not int or frames <= 0 or manifest['frames'] != frames + 1:
        raise ValueError('Dolphin trace/input frame accounting differs')
    source = Path(binding['path']).resolve()
    if source != (case / 'capture/observed-inputs.jsonl').resolve():
        raise ValueError('native replay must consume the actual normalized observed suffix')
    checks = [(source, binding['sha256']),
              (case / 'capture/observed-inputs.kpd', binding['kpd_sha256']),
              (case / 'capture/inputs.jsonl', result['input_export_sha256']),
              (case / 'capture/oracle.csv', result['oracle_sha256'])]
    for path, expected in checks:
        if digest(path) != expected:
            raise ValueError(f'finalized Dolphin artifact changed: {path.name}')
    if manifest['input_export']['sha256'] != result['input_export_sha256']:
        raise ValueError('producer input receipt does not bind actual export')
    sync = manifest['sync']
    if (sync['stage'] != 'YosshiHomeGalaxy' or sync['scenario'] != 1 or sync.get('rng_forced') is not False
            or manifest['raw_input_suffix']['source_start'] != sync['source_frame'] + 1):
        raise ValueError('natural ready origin or input phase is inconsistent')
    return result, manifest, source, frames


def frozen_bundle(root):
    identity = json.loads((root / 'identity.json').read_text())
    source = json.loads((root / 'launcher/source.json').read_text())
    binary = root / 'petari_headless'
    if source['source'] != identity['source'] or digest(binary) != identity['binary_sha256']:
        raise ValueError('frozen native source/binary identity changed')
    if digest(root / 'libclang_rt.asan_osx_dynamic.dylib') != identity['asan_sha256']:
        raise ValueError('frozen ASan runtime changed')
    for name, expected in source['files'].items():
        if digest(root / 'launcher' / name) != expected:
            raise ValueError('frozen native launcher support changed: ' + name)
    return identity, source, binary


def compare(case, output, result, dolphin, frames):
    manifest = json.loads((output / 'native.csv.manifest.json').read_text())
    if manifest.get('status') != 'complete' or manifest.get('trace_frames') != frames + 1:
        raise ValueError('native capture incomplete; do not compare partial outputs')
    if manifest['input_sha256'] != result['delivered_input']['sha256']:
        raise ValueError('native consumed a different input tape')
    if (manifest.get('inputs_from_sync') is not True or manifest.get('input_origin') != 'first loop after ready anchor'
            or manifest['sync_origin'].get('rng_forced') is not False):
        raise ValueError('native input/seed phase differs')
    a, b = tracediff.read_trace(output / 'native.csv'), tracediff.read_trace(case / 'capture/oracle.csv')
    for current, producer in ((a, manifest), (b, dolphin)):
        if (producer.get('observer_contract_incomplete') is not True or
                producer.get('field_availability', {}).get('pending_state_id') != EXCLUDED['pending_state_id'] or
                any(row['pending_state_id'] for row in current.rows.values())):
            raise ValueError('unavailable observer contract is not explicit and blank')
        current.columns = [name for name in current.columns if name not in EXCLUDED]
        current.rows = {key: {name: value for name, value in row.items() if name not in EXCLUDED}
                        for key, row in current.rows.items()}
    return dict(status='UNQUALIFIED_DIAGNOSTIC', qualification='BLOCKED', measured_qualified_frames=None,
                frames=frames+1, excluded_fields=EXCLUDED,
                natural_ready_rng=dict(native=a.rows[(0,)]['rng_seed'], dolphin=b.rows[(0,)]['rng_seed']),
                rng_override=False, raw_first_difference=raw_first(a,b),
                default_tolerance=tracediff.compare(a,b,expected_frames=frames+1),
                zero_tolerance=tracediff.compare(a,b,policy=tracediff.Policy(abs_tol=0,rel_tol=0),expected_frames=frames+1),
                native_trace_sha256=digest(output/'native.csv'), dolphin_trace_sha256=result['oracle_sha256'],
                actual_input=result['delivered_input'], native_manifest=manifest,
                clear_witness=result.get('stage_clear'),
                limitations=['Host-intent reconstruction/proxy anchor differs from original consumed GX inputs.',
                             'Yoshi pointer/shoot are recorded host intent before KPAD filtering; synthetic shake retained.',
                             'Read-only connection export does not establish callback/intra-call timing equivalence.',
                             'Unavailable pending state, unqualified clock/FP policy and absent actor sidecars prevent admission.'])


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--case',type=Path,required=True)
    parser.add_argument('--native-bundle',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--recording',type=Path)
    args=parser.parse_args()
    case,bundle,output=(game_private_path(path) for path in (args.case,args.native_bundle,args.output))
    result,dolphin,inputs,frames=finalized_inputs(case)
    identity,support,binary=frozen_bundle(bundle)
    output.mkdir(parents=True,exist_ok=False)
    command=['nice','-n','9',sys.executable,str(bundle/'launcher/native/tools/headless.py'),'run',
             '--executable',str(binary),'--inputs',str(inputs),'--frames',str(frames),
             '--disc',str(Path.home()/'MacioGalaxy2-gamedata/SB4E01-extracted'),
             '--user',str(output/'user'),'--trace',str(output/'native.csv'),'--epoch-ticks','0',
             '--stage','YosshiHomeGalaxy','--scenario','1','--sync-stage','YosshiHomeGalaxy',
             '--sync-scenario','1','--inputs-from-sync']
    env={key:value for key,value in os.environ.items() if not key.startswith(('PETARI_','ASTRAL_'))}
    env.update(PETARI_MODS='none',PETARI_TRACE_BOOT='1',PETARI_CACHE_DIR=str(output/'cache'),
               LC_ALL='C',LANG='C',ASAN_OPTIONS='detect_odr_violation=0')
    receipt=dict(schema_version=1,status='prepared',host=socket.gethostname(),source=identity['source'],
                 binary_sha256=identity['binary_sha256'],support=support,argv=command,
                 actual_input_sha256=result['delivered_input']['sha256'],
                 environment={key:env[key] for key in ('PETARI_MODS','PETARI_TRACE_BOOT','PETARI_CACHE_DIR','LC_ALL','LANG','ASAN_OPTIONS')},
                 hook='OFF; PETARI_TEST_RETURN_TO_SHIP absent',wall_guard_seconds=540,
                 nice_policy='outer nice+9 plus launcher nice+10; inherited positive niceness can only lower priority further',
                 driver_sha256=digest(Path(__file__)),started_epoch=time.time(),pid=None)
    write_json(output/'receipt.json',receipt)
    (output/'STATE').write_text(f'HOST {socket.gethostname()} | SOURCE {identity["source"]} | BINARY {identity["binary_sha256"]} | PHASE native actual-export followup prepared | OUTPUT {output} | PID none | RESUME fresh output if interrupted\n')
    with (output/'native.log').open('x') as log:
        child=subprocess.Popen(command,stdout=log,stderr=subprocess.STDOUT,env=env,start_new_session=True)
        receipt.update(status='running',pid=child.pid)
        write_json(output/'receipt.json',receipt)
        (output/'STATE').write_text(f'HOST {socket.gethostname()} | SOURCE {identity["source"]} | BINARY {identity["binary_sha256"]} | PHASE native actual-export running guard540 | OUTPUT {output} | PID {child.pid} | RESUME fresh output if interrupted\n')
        def interrupted(signum, _frame):
            try:
                os.killpg(child.pid,signal.SIGTERM)
                child.wait(timeout=8)
            except ProcessLookupError:
                pass
            except subprocess.TimeoutExpired:
                os.killpg(child.pid,signal.SIGKILL)
                child.wait()
            receipt.update(status='interrupted; outputs unaccepted',signal=signum,completed_epoch=time.time())
            write_json(output/'receipt.json',receipt)
            raise SystemExit(128+signum)
        previous={sig:signal.signal(sig,interrupted) for sig in (signal.SIGINT,signal.SIGTERM)}
        try:
            status=child.wait()
        finally:
            for sig,handler in previous.items(): signal.signal(sig,handler)
    receipt.update(status='finished' if status==0 else 'failed',exit_status=status,completed_epoch=time.time())
    write_json(output/'receipt.json',receipt)
    if status:
        raise RuntimeError('native run failed; retain log/partials; no gameplay comparison')
    if frozen_bundle(bundle)!=(identity,support,binary):
        raise ValueError('native identity/support receipt changed during replay')
    if finalized_inputs(case)!=(result,dolphin,inputs,frames):
        raise ValueError('Dolphin producer receipt changed during native replay')
    report=compare(case,output,result,dolphin,frames)
    if args.recording:
        recording=game_private_path(args.recording)
        expected='c879847d88ec64b31955ec381f607d6d5e29397979501c650df672a92a352d46'
        if digest(recording)!=expected: raise ValueError('original Yoshi recording changed')
        with recording.open() as stream: gx=list(csv.DictReader(stream))
        traces={'native':tracediff.read_trace(output/'native.csv'),'dolphin':tracediff.read_trace(case/'capture/oracle.csv')}
        report['recording_position_departure']={side:{str(tol):gx_departure(trace,gx,522,tol) for tol in (0,.0005,.01)} for side,trace in traces.items()}
        report['recording_comparison']='old GX source522+trace proxy; rounded coordinates, not consumed-input or RNG equivalence'
    write_json(output/'diagnostic.json',report)
    (output/'STATE').write_text(f'HOST {socket.gethostname()} | SOURCE {identity["source"]} | BINARY {identity["binary_sha256"]} | PHASE native complete diagnostic BLOCKED | OUTPUT {output} | PID none | RESUME verify receipts before handoff\n')
    print(json.dumps(dict(report=str(output/'diagnostic.json'),raw_first=report['raw_first_difference'],natural_ready_rng=report['natural_ready_rng'])))


if __name__=='__main__': main()
