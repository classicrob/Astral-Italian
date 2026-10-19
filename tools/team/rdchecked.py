"""Conservative PPC copy/scheduling equivalence; unsupported semantics fail closed.

Values are exact interned expression DAGs shared by both functions. Instruction
nodes retain multiplicity. Memory edges retain every possibly aliasing ordering;
only constant, disjoint intervals on an equal base value can omit an edge.
Forward CFGs are supported; loops and indirect control flow fail closed.
"""
import collections
import re

REGS = tuple([f'r{i}' for i in range(32)] + [f'f{i}' for i in range(32)] +
             [f'cr{i}' for i in range(8)] + [f'qr{i}' for i in range(8)] + ['lr', 'ctr', 'xer'])
ARGS = tuple([f'r{i}' for i in range(3, 11)] + [f'f{i}' for i in range(1, 9)])
CLOBBER = tuple([f'r{i}' for i in [0, *range(3, 13)]] +
                [f'f{i}' for i in range(14)] + [f'cr{i}' for i in (0, 1, 5, 6, 7)] + ['lr', 'ctr', 'xer'])
# Always include r4, conservatively accommodating a 64-bit result.
OBSERVE = tuple(['r1', 'r2', 'r3', 'r4', 'r13', 'f1', 'lr', 'cr2', 'cr3', 'cr4'] +
                [f'r{i}' for i in range(14, 32)] + [f'f{i}' for i in range(14, 32)])
COMMUTATIVE = {'add', 'mullw', 'and', 'or', 'xor'}
PURE = {
    'li': (1, 0), 'lis': (1, 0),
    'addi': (1, 1), 'addis': (1, 1), 'subfic': (1, 1),
    'add': (1, 2), 'subf': (1, 2), 'mullw': (1, 2), 'mulli': (1, 1),
    'and': (1, 2), 'or': (1, 2), 'xor': (1, 2), 'nor': (1, 2),
    'andi.': (1, 1), 'andis.': (1, 1), 'ori': (1, 1), 'oris': (1, 1), 'xori': (1, 1),
    'slw': (1, 2), 'srw': (1, 2), 'slwi': (1, 1), 'srwi': (1, 1),
    'rlwinm': (1, 1), 'clrlwi': (1, 1), 'clrrwi': (1, 1),
    'neg': (1, 1), 'extsb': (1, 1), 'extsh': (1, 1),
    'fadd': (1, 2), 'fadds': (1, 2), 'fsub': (1, 2), 'fsubs': (1, 2),
    'fmul': (1, 2), 'fmuls': (1, 2), 'fdiv': (1, 2), 'fdivs': (1, 2),
    'fmadd': (1, 3), 'fmadds': (1, 3), 'fmsub': (1, 3), 'fmsubs': (1, 3),
    'fnmsub': (1, 3), 'fnmsubs': (1, 3), 'fnmadd': (1, 3), 'fnmadds': (1, 3),
    'fneg': (1, 1), 'fabs': (1, 1), 'fnabs': (1, 1), 'frsp': (1, 1),
    'fctiwz': (1, 1), 'fres': (1, 1), 'frsqrte': (1, 1),
}
# Floating operations have FPSCR effects (rounding, exceptions, summaries).
# Preserve their relative order and never commute floating operands: signed
# zero/NaN payload semantics cannot be inferred from an algebraic identity.
WIDTH = {'lbz': 1, 'lha': 2, 'lhz': 2, 'lwz': 4, 'lfs': 4, 'lfd': 8,
         'stb': 1, 'sth': 2, 'stw': 4, 'stfs': 4, 'stfd': 8}
COND = {'beq', 'bne', 'blt', 'ble', 'bgt', 'bge', 'bso', 'bns'}

class Unproved(Exception):
    pass


def reg(x):
    if x not in REGS:
        raise Unproved('unknown register ' + x)
    return x


def decode(line):
    fields = line.strip().split(None, 1)
    if not fields:
        raise Unproved('empty instruction')
    op = fields[0]
    a = [x.strip() for x in fields[1].split(',')] if len(fields) == 2 else []
    d = {'op': op, 'args': a, 'use': [], 'def': [], 'kind': 'pure'}
    if op in ('mr', 'fmr') and len(a) == 2:
        d.update(kind='copy', use=[reg(a[1])], **{'def': [reg(a[0])]})
        if a[0][0] != a[1][0] or (op == 'mr') != a[0].startswith('r'):
            raise Unproved('copy register class')
    elif op.rstrip('.') in PURE or op in PURE:
        base = op if op in PURE else op[:-1]
        _, n = PURE[base]
        if len(a) < n + 1:
            raise Unproved('operand count')
        dst = reg(a[0])
        sources = [reg(x) for x in a[1:1+n]]
        # addi/addis interpret RA=0 as constant zero, unlike ordinary r0 uses.
        if op in ('addi', 'addis') and sources == ['r0']:
            sources = []
        const = a[1+n:]
        if any(re.search(r'\b(?:r|f|cr)\d+\b', x) for x in const):
            raise Unproved('unexpected register operand')
        d.update(use=sources, const=tuple(const), **{'def': [dst]})
        if op.endswith('.') and not op.startswith('f'):
            d['use'].append('xer')
        if op.endswith('.'):
            d['def'].append('cr1' if op.startswith('f') else 'cr0')
        if op == 'subfic':
            d['use'].append('xer')
            d['def'].append('xer')
        if op.startswith('f'):
            d['kind'] = 'float'
    elif op in WIDTH or op.endswith('u') and op[:-1] in WIDTH or op in ('psq_l', 'psq_st', 'psq_lu', 'psq_stu'):
        base = op[:-1] if op.endswith('u') else op
        paired = base in ('psq_l', 'psq_st')
        if paired:
            if len(a) != 4 or a[2] not in ('0', '1') or not re.fullmatch(r'qr[0-7]', a[3]):
                raise Unproved('paired memory operands')
            mode = (a[2], a[3])
            width = 4 if a[2] == '1' else 8
        else:
            mode, width = (), WIDTH[base]
        if len(a) != (4 if paired else 2):
            raise Unproved('memory operands')
        m = re.fullmatch(r'(.+)\((r\d+)\)', a[1])
        sda = not m and a[1].endswith('@sda21')
        if not m and not sda:
            raise Unproved('unknown memory address')
        val, addr = reg(a[0]), reg(m[2]) if m else 'r2'
        displacement = m[1] if m else a[1]
        if addr == 'r0':
            raise Unproved('zero-base memory')
        try:
            offset = int(displacement, 0)
            if 0x8000 <= offset <= 0xffff:
                offset -= 0x10000
        except ValueError:
            offset = displacement
        store = base.startswith('st') or base == 'psq_st'
        d['fp_memory'] = base in ('lfs', 'lfd', 'stfs', 'stfd')
        d.update(kind='store' if store else 'load', base=addr, offset=offset,
                 width=width, mode=mode, use=([val] if store else []) + [addr],
                 **{'def': ([] if store else [val])})
        if paired:
            if not val.startswith('f'):
                raise Unproved('paired destination register')
            d['use'].append(reg(a[3]))
        if sda:
            d['use'].append('r13')
        if op.endswith('u'):
            if (not store and val == addr) or not isinstance(offset, int):
                raise Unproved('update address')
            d['def'].append(addr)
    elif op in ('cmpw', 'cmplw', 'cmpwi', 'cmplwi', 'fcmpo', 'fcmpu'):
        cr = a.pop(0) if a and a[0].startswith('cr') else 'cr0'
        if len(a) != 2:
            raise Unproved('compare operands')
        immediate = op.endswith('i')
        d.update(use=[reg(a[0])] + ([] if immediate else [reg(a[1])]),
                 const=(a[1],) if immediate else (), **{'def': [reg(cr)]})
        if op.startswith('f'):
            d['kind'] = 'float'
        else:
            d['use'].append('xer')
    elif op in ('mflr', 'mfctr', 'mfxer') and len(a) == 1:
        d.update(use=[op[2:]], **{'def': [reg(a[0])]})
    elif op in ('mtlr', 'mtctr', 'mtxer') and len(a) == 1:
        d.update(use=[reg(a[0])], **{'def': [op[2:]]})
    elif op == 'bl' and len(a) == 1:
        # Numeric intra-function bl is not an ordinary external ABI call.
        if re.fullmatch(r'-?(?:0x[\da-f]+|\d+)', a[0]):
            raise Unproved('numeric call target')
        d.update(kind='call', use=['r1', 'r2', 'r13', 'cr1'] + list(ARGS), const=(a[0],), **{'def': list(CLOBBER)})
    elif op == 'blr' and not a:
        d.update(kind='term', use=list(OBSERVE), const=())
    elif op == 'b' or op.rstrip('+-') in COND:
        conditional = op != 'b'
        cr = a.pop(0) if conditional and len(a) == 2 else 'cr0'
        if len(a) != 1:
            raise Unproved('branch operands')
        try:
            target = int(a[0], 0)
        except ValueError:
            raise Unproved('external branch')
        d.update(kind='term', target=target, use=[reg(cr)] if conditional else [], const=())
    elif op == 'nop' and not a:
        d.update(const=())
    else:
        raise Unproved('unsupported opcode ' + op)
    return d


def blocks(lines):
    ins = [decode(x) for x in lines]
    if not ins:
        raise Unproved('empty function')
    starts = {0}
    for i, d in enumerate(ins):
        if d['kind'] == 'term':
            if i + 1 < len(ins):
                starts.add(i + 1)
            if 'target' in d:
                t = d['target']
                if t % 4 or not 0 <= t // 4 < len(ins):
                    raise Unproved('branch outside function')
                starts.add(t // 4)
    starts = sorted(starts)
    ids = {s: i for i, s in enumerate(starts)}
    result = []
    for bi, s in enumerate(starts):
        body = ins[s:starts[bi + 1] if bi + 1 < len(starts) else len(ins)]
        term = body[-1] if body[-1]['kind'] == 'term' else None
        succ = []
        if term and 'target' in term:
            succ.append(ids[term['target'] // 4])
        if not term or term['op'] not in ('b', 'blr'):
            if bi + 1 == len(starts):
                raise Unproved('unterminated function')
            succ.append(bi + 1)
        if any(x <= bi for x in succ):
            raise Unproved('cyclic CFG')
        result.append((body, tuple(succ)))
    return result


def machine_lines(records, formatted):
    """Drop objdiff alignment rows only; an undecoded real instruction must fail."""
    return [s for record, s in zip(records, formatted) if record.get('instruction') is not None]


def reject_partial_paired_reads(graph):
    """Scalar PS0 writes cannot stand in for a full paired-register value."""
    outgoing = {}
    for bi, (body, _) in enumerate(graph):
        predecessors = [p for p in range(bi) if bi in graph[p][1]]
        partial = set().union(*(outgoing[p] for p in predecessors)) if predecessors else set()
        for d in body:
            op = d['op']
            paired = op.startswith(('psq_st', 'ps_'))
            if paired and partial.intersection(d['use']):
                raise Unproved('paired read after PS0-only definition: ' + op)
            fdefs = {r for r in d['def'] if re.fullmatch(r'f\d+', r)}
            if op in ('lfs', 'lfsu', 'psq_l', 'psq_lu'):
                partial.difference_update(fdefs)
            elif op in ('fmr', 'lfd', 'lfdu') or op.startswith('f'):
                # Only known complete loads clear the conservative lane taint.
                partial.update(fdefs)
            elif d['kind'] == 'call':
                partial.update(fdefs)
        outgoing[bi] = partial


def checked_equal(left, right):
    """Return proof only if both exact value/effect DAGs and live boundaries agree."""
    try:
        L, R = blocks(left), blocks(right)
        reject_partial_paired_reads(L)
        reject_partial_paired_reads(R)
        if len(L) != len(R):
            raise Unproved('block count')
        for (a, sa), (b, sb) in zip(L, R):
            ta = a[-1]['op'] if a[-1]['kind'] == 'term' else None
            tb = b[-1]['op'] if b[-1]['kind'] == 'term' else None
            if sa != sb or ta != tb:
                raise Unproved('CFG/terminator mismatch')
        pool = {}
        def value(x):
            if x not in pool:
                pool[x] = len(pool)
            return pool[x]
        entry = {r: value(('entry', r)) for r in REGS}
        # Union liveness requires identity at every physical register crossing a
        # block boundary. Local temporaries may freely change register allocation.
        live = [set() for _ in L]
        for bi in reversed(range(len(L))):
            for graph in (L, R):
                body, succ = graph[bi]
                needed = set().union(*(live[s] for s in succ)) if succ else set(OBSERVE)
                for d in reversed(body):
                    needed.difference_update(d['def'])
                    needed.update(d['use'])
                live[bi].update(needed)
        def run(graph, side):
            ends, effects, summaries = {}, {}, []
            for bi, (body, succ) in enumerate(graph):
                preds = [p for p in range(bi) if bi in graph[p][1]]
                if bi and not preds:
                    raise Unproved('unreachable block')
                state = dict(entry)
                for r in live[bi]:
                    if preds:
                        incoming = tuple((p, ends[p][r]) for p in preds)
                        state[r] = incoming[0][1] if len({v for _, v in incoming}) == 1 else value(('phi', bi, r, incoming))
                mem = []
                if preds:
                    # All predecessor effects are represented in a single token;
                    # no instruction may schedule across a basic block boundary.
                    mem.append(('call', None, value(('incoming-effects', tuple((p, effects[p]) for p in preds)))))
                nodes = collections.Counter()
                fp = value(('fp-entry', bi))
                term_sig = None
                for d in body:
                    op, kind = d['op'], d['kind']
                    uses = tuple(state[r] for r in d['use'])
                    if kind == 'copy':
                        state[d['def'][0]] = uses[0]
                        continue
                    if op in COMMUTATIVE and len(uses) == 2:
                        uses = tuple(sorted(uses))
                    if kind == 'term':
                        term_sig = (op, uses, succ)
                        continue
                    addr = None
                    if kind in ('load', 'store'):
                        addr = (state[d['base']], d['offset'], d['width'])
                    def disjoint(other):
                        if addr is None or other is None:
                            return False
                        b, o, w = addr
                        b2, o2, w2 = other
                        return b == b2 and isinstance(o, int) and isinstance(o2, int) and (o + w <= o2 or o2 + w2 <= o)
                    deps = ()
                    if kind in ('load', 'store', 'call'):
                        deps = tuple(sorted({v for k, a, v in mem if
                            (kind == 'call' or k == 'call' or kind == 'store' or k == 'store') and not disjoint(a)}))
                    const = d.get('const', ())
                    # Include the address displacement/width, otherwise a load
                    # from another field could look like the same producer.
                    if addr is not None:
                        const = (d['offset'], d['width'], d.get('mode', ()))
                    sig = value(('node', bi, op, uses, const, deps, fp if kind == 'float' or d.get('fp_memory') else None))
                    nodes[sig] += 1
                    if kind in ('load', 'store', 'call'):
                        mem.append((kind, addr, sig))
                    if kind in ('float', 'call') or d.get('fp_memory'):
                        fp = value(('fp-after', sig, fp))
                    for j, r in enumerate(d['def']):
                        if addr is not None and r == d['base'] and op.endswith('u'):
                            state[r] = value(('address-add', addr[0], addr[1]))
                        elif kind == 'call' and r not in ('r3', 'r4', 'f1'):
                            # Unspecified volatile outputs (including LR's actual
                            # code address) cannot be correlated across binaries.
                            state[r] = value(('call-clobber', side, sig, r))
                        else:
                            state[r] = value(('result', sig, j))
                needed = set().union(*(live[s] for s in succ)) if succ else set(OBSERVE)
                boundary = tuple((r, state[r]) for r in sorted(needed))
                # Effect summary order is a multiset: individual effect nodes
                # already encode every required ordering edge.
                effects[bi] = tuple(sorted(v for _, _, v in mem))
                ends[bi] = state
                summaries.append((nodes, term_sig, boundary, effects[bi], fp))
            return summaries
        if run(L, 'L') != run(R, 'R'):
            raise Unproved('value/effect DAG or live boundary mismatch')
        copies = sum(decode(x)['kind'] == 'copy' for x in left + right)
        return True, f'{len(L)} blocks; exact value/effect DAGs, copy aliases ({copies}), ABI inputs/outputs and may-alias order checked'
    except (Unproved, ValueError, KeyError, IndexError) as e:
        return False, str(e)


# Opt-in memory value equalities; the default v1 entry point above is unchanged.

def decode_mem(line):
    fields = line.strip().split(None, 1)
    if not fields:
        raise Unproved('empty instruction')
    op = fields[0]
    a = [x.strip() for x in fields[1].split(',')] if len(fields) == 2 else []
    d = {'op': op, 'args': a, 'use': [], 'def': [], 'kind': 'pure'}
    if op in ('mr', 'fmr') and len(a) == 2:
        d.update(kind='copy', use=[reg(a[1])], **{'def': [reg(a[0])]})
        if a[0][0] != a[1][0] or (op == 'mr') != a[0].startswith('r'):
            raise Unproved('copy register class')
    elif op.rstrip('.') in PURE or op in PURE:
        base = op if op in PURE else op[:-1]
        _, n = PURE[base]
        if len(a) < n + 1:
            raise Unproved('operand count')
        dst = reg(a[0])
        sources = [reg(x) for x in a[1:1+n]]
        # addi/addis interpret RA=0 as constant zero, unlike ordinary r0 uses.
        if op in ('addi', 'addis') and sources == ['r0']:
            sources = []
        const = a[1+n:]
        if any(re.search(r'\b(?:r|f|cr)\d+\b', x) for x in const):
            raise Unproved('unexpected register operand')
        d.update(use=sources, const=tuple(const), **{'def': [dst]})
        if op.endswith('.') and not op.startswith('f'):
            d['use'].append('xer')
        if op.endswith('.'):
            d['def'].append('cr1' if op.startswith('f') else 'cr0')
        if op == 'subfic':
            d['use'].append('xer')
            d['def'].append('xer')
        if op.startswith('f'):
            d['kind'] = 'float'
    elif op in WIDTH or op.endswith('u') and op[:-1] in WIDTH or op in ('psq_l', 'psq_st', 'psq_lu', 'psq_stu'):
        base = op[:-1] if op.endswith('u') else op
        paired = base in ('psq_l', 'psq_st')
        if paired:
            if len(a) != 4 or a[2] not in ('0', '1') or not re.fullmatch(r'qr[0-7]', a[3]):
                raise Unproved('paired memory operands')
            mode = (a[2], a[3])
            width = 4 if a[2] == '1' else 8
        else:
            mode, width = (), WIDTH[base]
        if len(a) != (4 if paired else 2):
            raise Unproved('memory operands')
        m = re.fullmatch(r'(.+)\((r\d+)\)', a[1])
        sda = not m and a[1].endswith('@sda21')
        if not m and not sda:
            raise Unproved('unknown memory address')
        val, addr = reg(a[0]), reg(m[2]) if m else 'r2'
        displacement = m[1] if m else a[1]
        if addr == 'r0':
            raise Unproved('zero-base memory')
        try:
            offset = int(displacement, 0)
            if 0x8000 <= offset <= 0xffff:
                offset -= 0x10000
        except ValueError:
            offset = displacement
        store = base.startswith('st') or base == 'psq_st'
        d['fp_memory'] = base == 'stfs'
        d.update(kind='store' if store else 'load', base=addr, offset=offset,
                 width=width, mode=mode, use=([val] if store else []) + [addr],
                 **{'def': ([] if store else [val])})
        if paired:
            if not val.startswith('f'):
                raise Unproved('paired destination register')
            d['use'].append(reg(a[3]))
        if sda:
            d['use'].append('r13')
        if op.endswith('u'):
            if (not store and val == addr) or not isinstance(offset, int):
                raise Unproved('update address')
            d['def'].append(addr)
    elif op in ('cmpw', 'cmplw', 'cmpwi', 'cmplwi', 'fcmpo', 'fcmpu'):
        cr = a.pop(0) if a and a[0].startswith('cr') else 'cr0'
        if len(a) != 2:
            raise Unproved('compare operands')
        immediate = op.endswith('i')
        d.update(use=[reg(a[0])] + ([] if immediate else [reg(a[1])]),
                 const=(a[1],) if immediate else (), **{'def': [reg(cr)]})
        if op.startswith('f'):
            d['kind'] = 'float'
        else:
            d['use'].append('xer')
    elif op in ('mflr', 'mfctr', 'mfxer') and len(a) == 1:
        d.update(use=[op[2:]], **{'def': [reg(a[0])]})
    elif op in ('mtlr', 'mtctr', 'mtxer') and len(a) == 1:
        d.update(use=[reg(a[0])], **{'def': [op[2:]]})
    elif op == 'bl' and len(a) == 1:
        if re.match(r'^_(?:save|rest)(?:gpr|fpr|f)\d*_', a[0]) or re.match(r'^_(?:save|rest)f\d+$', a[0]):
            raise Unproved('nonstandard save/restore helper ABI')
        # Numeric intra-function bl is not an ordinary external ABI call.
        if re.fullmatch(r'-?(?:0x[\da-f]+|\d+)', a[0]):
            raise Unproved('numeric call target')
        d.update(kind='call', use=['r1', 'r2', 'r13', 'cr1'] + list(ARGS), const=(a[0],), **{'def': list(CLOBBER)})
    elif op == 'blr' and not a:
        d.update(kind='term', use=list(OBSERVE), const=())
    elif op == 'b' or op.rstrip('+-') in COND:
        conditional = op != 'b'
        cr = a.pop(0) if conditional and len(a) == 2 else 'cr0'
        if len(a) != 1:
            raise Unproved('branch operands')
        try:
            target = int(a[0], 0)
        except ValueError:
            raise Unproved('external branch')
        d.update(kind='term', target=target, use=[reg(cr)] if conditional else [], const=())
    elif op == 'nop' and not a:
        d.update(const=())
    else:
        raise Unproved('unsupported opcode ' + op)
    return d

def blocks_mem(lines):
    ins = [decode_mem(x) for x in lines]
    if not ins:
        raise Unproved('empty function')
    starts = {0}
    for i, d in enumerate(ins):
        if d['kind'] == 'term':
            if i + 1 < len(ins):
                starts.add(i + 1)
            if 'target' in d:
                t = d['target']
                if t % 4 or not 0 <= t // 4 < len(ins):
                    raise Unproved('branch outside function')
                starts.add(t // 4)
    starts = sorted(starts)
    ids = {s: i for i, s in enumerate(starts)}
    result = []
    for bi, s in enumerate(starts):
        body = ins[s:starts[bi + 1] if bi + 1 < len(starts) else len(ins)]
        term = body[-1] if body[-1]['kind'] == 'term' else None
        succ = []
        if term and 'target' in term:
            succ.append(ids[term['target'] // 4])
        if not term or term['op'] not in ('b', 'blr'):
            if bi + 1 == len(starts):
                raise Unproved('unterminated function')
            succ.append(bi + 1)
        if any(x <= bi for x in succ):
            raise Unproved('cyclic CFG')
        result.append((body, tuple(succ)))
    return result

def mark_private_spills_mem(graph):
    """Prove a narrow, dead-frame spill case; anything escaping stays an effect.

    Only single-block, call-free, fixed-size frames are eligible. A removable
    word store must lie strictly within the allocated frame and have no later
    unknown-base access, address escape, or differently typed overlapping read.
    """
    if len(graph) != 1:
        return
    body, succ = graph[0]
    if len(body) < 4 or succ:
        return
    first, restore, ret = body[0], body[-2], body[-1]
    if (first['op'] != 'stwu' or first['args'][0] != 'r1' or
            first.get('base') != 'r1' or not isinstance(first.get('offset'), int) or
            first['offset'] >= 0 or ret['op'] != 'blr' or
            restore['op'] != 'addi' or restore['args'][:2] != ['r1', 'r1']):
        return
    frame = -first['offset']
    try:
        if int(restore['args'][2], 0) != frame:
            return
    except ValueError:
        return
    if any(d['kind'] == 'call' for d in body):
        return
    for d in body[1:-2]:
        if 'r1' in d['def']:
            return
        if 'r1' in d['use'] and not (d['kind'] in ('load', 'store') and d.get('base') == 'r1'):
            return
        # Storing the stack pointer would escape the frame, even via r1-relative memory.
        if d['kind'] == 'store' and d['args'][0] == 'r1':
            return
    for i, d in enumerate(body[1:-2], 1):
        off = d.get('offset')
        if (d['op'] != 'stw' or d.get('base') != 'r1' or
                not isinstance(off, int) or not 4 <= off <= frame - 4):
            continue
        safe = True
        for later in body[i + 1:-2]:
            if later['kind'] not in ('load', 'store'):
                continue
            o, w = later.get('offset'), later.get('width')
            if later.get('base') != 'r1' or not isinstance(o, int):
                safe = False
                break
            if off + 4 <= o or o + w <= off:
                continue
            if o != off or w != 4 or later['op'] not in ('lwz', 'stw'):
                safe = False
                break
        if safe:
            d['private_spill'] = True

def checked_equal_mem(left, right):
    """Return proof only if both exact value/effect DAGs and live boundaries agree."""
    try:
        L, R = blocks_mem(left), blocks_mem(right)
        reject_partial_paired_reads(L)
        reject_partial_paired_reads(R)
        if len(L) != len(R):
            raise Unproved('block count')
        for (a, sa), (b, sb) in zip(L, R):
            ta = a[-1]['op'] if a[-1]['kind'] == 'term' else None
            tb = b[-1]['op'] if b[-1]['kind'] == 'term' else None
            if sa != sb or ta != tb:
                raise Unproved('CFG/terminator mismatch')
        mark_private_spills_mem(L)
        mark_private_spills_mem(R)
        pool = {}
        def value(x):
            if x not in pool:
                pool[x] = len(pool)
            return pool[x]
        entry = {r: value(('entry', r)) for r in REGS}
        # Union liveness requires identity at every physical register crossing a
        # block boundary. Local temporaries may freely change register allocation.
        live = [set() for _ in L]
        for bi in reversed(range(len(L))):
            for graph in (L, R):
                body, succ = graph[bi]
                needed = set().union(*(live[s] for s in succ)) if succ else set(OBSERVE)
                for d in reversed(body):
                    needed.difference_update(d['def'])
                    needed.update(d['use'])
                live[bi].update(needed)
        def run(graph, side):
            ends, effects, summaries = {}, {}, []
            for bi, (body, succ) in enumerate(graph):
                preds = [p for p in range(bi) if bi in graph[p][1]]
                if bi and not preds:
                    raise Unproved('unreachable block')
                state = dict(entry)
                for r in live[bi]:
                    if preds:
                        incoming = tuple((p, ends[p][r]) for p in preds)
                        state[r] = incoming[0][1] if len({v for _, v in incoming}) == 1 else value(('phi', bi, r, incoming))
                mem = []
                if preds:
                    # All predecessor effects are represented in a single token;
                    # no instruction may schedule across a basic block boundary.
                    mem.append(('call', None, value(('incoming-effects', tuple((p, effects[p]) for p in preds)))))
                nodes = collections.Counter()
                memory_values = {}
                reads = set()
                fp = value(('fp-entry', bi))
                term_sig = None
                for d in body:
                    op, kind = d['op'], d['kind']
                    uses = tuple(state[r] for r in d['use'])
                    if kind == 'copy':
                        state[d['def'][0]] = uses[0]
                        continue
                    if op in COMMUTATIVE and len(uses) == 2:
                        uses = tuple(sorted(uses))
                    if kind == 'term':
                        term_sig = (op, uses, succ)
                        continue
                    addr = None
                    if kind in ('load', 'store'):
                        addr = (state[d['base']], d['offset'], d['width'])
                    def disjoint(other):
                        if addr is None or other is None:
                            return False
                        b, o, w = addr
                        b2, o2, w2 = other
                        return b == b2 and isinstance(o, int) and isinstance(o2, int) and (o + w <= o2 or o2 + w2 <= o)
                    deps = ()
                    if kind in ('load', 'store', 'call'):
                        deps = tuple(sorted({v for k, a, v in mem if
                            (kind == 'call' or k == 'call' or kind == 'store' or k == 'store') and not disjoint(a)}))
                    load_key = None
                    if kind == 'load' and not op.endswith('u'):
                        load_key = (op, addr, d.get('mode', ()), uses)
                        if load_key in memory_values:
                            state[d['def'][0]] = memory_values[load_key][1]
                            continue
                    if kind in ('store', 'call'):
                        # A write through any unknown base or a call invalidates
                        # every possibly aliasing remembered load/store value.
                        memory_values = {key: item for key, item in memory_values.items()
                                         if kind != 'call' and disjoint(item[0])}
                    const = d.get('const', ())
                    # Include the address displacement/width, otherwise a load
                    # from another field could look like the same producer.
                    if addr is not None:
                        const = (d['offset'], d['width'], d.get('mode', ()))
                    sig = value(('node', bi, op, uses, const, deps, fp if kind == 'float' or d.get('fp_memory') else None))
                    if not d.get('private_spill'):
                        # Loads are value reads, not instruction-count obligations:
                        # identical reads of an unchanged memory version are one node.
                        if kind != 'load' or sig not in reads:
                            nodes[sig] += 1
                            if kind in ('load', 'store', 'call'):
                                mem.append((kind, addr, sig))
                        if kind == 'load':
                            reads.add(sig)
                    if kind == 'store' and op == 'stw':
                        # Only a full-width integer round trip is an identity.
                        # Narrow/sign-extending or floating conversions stay explicit.
                        key = ('lwz', addr, (), uses[1:])
                        memory_values[key] = (addr, uses[0])
                    if kind in ('float', 'call') or d.get('fp_memory'):
                        fp = value(('fp-after', sig, fp))
                    for j, r in enumerate(d['def']):
                        if addr is not None and r == d['base'] and op.endswith('u'):
                            state[r] = value(('address-add', addr[0], addr[1]))
                        elif kind == 'call' and r not in ('r3', 'r4', 'f1'):
                            # Unspecified volatile outputs (including LR's actual
                            # code address) cannot be correlated across binaries.
                            state[r] = value(('call-clobber', side, sig, r))
                        else:
                            state[r] = value(('result', sig, j))
                    if load_key is not None:
                        memory_values[load_key] = (addr, state[d['def'][0]])
                needed = set().union(*(live[s] for s in succ)) if succ else set(OBSERVE)
                boundary = tuple((r, state[r]) for r in sorted(needed))
                # Effect summary order is a multiset: individual effect nodes
                # already encode every required ordering edge.
                effects[bi] = tuple(sorted(v for _, _, v in mem))
                ends[bi] = state
                summaries.append((nodes, term_sig, boundary, effects[bi], fp))
            return summaries
        if run(L, 'L') != run(R, 'R'):
            raise Unproved('value/effect DAG or live boundary mismatch')
        copies = sum(decode_mem(x)['kind'] == 'copy' for x in left + right)
        return True, f'{len(L)} blocks; memory-versioned value/effect DAGs, copy aliases ({copies}), typed load reuse/word forwarding, private dead-frame spills, ABI and may-alias order checked'
    except (Unproved, ValueError, KeyError, IndexError) as e:
        return False, str(e)
