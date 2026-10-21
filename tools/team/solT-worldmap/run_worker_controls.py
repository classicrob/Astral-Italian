import datetime,fcntl,hashlib,json,os,subprocess,sys
from pathlib import Path
H=Path.home();O=H/'toolchains/sol-t-validation/worldmap51';V=O.parent
planpath=H/'MacioGalaxy2-team/scratch-port/solP/fifo-worker-generated/worker-r3/native-control-plan.json'
plan=json.loads(planpath.read_text());sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
assert sha(plan['fixture_source'])==plan['fixture_source_sha256'];assert sha(plan['control_runner'])==plan['control_runner_sha256'];assert sha(plan['actual_link_context'])==plan['actual_link_context_sha256']
assert sha(H/'MacioGalaxy2-team/scratch-port/solP/fifo-worker-generated/worker-r3/fifo_firstfault_worker_capture.hpp')==plan['header_sha256']
review=Path(sys.argv[1]);assert sha(review)==sys.argv[2]
def state(phase):
 s=f'UTC={datetime.datetime.now(datetime.timezone.utc).isoformat()} phase={phase} inputs=8c9430f8+overlay1048d955+toolchainfbd11526;worker-r3;controls={planpath} output={O}/fifo-worker-r3-control PID={os.getpid()} resume=native-controls-before-full540;deadline05UTC\n';(V/'STATE').write_text(s);(O/'STATE').write_text(s)
state('PRE-LOCK-CONTROLS');lock=(H/'MacioGalaxy2-team/slots/native.lock').open('a');fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB);os.setpriority(os.PRIO_PROCESS,0,19)
try:
 out=O/'fifo-worker-r3-control';out.mkdir();bounded=H/'MacioGalaxy2-team/scratch-port/solT/worldmap51-source/native/tools/bounded_run.py'
 for name,argv in [('compile',plan['compile_argv']),('controls',plan['run_argv'])]:
  state('RUNNING-WORKER-'+name)
  with (out/(name+'.log')).open('x') as f:
   rc=subprocess.run([sys.executable,str(bounded),'90',*argv],stdout=f,stderr=subprocess.STDOUT).returncode
  print(name,rc,flush=True);assert rc==0
 result=json.loads((out/'native-controls.json').read_text());assert result['status']=='SYNTHETIC_NATIVE_HEADER_CONTROLS_PASS_NOT_APP_OR_PARSER_ACCEPTANCE';assert result['app_output_absent']
 receipt={'label':'M1 iteration: 8c9430f89569b3b51777e6274c5f491009b69ba7 + overlay 1048d955 + toolchain fbd11526','status':'PASS','plan':str(planpath),'plan_sha256':sha(planpath),'review_receipt':str(review),'review_sha256':sha(review),'native_controls':str(out/'native-controls.json'),'native_controls_sha256':sha(out/'native-controls.json'),'limits':'synthetic actual-header controls; not actual parser/default-OFF app smoke'}
 (out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n');rpath=O/'worker-diag-receipt.json';r=json.loads(rpath.read_text());r['focused_controls']={'status':'PASS','receipt':str(out/'receipt.json'),'receipt_sha256':sha(out/'receipt.json')};rpath.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(receipt),flush=True)
finally:fcntl.flock(lock,fcntl.LOCK_UN);lock.close();state('RELEASED-NATIVE-CONTROLS')
