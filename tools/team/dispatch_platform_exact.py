"""Isolated chosen-context exact operator mapping; not wired into full proof."""
import hashlib
import json
import os
from pathlib import Path
import stat

KEY='__CF_USER_TEXT_ENCODING'
BASE_KEYS=frozenset(('HOME','PATH','LANG','LC_ALL','TMPDIR','PYTHONCOERCECLOCALE','PYTHONUTF8','PYTHONNOUSERSITE'))
SOURCE_NAMES=('dispatch_platform_exact.py','dispatch_platform_gate.py','dispatch_platform_chosen.py',
              'dispatch_full_budget.py','dispatch_exec_gate.py','dispatch_budget.py','dispatch_preflight.py')
POSTEXEC_CODE='''import os,json,hashlib,pathlib,sys
def checked(condition,message):
    if not condition: raise RuntimeError(message)
print("SOLP_EXEC_IDENTITY",os.getpid(),os.getpgrp(),os.getsid(0),os.getuid())
raw=pathlib.Path(sys.argv[1]).read_bytes()
checked(len(raw)<=16384 and hashlib.sha256(raw).hexdigest()==sys.argv[2],"postexec operator pin")
record=json.loads(raw); expected=record["env"]; actual=dict(os.environ)
checked(set(expected)=={"HOME","PATH","LANG","LC_ALL","TMPDIR","PYTHONCOERCECLOCALE","PYTHONUTF8","PYTHONNOUSERSITE","__CF_USER_TEXT_ENCODING"} and actual==expected,"postexec exact nine-key mismatch")
digest=hashlib.sha256(json.dumps(actual,sort_keys=True,separators=(",",":"),ensure_ascii=True).encode()).hexdigest()
checked(digest==record["env_sha256"],"postexec full mapping pin")
print("SOLP_POSTEXEC_ENV_SHA256",digest)
'''


def canonical(value):return json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=True).encode()
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def strict_json(raw):
    def unique(pairs):
        result={}
        for key,value in pairs:
            if key in result:raise ValueError('duplicate operator/request JSON key')
            result[key]=value
        return result
    return json.loads(raw,object_pairs_hook=unique)


def exact_env(actual,expected):
    if type(actual) is not dict or set(expected)!=BASE_KEYS|{KEY} or actual!=expected:
        raise RuntimeError('exact operator nine-key mapping mismatch')


def load_operator(path,pin,base_env):
    path=Path(path);st=path.lstat()
    if not stat.S_ISREG(st.st_mode) or stat.S_IMODE(st.st_mode)!=0o600 or st.st_uid!=os.getuid() or st.st_size>16384:
        raise RuntimeError('private bounded operator record required')
    raw=path.read_bytes()
    if len(raw)>16384 or hashlib.sha256(raw).hexdigest()!=pin:raise RuntimeError('operator raw pin mismatch')
    record=strict_json(raw)
    if (type(record) is not dict or set(record)!={'schema','env','env_sha256','named_value'}
            or type(record['schema']) is not int or record['schema']!=1):
        raise RuntimeError('exact operator record schema required')
    if type(base_env) is not dict or set(base_env)!=BASE_KEYS:raise RuntimeError('exact independent base eight required')
    env=record['env']
    if (type(env) is not dict or set(env)!=BASE_KEYS|{KEY} or any(type(v) is not str or '\0' in v for v in env.values())
            or any(env[k]!=base_env[k] for k in BASE_KEYS)):
        raise RuntimeError('exact full operator mapping/base required')
    rawvalue=env[KEY].encode('utf-8',errors='strict')
    if len(rawvalue)>512:raise RuntimeError('operator named value bound')
    named=record['named_value']
    expected={'key':KEY,'present':True,'representation':'strict UTF-8 hex','value_hex':rawvalue.hex(),
              'bytes':len(rawvalue),'sha256':hashlib.sha256(rawvalue).hexdigest()}
    if (type(named) is not dict or type(named.get('bytes')) is not int or named.get('present') is not True
            or named!=expected):raise RuntimeError('operator named exact bytes/pin mismatch')
    digest=hashlib.sha256(canonical(env)).hexdigest()
    if record['env_sha256']!=digest:raise RuntimeError('operator full environment pin mismatch')
    return dict(env),digest


def source_check(pins):
    if type(pins) is not dict or set(pins)!=set(SOURCE_NAMES):raise RuntimeError('exact chosen source inventory')
    base=Path(__file__).resolve().parent
    for name,pin in pins.items():
        if sha(base/name)!=pin:raise RuntimeError('chosen source bytes changed')


def payload(python,operator):
    return [python,'-c',POSTEXEC_CODE,operator['path'],operator['sha256']]
