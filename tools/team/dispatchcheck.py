#!/usr/bin/env python3
"""Crossed-dispatch oracle: compare what our compiled SOURCE objects bind against the ORIGINAL DOL.

Byte matching cannot see a vtable slot / nerve / callback bound to the wrong function when the data it lives in
is not owned by a split unit, and objdiff cannot see a wrong *address* that carries a plausible *name*.  This tool
reads the original DOL directly and checks, per site, that the address the DOL really uses is the address of the
symbol our source binds there:

  DATA  every ADDR32 relocation inside an allocated data section of every source object (vtable slots, function
        tables, PMFs): the DOL word at the containing data symbol's address must be the target's address.
  CODE  every relocation in a source function whose bytes equal the DOL function's bytes (relocated fields masked):
        the bl/b target, HA/LO or SDA21 operand the DOL instruction uses must be the target's address.

Target/container addresses come from symbols.txt; symbols it does not name are INFERRED from DOL instructions in
exact functions that reference them (bl, HA+LO, SDA21), and a function target nobody names is verified by BODY: the
source body must equal the DOL body at the slot's address and its own relocs must resolve there.

Verdicts: MATCH | MATCH_BODY | CROSSED (DOL points at another symbol we name) | WRONG | ALIAS (same name, another
copy) | LOCALDRIFT:* (anchored only by unit-local section offsets; advisory) | UNRESOLVED:<reason> | CONFLICT.
Row field `kind`: dispatch (function/vtable/nerve-instance target) or data (other objects).
Read-only: objects, config/SB4E01/{symbols,splits}.txt, the DOL.

Usage: dispatchcheck.py [--root TREE] [--dol main.dol] [--prefix Game/X/] [--out rows.jsonl]
                        [--allow allow.txt] [--strict] [--quiet]
  --allow FILE  lines "<unit> <container-or-function> <offset-hex>" of reviewed non-bugs
  --strict      exit 1 if any CROSSED/WRONG/CONFLICT row of kind dispatch is not allow-listed
"""
import argparse, bisect, collections, json, pathlib, re, struct, sys

SDA13 = 0x807D7320  # r13 / r2 as set by __start in the original DOL
SDA2 = 0x807DECA0
R_ADDR32, R_ADDR16, R_ADDR16_LO, R_ADDR16_HI, R_ADDR16_HA, R_REL24, R_REL14, R_SDA21 = 1, 3, 4, 5, 6, 10, 11, 109
LOCALNAME = re.compile(r'^(@\d+|lbl_.*|fn_.*|gap_.*|\$\d+)$')
MASK = {R_REL24: 0x03FFFFFC, R_REL14: 0x0000FFFC, R_ADDR16_LO: 0xFFFF, R_ADDR16_HA: 0xFFFF, R_ADDR16: 0xFFFF,
        R_ADDR16_HI: 0xFFFF, R_SDA21: 0x001FFFFF}


def u32(b, o=0):
    return struct.unpack_from('>I', b, o)[0]


def s16(n):
    return n - 65536 if n & 0x8000 else n


class Dol:
    def __init__(self, path):
        self.raw = pathlib.Path(path).read_bytes()
        self.secs = [(u32(self.raw, 0x48 + i * 4), u32(self.raw, 0x90 + i * 4), u32(self.raw, i * 4))
                     for i in range(18) if u32(self.raw, 0x90 + i * 4)]

    def bytes(self, addr, n):
        for a, z, off in self.secs:
            if a <= addr and addr + n <= a + z:
                return self.raw[off + addr - a:off + addr - a + n]
        return None

    def word(self, addr):
        b = self.bytes(addr, 4)
        return None if b is None else u32(b)


class Symbols:
    def __init__(self, path):
        self.by_name, self.dups = {}, set()
        self.by_addr = collections.defaultdict(list)
        for line in pathlib.Path(path).read_text().splitlines():
            m = re.match(r'(.+?) = (\.\w+):0x([\da-fA-F]+);(.*)', line)
            if not m:
                continue
            name, sec, a, meta = m.groups()
            a = int(a, 16)
            z = re.search(r'\bsize:0x([\da-fA-F]+)', meta)
            t = re.search(r'\btype:(\w+)', meta)
            if name in self.by_name:
                self.dups.add(name)
            self.by_name[name] = (a, int(z[1], 16) if z else 0, sec, t[1] if t else '')
            self.by_addr[a].append(name)
        self.addrs = sorted(self.by_addr)

    def addr(self, name):
        return None if name in self.dups or name not in self.by_name else self.by_name[name][0]

    def name_at(self, addr):
        if addr in self.by_addr:
            return '/'.join(self.by_addr[addr])
        i = bisect.bisect_right(self.addrs, addr) - 1
        if i >= 0:
            a = self.addrs[i]
            for n in self.by_addr[a]:
                if a <= addr < a + max(self.by_name[n][1], 1):
                    return f'{n}+0x{addr - a:X}'
        return None


class ELF:
    def __init__(self, path):
        raw = self.raw = pathlib.Path(path).read_bytes()
        h = struct.unpack_from('>16sHHIIIIIHHHHHH', raw)
        if h[0][:6] != b'\x7fELF\x01\x02':
            raise ValueError('not ELF32 BE')
        self.s = [struct.unpack_from('>IIIIIIIIII', raw, h[6] + i * h[11]) for i in range(h[12])]
        strs = raw[self.s[h[13]][4]:self.s[h[13]][4] + self.s[h[13]][5]]
        self.names = [strs[s[0]:strs.find(b'\0', s[0])].decode() for s in self.s]
        self.syms = []
        for s in self.s:
            if s[1] == 2:
                st = raw[self.s[s[6]][4]:self.s[s[6]][4] + self.s[s[6]][5]]
                for off in range(0, s[5], 16):
                    n, a, z, info, _o, sec = struct.unpack_from('>IIIBBH', raw, s[4] + off)
                    self.syms.append(dict(name=st[n:st.find(b'\0', n)].decode(errors='replace'), off=a, size=z,
                                          bind=info >> 4, type=info & 15, sec=sec))
        self.relocs = []  # (target_sec, off, type, symidx, addend)
        for s in self.s:
            if s[1] == 4:
                for off in range(0, s[5], 12):
                    a, info, add = struct.unpack_from('>IIi', raw, s[4] + off)
                    self.relocs.append((s[7], a, info & 255, info >> 8, add))
        self.rel_by_sec = collections.defaultdict(dict)
        self.rel_collisions = set()
        for sec, off, typ, symidx, addend in self.relocs:
            if typ in (R_ADDR16, R_ADDR16_LO, R_ADDR16_HI, R_ADDR16_HA) and self.is_code(sec):
                off &= ~3  # halfword relocations point at instruction+2: key them by the instruction
            if off in self.rel_by_sec[sec]:
                self.rel_collisions.add((sec, off))
            self.rel_by_sec[sec][off] = (typ, symidx, addend)

    def content(self, sec, off, n):
        s = self.s[sec]
        return self.raw[s[4] + off:s[4] + off + n]

    def is_code(self, sec):
        return 0 < sec < len(self.s) and bool(self.s[sec][2] & 4)

    def is_data(self, sec):
        return 0 < sec < len(self.s) and bool(self.s[sec][2] & 2) and not (self.s[sec][2] & 4) and self.s[sec][1] != 8


def drifty(sym):
    return sym['bind'] == 0 and bool(LOCALNAME.match(sym['name']) or sym['type'] == 3)


def load_splits(path):
    bases, ends, unit = {}, {}, None
    for line in pathlib.Path(path).read_text().splitlines():
        if line and not line[0].isspace() and line.endswith(':'):
            unit = re.sub(r'\.(cpp|c)$', '', line[:-1])
        m = re.search(r'^\s+(\.\w+)\s+start:0x([\da-fA-F]+)\s+end:0x([\da-fA-F]+)', line)
        if m and unit:
            bases[unit, m[1]] = int(m[2], 16)
            ends[unit, m[1]] = int(m[3], 16)
    return bases, ends


def mangled_class(m):
    """CodeWarrior mangled class qualifier of `<qualified-name>F<args>` -> the same text without the argument list."""
    i, n = 0, 0
    if m.startswith('Q'):
        n, i = int(m[1]), 2  # Q<single digit>
    else:
        n = 1
    for _ in range(n):
        j = i
        while j < len(m) and m[j].isdigit():
            j += 1
        if j == i:
            return None
        ln = int(m[i:j]); i = j + ln
    return m[:i] if i <= len(m) else None


def is_dispatch_name(n):
    return n.startswith(('__vt__', 'sInstance__', 'execute__', 'executeOnEnd__'))


def original_dol(root, dolpath=None):
    """Locate the original DOL and verify it is the retail one (sha1 == config/SB4E01/build.sha1)."""
    import hashlib
    root = pathlib.Path(root)
    path = pathlib.Path(dolpath) if dolpath else root / 'orig/SB4E01/sys/main.dol'
    want = (root / 'config/SB4E01/build.sha1').read_text().split()[0]
    got = hashlib.sha1(path.read_bytes()).hexdigest()
    if got != want:
        sys.exit(f'{path}: sha1 {got} is not the original DOL ({want})')
    return str(path)


class Prog:
    def __init__(self, root, dolpath=None):
        self.root = pathlib.Path(root)
        dolpath = original_dol(root, dolpath)
        self.sy = Symbols(self.root / 'config/SB4E01/symbols.txt')
        self.dol = Dol(dolpath)
        self.bases, self.ends = load_splits(self.root / 'config/SB4E01/splits.txt')
        base = self.root / 'build/SB4E01/src'
        self.elfs, self.errors = {}, []
        for p in sorted(base.rglob('*.o')):
            unit = str(p.relative_to(base).with_suffix(''))
            try:
                self.elfs[unit] = ELF(p)
            except Exception as ex:
                self.errors.append((unit, str(ex)))
        self.inferred = collections.defaultdict(lambda: collections.defaultdict(list))
        self.func_info = {}
        self.exact_stream = []
        self.nonexact = []
        self._chains = None
        self.ctor_inst = {}

    # ---- anchors -----------------------------------------------------------------------------------------------
    def sym_addr(self, e, unit, sym):
        """symbols.txt address of an object symbol (None if not mapped), without any addend."""
        if sym['type'] == 3:
            return None
        name = sym['name']
        if sym['bind'] == 0:
            if LOCALNAME.match(name):
                return None
            a = self.sy.addr(name)
            if a is None:
                return None
            # a unit-local symbol of a common name (static inline, anonymous namespace) is the unit's own only if
            # its address lies in the unit's section
            sec = e.names[sym['sec']] if 0 < sym['sec'] < len(e.s) else None
            lo, hi = self.bases.get((unit, sec)), self.ends.get((unit, sec))
            if sym['sec'] and lo is not None and not (lo <= a < hi):
                return None
            return a
        return self.sy.addr(name)

    def local_addr(self, e, unit, sym, addend=0):
        """unit-section-offset anchored address for object-local symbols (advisory)."""
        sec = sym['sec']
        if sec and sec < len(e.s):
            b = self.bases.get((unit, e.names[sec]))
            if b is not None:
                return b + (0 if sym['type'] == 3 else sym['off']) + addend
        return None

    def target(self, e, unit, symidx, addend):
        """-> (address or None, how) with how in sym|inferred|local|<reason>"""
        t = e.syms[symidx]
        a = self.sym_addr(e, unit, t)
        if a is not None:
            return a + addend, 'sym'
        if t['name'].startswith('sInstance__') and 'Nrv' in t['name'] and t['bind'] != 0:
            # independent anchor: the instance the original's sinit hands to the constructor symbols.txt names for this class
            ka = self.sy.addr('__ct__' + t['name'][len('sInstance__'):] + 'Fv')
            if ka is not None and ka in self.ctor_inst:
                return self.ctor_inst[ka] + addend, 'chain'
        inf = self.inferred.get(t['name']) if not drifty(t) and t['type'] != 3 and t['bind'] != 0 else None
        if inf and len(inf) == 1:
            return next(iter(inf)) + addend, 'inferred'
        if drifty(t) or t['type'] == 3 or (t['bind'] == 0 and t['sec']):
            la = self.local_addr(e, unit, t, addend)
            if la is not None:
                return la, 'local'
            return None, 'local_target_unmapped'
        if inf:
            return None, 'inferred_conflict'
        if t['name'] in self.sy.dups:
            return None, 'target_name_ambiguous'
        return None, 'target_unmapped'

    def func_anchor(self, unit, e, f):
        if f['bind'] == 0 and LOCALNAME.match(f['name']):
            b = self.bases.get((unit, e.names[f['sec']]))
            return None if b is None else b + f['off']
        a = self.sym_addr(e, unit, f)
        if a is None:
            return None
        if f['bind'] == 2:
            lo, hi = self.bases.get((unit, e.names[f['sec']])), self.ends.get((unit, e.names[f['sec']]))
            if lo is not None and not (lo <= a < hi):
                return 'elsewhere'  # the linker kept another unit's copy of this weak function
        return a

    # ---- code ---------------------------------------------------------------------------------------------------
    def exact_functions(self):
        """Yield (unit, e, f, faddr, dolbytes, rels) for functions equal to the DOL up to relocated fields."""
        for unit, e in self.elfs.items():
            for f in e.syms:
                if f['type'] != 2 or not f['size'] or not f['name'] or not e.is_code(f['sec']):
                    continue
                key = (unit, f['name'], f['off'])
                faddr = self.func_anchor(unit, e, f)
                if faddr is None or faddr == 'elsewhere':
                    self.func_info[key] = (None, faddr)
                    continue
                dolb = self.dol.bytes(faddr, f['size'])
                if dolb is None:
                    self.func_info[key] = (None, faddr)
                    continue
                srcb = e.content(f['sec'], f['off'], f['size'])
                rels = {o - f['off']: r for o, r in e.rel_by_sec[f['sec']].items() if f['off'] <= o < f['off'] + f['size']}
                exact = not any(f['off'] <= o < f['off'] + f['size'] for sc, o in e.rel_collisions if sc == f['sec'])
                for i in range(0, f['size'], 4):
                    sw, dw = u32(srcb, i), u32(dolb, i)
                    r = rels.get(i)
                    if r and r[0] in MASK:
                        m = ~MASK[r[0]] & 0xFFFFFFFF
                        sw &= m; dw &= m
                    if sw != dw:
                        exact = False
                        break
                self.func_info[key] = (exact, faddr)
                if exact:
                    yield unit, e, f, faddr, dolb, rels
                else:
                    self.nonexact.append((unit, e, f, faddr, dolb, rels))

    @staticmethod
    def dol_operand(typ, insn, at):
        """Decode what the DOL instruction really references. -> ('addr', value) | ('hi', imm) | ('lo', imm)"""
        if typ == R_REL24:
            if insn >> 26 != 18 or insn & 2:
                return None
            d = insn & 0x03FFFFFC
            return 'addr', (at + (d - 0x4000000 if d & 0x2000000 else d)) & 0xFFFFFFFF
        if typ == R_REL14:
            if insn >> 26 != 16 or insn & 2:
                return None
            return 'addr', (at + s16(insn & 0xFFFC)) & 0xFFFFFFFF
        if typ == R_SDA21:
            ra = (insn >> 16) & 31
            base = SDA13 if ra == 13 else SDA2 if ra == 2 else None
            return None if base is None else ('addr', (base + s16(insn & 0xFFFF)) & 0xFFFFFFFF)
        if typ in (R_ADDR16_HA, R_ADDR16_HI):
            return 'hi', insn & 0xFFFF
        if typ in (R_ADDR16_LO, R_ADDR16):
            return 'lo', insn & 0xFFFF
        return None

    def infer(self, exact):
        """Learn addresses of symbols.txt-unnamed global/weak symbols from exact functions' instructions."""
        for unit, e, f, faddr, dolb, rels in exact:
            hi = {}
            for off in sorted(rels):
                typ, symidx, addend = rels[off]
                t = e.syms[symidx]
                if t['type'] == 3 or t['bind'] == 0 or LOCALNAME.match(t['name']):
                    continue
                op = self.dol_operand(typ, u32(dolb, off), faddr + off)
                if not op:
                    continue
                key = (symidx, addend)
                if op[0] == 'addr':
                    self.inferred[t['name']][(op[1] - addend) & 0xFFFFFFFF].append((unit, f['name'], off))
                elif op[0] == 'hi' and typ == R_ADDR16_HA:
                    hi[key] = op[1]
                elif op[0] == 'lo' and key in hi:
                    val = ((hi[key] << 16) + s16(op[1])) & 0xFFFFFFFF
                    self.inferred[t['name']][(val - addend) & 0xFFFFFFFF].append((unit, f['name'], off))

    def dol_ctor_vtables(self):
        """vtable address per class from the DOL bodies of constructors (symbols.txt-named or inferred): the last
        `lis/addi`-built .data address stored at offset 0 of `this`."""
        ctors = {}
        for name, (a, size, sec, typ) in self.sy.by_name.items():
            if name.startswith('__ct__') and sec == '.text' and name not in self.sy.dups:
                ctors[name] = a
        for name, addrs in self.inferred.items():
            if name.startswith('__ct__') and len(addrs) == 1 and name not in ctors:
                ctors[name] = next(iter(addrs))
        out = {}
        for name, a in ctors.items():
            cls = mangled_class(name[6:])
            b = self.dol.bytes(a, 0x80) or self.dol.bytes(a, 0x20)
            if cls is None or b is None:
                continue
            regs, last = {}, None  # regs: reg -> (value, base-constant)
            for i in range(0, len(b), 4):
                w = u32(b, i)
                op, rt, ra = w >> 26, (w >> 21) & 31, (w >> 16) & 31
                if op == 15 and ra == 0:
                    v = ((w & 0xFFFF) << 16) & 0xFFFFFFFF
                    regs[rt] = (v, v)
                elif op == 14 and ra in regs:
                    v0, base = regs[ra]
                    v = (v0 + s16(w & 0xFFFF)) & 0xFFFFFFFF
                    # lis result (low half zero) + addi completes the address; anything else is an adjustment
                    regs[rt] = (v, v if v0 & 0xFFFF == 0 else base)
                elif op == 36 and ra == 3 and (w & 0xFFFF) == 0 and rt in regs:
                    if 0x80600000 <= regs[rt][1] < 0x80730000:
                        last = regs[rt][1]
                elif w == 0x4E800020:  # blr
                    break
            if last is not None:
                out.setdefault('__vt__' + cls, set()).add(last)
        return {k: next(iter(v)) for k, v in out.items() if len(v) == 1}

    # ---- verdicts -----------------------------------------------------------------------------------------------
    def verdict(self, got, expect, tname):
        if got == (expect & 0xFFFFFFFF):
            return 'MATCH'
        names = self.sy.by_addr.get(got)
        if names and any(n == tname for n in names):
            return 'ALIAS'
        return 'CROSSED' if names else 'WRONG'

    def run_code(self, rows, stats):
        for unit, e, f, faddr, dolb, rels in self.exact_stream:
            for off, (typ, symidx, addend) in sorted(rels.items()):
                t = e.syms[symidx]
                row = dict(layer='code', unit=unit, function=f['name'], offset=off, target=t['name'], site=faddr + off)
                row['kind'] = 'dispatch' if (t['type'] == 2 or is_dispatch_name(t['name'])) else 'data'
                stats['code_relocs'] += 1

                def done(v, **kw):
                    row.update(verdict=v, **kw)
                    rows.append(row)
                    stats[v.split(':')[0].lower()] += 1
                if typ not in MASK:
                    done('UNRESOLVED:reloc_type_%d' % typ); continue
                expect, how = self.target(e, unit, symidx, addend)
                if expect is None:
                    done('UNRESOLVED:' + how); continue
                op = self.dol_operand(typ, u32(dolb, off), faddr + off)
                if op is None:
                    done('UNRESOLVED:dol_operand_not_decodable'); continue
                if op[0] == 'hi':
                    want = ((expect + 0x8000) >> 16 if typ == R_ADDR16_HA else expect >> 16) & 0xFFFF
                    v = 'MATCH' if op[1] == want else 'WRONG'
                    row.update(expect=expect, dol=op[1] << 16)
                elif op[0] == 'lo':
                    v = 'MATCH' if op[1] == expect & 0xFFFF else 'WRONG'
                    row.update(expect=expect, dol=op[1])
                else:
                    v = self.verdict(op[1], expect, t['name'])
                    row.update(expect=expect, dol=op[1])
                    if v in ('CROSSED', 'WRONG'):
                        row['dol_name'] = self.sy.name_at(op[1])
                if how == 'local' and v in ('CROSSED', 'WRONG'):
                    v = 'LOCALDRIFT:' + v
                elif v == 'MATCH' and how not in ('sym', 'chain'):
                    v = 'MATCH_INFERRED' if how == 'inferred' else 'MATCH_LOCAL'  # not independent evidence
                row['how'] = how
                done(v)


    def run_seq(self, rows, stats):
        """Advisory: in functions that are not byte-exact, pair the k-th branch / SDA21 operand of the source with
        the k-th one in the DOL (only when the counts agree). Same-size functions only."""
        for unit, e, f, faddr, dolb, rels in self.nonexact:
            if len(dolb) != f['size']:
                continue
            for typ, want in ((R_REL24, 18), (R_SDA21, None)):
                src = [(o, rels[o]) for o in sorted(rels) if rels[o][0] == typ]
                if typ == R_REL24:
                    dol = [i for i in range(0, len(dolb), 4) if u32(dolb, i) >> 26 == 18 and u32(dolb, i) & 3 == 1]
                else:
                    dol = [i for i in range(0, len(dolb), 4) if (u32(dolb, i) >> 26) in range(32, 56) and ((u32(dolb, i) >> 16) & 31) in (2, 13)
                           or ((u32(dolb, i) >> 26) == 14 and ((u32(dolb, i) >> 16) & 31) in (2, 13))]
                if not src or len(src) != len(dol):
                    stats['seq_skipped'] += bool(src)
                    continue
                for (o, (t_, si, ad)), di in zip(src, dol):
                    t = e.syms[si]
                    if not (t['type'] == 2 or is_dispatch_name(t['name'])):
                        continue
                    expect, how = self.target(e, unit, si, ad)
                    if expect is None or how == 'local':
                        continue
                    op = self.dol_operand(typ, u32(dolb, di), faddr + di)
                    if op is None or op[0] != 'addr':
                        continue
                    stats['seq_checked'] += 1
                    if op[1] != expect & 0xFFFFFFFF:
                        v = self.verdict(op[1], expect, t['name'])
                        if v == 'ALIAS':
                            continue
                        rows.append(dict(layer='seq', unit=unit, function=f['name'], offset=o, kind='dispatch', site=faddr + di,
                                         verdict='ADVISORY:' + v, target=t['name'], expect=expect, dol=op[1],
                                         dol_name=self.sy.name_at(op[1]), dol_offset=di))
                        stats['advisory'] += 1


    # ---- semantic nerve chain ------------------------------------------------------------------------------------
    def dol_chains(self):
        if self._chains is None:
            self._chains = self._dol_chains()
            self.ctor_inst = {c[0]: y for y, c in self._chains.items()}
        return self._chains

    def _dol_chains(self):
        """instance address -> (ctor, vtable, execute, exe-target) decoded from DOL only (sinit -> ctor -> vtable ->
        slot 2 -> first branch of the execute wrapper)."""
        d = self.dol
        inst = {}
        for name, (a, size, sec, typ) in self.sy.by_name.items():
            if not name.startswith('__sinit_') or not size or sec != '.text':
                continue
            b = d.bytes(a, size)
            regs = {}
            for i in range(0, size, 4):
                w = u32(b, i)
                op, rt, ra = w >> 26, (w >> 21) & 31, (w >> 16) & 31
                if op == 15 and ra == 0:
                    regs[rt] = ((w & 0xFFFF) << 16) & 0xFFFFFFFF
                elif op == 14:
                    if ra in (2, 13):
                        regs[rt] = ((SDA13 if ra == 13 else SDA2) + s16(w & 0xFFFF)) & 0xFFFFFFFF
                    elif ra in regs:
                        regs[rt] = (regs[ra] + s16(w & 0xFFFF)) & 0xFFFFFFFF
                    else:
                        regs.pop(rt, None)
                elif op == 18 and w & 1:
                    t = (a + i + ((w & 0x03FFFFFC) - (0x4000000 if w & 0x2000000 else 0))) & 0xFFFFFFFF
                    if 3 in regs and 0x807CF000 <= regs[3] < 0x807E0000:
                        inst.setdefault(regs[3], t)
                    regs.pop(3, None)
                elif op in (24, 25):  # ori
                    pass
        chains = {}
        for y, k in inst.items():
            kb = d.bytes(k, 0x20)
            if kb is None:
                continue
            regs, vt = {}, None
            for i in range(0, 0x20, 4):
                w = u32(kb, i)
                op, rt, ra = w >> 26, (w >> 21) & 31, (w >> 16) & 31
                if op == 15 and ra == 0:
                    regs[rt] = ((w & 0xFFFF) << 16) & 0xFFFFFFFF
                elif op == 14 and ra in regs:
                    regs[rt] = (regs[ra] + s16(w & 0xFFFF)) & 0xFFFFFFFF
                elif op == 36 and ra == 3 and (w & 0xFFFF) == 0 and rt in regs:
                    vt = regs[rt]
                elif w == 0x4E800020:
                    break
            if vt is None:
                continue
            e = d.word(vt + 8)
            x = self.wrapper_targets(e) if e else None
            chains[y] = (k, vt, e, x)
        return chains

    def wrapper_targets(self, e):
        """Ordered call/branch targets (outside the wrapper itself) of the DOL execute wrapper at e."""
        size = self.sy.by_name.get(self.sy.name_at(e) or '', (0, 0))[1] if e in self.sy.by_addr else 0
        n = size if 0 < size <= 0x200 else 0x100
        eb = self.dol.bytes(e, n)
        if eb is None:
            return None
        out = []
        for i in range(0, n, 4):
            w = u32(eb, i)
            if w >> 26 == 18:
                t = (e + i + ((w & 0x03FFFFFC) - (0x4000000 if w & 0x2000000 else 0))) & 0xFFFFFFFF
                if not (e <= t < e + n):
                    out.append(t)
            if w == 0x4E800020 and (size or out):
                break
        return tuple(out) or None

    def src_exe(self):
        """class suffix -> (exe-target address or None, exe name): from the source vtable container `__vt__<cls>`,
        its slot-2 relocation, the execute wrapper it names and that wrapper's first branch."""
        out = {}
        for unit, e in self.elfs.items():
            funcs = {f['name']: f for f in e.syms if f['type'] == 2 and f['size'] and e.is_code(f['sec'])}
            conts = collections.defaultdict(list)
            for sym in e.syms:
                if sym['name'].startswith('__vt__') and e.is_data(sym['sec']) and sym['size']:
                    conts[sym['sec']].append(sym)
            for sec, off, typ, si, ad in e.relocs:
                if typ != R_ADDR32 or not e.is_data(sec):
                    continue
                c = next((c for c in conts.get(sec, []) if c['off'] <= off < c['off'] + c['size']), None)
                if c is None or off - c['off'] != 8:
                    continue
                t = e.syms[si]
                f = funcs.get(t['name'])
                if f is None or not t['name'].startswith('execute__'):
                    continue
                rels = sorted(o for o in e.rel_by_sec[f['sec']] if f['off'] <= o < f['off'] + f['size']
                              and e.rel_by_sec[f['sec']][o][0] == R_REL24)
                if not rels:
                    continue
                seq = []
                for o in rels:
                    typ2, si2, ad2 = e.rel_by_sec[f['sec']][o]
                    x, how = self.target(e, unit, si2, ad2)
                    seq.append((x if how == 'sym' else None, e.syms[si2]['name']))
                out[c['name'][len('__vt__'):]] = (tuple(a for a, _ in seq), seq[0][1], tuple(n for _, n in seq))
        return out

    def run_sem(self, rows, stats):
        chains = self.dol_chains()
        src = self.src_exe()
        stats['dol_nerve_chains'] = len(chains)
        stats['src_nerve_classes'] = len(src)

        def sem(unit, e, t, dol_addr, where, layer):
            cls = t['name'][len('sInstance__'):]
            row = dict(layer=layer, unit=unit, offset=where[1], function=where[0], kind='dispatch', target=t['name'], dol=dol_addr, site=where[2])
            if cls not in src or not any(a is not None for a in src[cls][0]):
                row['verdict'] = 'UNRESOLVED:sem_source_exe'
            elif dol_addr not in chains or chains[dol_addr][3] is None:
                row['verdict'] = 'UNRESOLVED:sem_dol_chain'
            else:
                xs, xd = src[cls][0], chains[dol_addr][3]
                row.update(src_exe=src[cls][1], dol_exe=self.sy.name_at(xd[0]), src_seq=src[cls][2][:4])
                if len(xs) != len(xd):
                    row['verdict'] = 'UNRESOLVED:sem_wrapper_shape'
                elif all(a is None or a == b for a, b in zip(xs, xd)):
                    row['verdict'] = 'MATCH'
                else:
                    row['verdict'] = 'CROSSED'
            stats['sem_' + row['verdict'].split(':')[0].lower()] += 1
            rows.append(row)
        for unit, e, f, faddr, dolb, rels in self.exact_stream:
            for off, (typ, si, ad) in rels.items():
                t = e.syms[si]
                if t['name'].startswith('sInstance__') and 'Nrv' in t['name'] and typ in (R_SDA21, R_ADDR16_LO):
                    op = self.dol_operand(typ, u32(dolb, off), faddr + off)
                    if op and op[0] == 'addr':
                        sem(unit, e, t, op[1], (f['name'], off, faddr + off), 'sem')
        for unit, e, f, faddr, dolb, rels in self.nonexact:
            if len(dolb) != f['size']:
                continue
            src_l = [(o, rels[o]) for o in sorted(rels) if rels[o][0] == R_SDA21]
            dol_l = [i for i in range(0, len(dolb), 4) if ((u32(dolb, i) >> 16) & 31) in (2, 13) and (u32(dolb, i) >> 26) in (*range(32, 56), 14)]
            if not src_l or len(src_l) != len(dol_l):
                continue
            for (o, (typ, si, ad)), di in zip(src_l, dol_l):
                t = e.syms[si]
                if t['name'].startswith('sInstance__') and 'Nrv' in t['name']:
                    op = self.dol_operand(typ, u32(dolb, di), faddr + di)
                    if op and op[0] == 'addr':
                        sem(unit, e, t, op[1], (f['name'], o, faddr + di), 'sem-seq')


    def run_names(self, rows, stats):
        """symbols.txt names along each DOL chain (instance, ctor, vtable, execute) must name the SAME class."""
        chains = self.dol_chains()
        def suffix(name, prefix, tail=''):
            if name and name.startswith(prefix):
                n = name[len(prefix):]
                return n[:-len(tail)] if tail and n.endswith(tail) else n
            return None
        for y, (k, v, e, x) in sorted(chains.items()):
            kinds = []
            for label, addr, prefix, tail in (('instance', y, 'sInstance__', ''), ('ctor', k, '__ct__', 'Fv'),
                                              ('vtable', v, '__vt__', ''), ('execute', e, 'execute__', 'CFP5Spine')):
                names = [n for n in self.sy.by_addr.get(addr, []) if n.startswith(prefix)] if addr else []
                if names:
                    kinds.append((label, addr, suffix(names[0], prefix, tail)))
            if len({c for _, _, c in kinds}) > 1:
                rows.append(dict(layer='names', unit='', function='<chain-%x>' % y, offset=y, kind='dispatch',
                                 verdict='CROSSED', site=y,
                                 detail={l: (hex(a), c) for l, a, c in kinds}))
                stats['names_crossed'] += 1
            elif kinds:
                stats['names_match'] += 1

    # ---- data ---------------------------------------------------------------------------------------------------
    def body_matches(self, e, unit, symidx, addend, w):
        """Is the source function the target names equal to the DOL code at w, with its relocs resolving there?"""
        t = e.syms[symidx]
        if t['type'] != 2 or addend or not t['size'] or not e.is_code(t['sec']):
            return None
        dolb = self.dol.bytes(w, t['size'])
        if dolb is None:
            return None
        srcb = e.content(t['sec'], t['off'], t['size'])
        rels = {o - t['off']: r for o, r in e.rel_by_sec[t['sec']].items() if t['off'] <= o < t['off'] + t['size']}
        for i in range(0, t['size'], 4):
            sw, dw = u32(srcb, i), u32(dolb, i)
            r = rels.get(i)
            if r and r[0] in MASK:
                m = ~MASK[r[0]] & 0xFFFFFFFF
                sw &= m; dw &= m
            if sw != dw:
                return False
        for off, (typ, si, ad) in rels.items():
            exp, how = self.target(e, unit, si, ad)
            op = self.dol_operand(typ, u32(dolb, off), w + off)
            if exp is None or op is None or op[0] != 'addr' or how != 'sym':
                return None  # a relocation we cannot verify independently: no MATCH_BODY
            if op[1] != exp & 0xFFFFFFFF:
                return False
        return True

    def run_data(self, rows, stats):
        for unit, e in self.elfs.items():
            cont = collections.defaultdict(list)
            for sym in e.syms:
                if e.is_data(sym['sec']) and sym['type'] in (0, 1) and sym['name']:
                    cont[sym['sec']].append(sym)
            relocated = set()
            for sec, off, typ, symidx, addend in e.relocs:
                if not e.is_data(sec):
                    continue
                relocated.add((sec, off))
                t = e.syms[symidx]
                row = dict(layer='data', unit=unit, section=e.names[sec], offset=off, target=t['name'])
                row['kind'] = 'dispatch' if (t['type'] == 2 or is_dispatch_name(t['name'])) else 'data'
                stats['data_relocs'] += 1

                def done(v, **kw):
                    row.update(verdict=v, **kw)
                    rows.append(row)
                    stats[v.split(':')[0].lower()] += 1
                cands = [c for c in cont[sec] if c['off'] <= off < c['off'] + max(c['size'], 1)]
                if not cands:
                    b = self.bases.get((unit, e.names[sec]))
                    if b is None:
                        done('UNRESOLVED:no_container'); continue
                    c = dict(name=f'<{e.names[sec]}>', off=0, size=e.s[sec][5], bind=1, type=1, sec=sec)
                    cands = [c]
                    stats['section_container'] += 1
                c = min(cands, key=lambda c: c['size'])
                row['container'] = c['name']
                row['slot'] = off - c['off']
                caddr = self.sym_addr(e, unit, c) if not c['name'].startswith('<') else self.bases.get((unit, e.names[sec]))
                chow = 'sym'
                if caddr is None:
                    inf = self.inferred.get(c['name']) if not drifty(c) and c['bind'] != 0 else None
                    if inf and len(inf) == 1:
                        caddr, chow = next(iter(inf)), 'inferred'
                    elif drifty(c) or c['bind'] == 0:
                        caddr, chow = self.local_addr(e, unit, c), 'local'
                if caddr is None:
                    done('UNRESOLVED:container_unmapped'); continue
                loc = caddr + off - c['off']
                w = self.dol.word(loc)
                if w is None:
                    done('UNRESOLVED:dol_not_file_backed'); continue
                if typ != R_ADDR32:
                    done('UNRESOLVED:reloc_type_%d' % typ); continue
                expect, how = self.target(e, unit, symidx, addend)
                row.update(dol_addr=loc, site=loc, dol=w, how=how, chow=chow)
                if expect is None:
                    if how == 'target_unmapped' and self.body_matches(e, unit, symidx, addend, w):
                        done('MATCH_BODY'); continue
                    done('UNRESOLVED:' + how); continue
                row['expect'] = expect
                v = self.verdict(w, expect, t['name'])
                if v in ('CROSSED', 'WRONG'):
                    row['dol_name'] = self.sy.name_at(w)
                    if how == 'local' or chow == 'local':
                        v = 'LOCALDRIFT:' + v
                elif v == 'MATCH' and (how not in ('sym', 'chain') or chow != 'sym'):
                    v = 'MATCH_INFERRED' if 'inferred' in (how, chow) else 'MATCH_LOCAL'
                done(v)
            # vtable shape: unrelocated words must equal the DOL words; the source vtable may not be larger
            for sec, cs in cont.items():
                for c in cs:
                    if not c['name'].startswith('__vt__') or drifty(c):
                        continue
                    caddr = self.sym_addr(e, unit, c)
                    if caddr is None:
                        inf = self.inferred.get(c['name'])
                        caddr = next(iter(inf)) if inf and len(inf) == 1 else None
                    if caddr is None:
                        continue
                    zsym = self.sy.by_name.get(c['name'], (0, 0))[1]
                    if zsym and c['size'] > zsym:
                        rows.append(dict(layer='data', unit=unit, container=c['name'], section=e.names[sec], kind='dispatch',
                                         offset=c['off'], verdict='WRONG', detail=f'vtable size src {c["size"]:#x} dol {zsym:#x}'))
                    for o in range(0, c['size'], 4):
                        if (sec, c['off'] + o) in relocated:
                            continue
                        w = self.dol.word(caddr + o)
                        if w is not None and w != u32(e.content(sec, c['off'] + o, 4)):
                            rows.append(dict(layer='data', unit=unit, container=c['name'], section=e.names[sec], kind='dispatch',
                                             offset=c['off'] + o, slot=o, verdict='WRONG', dol=w,
                                             detail='unrelocated vtable word differs'))

    def run(self, layers=('data', 'code')):
        rows, stats = [], collections.Counter()
        stats['objects'] = len(self.elfs)
        exact = list(self.exact_functions())
        self.exact_stream = exact
        stats['code_funcs_exact'] = len(exact)
        stats['code_funcs_unanchored'] = sum(1 for v in self.func_info.values() if v[1] is None)
        stats['code_weak_copy_elsewhere'] = sum(1 for v in self.func_info.values() if v[1] == 'elsewhere')
        stats['code_funcs_nonexact'] = sum(1 for v in self.func_info.values() if v[0] is False)
        self.infer(exact)
        self.dol_chains()
        for name, v in self.dol_ctor_vtables().items():
            if name not in self.sy.by_name and name not in self.inferred:
                self.inferred[name][v].append(('<dol-ctor>', name, 0))
        for name, v in self.dol_ctor_vtables().items():
            if name not in self.sy.by_name and name not in self.inferred:
                self.inferred[name][v].append(('<dol-ctor>', name, 0))
        conflicts = {n: a for n, a in self.inferred.items() if len(a) > 1}
        stats['inferred_symbols'] = len(self.inferred)
        stats['inferred_conflicts'] = len(conflicts)
        if 'data' in layers:
            self.run_data(rows, stats)
        if 'code' in layers:
            self.run_code(rows, stats)
            self.run_seq(rows, stats)
            self.run_sem(rows, stats)
            self.run_names(rows, stats)
        for n, addrs in sorted(conflicts.items()):
            rows.append(dict(layer='conflict', unit='', function=n, offset=0, kind='dispatch', verdict='CONFLICT',
                             detail={hex(a): [f'{u}:{fn}+{o:#x}' for u, fn, o in ev][:3] for a, ev in addrs.items()}))
        return rows, stats


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--root', default='.')
    ap.add_argument('--dol', help='original main.dol (default: <root>/orig/SB4E01/sys/main.dol); its sha1 must equal config/SB4E01/build.sha1')
    ap.add_argument('--layer', choices=('data', 'code', 'all'), default='all')
    ap.add_argument('--prefix', default='')
    ap.add_argument('--out')
    ap.add_argument('--allow')
    ap.add_argument('--strict', action='store_true')
    ap.add_argument('--quiet', action='store_true')
    ap.add_argument('--write-allow', metavar='FILE', help='write the current BAD rows as an allow-list (baseline)')
    a = ap.parse_args()
    prog = Prog(a.root, a.dol)
    inventory = []
    cfg = pathlib.Path(a.root) / 'objdiff.json'
    if cfg.exists():
        expected = {u['name'].removeprefix('main/') for u in json.loads(cfg.read_text())['units'] if u.get('base_path')}
        inventory = sorted(expected - set(prog.elfs))
    for unit, err in prog.errors:
        print('INVENTORY unreadable object', unit, err)
    for unit in inventory:
        print('INVENTORY missing object', unit)
    rows, stats = prog.run(('data', 'code') if a.layer == 'all' else (a.layer,))
    if a.prefix:
        rows = [r for r in rows if r['unit'].startswith(a.prefix)]
    if a.out:
        pathlib.Path(a.out).write_text('\n'.join(json.dumps(r) for r in rows) + '\n')
    allow = set()
    if a.allow:
        for line in pathlib.Path(a.allow).read_text().splitlines():
            if line.strip() and not line.startswith('#'):
                allow.add(tuple(line.split()[:3]))
    if not a.quiet:
        print(json.dumps(dict(stats)), file=sys.stderr)
        for v, n in collections.Counter((r['layer'], r['kind'], r['verdict']) for r in rows).most_common():
            print(n, *v)
    def akey(r):
        return (r['unit'] or '-', r.get('container') or r.get('function'), f"{r['offset']:x}")
    allbad = [r for r in rows if (r['verdict'] in ('CROSSED', 'WRONG', 'CONFLICT') or (r['verdict'].startswith('CROSSED') and False))
              and r['kind'] == 'dispatch']
    if a.write_allow:
        lines = sorted({' '.join(akey(r)) + f"  # {r['layer']} {r['verdict']} {r.get('target', '')[:60]}" for r in allbad})
        pathlib.Path(a.write_allow).write_text('# reviewed-open dispatch rows (unit container-or-function offset); remove a line when its family is fixed\n' + '\n'.join(lines) + '\n')
    bad = [r for r in allbad if akey(r) not in allow]
    stale = allow - {akey(r) for r in allbad} if a.allow and not a.prefix else set()
    for st in sorted(stale):
        print('STALE-ALLOW', *st)
    for r in bad:
        print('BAD', r['layer'], r['unit'], r.get('container') or r.get('function'), f"+{r['offset']:x}",
              r.get('target', ''), '->', r.get('dol_name') or r.get('detail') or hex(r.get('dol', 0)))
    if a.strict and (bad or stale or prog.errors or inventory):
        sys.exit(1)


if __name__ == '__main__':
    main()
