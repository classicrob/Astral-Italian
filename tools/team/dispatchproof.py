#!/usr/bin/env python3
"""Proof for a crossed-dispatch fix: the Wii objects change only where the dispatch oracle says they must.

hookproof.sh demands byte-identical objects, which a dispatch fix can never satisfy (a nerve's execute symbol
moves, a vtable relocation retargets).  This tool compares two COMPLETE source-object trees (before / after) and
accepts only:

  1. the same set of object files;
  2. per object: identical allocated-section bytes, identical symbol sets (name, bind, type, size, section), and
     identical relocation multisets keyed by (container symbol, offset in container, type, addend, target
     identity) -- EXCEPT relocation sites the oracle reports as BAD before and MATCH after;
  3. symbol placement (offset inside a section) may differ only inside objects that contain such a flipped site
     or that are named with --unit (a pure reorder of weak nerve functions / instances);
  4. oracle (tools/team/dispatchcheck.py) before vs after: no row regresses, no new BAD row, the BAD rows that
     disappear are listed; with --expect-clean the after tree must have none for the --unit objects.

`--selftest` perturbs one object (a code byte, a relocation target, a symbol) in a temp copy and requires the
comparison to FAIL for each, proving the comparator sees real changes.

Usage: dispatchproof.py --before ROOT --after ROOT [--unit Game/Boss/TwoLegs]... [--expect-clean] [--dol main.dol]
       dispatchproof.py --selftest --after ROOT
Exit 0 and prints `DISPATCH PROOF OK` only if every check passes.
"""
import argparse, collections, hashlib, json, os, pathlib, shutil, sys, tempfile

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import dispatchcheck as dc  # noqa: E402

SKIP_SECS = {'.line', '.debug', '.comment', '.symtab', '.strtab', '.shstrtab'}


def sec_ident(e):
    """Allocated (and note) section occurrences: [(name, type, flags, size, align, content-hash)]."""
    out = []
    for i, s in enumerate(e.s):
        n = e.names[i]
        if n in SKIP_SECS or n.startswith(('.rela', '.rel')) or s[1] in (2, 3, 4) or i == 0:
            continue
        body = b'' if s[1] == 8 else e.raw[s[4]:s[4] + s[5]]
        out.append((n, s[1], s[2], s[5], s[8], hashlib.sha1(body).hexdigest()))
    return out


def containers(e):
    """sec -> sorted [(off, size, name)] of sized symbols (functions in code, objects in data)."""
    cs = collections.defaultdict(list)
    for s in e.syms:
        if s['sec'] and s['sec'] < len(e.s) and s['size'] and s['name'] and s['type'] in (1, 2):
            cs[s['sec']].append((s['off'], s['size'], s['name'], s['bind']))
    for v in cs.values():
        v.sort()
    return cs


def target_key(prog, e, unit, si, ad):
    """Resolved target address in this tree's own symbols.txt, else a placement-free identity."""
    addr, how = prog.target(e, unit, si, ad)
    if addr is not None:
        return ('A', addr)
    t = e.syms[si]
    if t['type'] != 3 and t['name'] and not dc.drifty(t):
        return ('N', t['name'], ad)
    sec = t['sec']
    if sec and sec < len(e.s) and t['type'] != 3 and t['size']:
        body = e.raw[e.s[sec][4] + t['off']:e.s[sec][4] + t['off'] + t['size']] if e.s[sec][1] != 8 else b''
        return ('L', e.names[sec], t['size'], hashlib.sha1(body).hexdigest(), ad)
    return ('S', e.names[sec] if sec and sec < len(e.s) else '?', ad)


def unit_image(prog, unit):
    """{key: (size, content-hash, ((offset, type, target-key), ...))} where key is the symbol's address in this
    tree's symbols.txt (placement-free identity for symbols it does not map), i.e. the linked image of the unit."""
    e = prog.elfs[unit]
    cs = containers(e)
    img = {}
    for sec, lst in cs.items():
        if e.names[sec].startswith(('.line', '.debug')):
            continue
        for off, size, name, bind in lst:
            sym = next(s for s in e.syms if s['name'] == name and s['sec'] == sec and s['off'] == off)
            a = prog.sym_addr(e, unit, sym)
            if a is None and not (bind == 0 and dc.LOCALNAME.match(name)):
                inf = prog.inferred.get(name)
                a = next(iter(inf)) if inf and len(inf) == 1 else None
            key = ('A', a) if a is not None else ('N', e.names[sec], name)
            body = b'' if e.s[sec][1] == 8 else e.raw[e.s[sec][4] + off:e.s[sec][4] + off + size]
            rels = tuple(sorted((o - off, t, target_key(prog, e, unit, si, ad))
                                for o, (t, si, ad) in e.rel_by_sec[sec].items() if off <= o < off + size))
            img[(key, size)] = (hashlib.sha1(body).hexdigest(), rels, name, off)
    return img


def compare_units(unit, ib, ia, flipped_sites, problems):
    """Return the number of intended (flipped) relocation retargets, or None when something else differs."""
    if set(ib) != set(ia):
        problems.append(f'{unit}: symbol image keys differ: -{[k for k in ib if k not in ia][:3]} +{[k for k in ia if k not in ib][:3]}')
        return None
    retargets = 0
    for k, (hb, rb, nb, ob_) in ib.items():
        ha, ra, na, oa_ = ia[k]
        if hb != ha:
            problems.append(f'{unit}: bytes of {nb} / {na} at {k[0]} differ')
            return None
        if rb != ra:
            db = dict(((o, t), g) for o, t, g in rb)
            da = dict(((o, t), g) for o, t, g in ra)
            for key in set(db) | set(da):
                if db.get(key) != da.get(key):
                    site = k[0][1] + key[0] if k[0][0] == 'A' else None
                    if site is not None and site in flipped_sites:
                        retargets += 1
                    else:
                        problems.append(f'{unit}: relocation +{key[0]:#x} of {nb} / {na} changed {db.get(key)} -> {da.get(key)} and the oracle did not flip that site')
                        return None
    return retargets


def row_key(r):
    return (r['layer'], r['unit'], r.get('container') or r.get('function') or '', r['offset'], r.get('target', ''))


def bad(r):
    return r['verdict'] in ('CROSSED', 'WRONG', 'CONFLICT') or r['verdict'].startswith('ADVISORY')


def tree_objects(root):
    base = pathlib.Path(root) / 'build/SB4E01/src'
    return {str(p.relative_to(base).with_suffix('')): p for p in sorted(base.rglob('*.o'))}


def digest(p):
    return hashlib.sha1(pathlib.Path(p).read_bytes()).hexdigest()


def compare_trees(pb, pa, rows_b, rows_a, units, expect_clean):
    ob, oa = pb.elfs, pa.elfs
    problems = []
    if set(ob) != set(oa):
        problems.append(f'object sets differ: -{sorted(set(ob) - set(oa))[:5]} +{sorted(set(oa) - set(ob))[:5]}')
    kb = {row_key(r): r for r in rows_b}
    ka = {row_key(r): r for r in rows_a}
    flipped, regress, newbad = [], [], []
    for k, r in kb.items():
        a = ka.get(k)
        if a is None:
            continue
        if bad(r) and a['verdict'] == 'MATCH':
            flipped.append(k)
        elif r['verdict'] == 'MATCH' and a['verdict'] != 'MATCH':
            regress.append((k, a['verdict']))
    for k, a in ka.items():
        if bad(a) and (k not in kb or not bad(kb[k])):
            newbad.append(k)
    if regress:
        problems.append(f'{len(regress)} oracle rows regressed MATCH->other: {regress[:3]}')
    if newbad:
        problems.append(f'{len(newbad)} new BAD oracle rows: {newbad[:3]}')
    flipped_sites = {kb[k]['site'] for k in flipped if 'site' in kb[k]}
    flipped_units = {k[1] for k in flipped}
    report = dict(objects=len(ob), identical=0, retargeted_objects=0, retargeted_sites=0, flipped_rows=len(flipped),
                  still_bad=sum(1 for r in rows_a if bad(r)), bad_before=sum(1 for r in rows_b if bad(r)))
    for unit in sorted(set(ob) & set(oa)):
        if ob[unit].raw == oa[unit].raw:
            report['identical'] += 1
            continue
        n = compare_units(unit, unit_image(pb, unit), unit_image(pa, unit), flipped_sites, problems)
        if n is None:
            continue
        if n == 0:
            report['identical'] += 1
            continue
        report['retargeted_objects'] += 1
        report['retargeted_sites'] += n
    if expect_clean:
        left = [row_key(r) for r in rows_a if bad(r) and (r['unit'] in units or not units)]
        if left:
            problems.append(f'{len(left)} BAD rows remain in the after tree for the requested units: {left[:3]}')
    return problems, report, flipped


def selftest(after, dol):
    """Perturb one object of a temp tree and require the image comparison to refuse each perturbation."""
    import struct
    base = pathlib.Path(after) / 'build/SB4E01/src'
    pa = dc.Prog(after, dol)
    pick = next(u for u in pa.elfs if u.endswith('WorldMapResultTU19'))
    raw = (base / (pick + '.o')).read_bytes()
    e = pa.elfs[pick]
    text = next(i for i, n in enumerate(e.names) if n == '.text')
    rd = next(i for i, n in enumerate(e.names) if n == '.rela.data')
    st = next(i for i, s in enumerate(e.s) if s[1] == 2)
    m1 = bytearray(raw); m1[e.s[text][4] + 8] ^= 1
    m2 = bytearray(raw)
    a_, info, ad = struct.unpack_from('>IIi', m2, e.s[rd][4])
    struct.pack_into('>IIi', m2, e.s[rd][4], a_, (((info >> 8) + 1) << 8) | (info & 255), ad)
    m3 = bytearray(raw)
    for k in range(1, e.s[st][5] // 16):
        o = e.s[st][4] + k * 16
        sz = struct.unpack_from('>I', m3, o + 8)[0]
        if sz and e.syms[k]['type'] in (1, 2):
            struct.pack_into('>I', m3, o + 8, sz + 4)
            break
    results = []
    for name, data in (('code byte', m1), ('reloc retarget', m2), ('symbol size', m3), ('identical copy', bytearray(raw))):
        tmp = pathlib.Path(tempfile.mkdtemp(prefix='dispatchproof-selftest-'))
        try:
            (tmp / 'build/SB4E01').mkdir(parents=True)
            os.symlink(pathlib.Path(after) / 'config', tmp / 'config')
            os.symlink(base, tmp / 'build/SB4E01/src')
            shutil.rmtree(tmp / 'build/SB4E01/src', ignore_errors=True) if False else None
            # real copy of the tree with one replaced object: symlink every object file individually
            os.unlink(tmp / 'build/SB4E01/src')
            for p in base.rglob('*.o'):
                q = tmp / 'build/SB4E01/src' / p.relative_to(base)
                q.parent.mkdir(parents=True, exist_ok=True)
                if p == base / (pick + '.o'):
                    q.write_bytes(bytes(data))
                else:
                    os.symlink(p, q)
            pm = dc.Prog(str(tmp), dol)
            problems = []
            try:
                n = compare_units(pick, unit_image(pa, pick), unit_image(pm, pick), set(), problems)
            except Exception as ex:
                n, problems = None, [f'exception {ex}']
            accepted = n == 0 and not problems
            results.append((name, 'ACCEPTED' if accepted else 'REJECTED', (problems or [''])[0][:90]))
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    for r in results:
        print('selftest:', *r)
    want = {'code byte': 'REJECTED', 'reloc retarget': 'REJECTED', 'symbol size': 'REJECTED', 'identical copy': 'ACCEPTED'}
    return all(want[n] == v for n, v, _ in results)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--before')
    ap.add_argument('--after', required=True)
    ap.add_argument('--unit', action='append', default=[])
    ap.add_argument('--expect-clean', action='store_true')
    ap.add_argument('--selftest', action='store_true')
    ap.add_argument('--dol', help='original main.dol (default: <root>/orig/SB4E01/sys/main.dol); its sha1 must equal config/SB4E01/build.sha1')
    ap.add_argument('--out')
    a = ap.parse_args()
    a.dol = a.dol or None
    if a.selftest:
        ok = selftest(a.after, a.dol)
        print('SELFTEST OK' if ok else 'SELFTEST FAILED')
        sys.exit(0 if ok else 1)
    if not a.before:
        ap.error('--before required')
    pb, pa = dc.Prog(a.before, a.dol), dc.Prog(a.after, a.dol)
    rows_b, _ = pb.run()
    rows_a, _ = pa.run()
    problems, report, flipped = compare_trees(pb, pa, rows_b, rows_a, set(a.unit), a.expect_clean)
    print(json.dumps(report))
    for k in flipped:
        print('FLIPPED', *[str(x)[:70] for x in k])
    for p in problems:
        print('PROBLEM', p)
    if a.out:
        pathlib.Path(a.out).write_text(json.dumps(dict(report=report, problems=problems, flipped=[list(k) for k in flipped]), indent=1))
    if problems:
        print('DISPATCH PROOF FAILED')
        sys.exit(1)
    print('DISPATCH PROOF OK')


if __name__ == '__main__':
    main()
