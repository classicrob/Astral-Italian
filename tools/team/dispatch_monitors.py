"""Independent final execution/packaging monitor joins; no producer code runs."""
import hashlib
import json
import math
from pathlib import PurePosixPath


def need(value, reason):
    if not value:
        raise ValueError('dispatch monitor: '+reason)


def exact(value, fields, reason):
    need(isinstance(value,dict) and set(value)==set(fields),reason)


def digest(value):
    return hashlib.sha256(value).hexdigest()


def canonical(value):
    return json.dumps(value,sort_keys=True,separators=(',',':')).encode()


def number(value, reason, *, positive=False):
    need(type(value) in (int,float) and math.isfinite(value) and value >= 0,
         reason)
    need(not positive or value > 0,reason)
    return value


def check(stages, artifacts, contract, contract_sha256, phase_roots, payloads):
    """Check already confined/hash-verified artifact bytes, never attest sampling.

    `stages` is the receipt map of stage -> budget/cleanup/completion records.
    `artifacts` contains their independently opened bytes. The final caller
    contract and roots are operator-pinned before this function is reached.
    Terminal metadata publication needs its separately reviewed evidence; this
    function does not claim it monitors its own receipt or later writes.
    """
    exact(stages,{'execution','packaging'},'both final stages required')
    exact(artifacts,{'execution','packaging'},'both stage artifact sets required')
    exact(payloads,{'execution','packaging'},'both stage payload joins required')
    reserve=contract['terminal_metadata']['reserved_bytes']
    exact(contract['terminal_metadata'],{'maximum_allocated_bytes','reserved_bytes','scope'},
          'terminal metadata policy fields')
    need(type(reserve) is int and reserve==16777216 and
         contract['terminal_metadata']['maximum_allocated_bytes']==reserve and
         contract['terminal_metadata']['scope']=='monitor-copy-and-terminal-receipt-transfer-only',
         'terminal metadata reserve policy')
    root=PurePosixPath(phase_roots['before']).parent.parent.as_posix()
    need(PurePosixPath(phase_roots['after']).parent.parent.as_posix()==root,
         'incompatible original roots')
    required=contract['required_live_budget']
    need(required['minimum_free_bytes']==50*1024**3 and
         required['maximum_owned_growth_bytes']==4*1024**3 and
         required['sample_interval_seconds']==0.25,'operator budget policy')
    need(contract['scan_limits']['entries']==200000 and
         contract['scan_limits']['seconds']==2.0,'finite scan policy')
    need(contract['work_seconds']==2400 and contract['packaging_seconds']==600 and
         contract['cleanup_grace_seconds']==4,'stage work/cleanup policy')
    paths=[rec['path'] for records in stages.values() for rec in records.values()]
    need(len(paths)==6 and len(set(paths))==6,'monitor artifacts reused across stages')
    baselines=[];complete={}
    for stage in ('execution','packaging'):
        exact(stages[stage],{'budget','cleanup','completion'},'stage artifact records')
        exact(artifacts[stage],{'budget','cleanup','completion'},'stage artifact bytes')
        for name,raw in artifacts[stage].items():
            need(digest(raw)==stages[stage][name]['sha256'],'stage artifact hash join')
        b,c,e=[json.loads(artifacts[stage][n]) for n in ('budget','cleanup','completion')]
        exact(e,{'schema','status','stage','root','contract_sha256','started_monotonic',
                 'ended_monotonic','elapsed_seconds','work_limit_seconds',
                 'cleanup_grace_seconds','budget_sha256','cleanup_sha256','payload'},
              'final completion fields')
        need(type(e['schema']) is int and e['schema']==1 and e['status']=='COMPLETE' and
             e['stage']==stage and e['root']==root and e['contract_sha256']==contract_sha256,
             'final completion identity/status')
        need(e['budget_sha256']==digest(artifacts[stage]['budget']) and
             e['cleanup_sha256']==digest(artifacts[stage]['cleanup']),
             'completion must bind final budget and cleanup bytes')
        need(e['payload']==payloads[stage],'completion payload identity')
        start=number(e['started_monotonic'],'start',positive=True)
        end=number(e['ended_monotonic'],'end',positive=True)
        elapsed=number(e['elapsed_seconds'],'elapsed')
        limit=contract['work_seconds'] if stage=='execution' else contract['packaging_seconds']
        need(e['work_limit_seconds']==limit and e['cleanup_grace_seconds']==4 and
             end>=start and abs((end-start)-elapsed)<=0.000001 and elapsed<=limit+4,
             'final elapsed/deadline mismatch')
        need(b['root']==root and b['failure'] is None and
             b['minimum_free_bytes']==required['minimum_free_bytes']+reserve and
             b['maximum_owned_growth_bytes']==required['maximum_owned_growth_bytes']-reserve and
             b['sampling_seconds']==required['sample_interval_seconds'],
             'original root/enforced reserved limits/failure')
        need(type(b['samples']) is int and b['samples']>0,'missing samples')
        baseline=b['initial_allocated_bytes']
        need(type(baseline) is int and baseline>=0,'initial baseline')
        baselines.append(baseline)
        need(number(b['peak_observed_growth_bytes'],'growth')<=b['maximum_owned_growth_bytes'] and
             number(b['lowest_observed_free_bytes'],'floor')>=b['minimum_free_bytes'] and
             number(b['max_scan_seconds'],'scan')<=contract['scan_limits']['seconds'],
             'sampled limits exceeded or missing')
        need(c['all_owned_groups_absent'] is True and c['remaining_pgids']==[] and
             c['failures']==[] and c['execution_error'] is None,'failed final owned cleanup')
        bc=b['cleanup']
        need(bc['status']=='CONFIRMED' and bc['all_owned_groups_absence_confirmed'] is True and
             bc['remaining_groups']==[] and bc['error'] is None and
             bc['base_report_read_error'] is None and bc['base_report']==c,
             'budget/cleanup final outcome disagreement')
        complete[stage]=e
    need(baselines[0]==baselines[1],'packaging rebased the original allocation')
    need(complete['packaging']['started_monotonic']>=complete['execution']['ended_monotonic'],
         'packaging precedes execution teardown')
    return {'status':'FINAL_MONITOR_JOINS_MATCH','root':root,'initial_allocated_bytes':baselines[0],
            'semantics':'sampled observations only; no syscall latency/quota guarantee or terminal self-monitoring'}


def terminal_check(raw, pinned_sha256, receipt_bytes, receipt, contract, root,
                   baseline, file_bytes, secondary_archive):
    """Check detached, operator-pinned publication without a receipt hash cycle.

    File bytes have already passed the importer's confinement/symlink checks.
    Producer allocation observations remain host measurements, not allocations
    inferred from the receiving filesystem. The final sidecar has its own fixed
    reserve and external hash; it never contains its own hash.
    """
    need(digest(raw)==pinned_sha256,'detached publication operator hash')
    t=json.loads(raw)
    exact(t,{'schema','type','status','root','contract_sha256','receipt_sha256',
             'transfer_sha256','maximum_allocated_bytes','publication_reserve_bytes',
             'original_baseline_bytes','allocated_before_metadata','allocated_after_metadata',
             'free_before_publication','files'},'terminal publication fields')
    need(type(t['schema']) is int and t['schema']==1 and
         t['type']=='DISPATCH_TERMINAL_PUBLICATION_V1' and t['status']=='COMPLETE' and
         t['root']==root,'terminal publication identity/status')
    need(t['contract_sha256']==digest(canonical(contract)), 'terminal contract join')
    need(t['maximum_allocated_bytes']==16777216 and t['publication_reserve_bytes']==65536,
         'terminal publication cap/reserve')
    need(len(raw)<=65536,'terminal sidecar serialized cap')
    expected={'receipt.json','transfer.json'}
    expected.update(rec['path'] for records in receipt['monitors'].values() for rec in records.values())
    need(len(expected)==8 and set(t['files'])==expected and set(file_bytes)==expected,
         'terminal exact eight metadata files')
    need(t['receipt_sha256']==digest(receipt_bytes) and file_bytes['receipt.json']==receipt_bytes,
         'detached receipt join')
    need(t['transfer_sha256']==digest(file_bytes['transfer.json']),'detached transfer join')
    transfer=json.loads(file_bytes['transfer.json'])
    need(transfer['receipt_sha256']==digest(receipt_bytes) and
         transfer['primary_archive']==receipt['archive'] and
         transfer['secondary_archive']==secondary_archive,'terminal transfer/archive identities')
    serialized=0;reported_allocated=0
    for path,rec in t['files'].items():
        exact(rec,{'sha256','bytes','allocated_bytes'},'terminal member fields')
        need(type(rec['bytes']) is int and rec['bytes']>=0 and
             type(rec['allocated_bytes']) is int and rec['allocated_bytes']>=0,
             'terminal member size/allocation')
        need(rec['sha256']==digest(file_bytes[path]) and rec['bytes']==len(file_bytes[path]),
             'terminal member hash/size')
        serialized+=rec['bytes'];reported_allocated+=rec['allocated_bytes']
    need(serialized<=16777216,'terminal serialized byte cap')
    for name in ('original_baseline_bytes','allocated_before_metadata','allocated_after_metadata',
                 'free_before_publication'):
        need(type(t[name]) is int and t[name]>=0,'terminal allocation observation')
    before=t['allocated_before_metadata'];after=t['allocated_after_metadata']
    need(t['original_baseline_bytes']==baseline and before>=baseline and after>=before,
         'terminal original baseline/order')
    need(reported_allocated<=after-before and after-before+65536<=16777216,
         'terminal observed allocation cap/reserve')
    need(after-baseline+65536<=4*1024**3 and
         t['free_before_publication']-65536>=50*1024**3,
         'terminal total growth/floor reserve')
    return {'status':'DETACHED_TERMINAL_PUBLICATION_JOINS_MATCH',
            'sha256':pinned_sha256,'terminal_measured_growth_bytes':after-before,
            'publication_reserve_bytes':65536,'semantics':'host observations plus final-file reserve, no self-hash or quota guarantee'}
