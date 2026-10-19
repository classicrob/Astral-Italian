#!/usr/bin/env python3
"""Classify REALDIFF-lead.md rows for the requested Game directories.

Strict 'a' and per-definition 'a-rebind' run first; 'c-checked' additionally
proves copy aliases and basic-block scheduling with exact value/effect DAGs.
Unsupported c-checked opcodes, cyclic CFGs, uncertain aliasing and mismatches
remain REAL. Literal bytes and referenced strings are resolved before proof.
Run from the worktree root. --out writes mangled names and certificates.
"""
import sys, os, re, struct, subprocess, collections
ROOT = os.getcwd()
sys.path.insert(0, f"{ROOT}/tools/team")
import sg  # noqa
from rdchecked import checked_equal, machine_lines

args = [a for a in sys.argv[1:] if not a.startswith('--')]
if '--rows' in sys.argv: args.remove(sys.argv[sys.argv.index('--rows') + 1])
out = sys.argv[sys.argv.index('--out') + 1] if '--out' in sys.argv else None
if out in args: args.remove(out)
RD = os.path.expanduser('~/MacioGalaxy2-team/trial/REALDIFF-lead.md')

rows = collections.defaultdict(list)
cur = None
for l in open(RD):
    m = re.match(r'## (\w+):', l)
    if m: cur = m.group(1); continue
    if cur in args and l.startswith('| Game/'):
        c = [x.strip() for x in l.split('|')]
        rows[c[1]].append(c[2].strip('`'))
if '--rows' in sys.argv:
    rows = collections.defaultdict(list)
    for l in open(sys.argv[sys.argv.index('--rows') + 1]):
        if l.strip(): u_, f_ = l.split(None, 1); rows[u_].append(f_.strip())
ONLY = set(os.environ['ONLY'].split(',')) if os.environ.get('ONLY') else None
if ONLY: rows = collections.defaultdict(list, {u: f for u, f in rows.items() if u.split('/')[-1] in ONLY})

# target literal values from the DOL
dol = open(f'{ROOT}/orig/SB4E01/sys/main.dol', 'rb').read()
offs = struct.unpack('>18I', dol[0:72]); addrs = struct.unpack('>18I', dol[72:144]); sizes = struct.unpack('>18I', dol[144:216])
def rd(a, n):
    for o, ad, s in zip(offs, addrs, sizes):
        if ad <= a < ad + s: return dol[o + a - ad:o + a - ad + n]
    return b'\0' * n  # bss
tsym = {}
taddr = {}
for l in open(f'{ROOT}/config/SB4E01/symbols.txt'):
    m = re.match(r'(\S+) = (\.\w+):0x([0-9A-F]+); // type:object size:0x([0-9A-F]+)', l)
    if m and m.group(2) in ('.sdata2', '.rodata', '.sbss2'): tsym[m.group(1)] = (int(m.group(3), 16), int(m.group(4), 16))
    m2 = re.match(r'(\S+) = (\.\w+):0x([0-9A-F]+);', l)
    if m2: taddr[m2.group(1)] = (m2.group(2), int(m2.group(3), 16))
splits = collections.defaultdict(dict); _cu = None
for l in open(f'{ROOT}/config/SB4E01/splits.txt'):
    if l.strip().endswith('.cpp:'): _cu = l.strip()[:-5]; continue
    mm = re.match(r'\s+(\.\w+)\s+start:0x(\w+) end:0x(\w+)', l)
    if mm and _cu: splits[_cu][mm.group(1)] = int(mm.group(2), 16)

def ooffs(obj):
    r = {}
    t = subprocess.run(['build/binutils/powerpc-eabi-objdump', '-t', obj], capture_output=True, text=True).stdout
    for l in t.splitlines():
        m = re.match(r'([0-9a-f]{8}) (.{7}) (\.\w+)\s+([0-9a-f]{8}) (\S+)$', l)
        if m: r[m.group(5)] = (m.group(3), int(m.group(1), 16))
    for sec in ('.data', '.rodata', '.sdata', '.bss', '.sbss'): r.setdefault('...' + sec[1:] + '.0', (sec, 0))
    return r

BLOBS = {}
def secblob(obj, sec):
    k = (obj, sec)
    if k not in BLOBS:
        hx = subprocess.run(['build/binutils/powerpc-eabi-objdump', '-s', '-j', sec, obj], capture_output=True, text=True).stdout
        d = bytearray()
        for hl in hx.splitlines():
            mm = re.match(r' ([0-9a-f]{4,8}) ((?:[0-9a-f]{2,8} ){1,4})', hl + ' ')
            if mm: d += bytes.fromhex(mm.group(2).replace(' ', ''))
        BLOBS[k] = bytes(d)
    return BLOBS[k]

def cstr(b):
    if not b: return None
    i = b.find(b'\0')
    if i < 1: return None
    t = b[:i]
    if all(32 <= c < 127 or c >= 0x80 for c in t): return t
    return None

def osyms(obj):
    """name -> bytes for literal objects in our .o"""
    r = {}
    try:
        t = subprocess.run(['build/binutils/powerpc-eabi-objdump', '-t', obj], capture_output=True, text=True).stdout
        secs = {}
        for l in t.splitlines():
            m = re.match(r'([0-9a-f]{8}) (.{7}) (\.\w+)\s+([0-9a-f]{8}) (\S+)$', l)
            if m and m.group(3) in ('.sdata2', '.rodata', '.sbss2'):
                secs.setdefault(m.group(3), []).append((int(m.group(1), 16), int(m.group(4), 16), m.group(5)))
        for sec, lst in secs.items():
            hx = subprocess.run(['build/binutils/powerpc-eabi-objdump', '-s', '-j', sec, obj], capture_output=True, text=True).stdout
            d = bytearray()
            for hl in hx.splitlines():
                mm = re.match(r' ([0-9a-f]{4,8}) ((?:[0-9a-f]{2,8} ){1,4})', hl + ' ')
                if mm: d += bytes.fromhex(mm.group(2).replace(' ', ''))
            d = bytes(d)
            for o, s, n in lst: r[n] = d[o:o + s] if d else b'\0' * s
    except Exception:
        pass
    return r

LIT = re.compile(r'(lbl_[0-9A-F]{8}|@\d+)(@sda21|@ha|@l)?')
def norm(ins, vals):
    s = re.sub(r'\b0x[0-9a-f]+\(r1\)', 'S(r1)', ins)
    s = re.sub(r'^(addi\s+\w+, r1, )0x[0-9a-f]+$', r'\1S', s)
    s = re.sub(r'\b[rf](?:[02-9]|[12]\d|3[01])\b', 'R', s)
    s = re.sub(r'^(b\w*)\s+(?:cr\d,\s*)?0x[0-9a-f]+$', r'\1 B', s)
    w = 8 if s.startswith(('lfd', 'psq_l')) else 4
    def lit(m):
        v = vals(m.group(1))
        if v is not None: v = v[:w]
        return ('LIT[' + v.hex() + ']' if v is not None else m.group(1)) + (m.group(2) or '')
    return LIT.sub(lit, s)

def litonly(ins, vals):
    w = 8 if ins.startswith(('lfd', 'psq_l')) else 4
    def lit(m):
        v = vals(m.group(1))
        if v is not None: v = v[:w]
        return ('LIT[' + v.hex() + ']' if v is not None else m.group(1)) + (m.group(2) or '')
    return LIT.sub(lit, ins)

TOK = re.compile(r'(0x[0-9a-f]+\(r1\)|\b[rf]\d+\b|[^\s,()]+|[(),])')
def strict_equal(L, R):
    """same order; consistent bijective maps for GPR, FPR and r1 stack offsets"""
    if len(L) != len(R): return False, 'count'
    mp = {}; rv = {}
    def bind(a, b):
        if mp.get(a, b) != b or rv.get(b, a) != a: return False
        mp[a] = b; rv[b] = a; return True
    for k, (a, b) in enumerate(zip(L, R)):
        ta, tb = TOK.findall(a), TOK.findall(b)
        if len(ta) != len(tb): return False, f'@{k*4:#x}: {a} / {b}'
        # addi rX, r1, IMM is a stack address: treat IMM as a stack slot
        sa = ta[0] == 'addi' and len(ta) >= 6 and ta[3] == 'r1'
        for i, (x, y) in enumerate(zip(ta, tb)):
            if x == y and not re.fullmatch(r'[rf]\d+|0x[0-9a-f]+\(r1\)', x) and not (sa and i == 5): continue
            if re.fullmatch(r'r(1|2|13)', x) or re.fullmatch(r'r(1|2|13)', y):
                if x != y: return False, f'@{k*4:#x}: {a} / {b}'
                continue
            if (re.fullmatch(r'r([3-9]|10)', x) or re.fullmatch(r'r([3-9]|10)', y) or re.fullmatch(r'f[1-8]', x) or re.fullmatch(r'f[1-8]', y)) and x != y:
                return False, f'@{k*4:#x} ABI arg/return register renamed: {a} / {b}'
            if re.fullmatch(r'r\d+', x) and re.fullmatch(r'r\d+', y):
                if not bind('g' + x, 'g' + y): return False, f'@{k*4:#x} reg map: {a} / {b}'
            elif re.fullmatch(r'f\d+', x) and re.fullmatch(r'f\d+', y):
                if not bind('f' + x, 'f' + y): return False, f'@{k*4:#x} fpr map: {a} / {b}'
            elif re.fullmatch(r'0x[0-9a-f]+\(r1\)', x) and re.fullmatch(r'0x[0-9a-f]+\(r1\)', y):
                if not bind('s' + x[:-4], 's' + y[:-4]): return False, f'@{k*4:#x} stack map: {a} / {b}'
            elif sa and i == 5 and x.startswith('0x') and y.startswith('0x'):
                if not bind('s' + x, 's' + y): return False, f'@{k*4:#x} stack map: {a} / {b}'
            else:
                return False, f'@{k*4:#x}: {a} / {b}'
    return True, ''

REG = re.compile(r'[rf]\d+$')
COMM = ('fadds', 'fadd', 'add', 'mullw', 'and', 'or', 'xor')  # fmul/fmuls/fmadd NOT commutative on Gekko (C operand precision)
NODEF = ('st', 'cmp', 'b', 'mt', 'tw', 'dcb', 'icb', 'sync', 'isync', 'fcmp', 'psq_st')
VOL = {'gr0', 'gr11', 'gr12', 'ff0', 'ff9', 'ff10', 'ff11', 'ff12', 'ff13'}
def fixed(x):
    return bool(re.fullmatch(r'r(1|2|13|[3-9]|10)|f[1-8]', x))
def rebind_equal(L, R):
    """same order; per-definition register renaming (a value's register may differ per live range), checked at every use,
    with mappings reconciled at branch targets; ABI r1/r2/r13/r3-r10/f1-f8 identical; stack slots one bijection;
    commutative ops may swap their two source operands."""
    if len(L) != len(R): return False, 'count'
    mp, rv, smp, srv = {}, {}, {}, {}
    snaps = collections.defaultdict(list)
    dead = False
    def key(x): return ('g' if x[0] == 'r' else 'f') + x
    def use(x, y):
        if x == y and fixed(x): return True
        if fixed(x) or fixed(y): return False
        if x[0] != y[0]: return False
        kx, ky = key(x), key(y)
        if kx not in mp and ky not in rv:   # live-in (callee-saved/value from entry): bind once
            mp[kx] = ky; rv[ky] = kx; return True
        return mp.get(kx) == ky
    def define(x, y):
        if x == y and fixed(x): return True
        if fixed(x) or fixed(y) or x[0] != y[0]: return False
        kx, ky = key(x), key(y)
        if kx in mp: rv.pop(mp.pop(kx), None)
        if ky in rv: mp.pop(rv.pop(ky), None)
        mp[kx] = ky; rv[ky] = kx; return True
    def sbind(a, b):
        if smp.get(a, b) != b or srv.get(b, a) != a: return False
        smp[a] = b; srv[b] = a; return True
    for k, (a, b) in enumerate(zip(L, R)):
        if k in snaps:
            states = snaps[k] + ([] if dead else [(dict(mp), dict(rv))])
            m0 = states[0][0]
            for m, _ in states[1:]:
                for kk in set(m) & set(m0):
                    if m[kk] != m0[kk]: return False, f'@{k*4:#x} join mapping conflict ({kk})'
            mp = dict(m0); rv = {v: kk for kk, v in mp.items()}
            for m, _ in states[1:]:
                for kk, v in m.items():
                    if kk not in mp and v not in rv: mp[kk] = v; rv[v] = kk
        dead = False
        ta, tb = TOK.findall(a), TOK.findall(b)
        if len(ta) != len(tb) or ta[0] != tb[0]: return False, f'@{k*4:#x}: {a} / {b}'
        op = ta[0]
        sa = op == 'addi' and len(ta) >= 6 and ta[3] == 'r1'
        regs = [(i, x, y) for i, (x, y) in enumerate(zip(ta, tb)) if REG.match(x) or REG.match(y)]
        # non-register tokens must be equal (stack slots via bijection)
        for i, (x, y) in enumerate(zip(ta, tb)):
            if REG.match(x) or REG.match(y): continue
            if re.fullmatch(r'0x[0-9a-f]+\(r1\)', x) and re.fullmatch(r'0x[0-9a-f]+\(r1\)', y):
                if not sbind(x, y): return False, f'@{k*4:#x} stack map: {a} / {b}'
            elif sa and i == 5:
                if not sbind(x, y): return False, f'@{k*4:#x} stack map: {a} / {b}'
            elif x != y: return False, f'@{k*4:#x}: {a} / {b}'
        isdef = regs and not op.startswith(NODEF) and regs[0][0] == 1
        srcs = regs[1:] if isdef else regs
        ok = all(use(x, y) for _, x, y in srcs)
        if not ok and op in COMM and len(srcs) == 2:
            (_, x1, y1), (_, x2, y2) = srcs
            ok = use(x1, y2) and use(x2, y1)
        if not ok: return False, f'@{k*4:#x} use map: {a} / {b}'
        if isdef and not define(regs[0][1], regs[0][2]): return False, f'@{k*4:#x} def map: {a} / {b}'
        if op in ('bl', 'blrl', 'bctrl'):
            for kk in list(mp):
                if kk in VOL: rv.pop(mp.pop(kk), None)
        m = re.match(r'^b\w*\s+(?:cr\d,\s*)?(0x[0-9a-f]+)$', a)
        if m and not op.startswith('bl'):
            snaps[int(m.group(1), 16) // 4].append((dict(mp), dict(rv)))
        if op in ('b', 'blr', 'bctr'): dead = True
    return True, ''

benign, real = [], []
checked_evidence = []
for unit, fns in sorted(rows.items()):
    u = unit[len('Game/'):] if False else unit
    d = None
    for _try in range(4):
        try:
            d, _ = sg.build_and_diff(sg.norm_unit(unit)); break
        except SystemExit as e:
            err = e; break
        except Exception as e:
            err = e; import time; time.sleep(3)
    if d is None:
        real.extend((unit, f, f'build failed: {err}') for f in fns); continue
    obj = f'build/SB4E01/src/{unit}.o'
    ov = osyms(obj); oo = ooffs(obj)
    def tval(n):
        if n in tsym: a, s = tsym[n]; return rd(a, min(s, 8))
        return None
    def oval(n):
        return ov[n][:8] if n in ov else None
    left, right = d['left']['symbols'], d['right']['symbols']
    for f in fns:
        L = next((s for s in left if s['name'] == f), None)
        R = next((s for s in right if s['name'] == f), None)
        if not L or not R or not R.get('instructions'):
            real.append((unit, f, 'not compiled/paired')); continue
        bases = {'L': int(L.get('address', 0)), 'R': int(R.get('address', 0))}
        def dfmt(i, side):
            t = dfmt0(i, side)
            ins = i.get('instruction', i) if isinstance(i, dict) else {}
            bd = ins.get('branch_dest')
            if bd is not None and re.search(r'0x[0-9a-f]+$', t):
                t = re.sub(r'0x[0-9a-f]+$', hex(int(bd) - bases[side]), t)  # branch target relative to function start
            return t
        def dfmt0(i, side):
            t = sg.fmt(i)
            rel = i.get('instruction', i).get('relocation') if isinstance(i, dict) else None
            if not rel: return t
            add = int(rel.get('addend', 0) or 0)
            mm = re.search(r'(lbl_[0-9A-F]{8}|\.\.\.\w+\.0|@\d+)(@ha|@l|@sda21)?', t)
            if not mm: return t
            n = mm.group(1)
            DS = ('.data', '.rodata', '.sdata', '.bss', '.sbss', '.sdata2')
            if side == 'L' and n in tsym and not add: return t   # target literal: value-normalized later
            if side == 'R' and n in ov and not add and not n.startswith('...'): return t
            if side == 'L' and n in taddr and taddr[n][0] in DS:
                sec, a = taddr[n]
                st = cstr(rd(a + add, 256)) if sec != '.bss' else None
                if st: return t.replace(n, 'STR[' + st.hex() + ']', 1)
                base = splits.get(unit, {}).get(sec)
                if base is not None: return t.replace(n, f'{sec}+{a - base + add:#x}', 1)
            if side == 'R' and n in oo and oo[n][0] in DS:
                sec, a = oo[n]
                st = cstr(secblob(obj, sec)[a + add:a + add + 256]) if sec not in ('.bss', '.sbss') else None
                if st: return t.replace(n, 'STR[' + st.hex() + ']', 1)
                return t.replace(n, f'{sec}+{a + add:#x}', 1)
            return t
        la = [litonly(dfmt(i, 'L'), tval) for i in L['instructions']]
        ra = [litonly(dfmt(i, 'R'), oval) for i in R['instructions']]
        ok, why = strict_equal(la, ra)
        if ok:
            benign.append((unit, f, f'a: {len(la)} instrs, same order, consistent bijective GPR/FPR/stack-slot mapping (ABI r3-r10/f1-f8 identical), literal values + referenced strings equal')); continue
        ok2, why2 = rebind_equal(la, ra)
        if ok2:
            benign.append((unit, f, f'a-rebind: {len(la)} instrs, same order and opcodes; per-definition register renaming checked at every use and reconciled at branch joins (ABI r1/r2/r13/r3-r10/f1-f8 identical, volatile regs dropped at calls), one stack-slot bijection, commutative source swaps only, literal values + referenced strings equal')); continue
        # objdiff inserts alignment placeholders; they are not machine instructions.
        lc = machine_lines(L['instructions'], la)
        rc = machine_lines(R['instructions'], ra)
        ok3, why3 = checked_equal(lc, rc)
        if ok3:
            benign.append((unit, f, 'c-checked: ' + why3))
            checked_evidence.append({'unit': unit, 'name': f, 'left': lc, 'right': rc, 'certificate': why3})
            continue
        li = [norm(dfmt(i, 'L'), tval) for i in L['instructions']]
        ri = [norm(dfmt(i, 'R'), oval) for i in R['instructions']]
        if len(li) != len(ri):
            real.append((unit, f, f'instruction count {len(li)} vs {len(ri)}')); continue
        cl, cr = collections.Counter(li), collections.Counter(ri)
        if cl == cr:
            real.append((unit, f, f'REORDER-ONLY (not proven): c-checked {why3}; strict {why}'))
        else:
            dl = list((cl - cr).elements())[:2]; dr = list((cr - cl).elements())[:2]
            real.append((unit, f, f'target-only {dl} ours-only {dr}'))

lines = [f'{f}  # proof: c-checked: rdclass {u}: {p.removeprefix("c-checked: ")}' if p.startswith('c-checked: ') else f'{f}  # proof: rdclass.py {u}: {p}' for u, f, p in benign]
if out:
    with open(out, 'w') as fh: fh.write('\n'.join(lines) + '\n')
    import json
    with open(out + '.c-checked.json', 'w') as fh: json.dump(checked_evidence, fh, indent=2)
print(f'benign {len(benign)}  real {len(real)}')
print('classes ' + str(dict(collections.Counter(p.split(':', 1)[0] for _, _, p in benign))))
for u, f, p in real: print(f'REAL {u} {f}: {p}')
