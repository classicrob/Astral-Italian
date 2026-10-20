#!/usr/bin/env python3
"""Inventory native-layout-sensitive C++ expressions; default mode never runs a compiler.

This is a lexical candidate inventory, not a C++ type checker or a proof of safety.
Unresolved casts, macros, binding provenance and layouts remain UNKNOWN. See
 docs/port/OFFSET_AUDIT.md for the schema, collection plan and coverage limits.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass

SCHEMA = 1
SUFFIXES = {'.c', '.cc', '.cpp', '.cxx', '.h', '.hh', '.hpp', '.hxx', '.inc', '.inl'}
DOMAINS = {'host_object', 'serialized', 'wire', 'hardware', 'unknown'}
DEFAULT_SCOPES = ['src/Game', 'include', 'libs']


def sha(data):
    return hashlib.sha256(data).hexdigest()


def load(path):
    value = json.loads(Path(path).read_text())
    if value.get('schema') != SCHEMA:
        raise ValueError(f'{path}: expected schema {SCHEMA}')
    return value


@dataclass
class Token:
    text: str
    start: int
    end: int
    line: int


# Strings/comments are consumed as units so fake casts inside them are not inventoried.
LEX = re.compile(r'(?P<space>\s+)|(?P<comment>//(?:\\\r?\n|[^\n])*|/\*.*?\*/)|'
                 r'(?P<raw>(?:u8|u|U|L)?R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\(.*?\)(?P=delimiter)")|'
                 r'(?P<string>(?:u8|u|U|L)?"(?:\\.|[^"\\])*"|(?:u|U|L)?\'(?:\\.|[^\'\\])*\')|'
                 r'(?P<id>[A-Za-z_$][\w$]*)|(?P<number>0[xX][\da-fA-F]+[uUlL]*|\d+[uUlL]*)|'
                 r'(?P<punct>->\*|->|::|&&|\|\||<<|>>|==|!=|<=|>=|\+=|-=|\+\+|--|.)', re.S)


def lex(source):
    tokens, line = [], 1
    for match in LEX.finditer(source):
        text = match.group()
        if match.lastgroup not in {'space', 'comment', 'string', 'raw'}:
            tokens.append(Token(text, match.start(), match.end(), line))
        line += text.count('\n')
    return tokens


def directive_source(source):
    """Erase comments/literals without shifting source byte/line positions."""
    chars = list(source)
    for match in LEX.finditer(source):
        if match.lastgroup in {'comment', 'string', 'raw'}:
            for i in range(match.start(), match.end()):
                if chars[i] not in '\r\n':
                    chars[i] = ' '
    return ''.join(chars)


def guard_lines(source):
    """Three-valued native reachability; unfamiliar #if expressions stay unknown."""
    native_known = True

    def evaluate(expr):
        expr = expr.strip()
        if 'GALAXY_NATIVE' in expr and not native_known:
            return None
        if expr in {'1', 'GALAXY_NATIVE', 'defined(GALAXY_NATIVE)', 'defined GALAXY_NATIVE'}:
            return True
        if expr in {'0', '!GALAXY_NATIVE', '!defined(GALAXY_NATIVE)', '!defined GALAXY_NATIVE'}:
            return False
        return None

    stack, result = [], {}
    # Only LF is a C++ physical line boundary here. Latin-1 bytes such as 0x85
    # can be part of Shift-JIS/UTF-8 strings, not extra preprocessor lines.
    for number, line in enumerate(directive_source(source).split('\n'), 1):
        if re.match(r'\s*#\s*(define|undef)\s+GALAXY_NATIVE\b', line):
            native_known = False
        m = re.match(r'\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)', line)
        if m:
            directive, tail = m.groups()
            if directive in {'if', 'ifdef', 'ifndef'}:
                test = evaluate(tail) if directive == 'if' else (True if native_known and tail.strip() == 'GALAXY_NATIVE' else None)
                if directive == 'ifndef' and test is not None:
                    test = not test
                stack.append([test, test])  # current arm, any earlier arm taken
            elif directive == 'else' and stack:
                prior = stack[-1][1]
                stack[-1] = [None if prior is None else not prior, True]
            elif directive == 'elif' and stack:
                prior, test = stack[-1][1], evaluate(tail)
                arm = False if prior is True else test if prior is False else None
                taken = True if prior is True or test is True else False if prior is False and test is False else None
                stack[-1] = [arm, taken]
            elif directive == 'endif' and stack:
                stack.pop()
        states = [entry[0] for entry in stack]
        result[number] = 'excluded' if False in states else 'unknown' if None in states else 'included'
    return result


def paired(tokens):
    stack, pairs, errors = [], {}, []
    for i, token in enumerate(tokens):
        if token.text in {'(', '[', '{'}:
            stack.append(i)
        elif token.text in {')', ']', '}'}:
            if stack and tokens[stack[-1]].text == {')': '(', ']': '[', '}': '{'}[token.text]:
                opening = stack.pop()
                pairs[opening], pairs[i] = i, opening
            else:
                errors.append({'line': token.line, 'reason': 'unpaired closing delimiter (possibly preprocessor arms)'})
    errors.extend({'line': tokens[i].line, 'reason': 'unpaired opening delimiter (possibly macro)'} for i in stack)
    return pairs, errors


def statement(tokens, index):
    start = index
    while start and tokens[start - 1].text not in {';', '{', '}'}:
        start -= 1
    end = index + 1
    while end < len(tokens) and tokens[end].text not in {';', '{', '}'}:
        end += 1
    return start, end


def integer(text):
    try:
        clean = re.sub('[uUlL]+$', '', text)
        return int(clean, 8) if re.fullmatch(r'0[0-7]+', clean) else int(clean, 0)
    except ValueError:
        return None


def scan_text(path, data):
    # latin-1 is lossless: start/end indices are byte offsets even for Shift-JIS source.
    source = data.decode('latin-1')
    tokens = lex(source)
    pairs, errors = paired(tokens)
    guards = guard_lines(source)
    rows, seen = [], set()
    offsetof_macro_redefined = bool(re.search(r'^\s*#\s*(define|undef)\s+offsetof\b', directive_source(source), re.M))

    def operand_end(index):
        """Keep unary/postfix chains in evidence without claiming their type."""
        while index < len(tokens) and tokens[index].text in {'&', '*', '+', '-', '!', '~'}:
            index += 1
        if index >= len(tokens):
            return index
        if tokens[index].text == '(' and index in pairs:
            index = pairs[index] + 1
        else:
            index += 1
        while index < len(tokens):
            if tokens[index].text in {'.', '->', '::'} and index+1 < len(tokens):
                index += 2
            elif tokens[index].text in {'[', '('} and index in pairs:
                index = pairs[index] + 1
            else:
                break
        return index

    def add(index, end, kind, cast_type=None):
        key = (tokens[index].start, kind)
        if key in seen:
            return
        seen.add(key)
        left, right = statement(tokens, index)
        context = tokens[left:right]
        arithmetic = [j for j in range(left, right) if tokens[j].text in {'+', '-', '[', '+=', '-='}]
        constants = sorted({n for j in arithmetic for t in tokens[j+1:min(j+4, right)]
                            if (n := integer(t.text)) is not None})
        expression = source[tokens[index].start:tokens[end - 1].end]
        context_text = source[context[0].start:context[-1].end] if context else expression
        row_id = sha(f'{path}\0{tokens[index].start}\0{kind}'.encode())[:24]
        rows.append({'id': row_id, 'file': path, 'line': tokens[index].line,
                     'byte_offset': tokens[index].start, 'end_byte_offset': tokens[end - 1].end,
                     'kind': kind, 'expression': expression, 'expression_sha256': sha(expression.encode('latin-1')),
                     'context': context_text, 'cast_type': cast_type,
                     'arithmetic_in_statement': bool(arithmetic), 'nearby_integer_constants': constants,
                     'has_offsetof': any(t.text == 'offsetof' for t in context),
                     'native_guard': guards.get(tokens[index].line, 'unknown'),
                     'source_sha256': sha(data),
                     'offsetof_macro_redefined': offsetof_macro_redefined})

    for i, token in enumerate(tokens):
        if re.fullmatch(r'_[0-9a-fA-F]+(?:_[A-Za-z][\w$]*)?', token.text):
            member = i > 0 and tokens[i - 1].text in {'.', '->', '::'}
            add(i, i + 1, 'unnamed_member' if member else 'unnamed_identifier')
        if token.text in {'reinterpret_cast', 'static_cast', 'const_cast', 'dynamic_cast'} and i + 1 < len(tokens) and tokens[i+1].text == '<':
            # Handle nested template closing >> without pretending to infer their types.
            depth, end = 1, i + 2
            while end < len(tokens) and depth:
                depth += tokens[end].text.count('<') - tokens[end].text.count('>')
                end += 1
            if depth == 0:
                type_tokens = tokens[i+2:end-1]
                if type_tokens:
                    # Even static_cast<Alias> may be a pointer cast. Inventory
                    # every named cast; unknown typedefs never disappear.
                    last = end
                    if end < len(tokens) and tokens[end].text == '(' and end in pairs:
                        last = pairs[end] + 1
                    add(i, last, 'named_pointer_cast', ' '.join(t.text for t in type_tokens))
        if token.text in {'uintptr_t', 'intptr_t'} and i + 1 < len(tokens) and tokens[i+1].text == '(' and i+1 in pairs:
            add(i, pairs[i+1] + 1, 'functional_address_cast', token.text)
        if token.text == '(' and i in pairs:
            end = pairs[i]
            middle = tokens[i+1:end]
            # A broad type-shaped C cast inventory. Multiplication/call ambiguities
            # remain candidates rather than being silently dropped by guessed types.
            if middle and len(middle) < 80 and any(t.text in {'*', 'uintptr_t', 'intptr_t'} for t in middle):
                if all(re.fullmatch(r'[\w$]+', t.text) or t.text in {'*', '&', '::', '<', '>', '>>', ','} for t in middle):
                    last = end + 1
                    declarator = last < len(tokens) and tokens[last].text in {';', '{', 'const'}
                    if last < len(tokens) and not declarator:
                        last = operand_end(last)
                    add(i, last, 'possible_pointer_declarator' if declarator else 'c_pointer_cast', ' '.join(t.text for t in middle))
            elif middle and len(middle) < 20 and end + 1 < len(tokens):
                # A typedef can hide the pointer completely. Retain type-shaped
                # cast syntax, including ambiguous `(name)(expr)`, as UNKNOWN.
                type_shaped = all(re.fullmatch(r'[A-Za-z_$][\w$]*', t.text) or t.text == '::' for t in middle)
                operand = tokens[end+1].text
                if type_shaped and (re.fullmatch(r'[A-Za-z_$][\w$]*|0[xX][\da-fA-F]+|\d+', operand) or operand in {'(', '&', '*'}):
                    last = operand_end(end+1)
                    add(i, last, 'possible_alias_cast', ' '.join(t.text for t in middle))
        if token.text in {'+', '-', '+=', '-='} and i and i+1 < len(tokens):
            rhs = tokens[i+1].text
            if integer(rhs) is not None or rhs in {'sizeof', 'offsetof'}:
                # Pointer type is not inferred from names: scalar arithmetic
                # remains an explicit UNKNOWN candidate until AST/type binding.
                start = i-1
                if tokens[start].text == ']' and start in pairs:
                    start = max(0, pairs[start]-1)
                while start > 1 and tokens[start-1].text in {'.', '->', '::'}:
                    start -= 2
                add(start, operand_end(i+1), 'address_arithmetic_candidate')
        # Implicit this+K and non-cast offsetof references are retained independently.
        if token.text == 'this' and i + 1 < len(tokens) and tokens[i+1].text in {'+', '-', '['}:
            add(i, min(i+3, len(tokens)), 'this_arithmetic')
        if token.text == 'offsetof':
            end = i + 1
            if end < len(tokens) and tokens[end].text == '(' and end in pairs:
                end = pairs[end] + 1
            add(i, end, 'offsetof')
    return sorted(rows, key=lambda r: (r['byte_offset'], r['kind'])), errors


def parse_layout_dump(text):
    """Clang's human record dump; only direct named fields are imported.

    Nested/base fields and bitfields remain in raw output; access binding to them
    requires a qualified direct record and cannot accidentally use another level.
    """
    records, record, current_name = {}, None, None

    def finish():
        if current_name is not None:
            if current_name in records and records[current_name] != record:
                raise ValueError(f'conflicting layouts for {current_name}; preserve separate compilation contexts')
            records[current_name] = record

    for line in text.splitlines():
        head = re.match(r'\s*0 \| (?:struct|class|union) (.+?)\s*$', line)
        if head:
            finish()
            current_name = head.group(1)
            record = {'fields': {}}
            continue
        if record is None:
            continue
        size = re.search(r'\[sizeof=(\d+),.*?align=(\d+)', line)
        if size:
            record.update(size=int(size[1]), align=int(size[2]))
            continue
        field = re.match(r'\s*(\d+)(?::(\d+)-(\d+))? \|   (\S.*)$', line)
        if field:
            declaration = field[4]
            if declaration.endswith('(base)') or declaration.endswith('(primary base)'):
                continue
            name = re.search(r'([A-Za-z_$][\w$]*)(?:\[[^\]]*\])*$', declaration)
            if name:
                entry = {'offset_bits': int(field[1]) * 8 + int(field[2] or 0), 'type': declaration[:name.start()].strip()}
                if field[3] is not None:
                    entry['size_bits'] = int(field[3]) - int(field[2]) + 1
                record['fields'][name[1]] = entry
    finish()
    return records


def verify_pins(root, files):
    errors = []
    for name, expected in files.items():
        path = Path(name) if Path(name).is_absolute() else root / name
        if not path.is_file() or sha(path.read_bytes()) != expected:
            errors.append(f'stale or missing input: {name}')
    return errors


def artifact(root, path, abi):
    if path is None:
        return None, ['layout artifact not supplied']
    data = load(path)
    errors = verify_pins(root, data.get('files', {}))
    if data.get('abi') != abi:
        errors.append(f'expected ABI {abi}')
    producer = data.get('producer', {})
    if not producer.get('command') or not producer.get('kind') or not data.get('files'):
        errors.append('missing producer command/kind or source pins')
    if producer.get('accuracy') not in {'measured', 'approximation'}:
        errors.append('layout is not measured or explicitly approximate')
    return data, errors


def classify(row, binding, layouts, errors):
    row.update(classification='UNKNOWN', gaps=[], domain='unknown')
    if row['native_guard'] == 'excluded':
        row['classification'] = 'NATIVE_EXCLUDED'
        return
    if row['kind'] == 'offsetof' and row['native_guard'] == 'included' and not row.get('offsetof_macro_redefined'):
        row['classification'] = 'LAYOUT_AWARE_EXPRESSION'
        row['gaps'].append('member identity and containing expression still need review')
        return
    if not binding:
        row['gaps'].append('no exact type/field/domain access binding')
        return
    row['binding'] = binding
    domain = binding.get('domain', 'unknown')
    row['domain'] = domain
    if domain not in DOMAINS:
        row['gaps'].append('invalid domain')
    if binding.get('source_sha256') != row['source_sha256'] or binding.get('expression_sha256') != row['expression_sha256']:
        row['gaps'].append('stale or missing binding source/expression hash')
    if not binding.get('evidence'):
        row['gaps'].append('binding has no evidence')
    if row['native_guard'] == 'unknown':
        row['gaps'].append('native preprocessor reachability unresolved')
    if row['gaps']:
        return
    if domain in {'serialized', 'wire', 'hardware'}:
        row['classification'] = 'NON_HOST_DOMAIN'
        return
    if domain != 'host_object':
        row['gaps'].append('host object vs serialized/wire/hardware domain unresolved')
        return
    record, field = binding.get('record'), binding.get('field')
    row['record'], row['field'] = record, field
    for abi in ('wii', 'native'):
        row['gaps'].extend(f'{abi}: {error}' for error in errors[abi])
        item = (layouts[abi] or {}).get('records', {}).get(record, {}).get('fields', {}).get(field)
        if item is None:
            row['gaps'].append(f'{abi}: record/field not in layout')
        else:
            row[f'{abi}_offset_bits'] = item['offset_bits']
    if row['gaps']:
        return
    wii, native = row['wii_offset_bits'], row['native_offset_bits']
    row['layout_offset_differs'] = wii != native
    if row['kind'] in {'unnamed_member', 'unnamed_identifier'}:
        row['classification'] = 'TYPED_MEMBER_ADJUSTS'
        row['gaps'].append('typed field moves naturally; spelling alone establishes no raw access bug')
        return
    raw = binding.get('raw_offset_bytes')
    if not isinstance(raw, int):
        row['gaps'].append('no bound literal offset from actual base record')
        return
    row['raw_offset_bytes'] = raw
    if raw * 8 != wii:
        row['gaps'].append('raw offset does not identify supplied Wii field')
        return
    approximate = any(layouts[abi]['producer'].get('accuracy') != 'measured' for abi in ('wii', 'native'))
    if raw * 8 != native:
        row['classification'] = 'LAYOUT_MISMATCH_CANDIDATE' if approximate else 'BOUND_LAYOUT_MISMATCH'
    else:
        row['classification'] = 'LAYOUT_SAME_OFFSET_CANDIDATE' if approximate else 'BOUND_SAME_FIELD_OFFSET'
    row['gaps'].append('bounds, access width, lifetime and runtime reachability are not certified by layout comparison')


def ast_accesses(root, manifest_path, rows, wii_layout, domains):
    """Import a pinned Clang JSON AST; never invoke Clang or infer a wire domain.

    Only one explicitly pinned primary source per artifact is accepted. Macro
    expansion locations and offset/type ambiguities stay unresolved.
    """
    meta = load(manifest_path)
    errors = verify_pins(root, meta.get('files', {}))
    primary = meta.get('primary_source')
    payload_path = Path(meta['ast'])
    if not payload_path.is_absolute():
        payload_path = Path(manifest_path).resolve().parent / payload_path
    raw = payload_path.read_bytes()
    if sha(raw) != meta.get('ast_sha256') or not meta.get('producer', {}).get('command'):
        errors.append('AST hash or producer command missing/stale')
    if primary not in meta.get('files', {}):
        errors.append('primary source not pinned')
    if errors:
        return {}, errors
    tree = json.loads(raw)
    records = (wii_layout or {}).get('records', {})
    selected = [row for row in rows if row['file'] == primary]
    nodes = []

    def visit(node, current_file):
        if not isinstance(node, dict):
            return
        loc = node.get('loc', {})
        begin = node.get('range', {}).get('begin', {})
        explicit = loc.get('file') or begin.get('file')
        if explicit:
            candidate = Path(explicit)
            if not candidate.is_absolute():
                candidate = root / candidate
            try:
                current_file = candidate.resolve().relative_to(root).as_posix()
            except ValueError:
                current_file = '<outside-root>'
        elif loc.get('includedFrom') or begin.get('includedFrom'):
            current_file = '<included-file>'
        if current_file == primary and 'offset' in begin and not any(k in begin for k in ('spellingLoc', 'expansionLoc')):
            nodes.append(node)
        for child in node.get('inner', []):
            visit(child, current_file)

    visit(tree, primary)
    starts = {}
    for node in nodes:
        start = node['range']['begin']['offset']
        starts.setdefault(start, []).append(node)

    def record_of(node):
        type_info = node.get('type', {})
        typename = type_info.get('desugaredQualType', type_info.get('qualType', ''))
        # Only the concrete pointer/ref record itself; pointer-to-pointer and
        # array indexing need a richer base relation and are not silently flattened.
        if typename.endswith(('*', '&')) and not typename.rstrip('*& ').endswith('*'):
            name = re.sub(r'\b(const|volatile|class|struct)\b', '', typename[:-1]).strip()
            if name in records:
                return name
        children = node.get('inner', [])
        if node.get('kind') in {'ParenExpr', 'ImplicitCastExpr', 'CStyleCastExpr',
                                'CXXReinterpretCastExpr', 'CXXConstCastExpr', 'CXXStaticCastExpr'} and len(children) == 1:
            # A call argument or a prior pointer increment is not the returned
            # pointer's allocation/base. Never search arbitrary descendants.
            if node.get('castKind') in {'DerivedToBase', 'BaseToDerived', 'UncheckedDerivedToBase'}:
                return None
            return record_of(children[0])
        return None

    def const_int(node):
        if node.get('kind') == 'IntegerLiteral':
            return int(node['value'], 0)
        children = node.get('inner', [])
        if len(children) == 1 and node.get('kind') in {'ParenExpr', 'ImplicitCastExpr', 'ConstantExpr'}:
            return const_int(children[0])
        if node.get('kind') == 'UnaryOperator' and node.get('opcode') in {'+', '-'} and len(children) == 1:
            value = const_int(children[0])
            return value if value is None or node['opcode'] == '+' else -value
        if node.get('kind') == 'BinaryOperator' and node.get('opcode') in {'+', '-', '*'} and len(children) == 2:
            a, b = const_int(children[0]), const_int(children[1])
            if a is not None and b is not None:
                return a+b if node['opcode'] == '+' else a-b if node['opcode'] == '-' else a*b
        return None

    result = {}
    for row in selected:
        candidates = starts.get(row['byte_offset'], [])
        record, field, offset = None, None, None
        if row['kind'] in {'unnamed_member', 'unnamed_identifier'}:
            # MemberExpr ranges begin at the base; use the member token's end.
            matches = [n for n in nodes if n.get('kind') == 'MemberExpr' and
                       n.get('name') == row['expression'] and
                       n.get('range', {}).get('end', {}).get('offset') == row['byte_offset']]
            identities = {(record_of(n.get('inner', [{}])[0]), n.get('name')) for n in matches}
            if len(identities) == 1:
                record, field = identities.pop()
        else:
            # A raw byte displacement must belong to an actual AST binary op.
            # Match either the inner byte cast or the complete outer cast range.
            matches = []
            for n in nodes:
                if n.get('kind') != 'BinaryOperator' or n.get('opcode') not in {'+', '-'}:
                    continue
                begin = n['range']['begin']['offset']
                ending = n['range'].get('end', {})
                end = ending.get('offset', -1) + ending.get('tokLen', 1)
                if not (row['byte_offset'] <= begin < row['end_byte_offset'] or begin == row['byte_offset']):
                    continue
                children = n.get('inner', [])
                if len(children) != 2:
                    continue
                count, rec = const_int(children[1]), record_of(children[0])
                pointer_info = children[0].get('type', {})
                pointer_type = pointer_info.get('desugaredQualType', pointer_info.get('qualType', ''))
                if count is None or rec is None or not re.search(r'(?:u8|s8|char|unsigned char|signed char)\s*\*', pointer_type):
                    continue  # no element-size guess for non-byte pointer addition
                count *= -1 if n['opcode'] == '-' else 1
                matches.append((rec, count))
            if len(set(matches)) == 1:
                record, offset = matches[0]
                fields = records[record]['fields']
                names = [name for name, spec in fields.items() if spec['offset_bits'] == offset*8]
                if len(names) == 1:
                    field = names[0]
        if not record or not field:
            continue
        domain = domains.get('records', {}).get(record, {})
        result[row['id']] = {'source_sha256': row['source_sha256'],
                             'expression_sha256': row['expression_sha256'],
                             'record': record, 'field': field, 'raw_offset_bytes': offset,
                             'domain': domain.get('domain', 'unknown') if domain.get('evidence') and domain.get('scope') == 'all_instances' else 'unknown',
                             'domain_hint': domain.get('domain', 'unknown'),
                             'evidence': [{'ast_manifest': str(manifest_path),
                                           'ast_sha256': sha(raw), 'producer': meta['producer']},
                                          domain.get('evidence', 'domain still unreviewed')],
                             'binding_method': 'clang_ast_primary_source_only'}
    return result, []


def scan(args):
    root = Path(args.root).resolve()
    scopes = args.scope or DEFAULT_SCOPES
    paths = set()
    for scope in scopes:
        base = (root / scope).resolve()
        if not base.is_relative_to(root) or not base.exists():
            raise ValueError(f'invalid or missing scope: {scope}')
        for path in ([base] if base.is_file() else base.rglob('*')):
            if path.is_file() and path.suffix in SUFFIXES and path.resolve().is_relative_to(root):
                paths.add(path)
    bindings = load(args.bindings) if args.bindings else {'accesses': {}}
    layouts, errors = {}, {}
    for abi in ('wii', 'native'):
        layouts[abi], errors[abi] = artifact(root, getattr(args, abi + '_layouts'), abi)
    rows, files, gaps = [], {}, []
    for path in sorted(paths):
        name, data = path.relative_to(root).as_posix(), path.read_bytes()
        files[name] = sha(data)
        found, parse_errors = scan_text(name, data)
        rows.extend(found)
        gaps.extend(dict(file=name, **entry) for entry in parse_errors)
    ast_errors, generated = [], {}
    domains = load(args.record_domains) if args.record_domains else {'records': {}}
    domain_errors = verify_pins(root, domains.get('files', {}))
    if args.record_domains and (domain_errors or not domains.get('files')):
        ast_errors.extend(domain_errors or ['domain map has no source pins'])
        domains = {'records': {}}
    for manifest in args.ast:
        derived, failures = ast_accesses(root, manifest, rows, layouts['wii'], domains)
        ast_errors.extend(failures)
        for key, value in derived.items():
            if key in generated and generated[key] != value:
                generated[key] = {'conflict': True}
            else:
                generated[key] = value
    for row in rows:
        supplied = bindings.get('accesses', {}).get(row['id'])
        auto = generated.get(row['id'])
        if auto:
            row['ast_binding'] = auto
        classify(row, supplied or auto, layouts, errors)
    used = {r['id'] for r in rows}
    unused = sorted(set(bindings.get('accesses', {})) - used)
    result = {'schema': SCHEMA, 'root': str(root), 'scopes': scopes,
              'tool_sha256': sha(Path(__file__).read_bytes()), 'files': files,
              'coverage': {'mode': 'lexical_candidates_with_explicit_bindings', 'compiler_executed': False,
                           'files': len(files), 'rows': len(rows), 'delimiter_gaps': len(gaps),
                           'unmatched_binding_ids': unused, 'semantic_completeness': False,
                           'limitations': ['macro expansion and implicit casts are not expanded',
                                           'type aliases, pointer provenance and field identities require pinned bindings',
                                           'balanced-delimiter recovery across conditional branches is advisory',
                                           'UNKNOWN rows and parser gaps prevent an all-clear conclusion']},
              'counts': dict(sorted(Counter(r['classification'] for r in rows).items())),
              'layout_errors': errors, 'ast_errors': ast_errors, 'parse_gaps': gaps, 'rows': rows}
    Path(args.out).write_text(json.dumps(result, indent=2) + '\n')
    if args.markdown:
        lines = ['# Offset audit', '', f'{len(files)} files; {len(rows)} candidates.', '',
                 'Source inventory only. UNKNOWN is not a safety verdict; this is not a complete semantic misread list.', '',
                 '| Classification | Count |', '| --- | ---: |']
        lines.extend(f'| {key} | {count} |' for key, count in result['counts'].items())
        lines.extend(['', '| ID | Location | Kind | Verdict | Wii/native bits |', '| --- | --- | --- | --- | --- |'])
        lines.extend(f"| {r['id']} | {r['file']}:{r['line']} | {r['kind']} | {r['classification']} | {r.get('wii_offset_bits', '?')}/{r.get('native_offset_bits', '?')} |" for r in rows)
        Path(args.markdown).write_text('\n'.join(lines) + '\n')
    print(json.dumps({'files': len(files), 'rows': len(rows), 'counts': result['counts'], 'out': args.out}))
    return 0


def import_dump(args):
    root = Path(args.root).resolve()
    files = {}
    for pin in args.pin:
        path = Path(pin) if Path(pin).is_absolute() else root / pin
        files[pin] = sha(path.read_bytes())
    raw = Path(args.import_layout_dump).read_bytes()
    producer = {'kind': 'clang_record_layout_dump', 'command': args.producer_command,
                'accuracy': args.accuracy, 'raw_sha256': sha(raw)}
    records = parse_layout_dump(raw.decode())
    if not records or any('size' not in r for r in records.values()):
        raise ValueError('empty/incomplete record-layout dump')
    result = {'schema': SCHEMA, 'abi': args.abi, 'producer': producer, 'files': files, 'records': records}
    Path(args.out).write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({'records': len(records), 'out': args.out}))
    return 0


def collect(args):
    """Explicit opt-in syntax-only probes; scan mode never enters this function."""
    root = Path(args.root).resolve()
    plan = load(args.collect_plan)
    destination = Path(args.out)
    destination.mkdir(parents=True, exist_ok=True)
    pins = plan.get('files', {})
    errors = verify_pins(root, pins)
    if not pins or errors:
        raise ValueError(f'collection plan needs matching source pins: {errors}')
    outputs = {}
    for abi in ('wii', 'native'):
        item = plan['probes'][abi]
        argv = item['command']
        if not isinstance(argv, list) or '-fsyntax-only' not in argv or not any('fdump-record-layouts' in x for x in argv):
            raise ValueError('explicit command must request syntax-only Clang record layouts')
        run = subprocess.run(argv, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=args.timeout)
        raw = run.stdout + b'\n' + run.stderr
        (destination / f'{abi}.txt').write_bytes(raw)
        if run.returncode:
            raise ValueError(f'{abi} probe failed: {run.returncode}; retained diagnostic dump')
        if verify_pins(root, pins):
            raise ValueError('inputs changed during collection')
        records = parse_layout_dump(raw.decode(errors='replace'))
        if not records or any('size' not in r for r in records.values()):
            raise ValueError(f'{abi}: empty/incomplete layout dump')
        result = {'schema': SCHEMA, 'abi': abi, 'producer': {'kind': 'clang_record_layout_dump',
                  'command': argv, 'accuracy': item['accuracy'], 'raw_sha256': sha(raw)},
                  'files': pins, 'records': records}
        compiler = shutil.which(argv[0])
        if compiler:
            executable = Path(compiler).resolve()
            result['producer']['executable'] = str(executable)
            result['producer']['executable_sha256'] = sha(executable.read_bytes())
        version = subprocess.run([argv[0], '--version'], cwd=root, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, timeout=args.timeout)
        result['producer']['version'] = (version.stdout + version.stderr).decode(errors='replace')
        target = destination / f'{abi}.json'
        target.write_text(json.dumps(result, indent=2) + '\n')
        outputs[abi] = str(target)
    print(json.dumps(outputs))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', default='.')
    parser.add_argument('--scope', action='append', help='repeatable source file/directory; default src/Game + include + libs')
    parser.add_argument('--out', required=True, help='JSON path, or output directory for --collect-plan')
    parser.add_argument('--markdown')
    parser.add_argument('--bindings')
    parser.add_argument('--ast', action='append', default=[], help='pinned primary-source Clang JSON AST manifest; no compiler run')
    parser.add_argument('--record-domains', help='pinned owner-reviewed record domains for AST-derived bindings')
    parser.add_argument('--wii-layouts')
    parser.add_argument('--native-layouts')
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument('--import-layout-dump')
    modes.add_argument('--collect-plan', help='EXPLICIT opt-in compiler commands; default scan never runs subprocesses')
    parser.add_argument('--abi', choices=('wii', 'native'))
    parser.add_argument('--producer-command', nargs='+')
    parser.add_argument('--accuracy', choices=('measured', 'approximation'), default='approximation')
    parser.add_argument('--pin', action='append', default=[])
    parser.add_argument('--timeout', type=int, default=120)
    args = parser.parse_args(argv)
    try:
        if args.import_layout_dump:
            if not args.abi or not args.producer_command or not args.pin:
                parser.error('dump import requires --abi, --producer-command and at least one --pin')
            return import_dump(args)
        return collect(args) if args.collect_plan else scan(args)
    except (OSError, ValueError, KeyError, TypeError, subprocess.TimeoutExpired) as error:
        print(f'offsetaudit: {error}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    sys.exit(main())
