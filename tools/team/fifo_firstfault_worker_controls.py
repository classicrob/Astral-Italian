#!/usr/bin/env python3
"""Run a T2-built synthetic header fixture; DOES NOT compile or run the app."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

HEADER_SHA='338e1492fd12f78374559acfe4eb0237079de95527430137cc729028b2454c4e'
PRIVATE=Path.home()/'MacioGalaxy2-gamedata/solP-fifo-firstfault'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def require(ok,why):
    if not ok:raise ValueError(why)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--binary',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    require(digest(a.header)==HEADER_SHA,'reviewed header hash')
    text=a.header.read_text();m=re.search(r'constexpr char CaptureParent\[\] = ("[^\n]*");',text);require(m is not None,'capture parent literal')
    parent=Path(json.loads(m[1]));require(parent.absolute()==parent.resolve() and PRIVATE in parent.parents,'private no-symlink capture root')
    require(parent.is_dir() and (parent.stat().st_mode&0o777)==0o700,'private parent700')
    output=parent/'firstfault.log';require(not output.exists() and not output.is_symlink(),'refuse existing capture; never overwrite gameplay evidence')
    require(not a.report.exists(),'report must be fresh')
    description=subprocess.run([str(a.binary),'--describe'],capture_output=True,text=True,timeout=10,check=True)
    require(description.stdout.strip()=='P_FIFO_WORKER_NATIVE_CONTROL_v1','fixture identity')
    rows=[]
    def run(mode,enabled):
        env=dict(os.environ);env.pop('PETARI_FIFO_FIRSTFAULT',None)
        if enabled:env['PETARI_FIFO_FIRSTFAULT']='1'
        result=subprocess.run([str(a.binary),mode],env=env,capture_output=True,text=True,timeout=30)
        row={'mode':mode,'env_enabled':enabled,'rc':result.returncode,'stdout':result.stdout,'stderr':result.stderr};rows.append(row)
        require(result.returncode==0 and 'CONTROL PASS' in result.stdout,'native control failed '+mode)
    def preserve(name):
        dst=parent/name;require(not dst.exists(),'preserve destination exists');output.rename(dst)
        return {'path':str(dst),'sha256':digest(dst),'bytes':dst.stat().st_size,'mode':oct(dst.stat().st_mode&0o777)}
    run('off',False);require(not output.exists(),'OFF created output')
    run('ranges',True);require(output.stat().st_size==0,'ranges must not trigger snapshots')
    range_file=preserve('control-ranges-empty.log')
    run('on',True);require((output.stat().st_mode&0o777)==0o600,'capture600')
    b=output.read_bytes();require(len(b)<=1024*1024,'two-snapshot cap')
    require(b.count(b'BEGIN P_FIFO_WORKER_FIRSTFAULT_v1')==2 and b.count(b'END P_FIFO_WORKER_FIRSTFAULT_v1')==2,'two complete snapshots')
    require(b'raw=41' in b and b'raw=39' in b and b'primitives=3' in b,'raw/totals coverage')
    normal=preserve('control-on.log')
    run('io',True);require(output.stat().st_size==0,'forced EINTR should not write file')
    io=preserve('control-io-empty.log')
    # Existing-file refusal: fixture must not alter this synthetic sentinel.
    output.write_bytes(b'CONTROL_SENTINEL\n');output.chmod(0o600);sentinel=digest(output)
    run('setupfail',True);require(digest(output)==sentinel,'existing-file refusal changed sentinel')
    existing=preserve('control-existing-file.log')
    # Parent permission rejection; always restore the review-required700 mode.
    try:
        parent.chmod(0o755);run('setupfail',True);require(not output.exists(),'insecure parent created file')
    finally:parent.chmod(0o700)
    require(not output.exists() and not output.is_symlink(),'app output must be absent')
    report={'status':'SYNTHETIC_NATIVE_HEADER_CONTROLS_PASS_NOT_APP_OR_PARSER_ACCEPTANCE','binary_sha256':digest(a.binary),'header_sha256':HEADER_SHA,'fixture_source_sha256':digest(Path(__file__).with_name('fifo_firstfault_worker_native_control.cpp')),'runner_sha256':digest(Path(__file__)),'cases':rows,'preserved':[range_file,normal,io,existing],'app_output':str(output),'app_output_absent':True,'limitations':['synthetic Reader/GX metadata, actual reviewed header','test-only write wrapper forces EINTR; production header unchanged','actual parser/default-OFF app smoke and typed host replay not claimed']}
    a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'status':report['status'],'report':str(a.report)}))
if __name__=='__main__':
    try:main()
    except (OSError,ValueError,subprocess.SubprocessError) as e:print('CONTROL FAILED '+str(e),file=sys.stderr);sys.exit(1)
