"""Fixed L-authorized direct shell rule. Source only; no execution admission."""
import hashlib
import os
from pathlib import Path
import stat
import dispatch_platform_exact as exact
import dispatch_full_budget as budget

ROOT=Path.home()/'MacioGalaxy2-team/scratch-port/solP/dispatch-platform-direct-rule-v1'
AUTHORITY=Path.home()/'MacioGalaxy2-team/scratch-port/solL/dispatch-direct-rule-authority-r1/AUTHORITY.json'
AUTHORITY_SHA='b315336764984939a563cb6fc291b2efdd1f7b2ecb8be962b3114c38fbeafee2'
OFFICIAL_SHA='434ee06a6667fd27b4fb58d4ea072d97835c4f6567831388bb5d8f514c4a6f59'
SYNTHETIC_SHA='403412f0d7dfe992232e0da59412324d3a92728c7541895961ae8520c7b025b6'
SOURCE_NAMES=('dispatch_direct_profile.py','dispatch_direct_gate.py','dispatch_direct_chosen.py',
 'dispatch_platform_exact.py','dispatch_full_budget.py','dispatch_exec_gate.py','dispatch_budget.py','dispatch_preflight.py')
LABELS=('official_direct_rule',)


def bounded(path,maximum):
    path=Path(path);st=path.lstat()
    if not stat.S_ISREG(st.st_mode) or st.st_size>maximum:raise RuntimeError('bounded regular source record required')
    with path.open('rb') as f:raw=f.read(maximum+1)
    if len(raw)>maximum:raise RuntimeError('source record grew beyond bound')
    return raw


def authority():
    raw=bounded(AUTHORITY,65536)
    if hashlib.sha256(raw).hexdigest()!=AUTHORITY_SHA:raise RuntimeError('independent authority pin')
    data=exact.strict_json(raw)
    official=bounded(AUTHORITY.parent/'official-command.raw',4096)
    synthetic=bounded(AUTHORITY.parent/'synthetic-command.raw',4096)
    if (len(official)!=972 or hashlib.sha256(official).hexdigest()!=OFFICIAL_SHA
        or len(synthetic)!=975 or hashlib.sha256(synthetic).hexdigest()!=SYNTHETIC_SHA):
        raise RuntimeError('independent raw command bytes differ')
    if b'\x00' in synthetic or b'\n' in synthetic or b'\r' in synthetic:raise RuntimeError('single raw shell command required')
    changed=official
    for span in reversed(data['substitution_spans_in_original']):
        offset=span['offset'];before=span['before'].encode();after=span['after'].encode()
        if changed[offset:offset+len(before)]!=before:raise RuntimeError('exact source substitution span')
        changed=changed[:offset]+after+changed[offset+len(before):]
    if changed!=synthetic:raise RuntimeError('unapproved command byte change')
    return data,synthetic.decode('utf-8',errors='strict')


def commands(plan):
    _,command=authority()
    return [['/bin/sh','-c',command]]


def validate_plan(plan):
    if type(plan) is not dict or set(plan)!=PLAN_FIELDS:raise RuntimeError('exact direct plan fields')
    if type(plan['schema']) is not int or plan['schema']!=1 or type(plan['execution_enabled']) is not bool:
        raise RuntimeError('direct plan schema')
    if plan['status']!='DISABLED_DESCENDANT_TIMEOUT_UNQUALIFIED' or plan['execution_enabled'] is not False:
        raise RuntimeError('source-only direct rule must remain disabled')
    if plan['root']!=str(ROOT) or plan['labels']!=list(LABELS):raise RuntimeError('fixed direct root/profile')
    if [plan[k] for k in ('total_seconds','work_seconds','cleanup_seconds','maximum_allocated_bytes','minimum_free_bytes')]!=[60,20,4,67108864,53687091200]:
        raise RuntimeError('fixed direct limits')
    if plan['completion_reserved_bytes']!=65536:raise RuntimeError('fixed metadata reserve')
    if type(plan['source_files']) is not dict or set(plan['source_files'])!=set(SOURCE_NAMES):raise RuntimeError('exact source inventory')
    for name,pin in plan['source_files'].items():
        if exact.sha(Path(__file__).with_name(name))!=pin:raise RuntimeError('direct source drift')
    data,_=authority()
    if plan['authority']!={'path':str(AUTHORITY),'sha256':AUTHORITY_SHA}:raise RuntimeError('independent authority reference')
    op=data['operator']
    if plan['operator']!={'path':op['path'],'sha256':op['sha256']} or plan['full_env_sha256']!=op['full_env_sha256']:
        raise RuntimeError('independent operator pin')
    for key in ('cwd','substitution_spans_in_original','output_binding','depfile_binding','source_fixture_sha256'):
        if plan[key]!=data[key]:raise RuntimeError('independent command/context binding drift')
    selected={k:data['tools'][k] for k in ('sh','wibo','wibo_real','sjiswrap','compiler','python_command','transform_dep')}
    if plan['tools']!=selected:raise RuntimeError('exact chain tool inventory/pins')
    for record in selected.values():
        if exact.sha(record['path'])!=record['sha256']:raise RuntimeError('chain tool drift')
    if plan['commands']!=commands(plan):raise RuntimeError('only exact L raw command argv')
    base=budget.launch_context(ROOT)
    if plan['base_env']!=base['env'] or plan['python']!={'resolved':base['python'],'sha256':base['python_sha256']}:
        raise RuntimeError('exact parent interpreter/base mapping')
    env,digest=exact.load_operator(op['path'],op['sha256'],base['env'])
    if digest!=plan['full_env_sha256']:raise RuntimeError('operator mapping digest')
    return env


PLAN_FIELDS=frozenset(('schema','status','execution_enabled','root','labels','total_seconds','work_seconds',
 'cleanup_seconds','maximum_allocated_bytes','minimum_free_bytes','completion_reserved_bytes','source_files',
 'authority','operator','full_env_sha256','cwd','substitution_spans_in_original','output_binding','depfile_binding',
 'source_fixture_sha256','tools','commands','base_env','python'))


def context(plan):
    env=validate_plan(plan)
    return {'python':plan['python']['resolved'],'python_sha256':plan['python']['sha256'],
       'gate':str(Path(__file__).with_name('dispatch_direct_gate.py').resolve()),
       'gate_sha256':plan['source_files']['dispatch_direct_gate.py'],'env':env,
       'targets':{'/bin/sh':dict(plan['tools']['sh'])}}


def require_admission():
    raise RuntimeError('direct shell/wibo/guest descendant and timeout boundary UNQUALIFIED: execution refused')
