#!/usr/bin/env python3
"""Conservative address-chain audit of Wii Nerve dispatch (no compiler/build).

Use --inventory-only, --class FQN (bounded), or explicit --sweep. See nervemap.md.
JSON contains addresses/hashes, never original instruction/data payloads.
"""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

SCHEMA = 1


def canonical_sha(value):
    return sha(json.dumps(value, sort_keys=True, separators=(',', ':')).encode())


def sha(data):
    return hashlib.sha256(data).hexdigest()


def signed(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def address(value):
    return None if value is None else f'0x{value:08x}'


def mask_cpp(text):
    """Preserve positions/newlines while removing comments and literal contents."""
    pattern = r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    return re.sub(pattern, lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


def balanced(text, start, left='(', right=')'):
    level = 0
    for i in range(start, len(text)):
        if text[i] == left:
            level += 1
        elif text[i] == right:
            level -= 1
            if not level:
                return i
    return None


def arguments(text):
    out, start, depth = [], 0, 0
    for i, c in enumerate(text):
        if c in '(<[':
            depth += 1
        elif c in ')>]':
            depth -= 1
        elif c == ',' and depth == 0:
            out.append(text[start:i].strip())
            start = i + 1
    return out + [text[start:].strip()]


def qualifier(tail):
    """Decode MW length-prefixed owner, leaving its function type unconsumed."""
    pos, count = 0, 1
    if tail.startswith('Q'):
        if len(tail) < 2 or not tail[1].isdigit():
            return None
        count, pos = int(tail[1]), 2
    parts = []
    for _ in range(count):
        m = re.match(r'\d+', tail[pos:])
        if not m:
            return None
        n = int(m[0])
        pos += len(m[0])
        if not n or pos + n > len(tail):
            return None
        parts.append(tail[pos:pos + n])
        pos += n
    return '::'.join(parts), tail[pos:]


def member(name):
    m = re.match(r'(.+?)__(Q\d.*|\d.*)$', name)
    if not m:
        return None
    q = qualifier(m[2])
    return (q[0], m[1], q[1]) if q else None


class Symbols:
    def __init__(self, path):
        self.path = Path(path)
        self.rows = []
        self.names = collections.defaultdict(list)
        self.at = collections.defaultdict(list)
        self.members = collections.defaultdict(list)
        for line, text in enumerate(self.path.read_text().splitlines(), 1):
            m = re.match(r'(.+?)\s*=\s*(\.?\w+):0x([\da-fA-F]+);(.*)', text)
            if not m:
                continue
            size = re.search(r'\bsize:0x([\da-fA-F]+)', m[4])
            row = dict(name=m[1].strip().strip('"'), section=m[2], address=int(m[3], 16),
                       size=int(size[1], 16) if size else 0, line=line,
                       size_explicit=size is not None,
                       symbol_type=(re.search(r'\btype:(\w+)', m[4])[1] if re.search(r'\btype:(\w+)', m[4]) else None),
                       function='type:function' in m[4])
            self.rows.append(row)
            self.names[row['name']].append(row)
            self.at[row['address']].append(row)
            parsed = member(row['name'])
            if parsed:
                self.members[(parsed[0], parsed[1])].append(row)
        self.functions = [r for r in self.rows if r['function'] and r['size']]

    def one(self, rows):
        # Even same-address aliases can hide competing ownership. Never pick one.
        return rows[0] if len(rows) == 1 else None

    def get(self, cls, meth):
        return self.one(self.members.get((cls, meth), []))

    def evidence(self, row):
        return dict(path=str(self.path), line=row['line'], symbol=row['name'],
                    address=address(row['address']), section=row['section'], size=row['size'],
                    size_explicit=row['size_explicit'], symbol_type=row['symbol_type'])


class Splits:
    def __init__(self, path):
        self.path, self.rows = Path(path), []
        unit = None
        for line in self.path.read_text().splitlines():
            if line and not line[0].isspace() and line.endswith(':'):
                unit = line[:-1]
            m = re.match(r'\s+(\.\w+)\s+start:0x([\da-fA-F]+)\s+end:0x([\da-fA-F]+)', line)
            if m and unit:
                self.rows.append((int(m[2], 16), int(m[3], 16), m[1], unit))

    def owners(self, addr, section=None):
        return sorted({unit for lo, hi, sec, unit in self.rows
                       if lo <= addr < hi and (section is None or sec == section)})

    def owns_extent(self, addr, size, section, unit):
        """One complete owned range, with no competing overlap at any byte."""
        if type(size) is not int or size <= 0:
            return False
        end = addr + size
        ranges = [(lo, hi, name) for lo, hi, sec, name in self.rows if sec == section and lo < end and addr < hi]
        return any(lo <= addr and end <= hi for lo, hi, _ in ranges) and all(name == unit for _, _, name in ranges)


class Dol:
    def __init__(self, path):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        if len(self.data) < 0x100:
            raise ValueError('truncated DOL header')
        self.segments = []
        for i in range(18):
            off, base, size = (struct.unpack_from('>I', self.data, p + 4 * i)[0]
                               for p in (0, 0x48, 0x90))
            if size:
                if off < 0x100 or off + size > len(self.data):
                    raise ValueError('DOL section outside file')
                self.segments.append((base, base + size, off, 'text' if i < 7 else 'data'))
        for i, (lo, hi, off, _) in enumerate(self.segments):
            for other_lo, other_hi, other_off, _ in self.segments[i + 1:]:
                if ((lo < other_hi and other_lo < hi) or
                    (off < other_off + other_hi - other_lo and other_off < off + hi - lo)):
                    raise ValueError('overlapping DOL memory or file sections')

    def read(self, addr, size):
        matches = [(lo, off) for lo, hi, off, _ in self.segments if lo <= addr and addr + size <= hi]
        if len(matches) != 1:
            raise ValueError('span is not uniquely DOL backed')
        lo, off = matches[0]
        return self.data[off + addr - lo:off + addr - lo + size]

    def word(self, addr):
        return struct.unpack('>I', self.read(addr, 4))[0]

    def span(self, addr, size):
        return dict(path=str(self.path), address=address(addr), size=size,
                    sha256=sha(self.read(addr, size)))


def branch(word, pc):
    if word >> 26 == 18:
        disp = signed(word & 0x3fffffc, 26)
        return (disp if word & 2 else pc + disp) & 0xffffffff
    if word >> 26 == 16:
        disp = signed(word & 0xfffc, 16)
        return (disp if word & 2 else pc + disp) & 0xffffffff
    return None


def trace(image, fn, initial, limit=20000, tracked_object=None):
    """Small constant-only PPC CFG interpreter. Unknown paths join to unknown.

    No pointer loads are treated as immutable constants. No indirect targets,
    relocation placeholders or adjusted-this thunks are guessed.
    """
    start, end = fn['address'], fn['address'] + fn['size']
    states, pending, events, issues = {}, [(start, dict(initial))], {}, set()
    steps = 0
    while pending:
        pc, regs = pending.pop()
        if not start <= pc < end or pc & 3:
            issues.add('control_flow_outside_extent')
            continue
        if pc in states:
            merged = {r: v for r, v in states[pc].items() if regs.get(r) == v}
            if merged == states[pc]:
                continue
            regs = merged
        states[pc] = dict(regs)
        steps += 1
        if steps > limit:
            issues.add('cfg_limit')
            break
        w = image.word(pc)
        op, rt, ra, rb = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        imm = signed(w & 0xffff, 16)
        old = dict(regs)
        nxt = pc + 4
        event = None
        if op in (14, 15):
            base = 0 if ra == 0 else regs.get(ra)
            regs.pop(rt, None)
            if base is not None:
                regs[rt] = (base + (imm << 16 if op == 15 else imm)) & 0xffffffff
        elif op in (24, 25, 26, 27, 28, 29):
            regs.pop(ra, None)
            if rt in old:
                value = (w & 0xffff) << (16 if op in (25, 27, 29) else 0)
                regs[ra] = (old[rt] | value) if op in (24, 25) else (old[rt] ^ value) if op in (26, 27) else (old[rt] & value)
        elif op == 31 and ((w >> 1) & 1023) == 444:
            regs.pop(ra, None)
            if rt in old and rb in old:
                regs[ra] = old[rt] | old[rb]
        elif op in (36, 37):
            base = 0 if ra == 0 else old.get(ra)
            event = dict(kind='store', address=None if base is None else (base + imm) & 0xffffffff,
                         value=old.get(rt))
            if tracked_object is not None:
                if event['address'] == tracked_object:
                    regs.pop('vptr', None)
                    if event['value'] is not None:
                        regs['vptr'] = event['value']
                elif event['address'] is not None and tracked_object - 3 <= event['address'] <= tracked_object + 3:
                    regs.pop('vptr', None)
                    issues.add('overlapping_unaligned_vptr_store')
                elif event['address'] is None and ra != 1:
                    issues.add('unresolved_relevant_store')
            if op == 37:
                regs.pop(ra, None)
        elif op in (18, 16):
            target = branch(w, pc)
            if w & 1 or (op == 18 and not start <= target < end):
                event = dict(kind='call', target=target, **{'r' + str(r): old.get(r) for r in range(32)})
                for r in (0, *range(3, 13)):
                    regs.pop(r, None)
                if not w & 1:
                    nxt = None
            elif op == 18:
                nxt = target
            else:
                pending.append((target, dict(regs)))
        elif w == 0x4e800020:
            event = dict(kind='return', r2=old.get(2), r13=old.get(13))
            if tracked_object is not None:
                event['vptr'] = old.get('vptr')
            nxt = None
        elif op == 19:
            # bctrl/bclrl (and nontrivial return dispatch) stop proof, not guess.
            issues.add('indirect_control_flow')
            nxt = None
        elif op in (10, 11):
            pass  # compares do not write GPRs; conditional branch handled above
        elif op in (32, 33, 34, 35, 40, 41, 42, 43, 46):
            for r in range(rt, 32) if op == 46 else (rt,):
                regs.pop(r, None)
            if op in (33, 35, 41, 43):
                regs.pop(ra, None)
        elif op in (38, 39, 44, 45, 47, *range(48, 56)):
            # Any unsupported store might partly overwrite a previously established
            # vptr. Stack stores with a known disjoint address are harmless.
            is_store = op in (38, 39, 44, 45, 47, 52, 53, 54, 55)
            if is_store:
                base = 0 if ra == 0 else old.get(ra)
                loc = None if base is None else (base + imm) & 0xffffffff
                width = 1 if op in (38, 39) else 2 if op in (44, 45) else 4 if op in (52, 53) else 8 if op in (54, 55) else (32 - rt) * 4
                event = dict(kind='unsupported_store', address=loc, width=width, stack=ra == 1)
                if tracked_object is not None and ((loc is None and ra != 1) or (loc is not None and loc < tracked_object + 4 and tracked_object < loc + width)):
                    regs.pop('vptr', None)
                    issues.add('unsupported_overlapping_or_unresolved_store')
            if op in (39, 45, 49, 51, 53, 55):
                regs.pop(ra, None)
        elif op == 31 and ((w >> 1) & 1023) == 339:
            regs.pop(rt, None)  # mfspr (including mflr), no constant result
        elif op in (59, 63):
            pass  # floating-point arithmetic does not materialize GPR addresses
        elif op == 31 and ((w >> 1) & 1023) in (0, 32, 144, 467, 598):
            pass  # cmp/cmpl, mtcrf, mtspr, sync
        else:
            # Unsupported instruction: discard possible destinations and fail closed.
            regs.pop(rt, None)
            regs.pop(ra, None)
            issues.add('unsupported_instruction')
        if event:
            # Revisited CFG edges must not retain the first, more optimistic value.
            if pc in events:
                event = {k: v if events[pc].get(k) == v else None for k, v in event.items()}
            events[pc] = dict(event, pc=pc)
        if nxt is not None and nxt < end:
            pending.append((nxt, regs))
    return list(events.values()), sorted(issues)


def wii_source(text):
    """Select only recognized Wii/native conditionals; retain other arms marked uncertain."""
    text = mask_cpp(text)
    lines, stack, uncertain, offset = [], [], [], 0
    for line in text.splitlines(keepends=True):
        m = re.match(r'\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)', line)
        if m:
            cmd, expr = m[1], m[2].strip()
            if cmd in ('if', 'ifdef', 'ifndef'):
                known = None
                if expr == 'GALAXY_NATIVE' or expr in ('defined(GALAXY_NATIVE)', 'defined GALAXY_NATIVE'):
                    known = False
                elif expr == '!defined(GALAXY_NATIVE)':
                    known = True
                elif expr in ('__MWERKS__', '__cplusplus'):
                    known = True
                elif cmd == 'if' and expr in ('0', '1'):
                    known = expr == '1'
                if cmd == 'ifndef' and known is not None:
                    known = not known
                stack.append(known)
            elif cmd == 'else' and stack:
                stack[-1] = None if stack[-1] is None else not stack[-1]
            elif cmd == 'elif' and stack:
                stack[-1] = None
            elif cmd == 'endif' and stack:
                stack.pop()
            lines.append(re.sub(r'[^\n]', ' ', line))
            offset += len(line)
            continue
        uncertain.append((offset, any(v is None for v in stack)))
        lines.append(line if False not in stack else re.sub(r'[^\n]', ' ', line))
        offset += len(line)
    return ''.join(lines), uncertain


def macro_defs(path, text):
    out = []
    for m in re.finditer(r'^[ \t]*#[ \t]*define[ \t]+(\w+)(\([^\n)]*\))?((?:[^\n]*\\\n)*[^\n]*)', text, re.M):
        body = m[3].replace('\\\n', ' ').strip()
        out.append(dict(name=m[1], params=arguments(m[2][1:-1]) if m[2] else None, body=body,
                        dispatch=bool(re.search(r'\bexecute\b|public\s+Nerve\b', body)),
                        path=path, line=text.count('\n', 0, m.start()) + 1))
    return out


def executor_call(body, formal):
    """Join the actual Spine formal -> mExecutor cast -> actual receiver use.

    Accept only complete, side-effect-free templates; finding a cast and a call
    independently is not provenance. Other forms deliberately remain UNKNOWN.
    """
    cast = (r'(?:reinterpret_cast|static_cast)\s*<\s*(?P<host>[\w:]+)\s*\*\s*>\s*'
            r'\(\s*' + re.escape(formal) + r'\s*->\s*mExecutor\s*\)')
    invoke = r'\s*->\s*(?P<method>\w+)\s*\((?P<args>[\s\S]*)\)\s*;\s*'
    direct = re.fullmatch(r'\s*' + cast + invoke, body)
    if direct and not re.search(r'[;{}]', direct['args']):
        return direct.groupdict()
    # Named local must be the same typed cast receiver that is invoked next.
    local = re.fullmatch(r'\s*(?P<decl>[\w:]+)\s*\*\s*(?P<var>\w+)\s*=\s*' + cast +
                         r'\s*;\s*(?P=var)' + invoke, body)
    if local and local['decl'] in (local['host'], 'auto') and not re.search(r'[;{}]', local['args']):
        return local.groupdict()
    return None


class Source:
    """Narrow lexical C++ binding extractor, NOT a preprocessor/compiler substitute."""
    def __init__(self, root):
        self.root = Path(root)
        self.files, self.macros, self.defs, self.producers = {}, [], collections.defaultdict(list), []
        self.unknown_forms = []
        self.nerve_classes = set()
        self.class_defs, self.instance_defs = collections.defaultdict(list), collections.defaultdict(list)
        for top in ('src/Game', 'include/Game'):
            for path in sorted((self.root / top).rglob('*')):
                if path.suffix not in ('.cpp', '.hpp', '.h', '.inc', '.inl'):
                    continue
                rel = path.relative_to(self.root).as_posix()
                raw = path.read_text(errors='replace')
                code, uncertainty = wii_source(raw)
                includes = re.findall(r'^\s*#\s*include\s*[<"]([^>"\n]+)', raw, re.M)
                self.files[rel] = dict(raw=raw, code=code, uncertain=uncertainty,
                                       sha256=sha(path.read_bytes()), includes=includes)
                for definition in macro_defs(rel, code):
                    definition['conditional_unknown'] = next((state for start, state in reversed(uncertainty)
                                                              if code.count('\n', 0, start) + 1 <= definition['line']), False)
                    self.macros.append(definition)
        self.reach_cache = {}
        for rel in self.files:
            self.parse(rel)

    def reachable(self, unit):
        if unit not in self.files and 'src/' + unit in self.files:
            unit = 'src/' + unit
        if unit in self.reach_cache:
            return self.reach_cache[unit]
        seen, pending = set(), [unit]
        while pending:
            path = pending.pop()
            if path in seen or path not in self.files:
                continue
            seen.add(path)
            for inc in self.files[path]['includes']:
                for candidate in ((Path(path).parent / inc).as_posix(), 'include/' + inc, 'src/' + inc):
                    if candidate in self.files:
                        pending.append(candidate)
        self.reach_cache[unit] = seen
        return seen

    def expand(self, rel):
        data = self.files[rel]
        code = data['code']
        # Blank all directives, including continuation lines. Preserve original positions.
        code = re.sub(r'^[ \t]*#(?:[^\n]*\\\n)*[^\n]*', lambda m: re.sub(r'[^\n]', ' ', m[0]), code, flags=re.M)
        reachable = self.reachable(rel)
        defs = collections.defaultdict(list)
        for macro in self.macros:
            if macro['path'] in reachable and macro['dispatch'] and macro['params'] is not None:
                defs[macro['name']].append(macro)
        chunks, spans, end, offset = [], [], 0, 0
        for m in re.finditer(r'\b(\w+)\s*\(', code):
            if m.start() < end or m[1] not in defs:
                continue
            stop = balanced(code, code.index('(', m.start()))
            if stop is None:
                continue
            definition = self.macro_at(m[1], rel, code.count('\n', 0, m.start()) + 1)
            if definition is None or definition['params'] is None:
                continue
            args = arguments(code[code.index('(', m.start()) + 1:stop])
            if len(args) != len(definition['params']):
                continue
            text = re.sub(r'\b(' + '|'.join(map(re.escape, definition['params'])) + r')\b',
                          lambda t: args[definition['params'].index(t[0])], definition['body'])
            text = re.sub(r'\s*##\s*', '', text)
            prefix = code[end:m.start()]
            chunks.extend((prefix, text))
            offset += len(prefix)
            spans.append((offset, offset + len(text), m.start(), definition))
            offset += len(text)
            end = stop + 1
        chunks.append(code[end:])
        return ''.join(chunks), spans

    def macro_at(self, name, rel, line):
        """Resolve only unambiguous active definitions; include ordering is not guessed."""
        reachable = self.reachable(rel)
        choices = [d for d in self.macros if d['name'] == name and d['path'] in reachable
                   and (d['path'] != rel or d['line'] <= line)]
        reason = None
        if any(d['conditional_unknown'] for d in choices):
            reason = 'conditional_macro_definition'
        elif any(re.search(r'^\s*#\s*undef\s+' + re.escape(name) + r'\b', self.files[p]['code'], re.M)
                 for p in reachable):
            reason = 'macro_undef_context'
        elif len({(None if d['params'] is None else tuple(d['params']), d['body']) for d in choices}) != 1:
            reason = 'missing_or_ambiguous_macro_definition'
        if reason:
            self.unknown_forms.append(dict(path=rel, line=line, reason=reason, macro=name))
            return None
        return choices[0]

    def nerve_expr(self, expr, rel, line, scopes):
        """Bind getters by their actual expansion, never by GET_NERVE spelling."""
        macros = []
        for _ in range(8):
            expr = expr.strip()
            while expr.startswith('(') and balanced(expr, 0) == len(expr) - 1:
                expr = expr[1:-1].strip()
            literal = re.fullmatch(r'&\s*(::)?([\w:]+)::sInstance', expr)
            if literal:
                target = literal[2]
                if not literal[1] and '::' not in target:
                    target = '::'.join(scopes + [target])
                # Object macros can rewrite identifiers even in qualified expressions.
                if self.unexpanded_macros(expr, rel, line):
                    return None, macros
                return target, macros
            call = re.fullmatch(r'(\w+)\s*\(([\s\S]*)\)', expr)
            if not call:
                return None, macros
            definition = self.macro_at(call[1], rel, line)
            if definition is None or definition['params'] is None:
                return None, macros
            args = arguments(call[2])
            if len(args) != len(definition['params']):
                return None, macros
            # Expanding macro-valued arguments before ## requires a real preprocessor.
            if any(self.unexpanded_macros(arg, rel, line) for arg in args):
                return None, macros
            params = definition['params']
            expr = re.sub(r'\b(' + '|'.join(map(re.escape, params)) + r')\b',
                          lambda t: args[params.index(t[0])], definition['body']) if params else definition['body']
            expr = re.sub(r'\s*##\s*', '', expr)
            macros.append(dict(name=definition['name'], path=definition['path'], line=definition['line'],
                               sha256=self.files[definition['path']]['sha256']))
        return None, macros

    def unexpanded_macros(self, text, rel, line):
        """Reject semantic tokens that could still be changed by preprocessing."""
        tokens = set(re.findall(r'\b\w+\b', text))
        reachable = self.reachable(rel)
        return any(d['name'] in tokens and d['path'] in reachable
                   and (d['path'] != rel or d['line'] <= line) for d in self.macros)

    def parse(self, rel):
        original = self.files[rel]['code']
        code, spans = self.expand(rel)
        # Convert expanded positions back to a pinned original invocation or source line.
        def evidence(pos):
            delta = 0
            macro = None
            for lo, hi, orig, definition in spans:
                if lo <= pos < hi:
                    pos, macro = orig, definition
                    break
                if hi <= pos:
                    # Original invocation length recovered with balanced parentheses.
                    stop = balanced(original, original.index('(', orig))
                    delta += (hi - lo) - (stop + 1 - orig)
            else:
                pos -= delta
            line = original.count('\n', 0, pos) + 1
            uncertain = False
            for start, state in self.files[rel]['uncertain']:
                if start > pos:
                    break
                uncertain = state
            out = dict(path=rel, line=line, sha256=self.files[rel]['sha256'], conditional_unknown=uncertain)
            if macro:
                out['macro'] = dict(name=macro['name'], path=macro['path'], line=macro['line'],
                                    sha256=self.files[macro['path']]['sha256'])
            return out
        # Brace traversal associates definitions with the actual namespace/class, never its state spelling.
        opens, class_headers = {}, {}
        for m in re.finditer(r'\b(namespace\s+([\w:]+)|(?:class|struct)\s+(\w+)\s*[^;{}]*)\s*\{', code):
            pos = code.index('{', m.start())
            opens[pos] = ('scope', m[2] or m[3])
            if m[3]:
                class_headers[pos] = m[0]
        function_pattern = r'\b(?:void|bool|int|s32|u32|[\w:]+\*)\s+([\w:]+)\s*\([^;{}]*\)\s*(?:const\s*)?(?:NO_INLINE\s*)?\{'
        headers = {}
        for m in re.finditer(function_pattern, code):
            opens[code.index('{', m.start())] = ('function', m[1])
            headers[code.index('{', m.start())] = m[0]
        stack = []
        for m in re.finditer(r'[{}]', code):
            if m[0] == '}':
                if stack:
                    stack.pop()
                continue
            kind, name = opens.get(m.start(), ('block', ''))
            scopes = [n for k, n in stack if k == 'scope']
            if m.start() in class_headers:
                cls = '::'.join(scopes + [name])
                header = class_headers[m.start()]
                stop = balanced(code, m.start(), '{', '}')
                body = code[m.start() + 1:stop] if stop is not None else ''
                if re.search(r':[^{}]*\bNerve\b', header):
                    self.nerve_classes.add(cls)
                supported = bool(re.fullmatch(r'(?:class|struct)\s+' + re.escape(name) +
                                             r'\s*:\s*public\s+(?:::)?Nerve\s*\{', header))
                ev = evidence(m.start())
                supported = supported and not self.unexpanded_macros(header, rel, ev['line'])
                static_match = re.search(r'\bstatic\s+' + re.escape(name) + r'\s+sInstance\s*;', body)
                static_instance = bool(static_match and not self.unexpanded_macros(static_match[0], rel, ev['line']))
                base = re.fullmatch(r'(?:class|struct)\s+' + re.escape(name) +
                                    r'\s*:\s*public\s+([\w:]+)\s*\{', header)
                local_apis = re.findall(r'\b(?:void|bool|auto|[\w:]+\*)\s+(initNerve|setNerve)\s*\(', body)
                self.class_defs[cls].append(dict(class_name=cls, direct_nerve_base=supported,
                                                  static_instance=static_instance, single_public_base=base[1] if base else None,
                                                  local_apis=local_apis, evidence=ev))
            if kind == 'function':
                stop = balanced(code, m.start(), '{', '}')
                if stop is not None:
                    body = code[m.start() + 1:stop]
                    full = '::'.join(scopes + [name])
                    if name.split('::')[-1] == 'execute':
                        cls = full.rsplit('::', 1)[0]
                        ev = evidence(m.start())
                        formal = re.search(r'\(\s*(?:const\s+)?Spine\s*\*\s*(\w+)\s*\)', headers[m.start()])
                        source_call = executor_call(body, formal[1]) if formal else None
                        casts = [source_call['host']] if source_call else []
                        calls = [source_call['method']] if source_call else []
                        simple = source_call is not None
                        transition = None
                        getter_evidence = []
                        semantic_body = body
                        transition_call = re.search(r'->\s*setNerve\s*\(', body)
                        if simple and calls[0] == 'setNerve' and transition_call:
                            begin = body.index('(', transition_call.start())
                            finish = balanced(body, begin)
                            if finish is not None:
                                expr = body[begin + 1:finish].strip()
                                transition, getter_evidence = self.nerve_expr(expr, rel, ev['line'], scopes)
                                semantic_body = body[:begin + 1] + body[finish:]
                        if self.unexpanded_macros(headers[m.start()] + semantic_body, rel, ev['line']):
                            simple = False
                        if calls and calls[0] == 'setNerve' and transition is None:
                            simple = False
                        if simple and calls[0] != 'setNerve' and source_call['args'].strip():
                            # A nonempty argument can change action/receiver semantics.
                            simple = False
                        self.defs[cls].append(dict(class_name=cls, target_class=casts[0] if simple else None,
                                                  target_method=calls[0] if simple else None,
                                                  transition_class=transition,
                                                  getter_evidence=getter_evidence,
                                                  kind='transition' if transition else 'direct_executor_member' if simple else 'state_controller_or_complex',
                                                  evidence=ev))
                    for call in re.finditer(r'\b(initNerve|setNerve)\s*\(', body):
                        a = body.index('(', call.start())
                        b = balanced(body, a)
                        if b is None:
                            continue
                        args = arguments(body[a + 1:b])
                        ev = evidence(m.start() + 1 + call.start())
                        target, getter_evidence = self.nerve_expr(args[0], rel, ev['line'], scopes) if args else (None, [])
                        prefix = body[:call.start()].rstrip()
                        receiver = 'this'
                        if name.split('::')[-1] == 'execute':
                            receiver = 'formal_executor' if simple and calls[0] == call[1] else 'unknown'
                        elif prefix.endswith('->'):
                            receiver = 'this' if re.search(r'\bthis\s*->\s*$', prefix) else 'unknown'
                        elif prefix.endswith('::') or prefix.endswith('.'):
                            receiver = 'unknown'
                        if receiver == 'unknown':
                            target = None
                        if self.unexpanded_macros(call[1], rel, ev['line']):
                            target = None
                        self.producers.append(dict(function=full, api=call[1], class_name=target,
                                                   receiver=receiver,
                                                   receiver_class=(source_call['host'] if simple else None) if name.split('::')[-1] == 'execute' else full.rsplit('::', 1)[0] if '::' in full else None,
                                                   getter_evidence=getter_evidence, evidence=ev))
            stack.append((kind, name))
        # Static definition type is a required ownership link, separate from the
        # class's execute definition. Aliases/inline/conditional variants are UNKNOWN.
        scope_spans = [(pos, balanced(code, pos, '{', '}'), value[1])
                       for pos, value in opens.items() if value[0] == 'scope']
        def qualify(text, scopes):
            prefix = '::'.join(scopes)
            if text.startswith('::'):
                return text[2:]
            if prefix and not text.startswith(prefix + '::'):
                return prefix + '::' + text
            return text
        for match in re.finditer(r'\b([\w:]+)\s+([\w:]+)::sInstance\s*;', code):
            scopes = [name for lo, hi, name in sorted(scope_spans) if hi is not None and lo < match.start() < hi]
            owner = qualify(match[2], scopes)
            ev = evidence(match.start())
            self.instance_defs[owner].append(dict(class_name=owner, instance_type=qualify(match[1], scopes),
                                                  preprocessing_bound=not self.unexpanded_macros(match[0], rel, ev['line']), evidence=ev))

    def binding(self, cls, unit):
        candidates = [d for d in self.defs.get(cls, []) if d['evidence']['path'] in self.reachable(unit)]
        classes = [d for d in self.class_defs.get(cls, []) if d['evidence']['path'] in self.reachable(unit)]
        instances = [d for d in self.instance_defs.get(cls, []) if d['evidence']['path'] in self.reachable(unit)]
        shadowed_base = any(k.endswith('::Nerve') and any(d['evidence']['path'] in self.reachable(unit) for d in defs)
                            for k, defs in self.class_defs.items())
        shadowed_base |= any(re.search(r'\busing\s+Nerve\s*=|\btypedef\b[^;]*\bNerve\s*;', self.files[p]['code'])
                             for p in self.reachable(unit))
        # Exact source location is retained; duplicates or conditional alternatives are not collapsed.
        if (len(candidates) != 1 or len(classes) != 1 or len(instances) != 1 or
            any(d['evidence']['conditional_unknown'] for d in candidates + classes + instances) or
            shadowed_base or not classes[0]['direct_nerve_base'] or not classes[0]['static_instance'] or
            not instances[0]['preprocessing_bound'] or instances[0]['instance_type'] != cls):
            return None
        return dict(candidates[0], class_declaration=classes[0], static_instance_definition=instances[0])

    def api_receiver(self, host, provider, unit, api):
        """Only a unique public single-base path with no hiding member is closed."""
        seen, evidence = set(), []
        reachable = self.reachable(unit)
        while host and host not in seen:
            seen.add(host)
            defs = [d for d in self.class_defs.get(host, []) if d['evidence']['path'] in reachable]
            if len(defs) != 1 or defs[0]['evidence']['conditional_unknown']:
                return None
            declaration = defs[0]
            evidence.append(declaration['evidence'])
            if host == provider:
                return evidence
            if api in declaration['local_apis']:
                return None
            base = declaration['single_public_base']
            if not base:
                return None
            if base.startswith('::'):
                host = base[2:]
            else:
                scopes = host.split('::')[:-1]
                host = None
                for count in range(len(scopes), -1, -1):
                    candidate = '::'.join(scopes[:count] + [base])
                    if any(d['evidence']['path'] in reachable for d in self.class_defs.get(candidate, [])):
                        host = candidate
                        break
        return None


def startup_bases(symbols, image):
    row = symbols.one(symbols.names.get('__init_registers', []))
    if not row or not unique_address(symbols, row):
        return {}, None
    events, issues = trace(image, row, {})
    returns = [e for e in events if e['kind'] == 'return']
    regs = {}
    if returns and not issues:
        for r in (2, 13):
            values = {e.get('r' + str(r)) for e in returns}
            if len(values) == 1 and None not in values:
                regs[r] = values.pop()
    return regs, dict(span=image.span(row['address'], row['size']), issues=issues,
                      returning_paths=len(returns), proof='entry-reachable constants agree on every supported return')


def source_function(row):
    parsed = member(row['name'])
    return None if not parsed else parsed[0] + '::' + parsed[1]


def unique_address(symbols, row):
    return len(symbols.at.get(row['address'], [])) == 1


def dispatch_consumer(symbols, image):
    """Require entry-reachable Spine-formal -> nerve -> vtable -> slot call.

    This bounded symbolic CFG does not search dead words for an ABI pattern.
    Unsupported instructions or indirect branches fail the consumer proof closed.
    """
    fn = symbols.get('Spine', 'update')
    if fn is None or not unique_address(symbols, fn):
        return dict(verified=False, reason='missing_or_aliased_Spine_update')
    start, end = fn['address'], fn['address'] + fn['size']
    states, pending, calls, issues = {}, [(start, {3: 'spine'})], {}, set()
    while pending and len(states) <= 4096:
        pc, regs = pending.pop()
        if not start <= pc < end or pc & 3:
            issues.add('consumer_control_outside_extent')
            continue
        if pc in states:
            merged = {r: v for r, v in states[pc].items() if regs.get(r) == v}
            if merged == states[pc]:
                continue
            regs = merged
        states[pc] = dict(regs)
        w = image.word(pc)
        op, rt, ra, rb = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        imm, old, nxt = signed(w & 0xffff, 16), dict(regs), pc + 4
        if op == 32:
            regs.pop(rt, None)
            tags = {('spine', 4): 'nerve', ('nerve', 0): 'vtable', ('vtable', 8): 'execute',
                    ('vtable', 12): 'onEnd'}
            if (old.get(ra), imm) in tags:
                regs[rt] = tags[(old.get(ra), imm)]
        elif op == 31 and ((w >> 1) & 1023) == 444:
            regs.pop(ra, None)
            if old.get(rt) is not None and old.get(rt) == old.get(rb):
                regs[ra] = old[rt]
        elif op == 14:
            regs.pop(rt, None)
            if imm == 0 and ra != 0 and ra in old:
                regs[rt] = old[ra]
        elif op == 31 and ((w >> 1) & 1023) == 467:
            spr = ((w >> 16) & 31) | (((w >> 11) & 31) << 5)
            if spr == 9:  # mtctr: the actual indirect target must carry the slot tag.
                regs.pop('ctr', None)
                if rt in old:
                    regs['ctr'] = old[rt]
        elif w == 0x4e800421:
            valid = old.get('ctr') == 'execute' and old.get(3) == 'nerve' and old.get(4) == 'spine'
            calls[pc] = calls.get(pc, True) and valid
            for r in (0, *range(3, 13), 'ctr'):
                regs.pop(r, None)
        elif w == 0x4e800020:
            nxt = None
        elif op in (16, 18):
            target = branch(w, pc)
            if w & 1:
                for r in (0, *range(3, 13), 'ctr'):
                    regs.pop(r, None)
            elif op == 18:
                nxt = target
            else:
                pending.append((target, dict(regs)))
        elif op in (36, 38, 44, 47, 52, 54, 10, 11) or (op == 31 and ((w >> 1) & 1023) in (0, 32, 144)):
            pass  # stores/compares do not assign a receiver GPR
        elif op in (37, 39, 45, 53, 55):
            regs.pop(ra, None)
        elif op in (15, 34, 40, 42) or (op == 31 and ((w >> 1) & 1023) == 339):
            regs.pop(rt, None)
        elif op in (24, 25, 26, 27, 28, 29):
            regs.pop(ra, None)
        else:
            issues.add('unsupported_consumer_instruction')
            regs.pop(rt, None)
            regs.pop(ra, None)
            regs.pop('ctr', None)
        if nxt is not None and nxt < end:
            pending.append((nxt, regs))
    if pending:
        issues.add('consumer_cfg_limit')
    found = [image.span(pc, 4) for pc, valid in calls.items() if valid]
    verified = len(found) == 1 and not issues
    return dict(verified=verified, slot=8 if verified else None,
                evidence=found, function=symbols.evidence(fn),
                issues=sorted(issues), reason=None if verified else 'unsupported_Spine_dispatch_consumer')


def wrapper_action(symbols, image, fn, bases, api_addresses):
    """Only bounded unadjusted direct tail actions; preserve the actual extent."""
    if not fn or not unique_address(symbols, fn) or fn['size'] not in (8, 12, 16):
        return None
    pc = fn['address']
    if image.word(pc) != 0x80640000:
        return None
    last = image.word(pc + fn['size'] - 4)
    if last >> 26 != 18 or last & 1:
        return None
    target = branch(last, pc + fn['size'] - 4)
    if fn['size'] == 8 and target not in api_addresses:
        return dict(kind='method', target=target, span=image.span(pc, fn['size']))
    # A transition must explicitly materialize r4 without adjusting the receiver.
    middle = [image.word(p) for p in range(pc + 4, pc + fn['size'] - 4, 4)]
    if target not in api_addresses or api_addresses[target] != 'setNerve' or not middle:
        return None
    if any(w >> 26 not in (14, 15, 24, 25) or
           (((w >> 16) & 31) if w >> 26 in (24, 25) else ((w >> 21) & 31)) != 4 for w in middle):
        return None
    events, issues = trace(image, fn, bases)
    calls = [e for e in events if e['kind'] == 'call']
    if issues or len(calls) != 1 or calls[0].get('r4') is None:
        return None
    return dict(kind='transition', target=target, singleton=calls[0]['r4'], call_pc=pc + fn['size'] - 4,
                span=image.span(pc, fn['size']))


def entry_alias_candidate(root, symbols, splits, image, relation):
    """Proposal-only Runtime ENTRY evidence. NEVER grants API/alias acceptance.

    The deliberately tiny assembler admits only entry-tagged stw/lwz GPR slots
    through r11 and one final blr. All words are compared in memory, not emitted.
    Function extent is explicit input metadata; labels keep their absent sizes.
    """
    result = dict(status='UNKNOWN', production_acceptance=False, reasons=[], entries=[], records=[])
    reasons = result['reasons']
    try:
        required = {'kind', 'unit', 'source', 'owner', 'first_entry', 'registers', 'dol_sha256',
                    'symbols_sha256', 'splits_sha256', 'references'}
        if set(relation) != required or relation['kind'] != 'mwcc_runtime_entry_at_function_start':
            raise ValueError('unsupported_relation_schema')
        if relation['unit'] != 'Runtime/runtime.c' or relation['source']['path'] != 'src/Runtime/runtime.c':
            raise ValueError('unsupported_source_unit')
        registers = relation['registers']
        if (not isinstance(registers, list) or not registers or
            any(type(reg) is not int or not 14 <= reg <= 31 for reg in registers) or
            registers != list(range(registers[0], registers[-1] + 1))):
            raise ValueError('unsupported_gpr_entry_sequence')
        if (relation['dol_sha256'] != sha(image.data) or relation['symbols_sha256'] != sha(symbols.path.read_bytes()) or
            relation['splits_sha256'] != sha(splits.path.read_bytes())):
            raise ValueError('image_or_metadata_pin_mismatch')
        source_path = Path(root) / relation['source']['path']
        raw = source_path.read_text()
        if sha(source_path.read_bytes()) != relation['source']['sha256']:
            raise ValueError('source_pin_mismatch')
        code, uncertainty = wii_source(raw)
        if re.search(r'^\s*#\s*include\b', code, re.M):
            raise ValueError('unmodeled_include_macros')
        definitions = macro_defs(relation['source']['path'], code)
        def line_at(pos):
            return code.count('\n', 0, pos) + 1
        def uncertain(line):
            return next((state for pos, state in reversed(uncertainty) if line_at(pos) <= line), False)
        def macro(name, line):
            choices = [d for d in definitions if d['name'] == name and d['line'] <= line]
            if (len(choices) != 1 or uncertain(choices[0]['line']) or
                re.search(r'^\s*#\s*undef\s+' + re.escape(name) + r'\b', code, re.M)):
                raise ValueError('unresolved_macro_provenance')
            return choices[0]
        def macro_evidence(definition):
            return dict(path=relation['source']['path'], line=definition['line'], name=definition['name'],
                        source_sha256=relation['source']['sha256'], definition_sha256=sha(definition['body'].encode()))
        def no_semantic_macros(tokens, line):
            if any(d['name'] in tokens and d['line'] <= line for d in definitions):
                raise ValueError('unmodeled_semantic_macro')
        def expand_entry(text, line, require_entry=True):
            chain, entry, register = [], False, None
            for _ in range(8):
                if text.startswith('entry '):
                    no_semantic_macros({'entry'}, line)
                    if entry:
                        raise ValueError('multiple_entry_directives')
                    entry, text = True, text[6:].strip()
                match = re.fullmatch(r'(\w+)\s*\(\s*(\d+)\s*\)', text)
                if not match:
                    if entry == require_entry and re.fullmatch(r'[A-Za-z_]\w*', text):
                        no_semantic_macros({text}, line)
                        return text, register, chain
                    raise ValueError('unsupported_entry_expansion')
                definition = macro(match[1], line)
                if definition['params'] is None or len(definition['params']) != 1:
                    raise ValueError('unsupported_entry_macro')
                current = int(match[2])
                if register is not None and register != current:
                    raise ValueError('entry_register_changed_by_macro')
                register = current
                text = re.sub(r'\b' + re.escape(definition['params'][0]) + r'\b', match[2], definition['body'])
                text = re.sub(r'\s*##\s*', '', text).strip()
                chain.append(macro_evidence(definition))
            raise ValueError('entry_macro_depth')
        def base_register(token, line):
            chain = []
            for _ in range(8):
                if re.fullmatch(r'r(?:[0-9]|[12][0-9]|3[01])', token):
                    no_semantic_macros({token}, line)
                    if token != 'r11':
                        raise ValueError('unsupported_save_restore_base')
                    return 11, chain
                definition = macro(token, line)
                if definition['params'] is not None:
                    raise ValueError('unsupported_register_alias')
                token = definition['body'].strip()
                chain.append(macro_evidence(definition))
            raise ValueError('register_alias_depth')
        owner_pin = relation['owner']
        owners = symbols.names.get(owner_pin['symbol'], [])
        if len(owners) != 1:
            raise ValueError('ambiguous_owner_symbol')
        owner = owners[0]
        start, end = owner['address'], owner['address'] + owner['size']
        def complete_owner(lo, hi, section, unit):
            return splits.owns_extent(lo, hi - lo, section, unit)
        result['records'] = [symbols.evidence(row) for row in symbols.rows if
                             start <= row['address'] < end or
                             (row['size'] and row['address'] < end and start < row['address'] + row['size'])]
        if (set(owner_pin) != {'symbol', 'section', 'address', 'size', 'unit', 'sha256'} or
            owner['symbol_type'] != 'function' or owner['section'] != '.text' or not owner['size_explicit'] or not owner['size'] or
            owner_pin['address'] != address(start) or owner_pin['size'] != owner['size'] or
            owner_pin['section'] != owner['section'] or owner_pin['unit'] != relation['unit']):
            raise ValueError('owner_type_or_explicit_extent_mismatch')
        if (not complete_owner(start, end, owner['section'], relation['unit']) or
            splits.owners(start, owner['section']) != [relation['unit']] or
            splits.owners(end - 1, owner['section']) != [relation['unit']] or
            not any(lo <= start and end <= hi and sec == owner['section'] and unit == relation['unit']
                    for lo, hi, sec, unit in splits.rows)):
            raise ValueError('missing_or_ambiguous_extent_ownership')
        if sha(image.read(start, owner['size'])) != owner_pin['sha256']:
            raise ValueError('owner_body_pin_mismatch')
        function = re.compile(r'\bstatic\s+asm\s+void\s+' + re.escape(owner['name']) + r'\s*\(\s*void\s*\)\s*\{')
        functions = list(function.finditer(code))
        if len(functions) != 1:
            raise ValueError('missing_or_ambiguous_source_function')
        function = functions[0]
        begin, finish = code.index('{', function.start()), balanced(code, code.index('{', function.start()), '{', '}')
        if finish is None:
            raise ValueError('unclosed_source_function')
        no_semantic_macros({'static', 'asm', 'void', owner['name']}, line_at(function.start()))
        words, entries, pending, noframe, operation = [], [], None, False, None
        for offset, text in enumerate(code[begin + 1:finish].splitlines()):
            line, text = line_at(begin + 1) + offset, text.strip()
            if not text:
                continue
            if uncertain(line):
                raise ValueError('conditional_source_body')
            if text == 'nofralloc' and not words and not entries and not noframe:
                no_semantic_macros({'nofralloc'}, line)
                noframe = True
            elif text == 'blr' and pending is None:
                no_semantic_macros({'blr'}, line)
                words.append(0x4e800020)
            elif re.fullmatch(r'\w+\s*\(\s*\d+\s*\)', text):
                if not noframe or pending is not None or 0x4e800020 in words:
                    raise ValueError('invalid_entry_position')
                name, register, chain = expand_entry(text, line)
                pending = dict(symbol=name, register=register, offset=4 * len(words), line=line, macros=chain)
                entries.append(pending)
            else:
                instruction = re.fullmatch(r'(stw|lwz)\s+(r\d+)\s*,\s*(-?\d+)\s*\(\s*(\w+)\s*\)', text)
                if instruction is None or pending is None or 0x4e800020 in words:
                    raise ValueError('unsupported_or_untagged_assembly')
                op, reg, displacement, base = instruction.groups()
                no_semantic_macros({op, reg}, line)
                rt, displacement = int(reg[1:]), int(displacement)
                ra, chain = base_register(base, line)
                if rt != pending['register'] or displacement != 4 * (rt - 32) or (operation and operation != op):
                    raise ValueError('entry_instruction_register_or_slot_mismatch')
                operation = op
                pending['base_macros'] = chain
                words.append(((36 if op == 'stw' else 32) << 26) | (rt << 21) | (ra << 16) | (displacement & 0xffff))
                pending = None
        if (not words or words[-1] != 0x4e800020 or words.count(0x4e800020) != 1 or pending is not None or
            not entries or entries[0]['offset'] != 0 or entries[0]['symbol'] != relation['first_entry'] or
            [entry['register'] for entry in entries] != relation['registers'] or
            len(words) != len(entries) + 1):
            raise ValueError('incomplete_entry_sequence')
        encoded = struct.pack('>' + 'I' * len(words), *words)
        if len(encoded) != owner['size'] or encoded != image.read(start, owner['size']):
            raise ValueError('source_body_or_extent_conflict')
        declarations = collections.defaultdict(list)
        for declaration in re.finditer(r'\bvoid\s+(\w+\s*\(\s*\d+\s*\))\s*\(\s*void\s*\)\s*;', code[:function.start()]):
            line = line_at(declaration.start())
            try:
                name, register, chain = expand_entry(declaration[1], line, require_entry=False)
            except ValueError:
                continue  # An unresolved relevant declaration will remain missing.
            declarations[name].append(dict(path=relation['source']['path'], line=line, register=register,
                                            conditional_unknown=uncertain(line), macros=chain,
                                            source_sha256=relation['source']['sha256']))
        expected_lines = {owner['line']}
        for entry in entries:
            prototypes = declarations.get(entry['symbol'], [])
            if (len(prototypes) != 1 or prototypes[0]['conditional_unknown'] or
                prototypes[0]['register'] != entry['register']):
                raise ValueError('missing_or_ambiguous_source_entry_prototype')
            no_semantic_macros({'void'}, prototypes[0]['line'])
            entry['declaration'] = prototypes[0]
            rows = symbols.names.get(entry['symbol'], [])
            if len(rows) != 1:
                raise ValueError('missing_or_ambiguous_entry_record')
            row = rows[0]
            if (row['symbol_type'] != 'label' or row['size_explicit'] or row['section'] != owner['section'] or
                row['address'] != start + entry['offset'] or splits.owners(row['address'], row['section']) != [relation['unit']]):
                raise ValueError('entry_type_address_size_or_owner_conflict')
            expected_lines.add(row['line'])
            entry['record'] = symbols.evidence(row)
            result['entries'].append(entry)
        if {record['line'] for record in result['records']} != expected_lines:
            raise ValueError('unrelated_or_overlapping_symbol_record')
        references = relation['references']
        if not isinstance(references, list) or not references:
            raise ValueError('missing_exact_entry_references')
        result['references'] = []
        for reference in references:
            if set(reference) != {'caller', 'pc', 'instruction_sha256', 'entry_symbol', 'call_kind'}:
                raise ValueError('unsupported_entry_reference_schema')
            caller_pin = reference['caller']
            if set(caller_pin) != {'symbol', 'section', 'address', 'size', 'unit', 'sha256'}:
                raise ValueError('unsupported_entry_caller_schema')
            callers = symbols.names.get(caller_pin['symbol'], [])
            if len(callers) != 1:
                raise ValueError('unresolved_entry_caller')
            caller = callers[0]
            if (not unique_address(symbols, caller) or caller['symbol_type'] != 'function' or
                not caller['size_explicit'] or not caller['size'] or caller_pin['address'] != address(caller['address']) or
                caller_pin['section'] != caller['section'] or caller_pin['size'] != caller['size'] or
                not complete_owner(caller['address'], caller['address'] + caller['size'], caller['section'], caller_pin['unit']) or
                splits.owners(caller['address'], caller['section']) != [caller_pin['unit']] or
                not any(lo <= caller['address'] and caller['address'] + caller['size'] <= hi and
                        sec == caller['section'] and unit == caller_pin['unit'] for lo, hi, sec, unit in splits.rows) or
                sha(image.read(caller['address'], caller['size'])) != caller_pin['sha256']):
                raise ValueError('entry_caller_pin_or_owner_conflict')
            selected = [entry for entry in entries if entry['symbol'] == reference['entry_symbol']]
            pc = int(reference['pc'], 16)
            word = image.word(pc)
            if (len(selected) != 1 or pc & 3 or not caller['address'] <= pc < caller['address'] + caller['size'] or
                word >> 26 != 18 or branch(word, pc) != start + selected[0]['offset'] or
                reference['call_kind'] not in ('linked', 'tail') or bool(word & 1) != (reference['call_kind'] == 'linked') or
                sha(image.read(pc, 4)) != reference['instruction_sha256']):
                raise ValueError('entry_reference_target_or_pin_conflict')
            events, issues = trace(image, caller, {})
            calls = [event for event in events if event['kind'] == 'call' and event['pc'] == pc and
                     event.get('target') == start + selected[0]['offset']]
            if issues or len(calls) != 1:
                raise ValueError('unresolved_entry_reference_reachability')
            result['references'].append(dict(pc=reference['pc'], caller=symbols.evidence(caller),
                                              entry_symbol=selected[0]['symbol'], offset=selected[0]['offset'],
                                              entry_record=selected[0]['record'], reachable=True))
        result.update(status='CANDIDATE_BOUND', owner=symbols.evidence(owner), source=relation['source'],
                      source_function=dict(line=line_at(function.start()), end_line=line_at(finish)),
                      encoded_body_sha256=sha(encoded), explicit_owner_bytes=owner['size'],
                      limitations='Candidate evidence only: no alias grant, API acceptance, ABI/lifetime or source-class theorem.')
    except (OSError, ValueError, TypeError, KeyError, AttributeError, struct.error) as error:
        reasons.append(str(error))
    return result


def _entry_dependency_reviews(path, root, symbols, splits, image, claim):
    """Opt-in dependency identity reviews never establish API semantics."""
    admitted, evidence = {}, dict(status='absent', accepted=[], refused=[])
    if path is None:
        return admitted, evidence
    def read_pin(pin):
        if (not isinstance(pin, dict) or set(pin) != {'path', 'sha256'} or
            not isinstance(pin['path'], str) or not isinstance(pin['sha256'], str)):
            raise ValueError('invalid_review_pin')
        p = Path(pin['path'])
        if not p.is_absolute():
            p = Path(root) / p
        if sha(p.read_bytes()) != pin['sha256']:
            raise ValueError('review_pin_mismatch')
        return json.loads(p.read_text())
    try:
        path = Path(path)
        manifest = json.loads(path.read_text())
        if not isinstance(manifest, dict) or manifest.get('schema') != 1 or not isinstance(manifest.get('relations'), list):
            raise ValueError('invalid_entry_review_manifest')
        evidence.update(status='reviewed_dependency_identity', path=str(path), sha256=sha(path.read_bytes()))
        api_digest, tool_digest = canonical_sha(claim), sha(Path(__file__).read_bytes())
        for item in manifest['relations']:
            result = dict(production_acceptance=False)
            try:
                if not isinstance(item, dict) or set(item) != {'relation', 'review'}:
                    raise ValueError('invalid_entry_review_item')
                relation, review = item['relation'], read_pin(item['review'])
                result.update(relation_sha256=canonical_sha(relation), review=item['review'])
                if review.get('api_claim_sha256') != api_digest:
                    raise ValueError('entry_review_for_other_API_claim')
                if (review.get('schema') != 1 or review.get('status') != 'accepted' or
                    review.get('scope') != 'runtime_entry_dependency_identity' or
                    not isinstance(review.get('reviewer'), str) or not review['reviewer'].strip() or
                    review.get('relation_sha256') != canonical_sha(relation) or review.get('tool_sha256') != tool_digest):
                    raise ValueError('unaccepted_or_stale_entry_identity_review')
                replay = read_pin(review['checker_replay'])
                if (replay.get('schema') != 1 or replay.get('status') != 'accepted' or
                    replay.get('scope') != 'entry_dependency_checker_replay' or replay.get('tool_sha256') != tool_digest or
                    not isinstance(replay.get('reviewer'), str) or not replay['reviewer'].strip()):
                    raise ValueError('unaccepted_or_stale_entry_checker_replay')
                if relation['owner'] not in claim['dependencies']:
                    raise ValueError('entry_relation_is_not_exact_API_dependency')
                if any(ref['caller'] != claim['provider'] for ref in relation['references']):
                    raise ValueError('entry_reference_is_not_exact_API_provider')
                candidate = entry_alias_candidate(root, symbols, splits, image, relation)
                result['candidate'] = candidate
                if candidate['status'] != 'CANDIDATE_BOUND':
                    raise ValueError('unbound_entry_relation')
                provider, owner = claim['provider'], relation['owner']
                lo, hi = int(owner['address'], 16), int(owner['address'], 16) + owner['size']
                begin = int(provider['address'], 16)
                actual = set()
                for pc in range(begin, begin + provider['size'], 4):
                    word = image.word(pc)
                    if word >> 26 in (16, 18) and lo <= branch(word, pc) < hi:
                        actual.add((pc, branch(word, pc), 'linked' if word & 1 else 'tail'))
                declared = []
                for ref in relation['references']:
                    entry = next(entry for entry in candidate['entries'] if entry['symbol'] == ref['entry_symbol'])
                    declared.append((int(ref['pc'], 16), lo + entry['offset'], ref['call_kind']))
                if not actual or len(set(declared)) != len(declared) or actual != set(declared):
                    raise ValueError('incomplete_direct_entry_reference_census')
                key = canonical_sha(owner)
                if key in admitted:
                    admitted[key] = None
                    raise ValueError('ambiguous_entry_dependency_reviews')
                admitted[key] = result
                evidence['accepted'].append(result)
            except (OSError, ValueError, KeyError, TypeError, AttributeError, StopIteration, struct.error) as error:
                result['reason'] = str(error)
                evidence['refused'].append(result)
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
        evidence.update(status='refused', reason=str(error))
    admitted = {key: value for key, value in admitted.items() if value is not None}
    evidence['accepted'] = list(admitted.values())
    return admitted, evidence


def api_contracts(path, root, symbols, splits, image, entry_reviews=None):
    """Strict by default; explicit reviewed ENTRY dependencies use all API gates."""
    return _validate_api_contracts(path, root, symbols, splits, image, entry_reviews)


def preview_api_contracts_with_entry_dependencies(path, root, symbols, splits, image, entry_reviews):
    """Unaccepted proposal report only; never returns a live contract map."""
    _, report = _validate_api_contracts(path, root, symbols, splits, image, entry_reviews)
    report['would_accept'] = report.pop('accepted')
    report.update(status='disabled_entry_integration_preview', production_acceptance=False)
    return report


def _validate_api_contracts(path, root, symbols, splits, image, entry_reviews_path=None):
    """Validate reviewed input pins, not infer semantics from a hash or a name.

    The external reviewer attests ONLY argument/receiver transport to nerve
    storage, including the closed dependency chain. This tool verifies the
    attestation's identity and all pins; it does not manufacture that review.
    """
    accepted, refused, caller_traces = {}, [], {}
    if path is None:
        return accepted, dict(status='missing', accepted=[], refused=[])
    path = Path(path).resolve()
    manifest = json.loads(path.read_text())
    if not isinstance(manifest, dict) or manifest.get('schema') != 1 or not isinstance(manifest.get('contracts'), list):
        raise ValueError('API manifest must have schema 1 and a contracts list')
    entries = manifest.get('contracts', [])
    def pinned_file(pin):
        p = Path(pin['path'])
        if not p.is_absolute():
            p = Path(root) / p
        return sha(p.read_bytes()) == pin['sha256']
    for entry in entries:
        reasons, derived, entry_evidence, extent_checks = [], [], None, []
        claim = entry.get('claim', {}) if isinstance(entry, dict) else {}
        digest = canonical_sha(claim)
        try:
            if set(claim) != {'kind', 'this_register', 'nerve_register', 'receiver_adjustment', 'api', 'behavior',
                              'dependencies_closed', 'dol_sha256', 'provider', 'dependencies', 'source',
                              'preconditions', 'allowed_callsites'}:
                reasons.append('unsupported_claim_fields')
            if claim.get('dol_sha256') != sha(image.data):
                reasons.append('whole_DOL_pin')
            if (claim.get('kind') != 'member' or claim.get('this_register') != 3 or
                claim.get('nerve_register') != 4 or claim.get('receiver_adjustment') != 0 or
                claim.get('api') not in ('initNerve', 'setNerve') or
                claim.get('behavior') != 'selected_input_reaches_nerve_storage_unchanged' or
                claim.get('dependencies_closed') is not True):
                reasons.append('unsupported_or_unclosed_ABI_contract')
            if (not isinstance(claim.get('preconditions'), list) or not claim['preconditions'] or
                not all(isinstance(p, str) and p for p in claim['preconditions'])):
                reasons.append('missing_reviewed_preconditions')
            callers = claim['allowed_callsites']
            if not callers:
                reasons.append('missing_reviewed_callsite_scope')
            entry_dependencies, entry_evidence = _entry_dependency_reviews(entry_reviews_path, root, symbols, splits, image, claim)
            providers = [claim['provider']] + claim.get('dependencies', []) + [c['caller'] for c in callers]
            for index, provider in enumerate(providers):
                if set(provider) != {'address', 'symbol', 'section', 'size', 'unit', 'sha256'}:
                    reasons.append('unsupported_provider_fields')
                addr = int(provider['address'], 16)
                rows = symbols.at.get(addr, [])
                unique, row = len(rows) == 1, rows[0] if len(rows) == 1 else None
                if not unique and 1 <= index <= len(claim.get('dependencies', [])) and canonical_sha(provider) in entry_dependencies:
                    row = symbols.one(symbols.names.get(provider['symbol'], []))
                    unique = row is not None
                owned = splits.owns_extent(addr, provider['size'], provider['section'], provider['unit'])
                extent_checks.append(dict(role='provider' if index == 0 else 'dependency' if index <= len(claim.get('dependencies', [])) else 'caller',
                                          pin=provider, full_extent_owned=owned))
                if (not unique or row['name'] != provider['symbol'] or
                    row['section'] != provider['section'] or row['size'] != provider['size'] or
                    (index == 0 and not row['function']) or not owned or
                    sha(image.read(addr, provider['size'])) != provider['sha256']):
                    reasons.append('unique_provider_span_or_TU_pin')
            for call in callers:
                if set(call) != {'caller', 'pc', 'instruction_sha256', 'call_kind', 'register_requirements'}:
                    reasons.append('unsupported_callsite_fields')
                pc, caller = int(call['pc'], 16), call['caller']
                start = int(caller['address'], 16)
                caller_fn = symbols.one(symbols.at.get(start, []))
                word = image.word(pc)
                if (not start <= pc < start + caller['size'] or pc & 3 or
                    caller_fn is None or not caller_fn['function'] or word >> 26 != 18 or
                    branch(word, pc) != int(claim['provider']['address'], 16) or
                    call['call_kind'] not in ('linked', 'tail') or bool(word & 1) != (call['call_kind'] == 'linked') or
                    sha(image.read(pc, 4)) != call['instruction_sha256']):
                    reasons.append('unverified_callsite_scope_or_branch')
                requirements = call.get('register_requirements')
                if (not isinstance(requirements, dict) or
                    any(not re.fullmatch(r'r(?:[0-9]|[12][0-9]|3[01])', reg) or type(value) is not int or
                        not 0 <= value <= 0xffffffff for reg, value in requirements.items()) or
                    (claim['api'] == 'initNerve' and requirements.get('r5') != 0)):
                    reasons.append('unsupported_or_missing_machine_preconditions')
                    continue
                # Never seed the caller with manifest assertions. Constants must
                # follow from actual entry-reachable instructions on every modeled
                # path to this exact BL/tail. Unknown loads/joins remain unknown.
                if caller_fn is None:
                    reasons.append('ambiguous_precondition_caller')
                    continue
                key = (caller_fn['section'], start, caller_fn['size'])
                if key not in caller_traces:
                    caller_traces[key] = trace(image, caller_fn, {})
                events, issues = caller_traces[key]
                calls = [event for event in events if event['kind'] == 'call' and event['pc'] == pc and
                         event.get('target') == int(claim['provider']['address'], 16)]
                observed = {reg: calls[0].get(reg) if len(calls) == 1 else None for reg in requirements}
                proved = not issues and len(calls) == 1 and all(observed[reg] == value for reg, value in requirements.items())
                derived.append(dict(pc=call['pc'], requirements=requirements, observed=observed,
                                    entry_initial_constants={}, reachable_call=len(calls) == 1, issues=issues, verified=proved))
                if not proved:
                    reasons.append('unproved_callsite_register_requirements')
            source_pins = claim['source']
            if not {'declaration', 'definition', 'header'} <= {s['role'] for s in source_pins}:
                reasons.append('missing_canonical_source_contract_pins')
            if not all(pinned_file(s) for s in source_pins):
                reasons.append('source_contract_pin')
            review_pin = entry['review']
            if not pinned_file(review_pin):
                reasons.append('review_receipt_pin')
            review_path = Path(review_pin['path'])
            if not review_path.is_absolute():
                review_path = Path(root) / review_path
            review = json.loads(review_path.read_text())
            if (review.get('status') != 'accepted' or review.get('claim_sha256') != digest or
                not review.get('reviewer') or review.get('scope') != 'nerve_argument_storage_transport' or
                review.get('dependencies_closed') is not True):
                reasons.append('missing_or_unaccepted_review')
            if not reasons:
                addr = int(claim['provider']['address'], 16)
                if addr in accepted:
                    accepted[addr] = None
                    reasons.append('ambiguous_reviewed_contracts')
                else:
                    accepted[addr] = dict(claim=claim, claim_sha256=digest, review=review_pin,
                                          reviewer=review['reviewer'], scope=review['scope'], derived_callsite_preconditions=derived,
                                          entry_dependency_reviews=entry_evidence, extent_checks=extent_checks)
        except (OSError, ValueError, KeyError, TypeError, AttributeError, struct.error):
            reasons.append('invalid_or_missing_contract_evidence')
        if reasons:
            refused.append(dict(claim_sha256=digest, reasons=sorted(set(reasons)), derived_callsite_preconditions=derived,
                                entry_dependency_reviews=entry_evidence, extent_checks=extent_checks))
    accepted = {addr: ev for addr, ev in accepted.items() if ev is not None}
    return accepted, dict(status='reviewed_input', path=str(path), sha256=sha(path.read_bytes()),
                         accepted=[accepted[a] for a in sorted(accepted)], refused=refused,
                         trust='External review is trusted evidence; hashes verify identity, not semantics.')


def contract_at(contracts, target, pc):
    contract = contracts.get(target)
    if contract and any(int(c['pc'], 16) == pc for c in contract['claim']['allowed_callsites']):
        return contract
    return None


def source_catalogue(path, root, symbols, splits):
    """Preserve independent occurrence denominators; catalogue is never dispatch proof."""
    if path is None:
        return dict(status='absent', complete_state_controller_universe=False, partitions=[],
                    reason='Nonstandard/state-controller occurrence denominator not supplied; no whole-game all-clear.')
    path = Path(path).resolve()
    checkpoint = json.loads(path.read_text())
    if checkpoint.get('schema') != 1:
        raise ValueError('source catalogue schema must be 1')
    def read_pin(pin):
        p = path.parent / pin['path']
        if sha(p.read_bytes()) != pin['sha256']:
            raise ValueError('source catalogue member hash mismatch')
        return json.loads(p.read_text())
    pins = read_pin(checkpoint['source_pins'])
    stale = []
    for rel, digest in pins['files'].items():
        try:
            if sha((Path(root) / rel).read_bytes()) != digest:
                stale.append(rel)
        except OSError:
            stale.append(rel)
    metadata_pins_match = (pins['symbols_sha256'] == sha(symbols.path.read_bytes()) and
                           pins['splits_sha256'] == sha(splits.path.read_bytes()))
    partitions = []
    for spec in checkpoint['partitions']:
        data = read_pin(spec)
        rows = data['rows']
        ids = [r['id'] for r in rows]
        if (len(rows) != spec['row_count'] or data['row_count'] != len(rows) or len(set(ids)) != len(ids) or
            data.get('source_commit') != checkpoint['source_commit'] or
            any(r.get('audit_status') != 'UNKNOWN' for r in rows)):
            raise ValueError('invalid catalogue denominator, identity, revision or asserted verdict')
        projected = []
        for row in rows:
            evidence = row.get('evidence', {})
            rel = evidence.get('path')
            pinned = rel in pins['files'] and evidence.get('sha256') == pins['files'][rel]
            projected.append(dict(id=row['id'], family=row['form'], class_name=row.get('class_name'),
                                  kind=row.get('kind'), evidence=evidence, audit_status='UNKNOWN',
                                  source_classification=row.get('source_classification'), reason=row.get('reason'),
                                  source_pin_current=pinned and rel not in stale,
                                  wii_projection_excluded=row.get('wii_projection_excluded', False)))
        families = collections.Counter(r['family'] for r in projected)
        partitions.append(dict(path=str(path.parent / spec['path']), sha256=spec['sha256'], rows=projected,
                               coverage=dict(occurrences=len(rows), UNKNOWN=len(rows), MATCH=0, MISMATCH=0,
                                             families=dict(sorted(families.items()))),
                               independent_denominator=True, scope=data.get('scope'),
                               source_classification_meaning='historical lexical coverage only, never dispatch closure'))
    return dict(status='pinned_source_catalogue' if not stale and metadata_pins_match else 'stale_candidate_catalogue',
                path=str(path), sha256=sha(path.read_bytes()), source_commit=checkpoint['source_commit'],
                extractor_sha256=checkpoint.get('extractor_sha256'), source_pins=checkpoint['source_pins'],
                metadata_pins_current=metadata_pins_match, stale_source_files=stale, partitions=partitions,
                complete_state_controller_universe=False, limits=checkpoint.get('limits', []),
                scope='Independent source occurrence partitions; no implicit merge/promotion into Nerve dispatch rows.')


def audit(root, dol_path, symbols_path, splits_path, selected=None, inventory=False, contracts_path=None, catalogue_path=None,
          entry_reviews_path=None):
    """Explicit ENTRY reviews close dependency identity only, never downstream joins."""
    if entry_reviews_path is not None and (contracts_path is None or inventory):
        raise ValueError('ENTRY reviews require API contracts and a non-inventory mode')
    return _audit(root, dol_path, symbols_path, splits_path, selected, inventory, contracts_path, catalogue_path,
                  entry_reviews_path=entry_reviews_path)


def preview_dispatch_with_entry_dependencies(root, dol_path, symbols_path, splits_path, contracts_path, entry_reviews):
    """Synthetic proposal report; existing dispatch joins remain mandatory."""
    report = _audit(root, dol_path, symbols_path, splits_path, contracts_path=contracts_path,
                    entry_reviews_path=entry_reviews)
    report.update(mode='disabled_entry_integration_preview', production_acceptance=False,
                  scope='UNACCEPTED conditional proposal only; transport identity does not establish source dispatch or lifetime.')
    report['coverage'] = {('would_' + key if key in ('MATCH', 'MISMATCH') else key): value for key, value in report['coverage'].items()}
    report['would_mismatches'] = report.pop('mismatches')
    if 'api_contracts' in report:
        report['api_contracts']['would_accept'] = report['api_contracts'].pop('accepted')
    for row in report['rows']:
        row['production_acceptance'] = False
        if row['status'] != 'UNKNOWN':
            row['status'] = 'WOULD_' + row['status']
    return report


def _audit(root, dol_path, symbols_path, splits_path, selected=None, inventory=False, contracts_path=None, catalogue_path=None,
           entry_reviews_path=None):
    root = Path(root)
    symbols, splits, source = Symbols(symbols_path), Splits(splits_path), Source(root)
    keys = {cls for cls, meth in symbols.members if meth == 'execute' and
            ((cls, 'sInstance') in symbols.members or (cls, '__vt') in symbols.members)} | source.nerve_classes | set(source.defs)
    all_keys = sorted(keys)
    chosen = all_keys if selected is None else [k for k in all_keys if k in selected]
    report = dict(schema=SCHEMA, mode='inventory' if inventory else 'bounded' if selected else 'sweep',
                  coverage=dict(universe=len(all_keys), selected=len(chosen), excluded=len(all_keys) - len(chosen),
                                denominator='union of source execute/declared Nerve classes and mapped singleton/vtable execute classes'),
                  pins=[dict(path=str(symbols.path), sha256=sha(symbols.path.read_bytes())),
                        dict(path=str(splits.path), sha256=sha(splits.path.read_bytes()))],
                  rows=[], mismatches=[], unknown_source_forms=source.unknown_forms,
                  scope='dispatch binding only; not lifetime, whole-function equivalence, or runtime attribution',
                  paired_objects='not consumed; no global symbol/relocation-name ownership inference')
    report['source_occurrences'] = dict(
        declared_classes=sorted(source.nerve_classes),
        class_declarations=[d for cls in sorted(source.class_defs) for d in source.class_defs[cls] if cls in keys],
        static_instance_definitions=[d for cls in sorted(source.instance_defs) for d in source.instance_defs[cls] if cls in keys],
        execute_definitions=[d for cls in sorted(source.defs) for d in source.defs[cls] if cls in keys],
        producer_calls=source.producers)
    report['nonstandard_catalogue'] = source_catalogue(catalogue_path, root, symbols, splits)
    report['unresolved_selectors'] = sorted(set(selected or []) - keys)
    report['pins'].extend(dict(path=str(root / p), sha256=d['sha256']) for p, d in source.files.items())
    try:
        report['source_revision'] = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True, stderr=subprocess.DEVNULL).strip()
    except (OSError, subprocess.CalledProcessError):
        report['source_revision'] = None
    if inventory:
        report['rows'] = [dict(class_name=k, status='UNKNOWN', reasons=['inventory_only']) for k in chosen]
        report['coverage'].update(MATCH=0, MISMATCH=0, UNKNOWN=len(chosen))
        return report
    image = Dol(dol_path)
    contracts, report['api_contracts'] = _validate_api_contracts(contracts_path, root, symbols, splits, image, entry_reviews_path)
    report['pins'].append(dict(path=str(image.path), sha256=sha(image.data)))
    bases, base_evidence = startup_bases(symbols, image)
    consumer = dispatch_consumer(symbols, image)
    report['dispatch_consumer'] = consumer
    report['startup_bases'] = dict(values={str(r): address(v) for r, v in bases.items()}, evidence=base_evidence)
    singleton_rows = {k: symbols.get(k, 'sInstance') for k in chosen}
    singletons = {r['address'] for r in singleton_rows.values() if r}
    units = {u for r in singleton_rows.values() if r for u in splits.owners(r['address'], r['section'])}
    functions = [r for r in symbols.functions if any(u.startswith('Game/') for u in splits.owners(r['address'], r['section']))
                 and (selected is None or set(splits.owners(r['address'], r['section'])) & units)]
    ctor_links, producers, unresolved = collections.defaultdict(list), [], []
    api_addresses = {r['address']: member(r['name'])[1] for r in symbols.functions
                     if member(r['name']) and member(r['name'])[1] in ('initNerve', 'setNerve') and
                     'PC5Nerve' in r['name']}
    ctor_entries = []
    for lo, hi, sec, unit in splits.rows:
        if sec == '.ctors' and (selected is None or unit in units):
            for loc in range(lo, hi, 4):
                try:
                    ctor_entries.append((image.word(loc), image.span(loc, 4), unit))
                except ValueError:
                    unresolved.append(dict(kind='ctor_registration', address=address(loc)))
    registered = {entry for entry, _, _ in ctor_entries}
    by_address = collections.defaultdict(list)
    for fn in symbols.functions:
        by_address[fn['address']].append(fn)
    for fn in functions:
        if fn['size'] > 0x20000 or fn['size'] % 4:
            unresolved.append(dict(kind='function_extent', symbol=symbols.evidence(fn)))
            continue
        try:
            events, issues = trace(image, fn, bases)
        except ValueError:
            unresolved.append(dict(kind='non_dol_backed_function', symbol=symbols.evidence(fn)))
            continue
        if fn['address'] in registered:
            for event in events:
                obj, ctor, writes, kind, ci = None, None, [], None, []
                if event['kind'] == 'call' and event.get('r3') in singletons:
                    obj = event['r3']
                    target = symbols.one(by_address.get(event.get('target'), []))
                    if target and target['size'] <= 512:
                        ctor = target
                        ce, ci = trace(image, target, {**bases, 3: obj}, tracked_object=obj)
                        writes = [e for e in ce if e['kind'] == 'store' and e.get('address') == obj]
                        returns = [e for e in ce if e['kind'] == 'return']
                        if not returns or any(e.get('vptr') is None for e in returns):
                            ci += ['unresolved_final_constructor_vptr']
                        # Calls or unknown stores could overwrite the object after the observed write.
                        if any(e['kind'] == 'call' or (e['kind'] == 'store' and e['address'] is None) for e in ce):
                            ci = ci + ['unresolved_constructor_side_effect']
                        kind = 'out_of_line_constructor'
                elif event['kind'] == 'store' and event.get('address') in singletons:
                    obj, writes, kind = event['address'], [event], 'inlined_constructor_store'
                if obj is not None:
                    late_stores = [e for e in events if e['kind'] == 'unsupported_store' and
                                   ((e['address'] is None and not e['stack']) or
                                    (e['address'] is not None and e['address'] < obj + 4 and obj < e['address'] + e['width']))]
                    if late_stores:
                        ci += ['initializer_partial_or_unresolved_store']
                    registrations = [ev for entry, ev, unit in ctor_entries if entry == fn['address']]
                    ctor_links[obj].append(dict(kind=kind, singleton=address(obj),
                        call=image.span(event['pc'], 4), initializer=symbols.evidence(fn),
                        registration=registrations, constructor=symbols.evidence(ctor) if ctor else None,
                        constructor_span=image.span(ctor['address'], ctor['size']) if ctor else None,
                        vtables=sorted({e['vptr'] for e in ce if e['kind'] == 'return' and e.get('vptr') is not None}) if ctor else sorted({e['value'] for e in writes if e.get('value') is not None}),
                        stores=[dict(evidence=image.span(e['pc'], 4), vtable=address(e.get('value'))) for e in writes],
                        issues=sorted(set(ci + issues))))
        for event in events:
            if event['kind'] != 'call' or event.get('target') not in api_addresses:
                continue
            producers.append(dict(function=fn, pc=event['pc'], singleton=event.get('r4'),
                                  api=api_addresses[event['target']], issues=issues,
                                  contract=contract_at(contracts, event['target'], event['pc'])))
    report['producer_coverage'] = dict(original_call_sites=len(producers), source_call_sites=len(source.producers),
                                      unresolved_original_singletons=sum(p['singleton'] is None for p in producers))
    report['unresolved_producers'] = [dict(function=symbols.evidence(p['function']), call=image.span(p['pc'], 4),
                                         reason='unresolved_singleton') for p in producers if p['singleton'] is None]
    report['unresolved_spans'] = unresolved
    execute_owners = collections.defaultdict(set)
    for cls, obj in singleton_rows.items():
        if not obj:
            continue
        links = ctor_links.get(obj['address'], [])
        if len(links) == 1 and not links[0]['issues'] and len(links[0]['vtables']) == 1:
            try:
                execute_owners[image.word(links[0]['vtables'][0] + 8)].add(cls)
            except ValueError:
                pass
    for cls in chosen:
        row = dict(class_name=cls, status='UNKNOWN', reasons=[], checks=[], chain={}, source=None)
        failures, unknown = [], []
        obj, vt, execute = (symbols.get(cls, m) for m in ('sInstance', '__vt', 'execute'))
        def need(condition, reason):
            if not condition:
                unknown.append(reason)
            return condition
        if not need(all((obj, vt, execute)), 'missing_or_ambiguous_symbol_binding'):
            row['reasons'] = unknown
            report['rows'].append(row)
            continue
        need(consumer['verified'], 'dispatch_slot_consumer_unverified')
        row['binding'] = {name: symbols.evidence(value) for name, value in (('singleton', obj), ('vtable', vt), ('execute', execute))}
        owners = splits.owners(obj['address'], obj['section'])
        need(len(owners) == 1 and owners[0].startswith('Game/'), 'unknown_singleton_TU_ownership')
        for sym in (obj, vt, execute):
            need(unique_address(symbols, sym), 'aliased_symbol_address')
        binding = source.binding(cls, owners[0]) if len(owners) == 1 else None
        row['source'] = binding
        need(binding is not None, 'unknown_or_ambiguous_source_ownership')
        links = ctor_links.get(obj['address'], [])
        row['chain']['initializers'] = links
        need(len(links) == 1, 'missing_or_multiple_constructor_producers')
        link = links[0] if len(links) == 1 else None
        observed_vt = None
        if link:
            need(not link['issues'], 'unresolved_constructor_control_or_side_effect')
            if need(len(link['vtables']) == 1, 'unresolved_or_multiple_vptr_writes'):
                observed_vt = link['vtables'][0]
                row['chain']['vtable'] = address(observed_vt)
                if observed_vt != vt['address']:
                    failures.append('CONSTRUCTOR_VTABLE_BINDING')
                if link['constructor']:
                    expected_ctor = symbols.get(cls, '__ct')
                    if need(expected_ctor is not None, 'unknown_constructor_symbol_binding'):
                        if link['constructor']['address'] != address(expected_ctor['address']):
                            failures.append('SINGLETON_CONSTRUCTOR_BINDING')
        relevant = [p for p in producers if p['singleton'] == obj['address']]
        row['chain']['producer_calls'] = []
        need(bool(relevant), 'no_resolved_original_nerve_producer')
        producer_covered = False
        for p in relevant:
            original = symbols.evidence(p['function'])
            ident = source_function(p['function'])
            if member(p['function']['name']) and member(p['function']['name'])[1] == 'execute':
                # A crossed execute label is precisely the defect under audit.
                # Bind transition producers through the constructed vtable, not that label.
                callback_owners = execute_owners.get(p['function']['address'], set())
                ident = next(iter(callback_owners)) + '::execute' if len(callback_owners) == 1 else None
            matches = [s for s in source.producers if s['function'] == ident and s['api'] == p['api'] and
                       len(owners) == 1 and s['evidence']['path'] in source.reachable(owners[0])]
            peers = [q for q in producers if q['function'] == p['function'] and q['api'] == p['api']]
            ev = dict(function=original, call=image.span(p['pc'], 4), singleton=address(p['singleton']),
                      source_candidates=matches, api_contract=p['contract'])
            need(p['contract'] is not None and p['contract']['claim']['api'] == p['api'], 'unreviewed_nerve_API_contract')
            need(not p['issues'], 'unresolved_original_producer_control_or_effect')
            row['chain']['producer_calls'].append(ev)
            if len(matches) == len(peers) == 1 and not matches[0]['evidence']['conditional_unknown']:
                provider = member(p['contract']['claim']['provider']['symbol']) if p['contract'] else None
                receiver = source.api_receiver(matches[0]['receiver_class'], provider[0], owners[0], p['api']) if provider else None
                ev['source_receiver_path'] = receiver
                need(receiver is not None and matches[0]['receiver'] != 'unknown', 'unbound_source_API_receiver_or_override')
                expected = symbols.get(matches[0]['class_name'], 'sInstance') if matches[0]['class_name'] else None
                if need(expected is not None, 'unresolved_source_producer_singleton'):
                    producer_covered = True
                    if expected['address'] != p['singleton']:
                        failures.append('PRODUCER_SINGLETON_BINDING')
            else:
                unknown.append('producer_source_order_or_ownership_unknown')
        need(producer_covered, 'no_owned_source_producer_pair')
        if observed_vt is not None:
            try:
                actual_execute = image.word(observed_vt + 8)
                row['chain']['execute_slot'] = dict(evidence=image.span(observed_vt + 8, 4), address=address(actual_execute))
                actual_rows = symbols.at.get(actual_execute, [])
                need(len(actual_rows) == 1, 'aliased_or_unmapped_execute_address')
                if actual_execute != execute['address']:
                    failures.append('EXECUTE_ADDRESS_BINDING')
                fn = symbols.one(by_address.get(actual_execute, []))
                action = wrapper_action(symbols, image, fn, bases, api_addresses)
                if need(action is not None, 'thunk_complex_or_nontrampoline_execute'):
                    if action['kind'] == 'transition':
                        contract = contract_at(contracts, action['target'], action['call_pc'])
                        need(contract is not None and contract['claim']['api'] == 'setNerve', 'unreviewed_transition_API_contract')
                        row['chain']['transition_api_contract'] = contract
                        provider = member(contract['claim']['provider']['symbol']) if contract else None
                        receiver = source.api_receiver(binding['target_class'], provider[0], owners[0], 'setNerve') if binding and provider and len(owners) == 1 else None
                        need(receiver is not None, 'unbound_source_transition_receiver_or_override')
                        row['chain']['transition_source_receiver_path'] = receiver
                    row['chain']['execute_wrapper'] = action['span']
                    row['chain']['action'] = {k: address(v) if k in ('target', 'singleton', 'call_pc') else v for k, v in action.items() if k != 'span'}
                    if binding and need(binding['target_class'] is not None, 'state_controller_or_complex_source_execute'):
                        if binding['kind'] == 'transition':
                            target = symbols.get(binding['transition_class'], 'sInstance')
                            if need(target is not None and unique_address(symbols, target), 'unknown_source_transition_singleton'):
                                row['expected_transition'] = symbols.evidence(target)
                                if action['kind'] != 'transition' or action.get('singleton') != target['address']:
                                    failures.append('SOURCE_TRANSITION_DISPATCH')
                        elif binding['target_method'] == 'setNerve':
                            unknown.append('unresolved_source_transition')
                        else:
                            target = symbols.get(binding['target_class'], binding['target_method'])
                            if need(target is not None and unique_address(symbols, target), 'unknown_or_aliased_source_method_address'):
                                row['expected_body'] = symbols.evidence(target)
                                if action['kind'] != 'method' or action['target'] != target['address']:
                                    failures.append('SOURCE_METHOD_DISPATCH')
            except ValueError:
                unknown.append('non_dol_backed_vtable_or_execute')
        # Never promote even a suspicious difference when chain/source ownership is unresolved.
        row['checks'] = sorted(set(failures))
        row['reasons'] = sorted(set(unknown))
        row['status'] = 'UNKNOWN' if unknown else 'MISMATCH' if failures else 'MATCH'
        dimensions = dict(symbol_ownership={'CONSTRUCTOR_VTABLE_BINDING', 'SINGLETON_CONSTRUCTOR_BINDING', 'EXECUTE_ADDRESS_BINDING'},
                          source_dispatch={'SOURCE_METHOD_DISPATCH', 'SOURCE_TRANSITION_DISPATCH'},
                          producer_binding={'PRODUCER_SINGLETON_BINDING'})
        row['relations'] = {name: 'UNKNOWN' if unknown else 'MISMATCH' if checks.intersection(failures) else 'MATCH'
                            for name, checks in dimensions.items()}
        report['rows'].append(row)
        if row['status'] == 'MISMATCH':
            report['mismatches'].append(dict(class_name=cls, checks=row['checks']))
    counts = collections.Counter(r['status'] for r in report['rows'])
    report['coverage'].update({k: counts[k] for k in ('MATCH', 'MISMATCH', 'UNKNOWN')})
    return report


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    p.add_argument('--dol', type=Path)
    p.add_argument('--symbols', type=Path)
    p.add_argument('--splits', type=Path)
    p.add_argument('--api-contracts', type=Path, help='separately reviewed, fully pinned helper transport contracts')
    p.add_argument('--entry-reviews', type=Path, help='opt-in reviewed Runtime ENTRY dependency identities; requires --api-contracts')
    p.add_argument('--source-catalogue', type=Path, help='pinned source-only nonstandard occurrence checkpoint; all UNKNOWN')
    mode = p.add_mutually_exclusive_group(required=True)
    mode.add_argument('--inventory-only', action='store_true')
    mode.add_argument('--class', dest='classes', action='append', help='exact fully qualified class (repeatable)')
    mode.add_argument('--sweep', action='store_true', help='explicit full audit; obtain team sweep authorization first')
    p.add_argument('--output', required=True, type=Path)
    a = p.parse_args()
    if a.entry_reviews is not None and (a.api_contracts is None or a.inventory_only):
        p.error('--entry-reviews requires --api-contracts and a non-inventory mode')
    root = a.root.resolve()
    try:
        result = audit(root, a.dol or root / 'orig/SB4E01/sys/main.dol',
                       a.symbols or root / 'config/SB4E01/symbols.txt',
                       a.splits or root / 'config/SB4E01/splits.txt', a.classes, a.inventory_only, a.api_contracts, a.source_catalogue,
                       a.entry_reviews)
    except (OSError, ValueError, struct.error) as e:
        p.exit(2, f'nervemap: input refused: {e}\n')
    a.output.parent.mkdir(parents=True, exist_ok=True)
    a.output.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n')
    print(json.dumps(dict(output=str(a.output), coverage=result['coverage'], mismatches=len(result['mismatches']))))
    return 2 if result['unresolved_selectors'] else 1 if result['mismatches'] else 0


if __name__ == '__main__':
    raise SystemExit(main())
