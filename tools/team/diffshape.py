#!/usr/bin/env python3
"""Cluster objdiff instruction differences without compiling or touching objects.

Shapes are descriptive, not equivalence proofs. --report selects the function
inventory; fuzzy is the current diff score (report score if an object is absent).
"""
from __future__ import annotations
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import csv
import difflib
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time

DEFAULT_SCRATCH = Path.home() / 'MacioGalaxy2-team/scratch-decomp2'
REG = re.compile(r'\br(?:[12]?\d|3[01])\b')
FREG = re.compile(r'\bf(?:[12]?\d|3[01])\b')
NUM = r'-?(?:0x[0-9a-fA-F]+|\d+)'
LOAD = re.compile(r'^(?:l(?:bz|hz|ha|wz|fs|fd|mw)|psq_l)')


def instructions(symbol):
    return [i['instruction'] for i in symbol.get('instructions', []) if i.get('instruction')]


def mnemonic(ins):
    return ins.get('formatted', '').split(' ', 1)[0]


def normal(ins, start=0, registers=False, stack=False, reloc=True, offsets=False):
    text = ins.get('formatted', '')
    if reloc and ins.get('relocation'):
        # Keep non-relocation operand structure; references differ across objects.
        parts = []
        for p in ins.get('parts', []):
            if 'opcode' in p: parts.append(p['opcode']['mnemonic'] + ' ')
            elif p.get('separator'): parts.append(', ')
            elif 'arg' in p:
                a = p['arg']
                parts.append('@REF' if a.get('reloc') else '@OFFSET' if offsets and ('signed' in a or 'unsigned' in a) else str(a.get('opaque', a.get('signed', a.get('unsigned', a.get('branch_dest', a))))))
            elif 'basic' in p: parts.append(p['basic'])
        text = ''.join(parts)
    elif mnemonic(ins).startswith('b') and not ins.get('relocation'):
        # Internal branch targets must be relative to the function, not the TU.
        text = re.sub(r'0x([0-9a-fA-F]+)$', lambda m: 'B' + str(int(m[1], 16) - start), text)
    if stack:
        text = re.sub(NUM + r'\(r1\)', 'STACK(r1)', text)
        text = re.sub(r'^(addi\s+\w+, r1, )' + NUM + r'$', r'\1STACK', text)
    if registers:
        text = REG.sub(lambda m: 'r1' if m[0] == 'r1' else 'G', text)
        text = FREG.sub('F', text)
    return text


def counterpart(left, right):
    found = next((s for s in right if s['name'] == left['name']), None)
    if found is None and 0 < left.get('target_symbol', 0) <= len(right):
        found = right[left['target_symbol'] - 1]
    return found


def swaps(a, b):
    """Count disjoint adjacent swaps; reject arbitrary permutations."""
    if len(a) != len(b): return None
    positions = []
    i = 0
    while i < len(a):
        if a[i] == b[i]: i += 1; continue
        if i + 1 >= len(a) or a[i] != b[i+1] or a[i+1] != b[i]: return None
        positions.append(i)
        i += 2
    return positions


def fp_order(a, b):
    for x, y in zip(a, b):
        if x == y: continue
        ax, ay = x.replace(',', '').split(), y.replace(',', '').split()
        if len(ax) != len(ay) or ax[0] != ay[0] or ax[0] not in ('fmul', 'fmuls', 'fmadd', 'fmadds', 'fnmadd', 'fnmadds'): return False
        if ax[1] != ay[1] or Counter(ax[2:]) != Counter(ay[2:]): return False
    return True


def shape(left, right):
    if right is None: return 'missing_counterpart', {}
    li, ri = instructions(left), instructions(right)
    ls, rs = int(left.get('address', 0)), int(right.get('address', 0))
    a = [normal(i, ls) for i in li]
    b = [normal(i, rs) for i in ri]
    delta = len(b) - len(a)
    changed = [(i, x, y) for i, (x, y) in enumerate(zip(a, b)) if x != y]
    detail = {'target_instructions': len(a), 'ours_instructions': len(b), 'length_delta': delta,
              'different_positions': len(changed), 'examples': [[x, y] for _, x, y in changed[:4]]}
    if left.get('match_percent', 0) >= 100: return 'matched_since_report', detail
    if not ri: return 'missing_instructions', detail
    if a == b:
        # objdiff ignores anonymous reference spelling; names alone must not
        # prevent finding register/stack/scheduling patterns elsewhere.
        raw_a, raw_b = [normal(i, ls, reloc=False) for i in li], [normal(i, rs, reloc=False) for i in ri]
        return ('relocation_identity_only' if raw_a == raw_b else 'same_length:relocation_refs_only'), detail
    def normed(seq, start, **kw): return [normal(x, start, **kw) for x in seq]
    if len(a) == len(b):
        if fp_order(a, b): return 'same_length:fp_operand_order', detail
        if normed(li, ls, stack=True) == normed(ri, rs, stack=True):
            return 'same_length:stack_offsets_only', detail
        g = [REG.sub(lambda m: 'r1' if m[0] == 'r1' else 'G', x) for x in a]
        h = [REG.sub(lambda m: 'r1' if m[0] == 'r1' else 'G', x) for x in b]
        if g == h:
            arg_changes = [i for i, x, y in changed if re.match(r'(?:mr|addi|li|lwz) r[3-9],', x)
                           and any(mnemonic(z) in ('bl', 'bctrl') for z in li[i+1:i+5])]
            return ('same_length:call_arg_gprs_only' if len(arg_changes) == len(changed) else 'same_length:gprs_only'), detail
        if normed(li, ls, registers=True) == normed(ri, rs, registers=True):
            return 'same_length:gprs_fprs_only', detail
        if normed(li, ls, registers=True, stack=True) == normed(ri, rs, registers=True, stack=True):
            return 'same_length:registers_and_stack_offsets', detail
        pos = swaps(a, b)
        if pos:
            detail['swaps'] = pos
            if all(LOAD.match(a[i]) and LOAD.match(a[i+1]) for i in pos):
                return f'same_length:adjacent_load_swaps={len(pos)}', detail
            if all(i < 16 and any(x.startswith('stw r0,') for x in a[i:i+2]) and
                   any(LOAD.match(x) for x in a[i:i+2]) for i in pos):
                return 'same_length:prologue_load_vs_lr_store', detail
            return f'same_length:adjacent_instruction_swaps={len(pos)}', detail
        if Counter(a[:16]) == Counter(b[:16]) and a[16:] == b[16:]:
            lr = next((i for i, x in enumerate(a[:16]) if x.startswith('stw r0,')), None)
            rr = next((i for i, x in enumerate(b[:16]) if x.startswith('stw r0,')), None)
            if lr is not None and rr is not None and lr != rr:
                moved_loads = [x for x in a[:16] if LOAD.match(x) and (a.index(x) < lr) != (b.index(x) < rr)]
                if moved_loads: return 'same_length:prologue_load_vs_lr_store', detail
        if normed(li, ls, offsets=True) == normed(ri, rs, offsets=True):
            offsets = []
            for i, _, _ in changed:
                x, y = li[i].get('relocation', {}), ri[i].get('relocation', {})
                for pa, pb in zip(li[i].get('parts', []), ri[i].get('parts', [])):
                    xa, ya = pa.get('arg', {}), pb.get('arg', {})
                    for key in ('signed', 'unsigned'):
                        if key in xa and key in ya and xa[key] != ya[key]:
                            offsets.append(int(ya[key]) - int(xa[key]))
            detail['relocation_offset_deltas'] = sorted(set(offsets))
            if any(mnemonic(x) == 'bctr' for x in li): return 'same_length:jump_table_ref_offsets_only', detail
            if offsets and len(set(offsets)) == 1 and offsets[0]: return 'same_length:pool_offsets_uniform_shift', detail
            return 'same_length:relocation_refs_only', detail
    nomr_a = [x for x in a if not x.startswith('mr ')]
    nomr_b = [x for x in b if not x.startswith('mr ')]
    if nomr_a == nomr_b: return f'extra_missing_mr:delta={delta:+d}', detail
    nob_a = [x for x in a if not x.startswith('b')]
    nob_b = [x for x in b if not x.startswith('b')]
    if nob_a == nob_b: return f'extra_missing_branch:delta={delta:+d}', detail
    # General signatures retain edit operations/opcode pairs, not symbols/addresses.
    aa, bb = normed(li, ls, registers=True, stack=True, reloc=True), normed(ri, rs, registers=True, stack=True, reloc=True)
    edits = []
    for tag, i, j, k, l in difflib.SequenceMatcher(None, aa, bb, autojunk=False).get_opcodes():
        if tag == 'equal': continue
        left_ops = ','.join(mnemonic(x) for x in li[i:j][:4])
        right_ops = ','.join(mnemonic(x) for x in ri[k:l][:4])
        edits.append(f'{tag}:{left_ops}>{right_ops}')
    detail['edits'] = edits
    signature = ';'.join(sorted(set(edits)))
    if len(signature) > 160:
        signature = signature[:120] + ':' + hashlib.sha256(signature.encode()).hexdigest()[:10]
    return f'mixed:delta={delta:+d}:' + (signature or 'operand_changes'), detail


def run_unit(root, base_root, unit, wanted, timeout, diff_cache=None):
    target = root / unit['target_path']
    base = base_root / unit.get('base_path', '__absent__')
    if not target.is_file() or not base.is_file():
        status = 'missing_target_object' if not target.is_file() else 'missing_base_object'
        return [(status, unit['name'], f['name'], int(f.get('size', 0)), f.get('fuzzy_match_percent', 0), {}) for f in wanted], None
    cmd = [str(root / 'build/tools/objdiff-cli'), '-L', 'error', 'diff', '-c', 'functionRelocDiffs=none', '-1', str(target), '-2', str(base), '-o', '-']
    try:
        cached = diff_cache / (unit['name'].replace('/', '_')+'.json') if diff_cache else None
        if cached and cached.is_file():
            data = json.loads(cached.read_text())
        else:
            p = subprocess.run(cmd, cwd=root, capture_output=True, text=True, timeout=timeout)
            if p.returncode: raise ValueError(p.stderr[-1000:])
            data = json.loads(p.stdout)
        syms = {s['name']: s for s in data['left']['symbols']}
        right = data.get('right', {}).get('symbols', [])
        rows = []
        for f in wanted:
            left = syms.get(f['name'])
            if left is None:
                rows.append(('missing_target_symbol', unit['name'], f['name'], int(f.get('size', 0)), f.get('fuzzy_match_percent', 0), {}))
                continue
            category, detail = shape(left, counterpart(left, right))
            rows.append((category, unit['name'], left['name'], int(left.get('size', 0)), left.get('match_percent', 0), detail))
        return rows, None
    except (ValueError, subprocess.TimeoutExpired) as e:
        return [('diff_error', unit['name'], f['name'], int(f.get('size', 0)), f.get('fuzzy_match_percent', 0), {}) for f in wanted], str(e)


def cross_tab(rows, rules_dir, out):
    rules = {}
    for path in sorted(rules_dir.glob('*.tsv')):
        with path.open() as f:
            reader = csv.DictReader(f, delimiter='\t')
            # Only miner lists have these inventory/evidence columns.
            if not {'unit', 'function', 'owner', 'evidence'} <= set(reader.fieldnames or []): continue
            for row in reader:
                key = (row['unit'].removeprefix('main/'), row['function'])
                rules.setdefault(key, []).append(path.stem)
    count, sizes, uncovered = Counter(), Counter(), []
    for cluster, unit, function, size, fuzzy, _ in rows:
        matched = rules.get((unit.removeprefix('main/'), function), [])
        for rule in matched or ['UNCOVERED']:
            count[cluster, rule] += 1
            sizes[cluster, rule] += size
        if not matched and cluster.startswith('same_length:'):
            uncovered.append((cluster, unit, function, size, fuzzy))
    with (out / 'cluster-rules.tsv').open('w') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(['cluster', 'rule', 'functions', 'bytes'])
        w.writerows((c, r, count[c, r], sizes[c, r]) for c, r in sorted(count))
    with (out / 'uncovered-same-length.tsv').open('w') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(['cluster', 'unit', 'function', 'size', 'fuzzy'])
        w.writerows(sorted(uncovered, key=lambda r: (-r[3], r[1], r[2])))
    return len(uncovered)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path.cwd())
    p.add_argument('--base-root', type=Path, help='read compiled base objects from this configured checkout')
    p.add_argument('--report', type=Path, default=DEFAULT_SCRATCH / 'report-base.json')
    p.add_argument('--out', type=Path, default=DEFAULT_SCRATCH / 'rules')
    p.add_argument('--workers', type=int, default=6)
    p.add_argument('--prefix', default='')
    p.add_argument('--timeout', type=float, default=60)
    p.add_argument('--diff-cache', type=Path, help='optional precomputed functionRelocDiffs=none JSONs named main_Game_Unit.json')
    p.add_argument('--rules-dir', type=Path, help='also cross-tab miner lists and list uncovered same-length functions')
    args = p.parse_args()
    if not 1 <= args.workers <= 16: p.error('--workers must be 1..16')
    start = time.monotonic()
    root, base_root = args.root.resolve(), (args.base_root or args.root).resolve()
    report = json.loads(args.report.read_text())
    config = {u['name']: u for u in json.loads((root / 'objdiff.json').read_text())['units']}
    jobs, rows, errors = [], [], []
    for u in report['units']:
        if not u['name'].removeprefix('main/').startswith(args.prefix): continue
        wanted = [f for f in u.get('functions', []) if float(f.get('fuzzy_match_percent', 0)) < 100]
        if not wanted: continue
        if u['name'] in config: jobs.append((config[u['name']], wanted))
        else:
            rows.extend(('unconfigured_unit', u['name'], f['name'], int(f.get('size', 0)), f.get('fuzzy_match_percent', 0), {}) for f in wanted)
    with ThreadPoolExecutor(args.workers) as pool:
        for got, error in pool.map(lambda job: run_unit(root, base_root, *job, args.timeout, args.diff_cache), jobs):
            rows.extend(got)
            if error: errors.append(error)
    rows.sort(key=lambda x: (x[0], x[1], x[2]))
    args.out.mkdir(parents=True, exist_ok=True)
    with (args.out / 'clusters.tsv').open('w') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(['cluster', 'unit', 'function', 'size', 'fuzzy'])
        w.writerows(row[:5] for row in rows)
    count, sizes = Counter(), Counter()
    with (args.out / 'cluster-details.jsonl').open('w') as f:
        for cluster, unit, function, size, fuzzy, detail in rows:
            count[cluster] += 1
            sizes[cluster] += size
            f.write(json.dumps(dict(cluster=cluster, unit=unit, function=function, size=size, fuzzy=fuzzy, **detail)) + '\n')
    summary = 'cluster\tfunctions\tbytes\n' + ''.join(f'{c}\t{count[c]}\t{sizes[c]}\n' for c in sorted(count, key=lambda c: (-sizes[c], c)))
    (args.out / 'cluster-summary.tsv').write_text(summary)
    meta = dict(root=str(root), base_root=str(base_root), report=str(args.report.resolve()), units=len(jobs), functions=len(rows),
                report_sha256=hashlib.sha256(args.report.read_bytes()).hexdigest(),
                scoring_config={'functionRelocDiffs': 'none'}, diff_cache=str(args.diff_cache) if args.diff_cache else None,
                seconds=time.monotonic()-start, errors=errors, source_revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip(),
                base_revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=base_root, text=True).strip())
    if args.rules_dir: meta['uncovered_same_length'] = cross_tab(rows, args.rules_dir, args.out)
    (args.out / 'clusters-meta.json').write_text(json.dumps(meta, indent=2) + '\n')
    print(summary, end='')
    print(f'{len(rows)} functions, {len(count)} clusters, {meta["seconds"]:.2f}s; {len(errors)} errors')
    return int(bool(errors))


if __name__ == '__main__': raise SystemExit(main())
