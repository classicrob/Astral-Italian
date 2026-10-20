#!/usr/bin/env python3
"""Read-only lexical inventory of native conditional/assembly gaps (not a compiler)."""
from __future__ import annotations

import argparse
import ast
from bisect import bisect_right
from collections import Counter
from dataclasses import dataclass, field
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/team'))
from lanefiles import lane_of

EXTENSIONS = {'.c', '.cc', '.cpp', '.cxx', '.h', '.hh', '.hpp', '.hxx', '.inc'}
LEXEME = re.compile(r'(?P<comment>//(?:[^\\\n]|\\[^\n]|\\\n)*|/\*[\s\S]*?\*/)|(?P<literal>(?:u8|u|U|L)?R"(?P<delim>[^ ()\\\t\r\n]{0,16})\([\s\S]*?\)(?P=delim)"|(?:u8|u|U|L)?"(?:\\[\s\S]|[^"\\])*"|(?:u8|u|U|L)?\'(?:\\[\s\S]|[^\'\\])*\')')
DIRECTIVE = re.compile(r'^\s*#\s*(\w+)\b(.*)', re.S)
MARKER = re.compile(r'\b(TODO|FIXME|PLACEHOLDER)\b', re.I)
CONTROL = set('if for while switch catch sizeof alignof decltype noexcept typeid asm __asm__ __asm static_assert static_cast reinterpret_cast const_cast dynamic_cast'.split())
CALLABLE = re.compile(r'(?P<name>(?:(?:[A-Za-z_]\w*(?:\s*<[^;{}()]*>)?\s*::\s*)*)(?:~?[A-Za-z_]\w*|operator\s*(?:\(\)|\[\]|[^\w\s({;]+|[A-Za-z_]\w*(?:\s*[*&])?)))\s*\(')
DECLARATION = re.compile(r'^(?:(?:static|extern|register|constexpr|const|volatile|unsigned|signed|long|short|typename|struct|class|enum)\s+)*(?:[A-Za-z_]\w*(?:::\w+)*(?:\s*<.*>)?)(?:\s+|\s*[*&]+\s*)(?:const\s+)?[A-Za-z_]\w*(?:\s*\[[^]]*\])*\s*(?:[=(].*|)?$', re.S)
RISK = re.compile(r'math|physics|collis|binder|gravity|geometry|\bmtx|transform|random|rng|\brand\b|sqrt|rsqrt|frsqr|recip|normaliz|psvec|psmtx|jma|velocity|accel|integrat|matrix|quaternion', re.I)
TITLES = {
    'a': 'MWERKS-only statements without a native alternative',
    'b': 'Executable statements removed by a native exclusion',
    'c': 'Empty or trivial native branches beside Wii logic',
    'd': 'Entire function body disappears under native conditionals',
    'e': 'Inline assembly without an executable native alternative',
    'f': 'TODO / FIXME / PLACEHOLDER occurrences in native branches',
}


def mask(text, comments=True, literals=True):
    def replace(match):
        selected = (comments and match['comment'] is not None) or (literals and match['literal'] is not None)
        if not selected:
            return match.group()
        value = ''.join('\n' if c == '\n' else ' ' for c in match.group())
        if match['literal'] is not None:
            value = '@' + value[1:]
        return value
    return LEXEME.sub(replace, text)


def conjunction(a, b):
    if a is False or b is False:
        return False
    return True if a is True and b is True else None


def disjunction(a, b):
    if a is True or b is True:
        return True
    return False if a is False and b is False else None


def negate(value):
    return None if value is None else not value


@dataclass
class Macro:
    defined: bool | None
    value: int | None


def profile(native):
    values = {'GALAXY_NATIVE': native, '__MWERKS__': not native, '__GNUC__': native,
              '__clang__': native, '__aarch64__': native, '__powerpc__': not native,
              '__PPC__': not native, '__POWERPC__': not native, '__ppc__': not native,
              'GEKKO': not native}
    return {k: Macro(v, int(v)) for k, v in values.items()}


def expression(text, env):
    """Tri-state evaluation; unknown project/configuration macros stay unknown."""
    unknown = Macro(None, None)
    text = re.sub(r'defined\s*(?:\(\s*(\w+)\s*\)|(\w+))',
                  lambda m: str(int(env.get(m[1] or m[2], unknown).defined)) if env.get(m[1] or m[2], unknown).defined is not None else '__unknown_defined', text)
    text = re.sub(r'\b(0[xX][0-9a-fA-F]+|\d+)[uUlL]+\b', r'\1', text)
    text = text.replace('&&', ' and ').replace('||', ' or ')
    text = re.sub(r'!(?!=)', ' not ', text).strip()
    def visit(node):
        if isinstance(node, ast.Constant) and isinstance(node.value, (int, bool)):
            return node.value
        if isinstance(node, ast.Name):
            return env.get(node.id, unknown).value
        if isinstance(node, ast.UnaryOp):
            value = visit(node.operand)
            if value is None:
                return None
            return {ast.Not: lambda: not value, ast.USub: lambda: -value,
                    ast.UAdd: lambda: value, ast.Invert: lambda: ~value}[type(node.op)]()
        if isinstance(node, ast.BoolOp):
            values = [None if (v := visit(n)) is None else bool(v) for n in node.values]
            value = values[0]
            for other in values[1:]:
                value = (conjunction if isinstance(node.op, ast.And) else disjunction)(value, other)
            return value
        if isinstance(node, ast.BinOp):
            a, b = visit(node.left), visit(node.right)
            if a is None or b is None:
                return None
            operations = {ast.BitAnd: lambda: a & b, ast.BitOr: lambda: a | b, ast.BitXor: lambda: a ^ b,
                          ast.LShift: lambda: a << b, ast.RShift: lambda: a >> b, ast.Add: lambda: a + b,
                          ast.Sub: lambda: a - b, ast.Mult: lambda: a * b, ast.Div: lambda: int(a / b), ast.Mod: lambda: a % b}
            return operations[type(node.op)]()
        if isinstance(node, ast.Compare) and len(node.ops) == 1:
            a, b = visit(node.left), visit(node.comparators[0])
            if a is None or b is None:
                return None
            return {ast.Eq: lambda: a == b, ast.NotEq: lambda: a != b, ast.Lt: lambda: a < b,
                    ast.LtE: lambda: a <= b, ast.Gt: lambda: a > b, ast.GtE: lambda: a >= b}[type(node.ops[0])] ()
        return None
    try:
        return visit(ast.parse(text, mode='eval').body)
    except (SyntaxError, KeyError, TypeError, ValueError, ZeroDivisionError, OverflowError):
        return None


def condition(kind, text, env):
    if kind in ('ifdef', 'ifndef'):
        result = env.get(text.strip(), Macro(None, None)).defined
        return negate(result) if kind == 'ifndef' else result
    value = expression(text, env)
    return None if value is None else bool(value)


@dataclass
class Branch:
    kind: str
    expression: str
    start: int
    end: int = 0
    choices: dict = field(default_factory=dict)


@dataclass
class Group:
    start: int
    end: int = 0
    branches: list = field(default_factory=list)
    parents: tuple = ()


@dataclass(frozen=True)
class Function:
    name: str
    start: int
    opening: int
    closing: int
    signature: str
    assembly: bool = False


def pairs(code):
    stack, result = [], {}
    for pos, char in enumerate(code):
        if char in '({[':
            stack.append((char, pos))
        elif char in ')}]' and stack and stack[-1][0] == {')': '(', '}': '{', ']': '['}[char]:
            _, start = stack.pop()
            result[start] = pos
    return result


def functions(code):
    paired = pairs(code)
    scopes = []
    for match in re.finditer(r'\b(namespace|class|struct)\s+([A-Za-z_]\w*(?:::\w+)*)(?:\s*<[^;{}]*>)?(?:\s+final)?\s*(?::[^;{}]+)?\{', code):
        opening = match.end() - 1
        if opening in paired:
            scopes.append((opening, paired[opening], match[2]))
    result = {}
    for match in CALLABLE.finditer(code):
        name = re.sub(r'\s+', '', match['name'])
        if name in CONTROL or name.split('::')[-1] in CONTROL:
            continue
        paren = match.end() - 1
        if paren not in paired:
            continue
        end = paired[paren]
        cursor = end + 1
        opening = None
        while cursor < len(code):
            boundary = re.search(r'[{};]', code[cursor:])
            if not boundary:
                break
            pos = cursor + boundary.start()
            suffix = code[end + 1:pos]
            if code[pos] != '{' or '=' in suffix or (')' in suffix and '(' not in suffix):
                break
            # Skip braced member initializers in constructors.
            if re.search(r'(?<!:):(?!:)', suffix) and re.search(r'[\w>]\s*$', code[:pos]) and pos in paired:
                cursor = paired[pos] + 1
                continue
            opening = pos
            break
        if opening is None or opening not in paired:
            continue
        boundary = max(code.rfind(';', 0, match.start()), code.rfind('{', 0, match.start()), code.rfind('}', 0, match.start()))
        prefix = code[boundary + 1:match.start()].strip()
        enclosing = [n for a, b, n in scopes if a < opening < b]
        constructor = name.lstrip('~').split('::')[-1] in [n.split('::')[-1] for n in enclosing] or '::' in name
        if not prefix and not constructor:
            continue
        if re.search(r'\b(return|if|for|while|switch|else|new|delete)\b', prefix):
            continue
        if '::' not in name and enclosing:
            name = '::'.join(enclosing + [name])
        fn = Function(name, match.start(), opening, paired[opening], re.sub(r'\s+', '', code[paren:end + 1]), bool(re.search(r'\b(?:asm|__asm__)\b', prefix)))
        # A constructor's member initializer also looks callable; keep the first header.
        result.setdefault(opening, fn)
    return list(result.values())


def statements(code, declarations=False):
    """Lexical executable statement candidates; pure declarations/directives excluded."""
    result = []
    for chunk in re.split(r'[;{}]', code):
        chunk = chunk.strip()
        if not chunk:
            continue
        if re.match(r'^(?:typedef|using|static_assert)\b', chunk):
            continue
        if DECLARATION.fullmatch(chunk) and not re.match(r'^(?:return|throw|delete|goto|asm|__asm__|__asm)\b', chunk):
            if declarations:
                result.append(chunk)
            continue
        result.append(chunk)
    return result


def stub(code):
    compact = re.sub(r'\s+', '', code)
    compact = compact.replace('{', '').replace('}', '')
    if compact in ('', ';'):
        return 'empty'
    if compact in ('return;', 'return0;', 'return0u;', 'return0U;', 'return0L;', 'returnnullptr;', 'returnNULL;'):
        return compact
    return None


class Source:
    def __init__(self, path, text):
        self.path, self.text = path, text
        self.masked = mask(text)
        self.lines = [0] + [m.end() for m in re.finditer('\n', text)]
        self.groups, self.directives, self.errors = [], [], []
        self.context = []
        stack = []
        offset = 0
        physical = self.masked.splitlines(keepends=True)
        i = 0
        while i < len(physical):
            start = offset
            line = physical[i]
            offset += len(line)
            i += 1
            while line.rstrip('\n').endswith('\\') and i < len(physical):
                line = line.rstrip('\n')[:-1] + physical[i]
                offset += len(physical[i])
                i += 1
            directive = DIRECTIVE.match(line)
            if not directive:
                continue
            kind, value = directive.groups()
            value = value.strip()
            self.directives.append((start, offset, kind, value))
            if kind in ('if', 'ifdef', 'ifndef'):
                group = Group(start, parents=tuple((g, len(g.branches) - 1) for g in stack))
                group.branches.append(Branch(kind, value, offset))
                self.groups.append(group)
                stack.append(group)
            elif kind in ('else', 'elif') and stack:
                stack[-1].branches[-1].end = start
                stack[-1].branches.append(Branch(kind, value, offset))
            elif kind == 'endif' and stack:
                group = stack.pop()
                group.branches[-1].end = start
                group.end = offset
            elif kind in ('else', 'elif', 'endif'):
                self.errors.append(f'line {self.line(start)}: unmatched #{kind}')
        if stack:
            self.errors.append('unterminated conditional')
            for group in stack:
                group.end = len(text)
                group.branches[-1].end = len(text)
        self.views = {}
        self.activity = {}
        for native in (True, False):
            env, states, active, segments = profile(native), [], True, []
            cursor = 0
            by_start = {g.start: g for g in self.groups}
            selected = {}
            for start, end, kind, value in self.directives:
                if start > cursor:
                    segments.append((cursor, start, active))
                if kind in ('if', 'ifdef', 'ifndef'):
                    choice = condition(kind, value, env)
                    states.append([active, choice, by_start[start], 0])
                    selected[(start, 0)] = choice
                    active = conjunction(active, choice)
                elif kind in ('elif', 'else') and states:
                    parent, taken, group, index = states[-1]
                    choice = True if kind == 'else' else condition('if', value, env)
                    branch = conjunction(negate(taken), choice)
                    states[-1][1] = disjunction(taken, choice)
                    states[-1][3] = index + 1
                    selected[(group.start, index + 1)] = branch
                    active = conjunction(parent, branch)
                elif kind == 'endif' and states:
                    active = states.pop()[0]
                elif kind in ('define', 'undef'):
                    definition = re.match(r'(\w+)(.*)', value, re.S)
                    if definition and active is not False:
                        name, tail = definition.groups()
                        if active is None:
                            env[name] = Macro(None, None)
                        elif kind == 'undef':
                            env[name] = Macro(False, 0)
                        else:
                            env[name] = Macro(True, expression(tail.strip() or '1', env) if not tail.startswith('(') else None)
                segments.append((start, end, False))
                cursor = end
            segments.append((cursor, len(text), active))
            view = list(self.masked)
            for a, b, enabled in segments:
                if enabled is False:
                    view[a:b] = ['\n' if ch == '\n' else ' ' for ch in self.masked[a:b]]
            self.views[native] = ''.join(view)
            self.activity[native] = segments
            for group in self.groups:
                for index, branch in enumerate(group.branches):
                    branch.choices[native] = selected.get((group.start, index))
        raw = list(self.masked)
        for a, b, _, _ in self.directives:
            raw[a:b] = ['\n' if ch == '\n' else ' ' for ch in self.masked[a:b]]
        self.code = ''.join(raw)
        self.functions_by_profile = {p: functions(view) for p, view in self.views.items()}
        self.functions = list({(f.opening, f.closing): f for fs in [self.functions_by_profile[False], functions(self.code), self.functions_by_profile[True]] for f in fs}.values())
        self.function_priority = {(f.opening, f.closing): 0 for f in self.functions_by_profile[True]}

    def line(self, pos):
        return bisect_right(self.lines, pos)

    def function_at(self, pos, end=None):
        candidates = [f for f in self.functions if f.opening < pos and (end if end is not None else pos) <= f.closing]
        return min(candidates, key=lambda f: (self.function_priority.get((f.opening, f.closing), 1), f.closing - f.opening)) if candidates else None

    def enabled(self, pos, native):
        # Directives are not executable; segment lookups are used only for source tokens.
        for a, b, active in self.activity[native]:
            if a <= pos < b:
                return active
        return None

    def native_context(self, pos, potential=False):
        for group in self.groups:
            if not any('GALAXY_NATIVE' in b.expression for b in group.branches):
                continue
            for branch in group.branches:
                if branch.start <= pos < branch.end and (branch.choices[True] is True or (potential and branch.choices[True] is None)) and branch.choices[False] is False:
                    return True
        return False


def scan_text(path, text):
    source = Source(path, text)
    findings, seen = [], set()
    def add(category, pos, fn, detail, evidence='', confidence='structural'):
        name = fn.name if fn else '<file scope>'
        key = (category, pos if category == 'f' else source.line(pos), name, detail)
        if key in seen:
            return
        seen.add(key)
        risk = 'high' if RISK.search(path + ' ' + name) else 'medium' if '/Player/' in path else 'review'
        findings.append(dict(category=category, path=path, line=source.line(pos), function=name,
                             lane=lane_of(path), risk=risk, confidence=confidence, detail=detail,
                             evidence=' '.join(evidence.split())[:220]))
    for group in source.groups:
        branch = group.branches[0]
        fn = source.function_at(group.start, group.end)
        if not fn or source.enabled(fn.opening, True) is False or any(parent.branches[index].choices[True] is False for parent, index in group.parents):
            continue
        native = source.views[True][group.start:group.end]
        wii = source.views[False][group.start:group.end]
        has_alternative = len(group.branches) > 1
        first_kind, cond = branch.kind, branch.expression
        wii_guard = (first_kind == 'ifdef' and cond == '__MWERKS__') or (first_kind == 'if' and re.fullmatch(r'(?:defined\s*\(?\s*)?__MWERKS__\s*\)?', cond) is not None)
        exclusion = (first_kind == 'ifndef' and cond == 'GALAXY_NATIVE') or (first_kind == 'if' and re.fullmatch(r'!\s*defined\s*(?:\(\s*GALAXY_NATIVE\s*\)|GALAXY_NATIVE)', cond) is not None)
        wii_guard = wii_guard or (first_kind == 'if' and '__MWERKS__' in cond and branch.choices[True] is False)
        exclusion = exclusion or (first_kind == 'if' and re.search(r'!\s*(?:defined\s*\(?\s*)?GALAXY_NATIVE\b', cond) is not None and branch.choices[True] is False)
        if wii_guard and not has_alternative and statements(wii, declarations=True):
            add('a', group.start, fn, 'MWERKS branch has statements and no alternative', source.code[branch.start:branch.end])
        if exclusion and not has_alternative and statements(wii):
            add('b', group.start, fn, 'Native exclusion drops executable statement candidates', wii)
        if has_alternative and any('GALAXY_NATIVE' in b.expression for b in group.branches) and statements(wii) and (kind := stub(native)):
            add('c', group.start, fn, f'Native branch is {kind}; Wii branch has logic', wii)
    def can_be_empty(start, end):
        groups = [g for g in source.groups if start <= g.start and g.end <= end]
        top = [g for g in groups if not any(other.start < g.start and g.end <= other.end for other in groups)]
        outside = list(source.views[True][start:end])
        for group in top:
            outside[group.start - start:group.end - start] = ' ' * (group.end - group.start)
        if re.sub(r'[\s{};]', '', ''.join(outside)):
            return False
        for group in top:
            selected = [b for b in group.branches if b.choices[True] is True]
            possible = selected or [b for b in group.branches if b.choices[True] is None]
            if not selected and not any(b.kind == 'else' for b in group.branches):
                continue
            if not any(can_be_empty(b.start, b.end) for b in possible):
                return False
        return True

    for fn in source.functions:
        if source.enabled(fn.opening, True) is False:
            continue
        body = source.code[fn.opening + 1:fn.closing]
        contained = [g for g in source.groups if fn.opening < g.start and g.end <= fn.closing]
        if not contained:
            continue
        outside = list(body)
        for group in contained:
            a, b = group.start - fn.opening - 1, group.end - fn.opening - 1
            outside[a:b] = ' ' * (b - a)
        if re.sub(r'[\s{};]', '', ''.join(outside)):
            continue
        native = source.views[True][fn.opening + 1:fn.closing]
        if stub(native) == 'empty' and statements(body, declarations=True):
            add('d', fn.start, fn, 'Whole function body disappears in the native projection', body)
        elif any(b.choices[True] is None for g in contained for b in g.branches) and statements(body, declarations=True) and can_be_empty(fn.opening + 1, fn.closing):
            add('d', fn.start, fn, 'Guard-only body can disappear; project/configuration macro is unknown', body, 'configuration-dependent')
    # Separate definitions on either side of a file-scope native split.
    for native_fn in source.functions_by_profile[True]:
        native_body = source.views[True][native_fn.opening + 1:native_fn.closing]
        kind = stub(native_body)
        if kind is None or not source.native_context(native_fn.start):
            continue
        for wii_fn in source.functions_by_profile[False]:
            if (native_fn.name, native_fn.signature) == (wii_fn.name, wii_fn.signature) and native_fn.opening != wii_fn.opening:
                wii_body = source.views[False][wii_fn.opening + 1:wii_fn.closing]
                if statements(wii_body):
                    add('c', native_fn.start, native_fn, f'Separate native definition is {kind}; Wii definition has logic', wii_body)
    paired = pairs(source.code)
    for match in re.finditer(r'\b(?:asm|__asm__|__asm)\b', source.code):
        fn = source.function_at(match.start())
        if not fn or fn.assembly or (source.enabled(match.start(), True) is False and source.enabled(match.start(), False) is False):
            continue
        cursor = match.end()
        suffix = re.match(r'\s*(?:(?:volatile|__volatile__|inline|goto)\s*)*([({])', source.code[cursor:])
        if not suffix:
            continue
        opening = cursor + suffix.end() - 1
        end = paired.get(opening, opening)
        parents = [g for g in source.groups if g.start < match.start() < g.end]
        alternative = any(len(g.branches) > 1 and source.enabled(match.start(), True) is False and statements(source.views[True][g.start:g.end]) and stub(source.views[True][g.start:g.end]) is None for g in parents)
        if not alternative:
            confidence = 'definition-excluded' if source.enabled(fn.opening, True) is False else 'configuration-dependent' if source.enabled(match.start(), True) is None else 'structural'
            detail = 'Whole definition excluded natively; verify external native provider/build membership' if confidence == 'definition-excluded' else 'Inline assembly has no executable native alternative in its conditional chain'
            add('e', match.start(), fn, detail, text[match.start():end + 1], confidence)
    marker_text = mask(text, comments=False)
    for match in MARKER.finditer(marker_text):
        if source.native_context(match.start(), potential=True):
            add('f', match.start(), source.function_at(match.start()), match.group().upper(), text[source.lines[source.line(match.start()) - 1]:].split('\n', 1)[0])
    return findings, source.errors


def sort_key(finding):
    return (finding['confidence'] == 'definition-excluded', {'high': 0, 'medium': 1, 'review': 2}[finding['risk']], finding['confidence'] == 'configuration-dependent', finding['path'], finding['line'], finding['function'])


def scan(root):
    findings, files, errors = [], [], []
    for directory in ('src', 'include', 'libs'):
        for path in sorted((root / directory).rglob('*')):
            extensionless_header = not path.suffix and 'include' in path.relative_to(root).parts
            if not path.is_file() or (path.suffix not in EXTENSIONS and not extensionless_header) or path.is_symlink():
                continue
            relative = path.relative_to(root).as_posix()
            data = path.read_bytes()
            found, issues = scan_text(relative, data.decode('utf-8', errors='replace'))
            findings.extend(found)
            files.append(dict(path=relative, sha256=hashlib.sha256(data).hexdigest()))
            errors.extend(f'{relative}: {issue}' for issue in issues)
    return dict(files_scanned=len(files), findings=sorted(findings, key=lambda f: (f['category'], sort_key(f))),
                counts={key: sum(f['category'] == key for f in findings) for key in TITLES},
                marker_counts=dict(Counter(f['detail'] for f in findings if f['category'] == 'f')),
                parse_errors=errors, files=files)


def markdown(report):
    lines = ['# Native conditional and assembly gap inventory', '',
             f"Read-only lexical scan of **{report['files_scanned']}** source/header files under `src/`, `include/`, `libs/`. No compiler, emulator or source edits. Base HEAD: `{report.get('head', 'unknown')}`.", '',
             'These are review candidates, not demonstrated gameplay bugs or proofs of semantic equivalence. Classes overlap. Math/physics/collision/RNG names rank first within each class, then Player paths. Intentional hardware no-ops remain visible. Class (e) also inventories whole definitions excluded natively, labeled definition-excluded and placed last; those need external-provider/build-membership review, not a claim of an empty compiled function. No allowlist hides candidates.', '',
             'Known profile: GALAXY_NATIVE on, MWERKS/PPC off, Clang/GNU-compatible/arm64 on. Other configuration macros remain unknown, with local unconditional defines tracked. Unknown guard-only bodies are labeled configuration-dependent. Includes are not expanded, macros are not executed, and constructor/declaration side effects and arbitrary native replacements require human review. Function names are best-effort lexical names, including header scopes; .inc fragments may lack their surrounding function.', '',
             'Lane ownership is exactly `tools/team/lanefiles.py:lane_of`; its legacy lane map can differ from the current P1 helper assignments.', '',
             '| Class | Candidates |', '| --- | ---: |']
    lines += [f'| ({key}) {title} | {report["counts"][key]} |' for key, title in TITLES.items()]
    lines += ['', 'Marker occurrences: ' + (', '.join(f'{key}={value}' for key, value in sorted(report['marker_counts'].items())) or '0') + '.', '']
    for category, title in TITLES.items():
        lines += [f'## ({category}) {title}', '']
        rows = sorted((f for f in report['findings'] if f['category'] == category), key=sort_key)
        if not rows:
            lines += ['No candidates.', '']
            continue
        lines += ['| Location | Function | Lane | Risk / confidence | Evidence |', '| --- | --- | --- | --- | --- |']
        for row in rows:
            def cell(text):
                return str(text).replace('|', '\\|').replace('`', "'").replace('\n', ' ')
            label = f"{row['path']}:{row['line']}"
            lines.append(f"| [{label}](../../{row['path']}#L{row['line']}) | `{cell(row['function'])}` | {row['lane']} | {row['risk']} / {row['confidence']} | {cell(row['detail'])}: `{cell(row['evidence'])}` |")
        lines.append('')
    lines += ['## Reproduce and limits', '',
              '```sh', 'nice -n 10 python3 tools/port/native_gaps.py --output docs/port/NATIVE_GAPS.md',
              "python3 -m unittest discover -s tools/port -p 'test_native_gaps.py'", '```', '',
              'The scan covers structural shapes (a–f), not whether a nonempty replacement is mathematically correct. Pure declarations/includes/pragmas are excluded from (b); (a) also retains declaration statements. C++ parsing is conservative rather than complete: overloads, initializer lists, unusual macros and unbalanced conditional fragments can need manual attribution. Unknown compile conditions cannot establish that a candidate executes in the native target. Confirm build membership and inspect callees before changing code.', '']
    if report['parse_errors']:
        lines += ['Conditional parsing issues (coverage needs review):', ''] + ['- ' + error for error in report['parse_errors']] + ['']
    return '\n'.join(lines)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--output', type=Path, default=ROOT / 'docs/port/NATIVE_GAPS.md')
    parser.add_argument('--json', type=Path, help='optional detailed findings and per-source hashes')
    args = parser.parse_args(argv)
    report = scan(args.root.resolve())
    head = subprocess.run(['git', '-C', str(args.root), 'rev-parse', 'HEAD'], capture_output=True, text=True)
    report['head'] = head.stdout.strip() if head.returncode == 0 else 'unknown'
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(markdown(report))
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({key: report[key] for key in ('files_scanned', 'counts', 'marker_counts', 'parse_errors')}))
    return 2 if report['parse_errors'] else 0


if __name__ == '__main__':
    sys.exit(main())
