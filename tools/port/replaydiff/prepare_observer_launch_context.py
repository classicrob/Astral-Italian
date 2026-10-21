#!/usr/bin/env python3
"""Prepare exact standalone launch DATA only: no child, signal, lock or env edits."""
import argparse
import hashlib
import json
from pathlib import Path
from observer_standalone_supervisor import MECHANISMS,STATIC_TARGETS


def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def prepare(root,python,work):
    root=Path(root).resolve();python=Path(python).resolve();work=Path(work).resolve()
    for name,expected in MECHANISMS.items():
        if digest(root/(name+'.py'))!=expected:raise ValueError('unreviewed supervision leaf '+name)
    for argv0,target in STATIC_TARGETS.items():
        if str(Path(argv0).resolve())!=target['path'] or digest(argv0)!=target['sha256']:raise ValueError('actual native target drift '+argv0)
    return dict(schema_version=1,status='SOURCE_CONTEXT_ONLY_UNQUALIFIED',python=str(python),python_sha256=digest(python),
                gate=str(root/'dispatch_exec_gate.py'),gate_sha256=MECHANISMS['dispatch_exec_gate'],targets=STATIC_TARGETS,
                mechanism_files={str(root/(name+'.py')):pin for name,pin in MECHANISMS.items()},
                env=dict(HOME=str(Path.home()),PATH='/opt/homebrew/bin:/usr/bin:/bin:/usr/sbin:/sbin',LANG='C',LC_ALL='C',
                         TMPDIR=str(work/'tmp'),PYTHONCOERCECLOCALE='0',PYTHONUTF8='1',PYTHONNOUSERSITE='1'),
                actual_child_context='UNRUN; no generic runner qualification/allocation',qualification='BLOCKED',qualified_frames=None)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--leaves',required=True);p.add_argument('--python',required=True);p.add_argument('--work',required=True);p.add_argument('--out',required=True)
    a=p.parse_args();r=prepare(a.leaves,a.python,a.work)
    with Path(a.out).open('x') as f:json.dump(r,f,indent=2);f.write('\n')
    print(json.dumps(dict(status=r['status'],python=r['python'],python_sha256=r['python_sha256'],sha256=digest(a.out))))
