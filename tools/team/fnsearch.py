#!/usr/bin/env python3
"""Deterministic, bounded, scratch-only MWCC search for one function.

Reads the configured Ninja edge directly (never runs Ninja/configure). The
selected source span is varied in an otherwise unchanged whole translation
unit, preserving file IPA and allowing every other function to be guarded.
Only a strictly better objdiff score with no other-function decrease survives.
The best survivor is emitted as a patch; --write TREE explicitly opts in to
writing the identical source in a named checkout. No flags/pragmas/asm search.
"""
from __future__ import annotations
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import csv
from dataclasses import dataclass
import difflib
import hashlib
import itertools
import json
import math
import os
from pathlib import Path
import random
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time

import flagsearch
import permute
import sg

SCRATCH = Path.home() / 'MacioGalaxy2-team/scratch-decomp2/rules'
SCALAR = r'(?:unsigned\s+int|signed\s+int|unsigned\s+long|signed\s+long|unsigned\s+short|signed\s+short|unsigned\s+char|signed\s+char|bool|int|unsigned|signed|long|short|char|s8|s16|s32|u8|u16|u32|f32|f64|float|double)'
INTEGER = re.compile(r'^(?:unsigned|signed|int|long|short|char|bool|[su](?:8|16|32))(?:\s+(?:int|long|short|char))?$')
IDENT = r'[A-Za-z_]\w*'
LITERAL = r'(?:true|false|nullptr|[+-]?(?:0x[0-9a-fA-F]+|\d+(?:\.\d*)?)(?:[uUlLfF]*))'
DECL = re.compile((r'(?m)^(?P<indent>[ \t]*)(?P<type>(?:const\s+)?' + SCALAR + r'|(?:const\s+)?' + IDENT + r'\s*\*|TVec3f)\s*(?P<pconst>const\s+)?\s+(?P<name>' + IDENT + r')\s*(?P<array>\[\s*\d+\s*\])?\s*(?:(?P<eq>=)\s*(?P<init>[^;\n{}]+)|\((?P<direct>[^;\n{}()]*)\))?;[ \t]*(?=\n|$)').replace(r'\s', r'[ \t]'))


def sha(data):
    return hashlib.sha256(data).hexdigest()


def run(argv, cwd, timeout=30):
    start = time.monotonic()
    env = os.environ.copy()
    env.pop('MG2_INTERACTIVE', None)
    proc = subprocess.Popen(argv, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            text=True, errors='replace', start_new_session=True, env=env)
    try:
        stdout, stderr = proc.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        os.killpg(proc.pid, signal.SIGKILL)
        stdout, stderr = proc.communicate()
        return dict(returncode=124, stdout=stdout, stderr=stderr + '\nTimed out', seconds=time.monotonic()-start)
    return dict(returncode=proc.returncode, stdout=stdout, stderr=stderr, seconds=time.monotonic()-start)


def compiler_edge(root, output):
    """Expand only the configured edge/rule, following Ninja variable scope."""
    text = re.sub(r'\$\r?\n[ \t]*', '', (root / 'build.ninja').read_text())
    globals_, rules, edges = {}, {}, {}
    scope = globals_
    for line in text.splitlines():
        if not line.strip() or line.lstrip().startswith('#'): continue
        if line.startswith('rule '):
            scope = rules.setdefault(line[5:].strip(), {})
        elif line.startswith('build '):
            match = re.fullmatch(r'build (.+?): (\S+)(?: (.*))?', line)
            if not match: raise ValueError('unsupported Ninja build syntax')
            outs, rule, ins = match.groups()
            edge = {'rule': rule, 'in': (ins or '').split('|')[0].strip(), 'out': outs.split('|')[0].strip()}
            scope = edge
            for out in edge['out'].split(): edges[out] = edge
        elif line[0].isspace():
            match = re.fullmatch(r'\s+(\w+)\s*=\s*(.*)', line)
            if match: scope[match[1]] = match[2]
        else:
            match = re.fullmatch(r'(\w+)\s*=\s*(.*)', line)
            if match: globals_[match[1]] = match[2]
            elif line.startswith(('include ', 'subninja ')): raise ValueError('Ninja include/subninja unsupported')
            scope = globals_
    if output not in edges: raise ValueError('no configured compile edge: ' + output)
    edge = edges[output]
    if edge['rule'] not in ('mwcc', 'mwcc_sjis', 'mwcc_extab', 'mwcc_sjis_extab'):
        raise ValueError('not an MWCC edge: ' + edge['rule'])
    variables = {**globals_, **rules[edge['rule']], **edge}
    def expand(value, depth=0):
        if depth > 12: raise ValueError('cyclic Ninja variable')
        def replace(m):
            key = m[1] or m[2]
            if key is None: return {'$': '$', ' ': ' ', ':': ':'}[m[3]]
            if key not in variables: raise ValueError('undefined Ninja variable: ' + key)
            return expand(variables[key], depth+1)
        return re.sub(r'\$\{(\w+)\}|\$(\w+)|\$([$ :])', replace, value)
    command = expand(rules[edge['rule']]['command'])
    parsed = flagsearch.parse_edge(command)
    parsed['command'] = command
    return parsed


def overlay(root, work, relative, text):
    """Mirror the source's ancestor siblings so relative includes keep working."""
    work.mkdir(parents=True)
    here, original = work, root
    for part in relative.parts:
        for child in original.iterdir():
            if child.name != part and not (here / child.name).exists():
                (here / child.name).symlink_to(child, target_is_directory=child.is_dir())
        if part == relative.name: break
        here = here / part
        here.mkdir()
        original = original / part
    source = work / relative
    source.write_text(text)
    return source


def score_functions(data):
    right = data.get('right', {}).get('symbols', [])
    names = {s['name'] for s in right if s.get('instructions')}
    return {s['name']: float(s.get('match_percent') or 0) if s['name'] in names else 0.0
            for s in data['left']['symbols'] if s.get('instructions') and int(s.get('size', 0)) > 0}


def drops(before, after, selected):
    return {name: [value, after.get(name, 0)] for name, value in before.items()
            if name != selected and after.get(name, 0) < value}


def selected_symbol(data, want):
    syms = [s for s in data['left']['symbols'] if s.get('instructions')]
    exact = [s for s in syms if s['name'] == want or s.get('demangled_name') == want]
    found = exact or [s for s in syms if want in s['name'] or want in s.get('demangled_name', '')]
    if len(found) != 1: raise ValueError('function is ambiguous/missing: ' + ', '.join(s['name'] for s in found[:12]))
    return found[0]


def source_span(source, symbol, selector=None, right=None):
    # Mask PP continuations outside the selected body, preserving source offsets.
    # permute's lexer intentionally rejects phase-2 splices in raw source.
    masked = flagsearch.lexical_mask(source)
    if selector: return permute.find_function(masked, selector)
    want = symbol.get('demangled_name', symbol['name']).split('(', 1)[0]
    defs = permute.definitions(masked)
    found = [(n, span) for n, _, span in defs if n == want]
    if not found:
        found = [(n, span) for n, _, span in defs if n == want.rsplit('::', 1)[-1]]
    if len(found) > 1 and right:
        lines = [i.get('instruction', {}).get('line_number') for i in right.get('instructions', [])]
        lines = [line for line in lines if line]
        found = [(n, span) for n, span in found if any(source.count('\n', 0, span.start)+1 <= line <= source.count('\n', 0, span.end)+1 for line in lines)]
    if len(found) != 1: raise ValueError('cannot uniquely extract function; pass --selector with its exact source signature')
    return found[0][1]


@dataclass(frozen=True)
class Edit:
    name: str
    replacements: tuple[tuple[int, int, str], ...]

    def apply(self, source):
        out = source
        for start, end, replacement in sorted(self.replacements, reverse=True):
            out = out[:start] + replacement + out[end:]
        return out


def known_types(fragment):
    result = {}
    pat = r'(?<![\w:])(?P<type>(?:const\s+)?' + SCALAR + r'|(?:const\s+)?TVec3f)\s*(?:&\s*)?(?P<name>' + IDENT + r')\b'
    for match in re.finditer(pat, fragment):
        result[match['name']] = match['type'].removeprefix('const ').strip()
    return result


def declaration_orders(length, limit=120):
    """Cover every position before filling a bounded deterministic sample."""
    identity = tuple(range(length))
    if math.factorial(length) <= limit: return list(itertools.permutations(identity))
    out, seen = [], set()
    def add(order):
        order = tuple(order)
        if order not in seen: seen.add(order); out.append(order)
    add(identity); add(reversed(identity))
    for i in range(length):
        for j in range(length):
            order = list(identity); item = order.pop(i); order.insert(j, item); add(order)
    for shift in range(1, length): add(identity[shift:]+identity[:shift])
    rng = random.Random(0)
    while len(out) < limit:
        order = list(identity); rng.shuffle(order); add(order)
    return out[:limit]


def generate_edits(source, span, vector_ok=False, macro_names=None, reserved_names=None):
    """Conservative lexical transforms; reject uncertain types/scope/effects."""
    fragment = source[span.start:span.end]
    macro_names = set(macro_names or ()) | set(re.findall(r'^\s*#\s*define\s+(\w+)', source, re.M))
    reserved_names = set(reserved_names or ())
    def has_macro(text): return bool(set(re.findall(IDENT, text)) & macro_names)
    tokens = permute.lex(fragment)
    if any(t.kind == 'pp' for t in tokens): raise ValueError('preprocessor inside selected definition is unsupported')
    if re.search(r'\b(?:asm|__asm|volatile|goto|setjmp|longjmp|__LINE__|__COUNTER__|__FILE__)\b', fragment):
        raise ValueError('selected definition has unsupported effects/location dependence')
    if re.search(r'\b__(?:LINE|COUNTER|FILE)__\b', source): raise ValueError('location-dependent translation unit')
    # References/address identity can distinguish moving local object storage.
    if re.search(r'\breinterpret_cast\b|\buintptr_t\b', fragment): vector_ok = False
    body_start, body_end = span.body_start + 1, span.body_end
    body = source[body_start:body_end]
    if re.search(r'\b(?:typedef|using|class|struct|union|enum)\b', flagsearch.lexical_mask(body)):
        raise ValueError('local type declarations require type-aware extraction; unsupported')
    ts = permute.lex(body)
    pairs = permute.paired(ts)
    language_parens = {'if', 'while', 'switch', 'for', 'sizeof', 'return'}
    call_regions = [(token.start, ts[pairs[i]].end) for i, token in enumerate(ts)
                    if token.text == '(' and i and re.fullmatch(IDENT, ts[i-1].text)
                    and ts[i-1].text not in language_parens]
    stack, parents, previous = [], {}, {}
    for i, token in enumerate(ts):
        parents[token.start] = stack[-1] if stack else -1
        previous[token.start] = ts[i-1].text if i else '{'
        if token.text == '{': stack.append(i)
        elif token.text == '}': stack.pop()
    type_fragment = flagsearch.lexical_mask(source[span.start:body_end])
    all_types = known_types(type_fragment)
    # Shadowed names are excluded from type-sensitive transformations.
    declarations = Counter()
    declared_type = r'(?:'+SCALAR+r'|'+IDENT+r'(?:::'+IDENT+r')*(?:\s*<[^;\n{}]+>)?)'
    declared = re.compile(r'\b(?:(?:const|volatile|register)\s+)?(?P<type>'+declared_type+r')(?:\s+[*&]*\s*|\s*[*&]+\s*)(?P<name>'+IDENT+r')\b')
    for m in declared.finditer(type_fragment):
        if m['type'] not in ('return', 'throw', 'delete', 'else', 'case', 'goto'):
            declarations[m['name']] += 1
    for m in re.finditer(r',\s*[*&]*\s*(?P<name>'+IDENT+r')\s*(?=[;=,\[])', type_fragment):
        declarations[m['name']] += 1
    all_types = {n: t for n, t in all_types.items() if declarations[n] <= 1}
    edits, decls, hoistable = [], [], {}
    for m in DECL.finditer(body):
        token_start = m.start() + len(m['indent'])
        if token_start not in parents or previous[token_start] not in ('{', ';', '}'): continue
        if m['type'] == 'TVec3f' and not vector_ok: continue
        # Only standalone declarations in ordinary brace blocks, never a for/if head.
        parent = parents[token_start]
        if parent >= 0 and ts[parent].text != '{': continue
        init = (m['init'] or m['direct'] or '').strip()
        t, n = m['type'].strip(), m['name']
        if has_macro(t) or n in macro_names: continue
        decls.append((m, parent))
        start, end = body_start+m.start(), body_start+m.end()
        if t == 'TVec3f' and init in all_types and init not in macro_names and all_types[init] == 'TVec3f':
            if m['eq']:
                new = f'{m["indent"]}TVec3f {n}({init});'
            elif m['direct']:
                new = f'{m["indent"]}TVec3f {n} = {init};'
            else: new = None
            if new: edits.append(Edit('vector_copy_initialization', ((start, end, new),)))
        if m['direct'] or m['array'] or ('const' in t and '*' not in t) or m['pconst']: continue
        if t == 'TVec3f' and init: continue
        if init and t != 'TVec3f':
            edits.append(Edit('split_scalar_initialization', ((start, end, m['indent']+t+' '+n+'; '+n+' = '+init+';'),)))
        opening = body_start + ts[parent].end if parent >= 0 else body_start
        prior = source[opening:start]
        if not prior.strip() or re.search(r'\b' + re.escape(n) + r'\b', prior): continue
        # Hoisting across labels/switches/captures could change lookup/lifetime.
        if re.search(r'\b(?:case|switch|default)\b|[\[\]]', prior): continue
        if has_macro(prior): continue  # expansion could implicitly use a shadowed name
        if init and re.search(r'\b' + re.escape(n) + r'\b', init): continue
        if t == 'TVec3f' and re.search(r'\b(?:return|break|continue|throw)\b', prior): continue
        prefix = ' ' + t + ' ' + n + ';'
        replacement = m['indent'] + n + ' = ' + init + ';' if init else ''
        edits.append(Edit('hoist_local_declaration', ((opening, opening, prefix), (start, end, replacement))))
        hoistable.setdefault(opening, []).append((t, n, start, end, replacement))
    for opening, members in hoistable.items():
        if not 2 <= len(members) <= 7: continue
        for order in declaration_orders(len(members)):
            prefix = ' ' + ' '.join(members[i][0]+' '+members[i][1]+';' for i in order)
            replacements = ((opening, opening, prefix),) + tuple((start, end, text) for _, _, start, end, text in members)
            edits.append(Edit('hoisted_declaration_order', replacements))
    # Common MWCC stack-order cases use comma-separated uninitialized locals.
    multiple = re.compile((r'(?m)^(?P<indent>[ \t]*)(?P<type>'+SCALAR+r'|TVec3f)\s+(?P<names>'+IDENT+r'(?:\s*,\s*'+IDENT+r')+);[ \t]*(?=\n|$)').replace(r'\s', r'[ \t]'))
    for m in multiple.finditer(body):
        at = m.start()+len(m['indent'])
        if at not in parents or previous[at] not in ('{', ';', '}'): continue
        if m['type'] == 'TVec3f' and not vector_ok: continue
        names = [name.strip() for name in m['names'].split(',')]
        if has_macro(m['type']) or any(name in macro_names for name in names): continue
        if not 2 <= len(names) <= 7: continue
        for order in declaration_orders(len(names)):
            if order == tuple(range(len(names))): continue
            new = m['indent']+m['type']+' '+', '.join(names[i] for i in order)+';'
            edits.append(Edit('comma_declaration_order', ((body_start+m.start(), body_start+m.end(), new),)))
    # Reorder adjacent declarations only when their initializers are effect-free.
    groups, group = [], []
    for m, parent in decls:
        if group and (parent != group[-1][1] or body[group[-1][0].end():m.start()].strip()):
            groups.append(group); group = []
        group.append((m, parent))
    if group: groups.append(group)
    for group in groups:
        if not 2 <= len(group) <= 7: continue
        names = {m['name'] for m, _ in group}
        safe = True
        for m, _ in group:
            init = (m['init'] or m['direct'] or '').strip()
            if m['type'] == 'TVec3f' and init: safe = False
            if init and not re.fullmatch(LITERAL, init):
                if init not in all_types or has_macro(init) or any(re.search(r'\b'+re.escape(n)+r'\b', init) for n in names): safe = False
        start, end = body_start+group[0][0].start(), body_start+group[-1][0].end()
        chunks = [m[0] for m, _ in group]
        # Match the original whitespace between declarations in every permutation.
        gaps = [body[a[0].end():b[0].start()] for a, b in zip(group, group[1:])]
        orders = declaration_orders(len(chunks))
        for order in orders if safe else []:
            if order == tuple(range(len(chunks))): continue
            replacement = chunks[order[0]] + ''.join(gap + chunks[i] for gap, i in zip(gaps, order[1:]))
            edits.append(Edit('local_declaration_order', ((start, end, replacement),)))
        splittable = all(m['type'] != 'TVec3f' and not m['array'] and not m['direct'] and not m['pconst']
                         and not ('const' in m['type'] and '*' not in m['type'])
                         and not has_macro(m['init'] or '')
                         and not any(re.search(r'\b'+re.escape(n)+r'\b', m['init'] or '') for n in names)
                         for m, _ in group)
        if splittable:
            bare = [m['type']+' '+m['name']+';' for m, _ in group]
            for order in orders:
                # Assignments stay in their original evaluation order.
                statements = [m['indent'] + (m['name']+' = '+m['init'].strip()+';' if m['init'] else '') for m, _ in group]
                replacement = group[0][0]['indent'] + ' '.join(bare[i] for i in order) + ' ' + statements[0]
                replacement += ''.join(gap + statement for gap, statement in zip(gaps, statements[1:]))
                edits.append(Edit('declaration_order_preserve_initializers', ((start, end, replacement),)))
    # Whole integer RHS or explicitly parenthesized binary expressions only.
    integer_names = {n for n, t in all_types.items() if INTEGER.fullmatch(t)}
    integer_literal = r'(?:true|false|[+-]?(?:0x[0-9a-fA-F]+|\d+)[uUlL]*)'
    operand = r'(?:'+IDENT+'|'+integer_literal+')'
    commutative = re.compile(r'(?P<a>' + operand + r')\s*(?P<op>==|!=|[+*|&^])\s*(?P<b>' + operand + r')')
    masked = flagsearch.lexical_mask(body)
    for m in commutative.finditer(masked):
        if not all(x in integer_names or re.fullmatch(integer_literal, x) for x in (m['a'], m['b'])) or m['a'] == m['b']: continue
        if m['a'] not in integer_names and m['b'] not in integer_names: continue
        if m['a'] in macro_names or m['b'] in macro_names: continue
        # A macro argument may be stringized/token-pasted. Only language-level
        # grouping/control parentheses can enclose a rewritten expression.
        if any(a < m.start() < m.end() < b for a, b in call_regions): continue
        left = masked[:m.start()].rstrip()
        right = masked[m.end():].lstrip()
        whole = (left.endswith('return') or (left.endswith('=') and not left.endswith(('==', '!=', '<=', '>=')))) and right.startswith(';')
        paren = left.endswith('(') and right.startswith(')')
        if whole or paren:
            middle = body[m.end('a'):m.start('b')]
            edits.append(Edit('commutative_integer_operands', ((body_start+m.start(), body_start+m.end(), m['b']+middle+m['a']),)))
    # Eliminate a copy-only integral temporary at a terminal return. Floating
    # temporaries are excluded because removing a conversion can remove rounding.
    for m, parent in decls:
        t, name = m['type'], m['name']
        init = (m['init'] or '').strip()
        tail = body[m.end():]
        returned = re.fullmatch(r'\s*return\s+'+re.escape(name)+r'\s*;\s*', tail)
        if parent != -1 or not returned or t not in ('int', 'long', 's32', 'u32', 'unsigned', 'unsigned int', 'unsigned long'): continue
        if init not in all_types or all_types[init] != t: continue
        if init in macro_names or name in macro_names: continue
        if len(re.findall(r'\b'+re.escape(name)+r'\b', body)) != 2: continue
        replacement = m['indent']+'return '+init+';' + '\n'*body[m.start():].count('\n')
        edits.append(Edit('inline_integral_copy_temporary', ((body_start+m.start(), body_end, replacement),)))
    # Return/else rewrites preserve the original condition evaluation exactly.
    for i, token in enumerate(ts):
        if token.text != 'if' or i+1 >= len(ts) or ts[i+1].text != '(': continue
        cp = pairs[i+1]; ob = cp+1
        cond = body[ts[i+1].end:ts[cp].start]
        # A staged builtin bool also works with a single-statement if body.
        if re.fullmatch(r'\s*!?\s*' + IDENT + r'\s*', cond):
            name = cond.strip().lstrip('!').strip()
            if all_types.get(name) == 'bool' and previous[token.start] in ('{', ';', '}'):
                if name in macro_names: continue
                fresh = 'fnsearchCondition'
                while fresh in reserved_names or re.search(r'\b'+fresh+r'\b', source): fresh += '_'
                new = 'bool '+fresh+' = ('+cond+'); if ('+fresh+')'
                edits.append(Edit('stage_builtin_bool', ((body_start+token.start, body_start+ts[cp].end, new),)))
        if ob >= len(ts) or ts[ob].text != '{': continue
        cb = pairs[ob]
        inside = ts[ob+1:cb]
        if not inside: continue
        parent = parents[token.start]
        scope_end = pairs[parent] if parent >= 0 else len(ts)
        has_else = cb+2 < len(ts) and ts[cb+1].text == 'else' and ts[cb+2].text == '{'
        if has_else:
            eob, ecb = cb+2, pairs[cb+2]
            # Removing else after an unconditional return does not move declarations
            # across any later statements: require a terminal if/else in its block.
            if ecb+1 == scope_end and inside[0].text == 'return' and inside[-1].text == ';' and sum(t.text == ';' for t in inside) == 1:
                start, end = body_start+ts[cb+1].start, body_start+ts[eob].end
                blank = re.sub(r'[^\r\n]', ' ', source[start:end])
                edits.append(Edit('terminal_else_to_early_return', ((start, end, blank), (body_start+ts[ecb].start, body_start+ts[ecb].end, ' '))))
            if parent == -1 and ecb+1 == scope_end and re.match(r'\s*void\s+', source[span.start:span.body_start]):
                start, end = body_start+ts[cb+1].start, body_start+ts[eob].end
                blank = re.sub(r'[^\r\n]', ' ', source[start:end])
                edits.append(Edit('void_terminal_else_to_early_return', ((body_start+ts[cb].start, body_start+ts[cb].start, ' return; '),
                                                                        (start, end, blank), (body_start+ts[ecb].start, body_start+ts[ecb].end, ' '))))
        else:
            after = cb+1
            if inside[0].text == 'return' and inside[-1].text == ';' and sum(t.text == ';' for t in inside) == 1 and after < scope_end and ts[after].text == 'return':
                semi = next((j for j in range(after, scope_end) if ts[j].text == ';'), None)
                if semi is not None and semi+1 == scope_end:
                    edits.append(Edit('early_return_to_terminal_else', ((body_start+ts[cb].end, body_start+ts[cb].end, ' else {'), (body_start+ts[semi].end, body_start+ts[semi].end, '}'))))
    # Top-level const on a pointer parameter is safe for arrow-only uses: no
    # overload binding/address taking/assignments can observe pointer constness.
    head = source[span.start:span.body_start]
    param = re.compile(r'\b(?:const\s+)?' + IDENT + r'\s*\*\s*(?P<name>' + IDENT + r')\s*(?=,|\))')
    for m in param.finditer(head):
        if has_macro(body): continue
        name = m['name']
        uses = list(re.finditer(r'\b'+re.escape(name)+r'\b', masked))
        if not uses or any(not masked[u.end():].lstrip().startswith('->') for u in uses): continue
        at = span.start + m.start('name')
        edits.append(Edit('const_pointer_parameter', ((at, at, 'const '),)))
    # Statement-looking macro arguments can be stringized too. No edit may
    # begin inside a call argument, even if it contains braces/semicolons.
    return [edit for edit in edits if not any(a < start-body_start < b
            for start, _, _ in edit.replacements for a, b in call_regions)]


def variants(source, span, budget, seed=0, vector_ok=False, macro_names=None, reserved_names=None):
    edits = generate_edits(source, span, vector_ok, macro_names, reserved_names)
    # Spread the budget among transform families before trying combinations.
    families = {}
    for edit in edits: families.setdefault(edit.name, []).append(edit)
    rng = random.Random(seed)
    for family in families.values(): rng.shuffle(family)
    ordered = []
    for group in itertools.zip_longest(*families.values()): ordered.extend(e for e in group if e is not None)
    out, seen = [], {sha(source.encode())}
    def append(text, label):
        key = sha(text.encode())
        if key in seen: return
        seen.add(key)
        out.append((label, text))
    for edit in ordered:
        append(edit.apply(source), edit.name)
        if len(out) >= budget: return out
    # Independent nonoverlapping edits compose without changing either proof.
    for a, b in itertools.combinations(ordered[:80], 2):
        if any(max(x[0], y[0]) <= min(x[1], y[1]) for x in a.replacements for y in b.replacements): continue
        append(Edit(a.name+'+'+b.name, a.replacements+b.replacements).apply(source), a.name+'+'+b.name)
        if len(out) >= budget: break
    return out


def vector_header_ok(root):
    path = root / 'libs/JSystem/include/JSystem/JGeometry/TVec.hpp'
    if not path.is_file(): return False
    text = path.read_text()
    segment = text.split('struct TVec3< f32 > : public Vec {', 1)
    return len(segment) == 2 and bool(re.search(r'inline\s+TVec3\(\)\s*\{\s*\}', segment[1])) and '~TVec3' not in segment[1]


class Search:
    def __init__(self, root, unit, edge, work, target, timeout, deadline=None):
        self.root, self.unit, self.edge, self.work, self.target, self.timeout = root, unit, edge, work, target, timeout
        self.deadline = deadline or time.monotonic()+480
        self.relative = Path(edge['source'])
        if self.relative.is_absolute() or '..' in self.relative.parts: raise ValueError('source must be repo-relative')

    def compile(self, index, text, label):
        if index and time.monotonic() >= self.deadline:
            return dict(index=index, transform=label, compile_seconds=0, diff_seconds=0,
                        source_sha256=sha(text.encode()), status='deadline_skipped'), None
        if shutil.disk_usage(self.work).free < 50_000_000_000:
            return dict(index=index, transform=label, compile_seconds=0, diff_seconds=0,
                        source_sha256=sha(text.encode()), status='disk_space_skipped'), None
        work = self.work / f'candidate-{index:05d}'
        source = overlay(self.root, work, self.relative, text)
        output = work / '.fnsearch-object'
        output.mkdir()
        argv = self.edge['argv'].copy()
        argv[argv.index('-o')+1] = str(output)
        result = run(argv, work, max(.05, min(self.timeout, self.deadline-time.monotonic())))
        row = dict(index=index, transform=label, compile_seconds=result['seconds'], diff_seconds=0.0,
                   source_sha256=sha(text.encode()), status='compile_failed', work=str(work))
        (output / 'compile.log').write_text(result['stdout']+result['stderr'])
        if result['returncode']:
            row['diagnostic'] = (result['stdout']+result['stderr'])[-1800:]
            return row, None
        obj = output / (source.stem+'.o')
        if not obj.is_file():
            row['diagnostic'] = 'compiler object missing'; return row, None
        for tail in self.edge['tails']:
            if len(tail) == 7:
                post = run([str(self.root / tail[0]), *tail[1:5], str(obj), str(obj)], work, max(.05, min(self.timeout, self.deadline-time.monotonic())))
                if post['returncode']:
                    row.update(status='postprocess_failed', diagnostic=post['stderr']); return row, None
        cfg = {'units': [{**self.unit, 'base_path': str(obj), 'target_path': str(self.target)}]}
        # Use a private project for unit-specific symbol mappings/options.
        config = output / 'project'
        config.mkdir()
        (config / 'objdiff.json').write_text(json.dumps(cfg))
        diff = run([str(self.root / 'build/tools/objdiff-cli'), '-L', 'error', 'diff', '-c', 'functionRelocDiffs=none', '-p', str(config), '-u', self.unit['name'], '-o', '-'], self.root, max(.05, min(self.timeout, self.deadline-time.monotonic())))
        row['diff_seconds'] = diff['seconds']
        if diff['returncode']:
            row.update(status='diff_failed', diagnostic=diff['stderr']); return row, None
        try: data = json.loads(diff['stdout'])
        except ValueError:
            row.update(status='diff_failed', diagnostic=diff['stdout'][-1800:]); return row, None
        row.update(status='scored', functions=score_functions(data), object_sha256=sha(obj.read_bytes()))
        return row, data

    def report(self, row):
        config = Path(row['work']) / '.fnsearch-object/project'
        result = run([str(self.root / 'build/tools/objdiff-cli'), '-L', 'error', 'report', 'generate', '-c', 'functionRelocDiffs=none', '-p', str(config), '-o', '-'], self.root, max(.05, min(self.timeout, self.deadline-time.monotonic())))
        if result['returncode']: raise ValueError('whole-unit report failed: '+result['stderr'])
        data = json.loads(result['stdout'])
        return {f['name']: float(f.get('fuzzy_match_percent', 0)) for u in data['units'] for f in u.get('functions', [])}


def dependency_hashes(root, baseline, relative):
    path = Path(baseline['work']) / '.fnsearch-object' / (relative.stem+'.d')
    if not path.is_file(): raise ValueError('MWCC dependency file missing')
    pins = flagsearch.parse_dependencies(path, Path(baseline['work']))
    temporary_source = str((Path(baseline['work']) / relative).resolve())
    if temporary_source not in pins: raise ValueError('source absent from dependency list')
    pins[str(root / relative)] = pins.pop(temporary_source)
    return pins


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('unit')
    p.add_argument('function', help='mangled name, demangled name, or unique substring')
    p.add_argument('--root', type=Path, default=Path.cwd())
    p.add_argument('--selector', help='exact source qualified name/signature if extraction is ambiguous')
    p.add_argument('--out', type=Path)
    p.add_argument('--workers', type=int, default=14)
    p.add_argument('--budget', type=int, default=160)
    p.add_argument('--rounds', type=int, default=2)
    p.add_argument('--seed', type=int, default=0)
    p.add_argument('--timeout', type=float, default=30)
    p.add_argument('--seconds', type=float, default=480, help='lock-held search budget, maximum 540 seconds')
    p.add_argument('--write', type=Path, metavar='TREE')
    args = p.parse_args()
    if not 1 <= args.workers <= 14 or not 1 <= args.budget <= 2000 or not 1 <= args.rounds <= 8:
        p.error('workers 1..14, budget 1..2000, rounds 1..8 required')
    if not 0 < args.seconds <= 540 or args.timeout <= 0: p.error('seconds must be >0..540 and timeout >0')
    root = args.root.resolve()
    lock = str(Path.home() / 'MacioGalaxy2-team/slots/server-heavy.lock')
    if os.environ.get('FNSEARCH_HEAVY_LOCKED') != lock:
        Path(lock).parent.mkdir(parents=True, exist_ok=True)
        env = {**os.environ, 'FNSEARCH_HEAVY_LOCKED': lock, 'FNSEARCH_LOCK_REQUESTED_AT': str(time.time())}
        print('fnsearch: waiting for server-heavy.lock', flush=True)
        os.execvpe('lockf', ['lockf', '-k', lock, sys.executable, str(Path(__file__).resolve()), *sys.argv[1:]], env)
    lock_wait_seconds = max(0, time.time() - float(os.environ.get('FNSEARCH_LOCK_REQUESTED_AT', time.time())))
    if shutil.disk_usage(tempfile.gettempdir()).free < 50_000_000_000: raise ValueError('less than 50 GB free')
    name = 'main/' + sg.norm_unit(args.unit)
    units = json.loads((root / 'objdiff.json').read_text())['units']
    unit = next((u for u in units if u['name'] == name), None)
    if not unit or not unit.get('base_path'): raise ValueError('unit has no configured source; configure is not run by this tool')
    edge = compiler_edge(root, unit['base_path'])
    relative = Path(edge['source'])
    source = (root / relative).read_text()
    original_sha = sha((root / relative).read_bytes())
    slug = re.sub(r'[^A-Za-z0-9_.-]+', '_', args.unit+'_'+args.function)[:130]
    out = (args.out or SCRATCH / 'searches' / (slug+'-'+time.strftime('%Y%m%d-%H%M%S'))).resolve()
    if out.exists() and any(out.iterdir()): raise ValueError('output must be empty: '+str(out))
    out.mkdir(parents=True, exist_ok=True)
    start = time.monotonic()
    rows, texts, seen, accepted = [], {}, set(), None
    with tempfile.TemporaryDirectory(prefix='decomp-fnsearch-') as temporary:
        work = Path(temporary)
        target = work / 'target.o'
        shutil.copyfile(root / unit['target_path'], target)
        search = Search(root, unit, edge, work, target, args.timeout, start+args.seconds)
        baseline, data = search.compile(0, source, 'baseline')
        if data is None: raise ValueError('baseline failed: '+baseline.get('diagnostic', baseline['status']))
        selected = selected_symbol(data, args.function)
        symbol = selected['name']
        right = next((s for s in data.get('right', {}).get('symbols', []) if s['name'] == symbol), None)
        span = source_span(source, selected, args.selector, right)
        original_span = span
        baseline.update(function_score=baseline['functions'].get(symbol, 0), guard_drops={}, status='baseline')
        baseline_report = search.report(baseline)
        if abs(baseline_report.get(symbol, 0) - baseline['function_score']) > 0.0001:
            raise ValueError('baseline diff/report scores disagree for selected function')
        pins = dependency_hashes(root, baseline, relative)
        macro_names, reserved_names = set(), set()
        for dependency in pins:
            content = Path(dependency).read_text(errors='replace')
            macro_names.update(re.findall(r'^\s*#\s*define\s+(\w+)', content, re.M))
            reserved_names.update(re.findall(IDENT, content))
        pins[str(root/'build.ninja')] = sha((root/'build.ninja').read_bytes())
        pins[str((root/unit['target_path']).resolve())] = sha(target.read_bytes())
        for tool in edge['argv'][:edge['compiler_index']+1]:
            tool_path = (root/tool).resolve()
            pins[str(tool_path)] = sha(tool_path.read_bytes())
        for tool in ('build/tools/wibo.real', 'build/tools/objdiff-cli', 'build/tools/dtk'):
            tool_path = (root/tool).resolve()
            if tool_path.is_file(): pins[str(tool_path)] = sha(tool_path.read_bytes())
        for module in (sys.modules[__name__], permute, flagsearch, sg):
            path = Path(module.__file__).resolve()
            pins[str(path)] = sha(path.read_bytes())
        rows.append(baseline)
        texts[0] = source
        seen.add(sha(source.encode()))
        current = source
        best_score = baseline['function_score']
        for round_index in range(args.rounds):
            remaining = args.budget - (len(rows)-1)
            if remaining <= 0 or best_score >= 100: break
            if round_index: span = source_span(current, selected, args.selector, right)
            proposed = [(label, text) for label, text in variants(current, span, remaining, args.seed+round_index, vector_header_ok(root), macro_names, reserved_names) if sha(text.encode()) not in seen]
            if not proposed: break
            jobs = []
            for label, text in proposed:
                seen.add(sha(text.encode()))
                index = len(texts)
                texts[index] = text
                jobs.append((index, text, label))
            with ThreadPoolExecutor(args.workers) as executor:
                for row, _ in executor.map(lambda job: search.compile(*job), jobs):
                    row['round'] = round_index+1
                    row['function_score'] = row.get('functions', {}).get(symbol, 0)
                    row['guard_drops'] = drops(baseline['functions'], row.get('functions', {}), symbol) if row['status'] == 'scored' else {}
                    if row['status'] == 'scored':
                        if row['guard_drops']: row['status'] = 'other_function_regressed'
                        elif row['function_score'] <= baseline['function_score']: row['status'] = 'no_improvement'
                        else: row['status'] = 'improved_diff'
                    rows.append(row)
            candidates = sorted([r for r in rows if r['status'] == 'improved_diff' and r['function_score'] > best_score], key=lambda r: (-r['function_score'], r['index']))
            next_best = None
            for row in candidates:
                if time.monotonic() >= search.deadline:
                    row['status'] = 'unverified_deadline'; break
                report = search.report(row)
                report_drops = drops(baseline_report, report, symbol)
                if report_drops or report.get(symbol, 0) < baseline_report.get(symbol, 0):
                    row.update(status='report_regressed', report_drops=report_drops)
                    continue
                if report.get(symbol, 0) <= baseline_report.get(symbol, 0):
                    row.update(status='no_report_improvement', report_drops={})
                    continue
                if abs(report.get(symbol, 0) - row['function_score']) > 0.0001:
                    row['status'] = 'score_disagreement'; continue
                row.update(status='accepted', report_score=report.get(symbol, 0), report_drops={})
                next_best = row
                break
            if next_best is None: break
            accepted, best_score = next_best, next_best['function_score']
            current = texts[accepted['index']]
        changed_inputs = [path for path, value in pins.items() if not Path(path).is_file() or sha(Path(path).read_bytes()) != value]
        if changed_inputs: raise ValueError('inputs changed during search: '+', '.join(changed_inputs))
        patch = ''
        if accepted:
            patch = ''.join(difflib.unified_diff(source.splitlines(keepends=True), current.splitlines(keepends=True), fromfile='a/'+relative.as_posix(), tofile='b/'+relative.as_posix()))
            (out / 'best.cpp').write_text(current)
        (out / 'best.patch').write_text(patch)
        for row in rows:
            row.pop('functions', None)
            row.pop('work', None)
        columns = ['index', 'transform', 'round', 'status', 'function_score', 'compile_seconds', 'diff_seconds', 'guard_drops']
        with (out / 'scores.tsv').open('w') as f:
            writer = csv.DictWriter(f, fieldnames=columns, delimiter='\t', extrasaction='ignore', lineterminator='\n')
            writer.writeheader(); writer.writerows(rows)
        compiles = [r['compile_seconds'] for r in rows if r['compile_seconds'] > 0]
        metadata = dict(unit=name, function=symbol, demangled=selected.get('demangled_name'), root=str(root),
                        source=str(relative), source_sha256=original_sha, source_span=[original_span.start, original_span.end],
                        flags_command=edge['command'], scoring_config={'functionRelocDiffs': 'none'}, revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip(),
                        dependency_sha256=pins, workers=args.workers, baseline=baseline['function_score'], best=best_score,
                        accepted_index=accepted['index'] if accepted else None, variants=len(rows)-1,
                        compiled_variants=sum(r['compile_seconds'] > 0 for r in rows[1:]),
                        compile_mean_seconds=sum(compiles)/len(compiles), compile_median_seconds=sorted(compiles)[len(compiles)//2],
                        compile_under_1s=sum(t < 1 for t in compiles), wall_seconds=time.monotonic()-start,
                        lock_wait_seconds=lock_wait_seconds,
                        search_seconds_limit=args.seconds,
                        baseline_report_score=baseline_report.get(symbol), rows=rows)
        (out / 'result.json').write_text(json.dumps(metadata, indent=2)+'\n')
        if args.write and accepted:
            tree = args.write.resolve()
            destination = tree / relative
            if not (tree/'.git').exists() or destination.is_symlink() or not destination.resolve().is_relative_to(tree):
                raise ValueError('--write requires a checkout and a regular source inside it')
            if sha(destination.read_bytes()) != original_sha: raise ValueError('--write source differs from searched baseline')
            destination.write_text(current)
            metadata['written_to'] = str(destination)
            (out/'result.json').write_text(json.dumps(metadata, indent=2)+'\n')
        print(json.dumps({k: metadata[k] for k in ('unit','function','baseline','best','variants','compile_mean_seconds','compile_median_seconds','wall_seconds')}, indent=2))
        print('patch:', out/'best.patch')
        print('scores:', out/'scores.tsv')
    return 0


if __name__ == '__main__':
    try: raise SystemExit(main())
    except (ValueError, OSError, KeyError) as error:
        print('fnsearch:', error, file=sys.stderr)
        raise SystemExit(2)
