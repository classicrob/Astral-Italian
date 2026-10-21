"""Final stage evidence and detached, bounded metadata publication.

No child processes. No archives read/written in the terminal metadata postlude.
This is source-only pending exact caller/importer review and allocation.
"""
import hashlib
import json
import time
from pathlib import Path
import shutil
import stat
import math

from dispatch_full_budget import bounded_allocated

METADATA_CAP = 16777216
PUBLICATION_RESERVE = 65536


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':')).encode()


def encoded(value):
    return (json.dumps(value, indent=2, sort_keys=True)+'\n').encode()


def sha(data):
    return hashlib.sha256(data).hexdigest()


def small_bytes(path):
    path = Path(path)
    info = path.lstat()
    if not stat.S_ISREG(info.st_mode) or info.st_size > METADATA_CAP-PUBLICATION_RESERVE:
        raise RuntimeError('monitor metadata is not bounded regular file')
    return path.read_bytes()


def complete(stage, root, contract_sha256, job, payload):
    """Call only AFTER successful with-job exit, never from a finally path."""
    if stage not in ('execution', 'packaging'):
        raise RuntimeError('unknown monitor stage')
    expected = {'actual_result_sha256'} if stage == 'execution' else {'primary_encoded_sha256', 'secondary_encoded_sha256'}
    if set(payload) != expected:
        raise RuntimeError('completion payload fields')
    budget_bytes = small_bytes(job.work/'disk-budget.json')
    cleanup_bytes = small_bytes(job.work/'owned-process-cleanup.json')
    budget = json.loads(budget_bytes)
    cleanup = json.loads(cleanup_bytes)
    if (budget['failure'] is not None or budget['cleanup']['status'] != 'CONFIRMED' or
            budget['cleanup']['base_report'] != cleanup or not cleanup['all_owned_groups_absent'] or
            cleanup['remaining_pgids'] or cleanup['failures'] or cleanup['execution_error'] is not None or job.groups):
        raise RuntimeError('cannot seal unsuccessful monitor teardown')
    ended = time.monotonic()
    if not all(type(x) in (int,float) and math.isfinite(x) for x in (job.start,ended,job.seconds)):
        raise RuntimeError('nonfinite monitor clock/limit')
    completion = {'schema':1,'status':'COMPLETE','stage':stage,'root':str(root),
                  'contract_sha256':contract_sha256,'started_monotonic':job.start,
                  'ended_monotonic':ended,'elapsed_seconds':ended-job.start,
                  'work_limit_seconds':job.seconds,'cleanup_grace_seconds':4,
                  'budget_sha256':sha(budget_bytes),'cleanup_sha256':sha(cleanup_bytes),'payload':payload}
    if completion['elapsed_seconds'] < 0 or completion['elapsed_seconds'] > job.seconds+4:
        raise RuntimeError('stage completion outside work plus cleanup bound')
    path = job.work/'completion.json'
    with path.open('xb') as stream:
        stream.write(encoded(completion))
    return path


def terminal_publish(root, out, contract, contract_pin, receipt, transfer, existing_files):
    """Copy six final monitor records + publish receipt/transfer, then sidecar.

    Existing file hashes are captured INSIDE the packaging monitor. This function
    does not reopen/hash/copy archives or encoded parts. Sidecar is detached and
    externally pinned; no receipt/transfer/publication self-hash cycle.
    """
    root, out = Path(root), Path(out)
    if out != root/'package':
        raise RuntimeError('terminal publication root')
    maximum = contract['terminal_metadata']['maximum_allocated_bytes']
    reserve = contract['terminal_metadata']['reserved_bytes']
    if maximum != METADATA_CAP or reserve != METADATA_CAP:
        raise RuntimeError('terminal metadata cap/reserve mismatch')
    before = bounded_allocated(root)
    blobs = {}
    monitors = {}
    for stage, work in [('execution', root/'execution'), ('packaging', root/'packaging-monitor')]:
        records = {}
        for label, source in [('budget', 'disk-budget.json'), ('cleanup', 'owned-process-cleanup.json'), ('completion', 'completion.json')]:
            name = 'monitors/'+stage+'/'+label+'.json'
            raw = small_bytes(work/source)
            blobs[name] = raw
            records[label] = {'path':name,'sha256':sha(raw)}
        monitors[stage] = records
    receipt['monitors'] = monitors
    blobs['receipt.json'] = encoded(receipt)
    transfer['receipt_sha256'] = sha(blobs['receipt.json'])
    transfer['regular_files'] = {**existing_files, **{name:sha(raw) for name, raw in blobs.items()}}
    blobs['transfer.json'] = encoded(transfer)
    if len(blobs) != 8 or sum(map(len, blobs.values()))+PUBLICATION_RESERVE > maximum:
        raise RuntimeError('serialized terminal metadata cap')
    # All eight paths must be absent. Archives/evidence already closed by caller.
    for name in blobs:
        if (out/name).exists() or (out/name).is_symlink():
            raise RuntimeError('terminal path already exists')
    for name, raw in blobs.items():
        path = out/name
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open('xb') as stream:
            stream.write(raw)
        path.chmod(0o600)
    after = bounded_allocated(root)
    free = shutil.disk_usage(root).free
    baseline = json.loads(blobs['monitors/execution/budget.json'])['initial_allocated_bytes']
    limits = contract['required_live_budget']
    if (after < before or after-before+PUBLICATION_RESERVE > maximum or
            after-baseline+PUBLICATION_RESERVE > limits['maximum_owned_growth_bytes'] or
            free-PUBLICATION_RESERVE < limits['minimum_free_bytes']):
        raise RuntimeError('terminal actual allocation/floor cap; no publication record')
    files = {name:{'sha256':sha(raw),'bytes':len(raw),'allocated_bytes':(out/name).stat().st_blocks*512}
             for name, raw in blobs.items()}
    publication = {'schema':1,'type':'DISPATCH_TERMINAL_PUBLICATION_V1','status':'COMPLETE',
                   'root':str(root),'contract_sha256':contract_pin,
                   'receipt_sha256':sha(blobs['receipt.json']),'transfer_sha256':sha(blobs['transfer.json']),
                   'maximum_allocated_bytes':maximum,'publication_reserve_bytes':PUBLICATION_RESERVE,
                   'original_baseline_bytes':baseline,'allocated_before_metadata':before,
                   'allocated_after_metadata':after,'free_before_publication':free,'files':files}
    raw = encoded(publication)
    if len(raw) > PUBLICATION_RESERVE:
        raise RuntimeError('publication record serialized cap')
    path = out/'terminal-publication.json'
    with path.open('xb') as stream:
        stream.write(raw)
    path.chmod(0o600)
    if path.stat().st_blocks*512 > PUBLICATION_RESERVE:
        # Preserve the failed record, but never return/announce accepted hash.
        raise RuntimeError('publication record actual allocation cap')
    return sha(raw)
