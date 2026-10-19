#!/usr/bin/env python3
"""Scratch-only, single-object MWCC setting search. Findings require owner review.

Never invokes a build/configure, shell, or compiler against a production output.
Default guard preserves EVERY existing exact function; fuzzy drops are reported.
Use --guard all for the stronger all-fuzzy-nondecrease view. No production flags
or proof/checker rules are changed. Explicit --range spans must cover complete
namespace-scope function definitions; this lexical first pass fails closed.
"""
from __future__ import annotations

import argparse
import collections
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import struct
import subprocess
import sys
import time

VERSION = 1
TOOL_PATH = Path(__file__).resolve()
TOOL_SHA256 = hashlib.sha256(TOOL_PATH.read_bytes()).hexdigest()
UNSUPPORTED = re.compile(
    r'(?:unknown|unrecognized|illegal|invalid|unsupported)\s+(?:#\s*)?(?:pragma|option|argument|flag)'
    r'|(?:pragma|option|flag)[^\n]*(?:ignored|not supported|not recognized|illegal|invalid)'
    r'|command.line[^\n]*(?:warning|error)', re.I)


def require(ok, message):
    if not ok:
        raise ValueError(message)


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_name(path.name + '.tmp')
    temp.write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')
    temp.replace(path)


def norm_unit(name):
    name = name.removeprefix('main/').removeprefix('src/')
    return re.sub(r'\.(cpp|c)$', '', name)


def builtin_variants():
    """Names grounded in MWCC help / actual compiler strings / repo pragmas.

    Compiler acceptance is ALSO tested, with illegal-pragmas diagnostics enabled.
    No combinations/cross products of settings are generated.
    """
    out = []
    names = ['opt_propagation', 'opt_common_subs', 'opt_loop_invariants',
             'opt_strength_reduction', 'opt_dead_code', 'scheduling',
             'peephole', 'auto_inline', 'defer_codegen', 'fp_contract']
    for name in names:
        for value in ('off', 'on'):
            out.append({'name': name + '-' + value, 'argv': ['-pragma', name + ' ' + value],
                        'pragma': name + ' ' + value, 'evidence': 'MWCC embedded pragma name; diagnostic probe required'})
    for depth in (0, 4, 8):
        out.append({'name': 'inline-depth-' + str(depth), 'argv': ['-inline', 'level=' + str(depth)],
                    'pragma': 'inline_depth(' + str(depth) + ')', 'evidence': 'MWCC -help all: inline level=0..8'})
    for goal in ('p', 's'):
        out.append({'name': 'O4-' + goal, 'argv': ['-O4,' + goal], 'pragma': None,
                    'evidence': 'MWCC -help all: -O4,p / -O4,s; global only'})
    return out


def parse_edge(command):
    """Parse argv, never execute shell text. Accept only known postprocessors."""
    require(not any(x in command for x in ('`', '$(', '\n', '\r')), 'shell expansion/multiline edge refused')
    lex = shlex.shlex(command, posix=True, punctuation_chars=';&|<>()')
    lex.whitespace_split = True
    chunks, current = [], []
    for token in lex:
        if token == '&&':
            require(current, 'empty compiler edge segment')
            chunks.append(current)
            current = []
        else:
            require(token not in (';', '|', '||', '&', '>', '<', '>>', '(', ')'), 'shell operator refused: ' + token)
            current.append(token)
    require(current, 'empty compiler command')
    chunks.append(current)
    argv = chunks[0]
    compiler = [i for i, a in enumerate(argv) if Path(a).name.lower() == 'mwcceppc.exe']
    require(len(compiler) == 1, 'expected exactly one MWCC executable')
    idx = compiler[0]
    allowed = {'wibo', 'wibo.exe', 'wine', 'wine64', 'sjiswrap.exe'}
    require(all(Path(a).name in allowed for a in argv[:idx]), 'unrecognized compiler wrapper')
    require(argv.count('-c') == 1 and argv.count('-o') == 1, 'expected one -c and one -o')
    require(argv.index('-c') + 1 < len(argv) and argv.index('-o') + 1 < len(argv), 'missing source/output operand')
    for tail in chunks[1:]:
        transform = len(tail) == 4 and Path(tail[1]).as_posix().endswith('tools/transform_dep.py')
        extab = (len(tail) == 7 and Path(tail[0]).name == 'dtk' and tail[1:3] == ['extab', 'clean']
                 and tail[3] == '--padding' and tail[5] == tail[6])
        require(transform or extab, 'unrecognized postprocessor (refused): ' + repr(tail))
    return {'argv': argv, 'compiler_index': idx, 'tails': chunks[1:],
            'source': argv[argv.index('-c') + 1], 'output': argv[argv.index('-o') + 1]}


def repo_path(root, value):
    p = (root / value).resolve()
    require(p.is_relative_to(root), 'input outside repository refused: ' + str(p))
    return p


def include_dirs(edge):
    argv, out, i = edge['argv'], [], 0
    while i < len(argv):
        a = argv[i]
        if a == '-i':
            require(i + 1 < len(argv), 'missing include operand')
            out.append(argv[i + 1]); i += 2
        elif a.startswith('-i') and len(a) > 2 and not a.startswith(('-inline', '-ipa', '-iso', '-inst')):
            raise ValueError('joined include option unsupported; require separate -i operand: ' + a)
        else:
            require(a not in ('-include', '-prefix', '-precompile', '-pch'), 'preinclude/PCH requires dedicated parser')
            i += 1
    return out


def rewrite_edge(edge, root, snapshot, source, output_dir, extra=()):
    """Typed operand rewrite; no string substitution or shell interpolation."""
    argv = edge['argv']
    ci = edge['compiler_index']
    out, i = [], 0
    while i < len(argv):
        a = argv[i]
        if i <= ci:
            out.append(str(repo_path(root, a)))
            if i == ci: out.extend(['-i', str(snapshot / Path(edge['source']).parent)])
            i += 1
        elif a == '-c':
            out.extend(extra)
            # Explicit source-parent fallback retains quoted relative .inc resolution.
            out.extend(['-w', 'illpragmas,cmdline', '-c', str(source)])
            i += 2
        elif a == '-o':
            out.extend(['-o', str(output_dir)]); i += 2
        elif a == '-i':
            rel = repo_path(root, argv[i + 1]).relative_to(root)
            out.extend(['-i', str(snapshot / rel)]); i += 2
        else:
            require(not a.startswith('@'), 'response-file arguments require parser; refused')
            out.append(a); i += 1
    require(out.count('-MMD') == 1, 'only edges with explicit -MMD are supported')
    return out


def lexical_mask(text):
    """Preserve offsets/newlines while masking comments/literals/directives."""
    require(not re.search(r'\b(?:u8|u|U|L)?R"', text), 'raw strings require parser; range refused')
    out = list(text); i = 0
    while i < len(text):
        if text.startswith('//', i):
            j = text.find('\n', i)
            if j < 0: j = len(text)
        elif text.startswith('/*', i):
            k = text.find('*/', i + 2); require(k >= 0, 'unterminated comment')
            j = k + 2
        elif text[i] in ('"', "'"):
            quote, j = text[i], i + 1
            while j < len(text):
                if text[j] == '\\': j += 2
                elif text[j] == quote: j += 1; break
                else: j += 1
            require(j <= len(text) and text[j - 1] == quote, 'unterminated literal')
        else:
            i += 1; continue
        for k in range(i, j):
            if out[k] != '\n': out[k] = ' '
        i = j
    masked = ''.join(out)
    lines = masked.splitlines(keepends=True); offset = 0; continued = False
    for line in lines:
        directive = continued or line.lstrip().startswith('#')
        continued = directive and line.rstrip('\r\n ').endswith('\\')
        if directive:
            for k in range(offset, offset + len(line)):
                if out[k] != '\n': out[k] = ' '
        offset += len(line)
    return ''.join(out)


def function_spans(text):
    """Conservative namespace-scope lexical functions, no class/extern/AST guesses."""
    masked = lexical_mask(text); stack = []; boundary = 0; spans = []
    for at, char in enumerate(masked):
        if char == '{':
            prefix = masked[boundary:at]
            parent_scope = all(x[0] == 'namespace' for x in stack)
            if parent_scope and re.fullmatch(r'\s*(?:inline\s+)?namespace(?:\s+[A-Za-z_]\w*)?\s*', prefix):
                stack.append(('namespace', at, at)); boundary = at + 1
            elif parent_scope and '(' in prefix and ')' in prefix and not re.search(r'\b(class|struct|union|enum|extern|namespace)\b|[=\[\]]', prefix):
                # A function declaration prefix; reject dangling/incomplete parentheses.
                require(prefix.count('(') == prefix.count(')'), 'unbalanced function signature')
                start = boundary + len(prefix) - len(prefix.lstrip())
                stack.append(('function', start, at))
            else:
                stack.append(('other', at, at))
        elif char == '}':
            require(stack, 'unbalanced closing brace')
            kind, start, _ = stack.pop()
            if kind == 'function': spans.append((start, at + 1))
            if all(x[0] == 'namespace' for x in stack): boundary = at + 1
        elif char == ';' and all(x[0] == 'namespace' for x in stack):
            boundary = at + 1
    require(not stack, 'unbalanced braces / preprocessing requires parser')
    return masked, spans


def validate_range(text, line_start, line_end):
    lines = text.splitlines(keepends=True)
    require(1 <= line_start <= line_end <= len(lines), 'range outside source line extent')
    start, end = sum(map(len, lines[:line_start - 1])), sum(map(len, lines[:line_end]))
    masked, spans = function_spans(text)
    selected = [(a, b) for a, b in spans if a < end and b > start]
    require(selected and all(start <= a and b <= end for a, b in selected), 'range cuts a function or has no recognized function')
    rest = list(masked[start:end])
    for a, b in selected:
        rest[a - start:b - start] = ' ' * (b - a)
    require(not ''.join(rest).strip().strip(';').strip(), 'range includes non-function declarations/namespace boundaries')
    # Push/pop must both execute unconditionally, not inside preprocessing branches.
    depth = 0
    for idx, line in enumerate(lines, 1):
        if idx == line_start: require(depth == 0, 'range starts inside conditional directive')
        m = re.match(r'^\s*#\s*(if|ifdef|ifndef|endif)\b', line)
        if m:
            depth += -1 if m[1] == 'endif' else 1
            require(depth >= 0, 'unbalanced conditional')
        if idx == line_end: require(depth == 0, 'range ends inside conditional directive')
    return start, end, len(selected)


def place_pragma(text, pragma, placement, line_range=None):
    require(pragma is not None, 'this variant is command-wide only')
    if placement == 'file':
        masked = lexical_mask(text)
        first = next((i for i, c in enumerate(masked) if not c.isspace()), len(text))
        start = text.rfind('\n', 0, first) + 1
        # A header conditional wrapping the body cannot safely receive the final pop.
        prefix = text[:start]
        depth = sum(1 if m[1] != 'endif' else -1 for m in re.finditer(r'^\s*#\s*(if|ifdef|ifndef|endif)\b', prefix, re.M))
        require(depth == 0, 'conditional file preamble requires explicit safe range')
        end = len(text)
    else:
        require(placement == 'range' and line_range is not None, 'explicit --range required')
        start, end, _ = validate_range(text, *line_range)
    line_number = text.count('\n', 0, start) + 1
    next_number = text.count('\n', 0, end) + 1
    begin = '#pragma push\n#pragma ' + pragma + '\n#line ' + str(line_number) + '\n'
    finish = '\n#pragma pop\n#line ' + str(next_number) + '\n'
    return text[:start] + begin + text[start:end] + finish + text[end:]


def unsupported_diagnostics(text):
    return [line.strip() for line in text.splitlines() if UNSUPPORTED.search(line)]


def split_bases(path, unit):
    current = None; result = {}
    for line in Path(path).read_text().splitlines():
        if line and not line[0].isspace() and line.endswith(':'):
            current = norm_unit(line[:-1])
        m = re.match(r'\s*(\.\w+)\s+start:0x([\da-fA-F]+)', line)
        if current == norm_unit(unit) and m: result[m[1]] = int(m[2], 16)
    return result


def elf_symbols(path):
    """Keep duplicate LOCAL symbols; never collapse to a name-keyed dictionary."""
    data = Path(path).read_bytes(); h = struct.unpack_from('>16sHHIIIIIHHHHHH', data)
    require(h[0][:6] == b'\x7fELF\x01\x02', 'expected ELF32 big endian')
    sections = [struct.unpack_from('>IIIIIIIIII', data, h[6] + i * h[11]) for i in range(h[12])]
    def contents(i):
        s = sections[i]; return data[s[4]:s[4] + s[5]]
    def string(blob, at): return blob[at:blob.find(b'\0', at)].decode()
    names = contents(h[13]); section_names = [string(names, s[0]) for s in sections]
    result = []
    for i, sec in enumerate(sections):
        if sec[1] != 2: continue
        strings = contents(sec[6]); table = contents(i)
        for at in range(0, sec[5], sec[9]):
            n, a, z, info, _, owner = struct.unpack_from('>IIIBBH', table, at)
            if info & 15 == 2 and z and 0 < owner < len(sections):
                result.append({'name': string(strings, n), 'offset': a, 'size': z,
                               'section': section_names[owner], 'section_index': owner,
                               'binding': info >> 4})
    return result


def score_rows(diff, target_symbols, bases):
    by = collections.defaultdict(list)
    for s in target_symbols:
        by[(s['name'], s['offset'], s['size'])].append(s)
    used = collections.Counter(); occurrence = collections.Counter(); rows = {}
    right = diff.get('right', {}).get('symbols', [])
    for symbol in diff['left']['symbols']:
        if not symbol.get('instructions') or symbol['name'].startswith(('gap_', '...')): continue
        name = symbol['name']; offset = int(symbol.get('address', 0)); size = int(symbol.get('size', 0))
        group = (name, offset, size); matches = by[group]
        require(used[group] < len(matches), 'objdiff/target ELF function binding missing: ' + repr(group))
        source = matches[used[group]]; used[group] += 1
        require(source['section'] in bases, 'missing target split base: ' + source['section'])
        address = bases[source['section']] + source['offset']; occ = occurrence[(address, name)]; occurrence[(address, name)] += 1
        key = f'{address:08X}:{occ}:{name}'
        index = int(symbol.get('target_symbol', 0)); paired = right[index - 1] if 1 <= index <= len(right) else None
        percent = float(symbol.get('match_percent', 0)); source_size = int(paired.get('size', 0)) if paired else 0
        rows[key] = {'key': key, 'name': name, 'target_address': f'0x{address:08X}', 'occurrence': occ,
                     'target_bytes': size, 'target_offset': offset, 'source_bytes': source_size, 'fuzzy_percent': percent,
                     'exact': percent == 100.0 and paired is not None and source_size == size,
                     'paired_source_name': paired.get('name') if paired else None}
    return rows


def merge_full_report(report, diagnostics, unit_name):
    """Full-report function credit is authoritative; diffJSON stays diagnostic."""
    units = [u for u in report.get('units', []) if norm_unit(u['name']) == norm_unit(unit_name)]
    require(len(units) == 1, 'expected exactly one isolated full-report unit')
    unit = units[0]; by = collections.defaultdict(list)
    for key, row in diagnostics.items():
        by[(row['name'], row['target_offset'], row['target_bytes'])].append(key)
    used = collections.Counter(); result = {}
    for f in unit.get('functions', []):
        if f['name'].startswith(('gap_', '...')): continue
        group = (f['name'], int(f.get('address', 0)), int(f.get('size', 0)))
        require(used[group] < len(by[group]), 'full-report/diff function binding missing: ' + repr(group))
        key = by[group][used[group]]; used[group] += 1; row = diagnostics[key]
        virtual = f.get('metadata', {}).get('virtual_address')
        if virtual is not None:
            require(int(virtual) == int(row['target_address'], 16), 'full-report/split actual-address disagreement')
        percent = float(f.get('fuzzy_match_percent', 0))
        result[key] = {**row, 'diff_percent': row['fuzzy_percent'], 'diff_exact': row['exact'],
                       'fuzzy_percent': percent, 'exact': percent == 100.0,
                       'metric': 'isolated_full_report.fuzzy_match_percent'}
    require(result.keys() == diagnostics.keys(), 'full-report/diff function inventory differs')
    credited = sum(r['target_bytes'] for r in result.values() if r['exact'])
    require(credited == int(unit.get('measures', {}).get('matched_code', 0)),
            'sum of FULLREPORT100 function bytes differs from unit matched_code')
    return result


def source_inventory(path):
    counts = collections.Counter(); out = {}
    for s in elf_symbols(path):
        if s['name'].startswith('gap_'): continue
        occ = counts[s['name']]; counts[s['name']] += 1
        out[s['name'] + ':' + str(occ)] = s['size']
    return out


def compare_scores(baseline, candidate, baseline_inventory=None, inventory=None):
    missing = sorted(baseline.keys() - candidate.keys()); added = sorted(candidate.keys() - baseline.keys())
    anchors, fuzzy_drops, diff_drops, diff_anchor_losses, changes = [], [], [], [], []
    for key, old in baseline.items():
        new = candidate.get(key)
        if old['exact'] and (new is None or not new['exact']): anchors.append(key)
        if new is None or new['fuzzy_percent'] < old['fuzzy_percent']: fuzzy_drops.append(key)
        if new is None or new.get('diff_percent', 0) < old.get('diff_percent', 0): diff_drops.append(key)
        if old.get('diff_exact') and (new is None or not new.get('diff_exact')): diff_anchor_losses.append(key)
        if new != old: changes.append({'key': key, 'baseline': old, 'candidate': new})
    def totals(rows):
        return {'exact_target_bytes': sum(r['target_bytes'] for r in rows.values() if r['exact']),
                'fuzzy_target_bytes': sum(r['target_bytes'] * r['fuzzy_percent'] / 100 for r in rows.values())}
    a, b = totals(baseline), totals(candidate)
    bi, ci = baseline_inventory or {}, inventory or {}
    return {'baseline_totals': a, 'candidate_totals': b,
            'exact_target_bytes_gained': b['exact_target_bytes'] - a['exact_target_bytes'],
            'fuzzy_target_bytes_gained': b['fuzzy_target_bytes'] - a['fuzzy_target_bytes'],
            'exact_anchor_losses': anchors, 'fuzzy_regressions': fuzzy_drops,
            'diffJSON_fuzzy_regressions': diff_drops, 'diffJSON_exact_losses': diff_anchor_losses,
            'target_inventory_missing': missing, 'target_inventory_added': added,
            'source_inventory_removed': sorted(bi.keys() - ci.keys()), 'source_inventory_added': sorted(ci.keys() - bi.keys()),
            'source_size_changes': [{'key': k, 'baseline': bi[k], 'candidate': ci[k]} for k in bi.keys() & ci.keys() if bi[k] != ci[k]],
            'qualifies_exact_anchor_guard': not anchors and not missing and not added,
            'qualifies_all_fuzzy_guard': not anchors and not missing and not added and not fuzzy_drops,
            'per_function_changes': changes}


def run_process(argv, cwd, log, timeout=180):
    started = time.monotonic()
    try:
        result = subprocess.run(argv, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, errors='replace', timeout=timeout,
                                env={**os.environ, 'MG2_INTERACTIVE': '1'})
        output = result.stdout; code = result.returncode
    except subprocess.TimeoutExpired as exc:
        output = (exc.stdout or b'').decode(errors='replace') if isinstance(exc.stdout, bytes) else (exc.stdout or '')
        output += '\nflagsearch timeout\n'; code = -1
    Path(log).write_text(output)
    return {'argv': argv, 'returncode': code, 'log': str(log), 'elapsed_seconds': time.monotonic() - started,
            'unsupported_diagnostics': unsupported_diagnostics(output)}


def parse_dependencies(path, snapshot):
    text = Path(path).read_text(errors='strict')
    # MWCC writes one dependency per line, Windows Z: paths under Wine/wibo.
    lines = text.splitlines()
    require(lines and ': ' in lines[0], 'missing MMD target/source delimiter')
    first = lines[0].split(': ', 1)[1]
    joined = []
    for line in [first] + lines[1:]:
        value = line.strip().rstrip('\\').strip().replace('\\', '/')
        if re.match(r'^[zZ]:', value): value = value[2:]
        if value: joined.append(value)
    require(joined, 'empty/malformed MMD dependency list')
    out = {}
    for value in joined:
        f = Path(value)
        if not f.is_absolute(): f = snapshot / f
        f = f.resolve()
        # Variant TU itself may be outside the readonly snapshot, always scratch.
        require(f.is_file(), 'missing MMD input: ' + str(f))
        out[str(f)] = sha(f)
    return out


def snapshot_inputs(root, edges, config_paths):
    paths = set(config_paths)
    for edge in edges:
        roots = [root / 'src'] + [repo_path(root, x) for x in include_dirs(edge)]
        for folder in roots:
            require(folder.is_dir(), 'missing snapshot input directory: ' + str(folder))
            for f in folder.rglob('*'):
                if f.is_file():
                    if f.is_symlink(): require(f.resolve().is_relative_to(root), 'external symlink input refused: ' + str(f))
                    paths.add(f.relative_to(root).as_posix())
        for a in edge['argv'][:edge['compiler_index'] + 1]: paths.add(repo_path(root, a).relative_to(root).as_posix())
        for tail in edge['tails']:
            tool = tail[0] if Path(tail[0]).name == 'dtk' else tail[1]
            paths.add(repo_path(root, tool).relative_to(root).as_posix())
        base = Path(edge['output']) / (Path(edge['source']).stem + '.o')
        if (root / base).is_file(): paths.add(base.as_posix())
    return {n: sha(root / n) for n in sorted(paths)}


def materialize_snapshot(root, out, pins, fingerprint):
    snapshot = out / 'snapshots' / fingerprint
    manifest = snapshot / 'snapshot.json'
    if manifest.is_file():
        cached = json.loads(manifest.read_text())
        require(cached == pins, 'snapshot cache manifest mismatch')
        require(all((snapshot / n).is_file() and sha(snapshot / n) == h for n, h in pins.items()), 'snapshot cache content changed')
        return snapshot
    require(not snapshot.exists(), 'incomplete snapshot exists; remove scratch directory to retry: ' + str(snapshot))
    snapshot.mkdir(parents=True)
    for n, h in pins.items():
        source = root / n; target = snapshot / n; target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, target); require(sha(target) == h, 'source changed while snapshotting: ' + n)
        target.chmod(0o444)
    write_json(manifest, pins); manifest.chmod(0o444)
    return snapshot


def rewrite_tail(tail, root, object_path):
    # Depfile normalization uses our parser instead of rewriting/running Python.
    if len(tail) == 4: return None
    return [str(repo_path(root, tail[0])), 'extab', 'clean', '--padding', tail[4], str(object_path), str(object_path)]


def run_variant(root, snapshot, unit, edge, work, variant, placement, line_range, probe=False):
    work.mkdir(parents=True, exist_ok=True)
    output_dir = work / 'object'; output_dir.mkdir(exist_ok=True)
    relative_source = repo_path(root, edge['source']).relative_to(root)
    source = work / relative_source
    source.parent.mkdir(parents=True, exist_ok=True)
    original = (snapshot / edge['source']).read_bytes()
    if probe:
        text = 'int flagsearch_probe(int x) { return (x + 3) * 9; }\n'
        if placement != 'global' and variant:
            text = '#pragma push\n#pragma ' + variant['pragma'] + '\n' + text + '#pragma pop\n'
        source.write_text(text)
    elif variant and placement != 'global':
        text = original.decode('utf-8')
        source.write_text(place_pragma(text, variant['pragma'], placement, line_range))
    else:
        source.write_bytes(original)
    # Source basename is unchanged, so MWCC's directory -o gives the expected .o/.d.
    extra = variant['argv'] if variant and placement == 'global' else []
    source_hash = sha(source)
    source.chmod(0o444)
    command = rewrite_edge(edge, root, snapshot, source.relative_to(work), output_dir, extra)
    result = run_process(['nice', '-n', '10'] + command, work, work / 'compiler.log')
    require(sha(source) == source_hash, 'compiler mutated variant source')
    result.update(variant=variant['name'] if variant else 'baseline', placement=placement,
                  source_sha256=sha(source), scratch_source=str(source))
    if result['returncode'] or result['unsupported_diagnostics']:
        result['status'] = 'unsupported' if result['unsupported_diagnostics'] else 'compile_failed'
        return result
    obj = output_dir / (source.stem + '.o'); dep = output_dir / (source.stem + '.d')
    require(obj.is_file() and dep.is_file(), 'compiler output missing from scratch')
    result['MMD_dependencies_sha256'] = parse_dependencies(dep, work)
    require(all(Path(n).is_relative_to(snapshot) or Path(n) == source for n in result['MMD_dependencies_sha256']), 'compiler read outside snapshot/scratch source')
    if probe:
        result.update(status='probe_accepted', object_sha256=sha(obj)); return result
    for i, tail in enumerate(edge['tails']):
        argv = rewrite_tail(tail, root, obj)
        if argv:
            post = run_process(argv, snapshot, work / ('post-' + str(i) + '.log'))
            result.setdefault('postprocessors', []).append(post)
            if post['returncode']: result['status'] = 'postprocess_failed'; return result
    cfg = {'units': [{**unit, 'name': 'main/' + norm_unit(unit['name']),
                      'target_path': str(snapshot / unit['target_path']), 'base_path': str(obj)}]}
    write_json(work / 'objdiff.json', cfg)
    diff_path = work / 'diff.json'
    diff = run_process([str(root / 'build/tools/objdiff-cli'), '-L', 'error', 'diff', '-p', str(work),
                        '-u', 'main/' + norm_unit(unit['name']), '-o', str(diff_path)], snapshot, work / 'objdiff.log')
    result['objdiff_command'] = diff
    if diff['returncode']: result['status'] = 'objdiff_failed'; return result
    bases = split_bases(snapshot / 'config/SB4E01/splits.txt', unit['name'])
    diagnostics = score_rows(json.loads(diff_path.read_text()), elf_symbols(snapshot / unit['target_path']), bases)
    full_path = work / 'full-report.json'
    full = run_process([str(root / 'build/tools/objdiff-cli'), 'report', 'generate', '-p', str(work), '-o', str(full_path)], snapshot, work / 'full-report.log')
    result['full_report_command'] = full
    if full['returncode']: result['status'] = 'full_report_failed'; return result
    result['functions'] = merge_full_report(json.loads(full_path.read_text()), diagnostics, unit['name'])
    result['full_report_sha256'] = sha(full_path)
    result['source_inventory'] = source_inventory(obj)
    result.update(status='ok', object_sha256=sha(obj), scratch_object=str(obj), objdiff_sha256=sha(diff_path))
    return result


def unit_search(root, snapshot, unit, edge, out, fingerprint, variants, placements, line_range):
    folder = out / 'runs' / fingerprint / norm_unit(unit['name'])
    folder.mkdir(parents=True, exist_ok=True)
    def cached_run(label, variant, placement, probe=False):
        dest = folder / label; cache = dest / 'result.json'
        if cache.is_file():
            r = json.loads(cache.read_text())
            require(r.get('cache_fingerprint') == fingerprint, 'run cache fingerprint mismatch')
            if r.get('scratch_object'): require(sha(r['scratch_object']) == r['object_sha256'], 'cached object changed')
            if r.get('scratch_source'): require(sha(r['scratch_source']) == r['source_sha256'], 'cached source changed')
            return r
        try:
            r = run_variant(root, snapshot, unit, edge, dest, variant, placement, line_range, probe)
        except Exception as exc:
            r = {'status': 'rejected', 'error': str(exc), 'variant': label, 'placement': placement}
        r['cache_fingerprint'] = fingerprint; write_json(cache, r); return r
    baseline = cached_run('baseline', None, 'global')
    result = {'unit': unit['name'], 'baseline': baseline, 'variants': []}
    if baseline['status'] != 'ok': return result
    # Negative control must diagnose illegal pragmas; otherwise pragma support is untrusted.
    negative = {'name': 'negative-control', 'argv': ['-pragma', 'flagsearch_invalid_pragma_xyz on'],
                'pragma': 'flagsearch_invalid_pragma_xyz on'}
    neg = cached_run('negative-control', negative, 'global', True); result['negative_control'] = neg
    require(neg['status'] == 'unsupported', 'compiler failed to diagnose unknown pragma; refusing positive search')
    for placement in placements:
        for v in variants:
            if placement != 'global' and v['pragma'] is None:
                result['variants'].append({'variant': v['name'], 'placement': placement, 'status': 'unsupported_placement'})
                continue
            probe = cached_run('probe-' + placement + '-' + v['name'], v, placement, True)
            if probe['status'] != 'probe_accepted':
                result['variants'].append({**probe, 'status': 'unsupported_probe'}); continue
            row = cached_run(placement + '-' + v['name'], v, placement)
            if row['status'] == 'ok':
                row['comparison'] = compare_scores(baseline['functions'], row['functions'], baseline['source_inventory'], row['source_inventory'])
            result['variants'].append(row)
    write_json(folder / 'unit-result.json', result)
    return result


def make_report(results, guard, top, fingerprint):
    ranked = []
    for u in results:
        for v in u.get('variants', []):
            if 'comparison' not in v: continue
            c = v['comparison']; qualified = c['qualifies_exact_anchor_guard' if guard == 'anchors' else 'qualifies_all_fuzzy_guard']
            ranked.append({'unit': u['unit'], 'variant': v['variant'], 'placement': v['placement'], 'qualified': qualified, **c})
    ranked.sort(key=lambda c: (not c['qualified'], -c['exact_target_bytes_gained'], -c['fuzzy_target_bytes_gained'], c['unit'], c['variant']))
    lines = ['# Scratch MWCC setting search', '', 'Diagnostic only; original-boundary evidence and owner review required before production changes.', '',
             'Primary metric: fresh isolated FULLREPORT per-function fuzzy_match_percent=100 / matched_code. DiffJSON is secondary.', '',
             'Guard: ' + ('EXACT ANCHORS ONLY; fuzzy regressions are visible and may qualify.' if guard == 'anchors' else 'ALL FUZZY NONDECREASE plus exact anchors.'),
             'Snapshot: `' + fingerprint + '`', '', '| Unit | Setting / placement | Qualifies | Exact bytes Δ | Fuzzy bytes Δ | Exact losses | Fuzzy drops | Inventory changes |',
             '|---|---|---|---:|---:|---:|---:|---:|']
    for r in ranked[:top]:
        inventory = len(r['source_inventory_removed']) + len(r['source_inventory_added']) + len(r['target_inventory_missing']) + len(r['target_inventory_added'])
        lines.append(f"| {r['unit']} | {r['variant']} / {r['placement']} | {r['qualified']} | {r['exact_target_bytes_gained']:+d} | {r['fuzzy_target_bytes_gained']:+.3f} | {len(r['exact_anchor_losses'])} | {len(r['fuzzy_regressions'])} | {inventory} |")
    invalid = sum(v.get('status') != 'ok' for u in results for v in u.get('variants', []))
    lines += ['', f'Units: {len(results)}; scored variants: {len(ranked)}; unsupported/failed variants: {invalid}.',
              'Both full-report and diffJSON per-function scores, source inventories, commands, diagnostics, SHA256 and MMD inputs are in report.json and runs/.',
              'Not an equivalence certificate: data/reference preservation and native semantics remain owner obligations.', '']
    for unit in results:
        if unit.get('error') or unit.get('baseline', {}).get('status') != 'ok':
            lines.append('FAILED ' + unit['unit'] + ': ' + str(unit.get('error') or unit.get('baseline', {}).get('error') or unit.get('baseline', {}).get('status')))
    return ranked, '\n'.join(lines)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path.cwd())
    parser.add_argument('--unit', action='append', default=[])
    parser.add_argument('--units-file', type=Path)
    parser.add_argument('--out', type=Path)
    parser.add_argument('--top', type=int, default=20)
    parser.add_argument('--jobs', type=int, choices=(1, 2), default=1)
    parser.add_argument('--variant', action='append', help='individual built-in name; repeat to select (default all)')
    parser.add_argument('--placement', action='append', choices=('global', 'file', 'range'))
    parser.add_argument('--range', dest='line_range', help='inclusive source lines START:END, complete functions only')
    parser.add_argument('--guard', choices=('anchors', 'all'), default='anchors')
    parser.add_argument('--list-variants', action='store_true')
    args = parser.parse_args(argv)
    variants = builtin_variants()
    if args.list_variants:
        print(json.dumps(variants, indent=2)); return 0
    require(args.out is not None, '--out is required')
    root, out = args.root.resolve(), args.out.resolve()
    require(not out.is_relative_to(root), '--out must be outside checkout to protect primary reports/objects')
    units = args.unit[:]
    if args.units_file:
        units += [line.split('#', 1)[0].strip() for line in args.units_file.read_text().splitlines() if line.split('#', 1)[0].strip()]
    units = list(dict.fromkeys(norm_unit(u) for u in units)); require(units, '--unit or --units-file required')
    if args.variant:
        names = set(args.variant); require(names <= {v['name'] for v in variants}, 'unknown variant name')
        variants = [v for v in variants if v['name'] in names]
    line_range = None
    if args.line_range:
        m = re.fullmatch(r'(\d+):(\d+)', args.line_range); require(m, '--range must be START:END')
        line_range = tuple(map(int, m.groups()))
    placements = list(dict.fromkeys(args.placement or (['range'] if line_range else ['global'])))
    require('range' not in placements or line_range, '--placement range requires --range')
    require(args.top > 0, '--top must be positive')
    cfg = json.loads((root / 'objdiff.json').read_text()); catalog = {norm_unit(u['name']): u for u in cfg['units']}
    selected, edges = [], []
    for name in units:
        require(name in catalog, 'unit not configured: ' + name); u = catalog[name]
        require(u.get('base_path') and (root / u['target_path']).is_file(), 'existing target/compiler edge required: ' + name)
        command = subprocess.check_output(['ninja', '-t', 'commands', u['base_path']], cwd=root, text=True).splitlines()
        require(command, 'missing compiler edge: ' + name); edge = parse_edge(command[-1]); include_dirs(edge)
        require(Path(edge['source']).suffix in ('.cpp', '.c'), 'only C/C++ units supported')
        if line_range: validate_range((root / edge['source']).read_text(), *line_range)
        selected.append(u); edges.append(edge)
    out.mkdir(parents=True, exist_ok=True)
    config_paths = ['objdiff.json', 'build.ninja', 'configure.py', 'config/SB4E01/symbols.txt',
                    'config/SB4E01/splits.txt', 'orig/SB4E01/sys/main.dol', 'build/tools/objdiff-cli']
    config_paths += [u['target_path'] for u in selected]
    print('flagsearch: hashing readonly source/include/compiler inputs', file=sys.stderr, flush=True)
    pins = snapshot_inputs(root, edges, config_paths)
    print(f'flagsearch: {len(pins)} pinned inputs; preparing readonly snapshot', file=sys.stderr, flush=True)
    request = {'tool_version': VERSION, 'tool_sha256': TOOL_SHA256, 'root': str(root), 'units': selected,
               'edges': edges, 'variants': variants, 'placements': placements, 'range': line_range,
               'environment': {k: os.environ.get(k) for k in ('WINEPREFIX', 'MWCIncludes', 'MWLibraries', 'MWLibraryFiles')},
               'inputs_sha256': pins}
    fingerprint = digest(request); write_json(out / 'request.json', {**request, 'fingerprint': fingerprint})
    snapshot = materialize_snapshot(root, out, pins, digest(pins))
    jobs = list(zip(selected, edges))
    results = []
    def work(job): return unit_search(root, snapshot, job[0], job[1], out, fingerprint, variants, placements, line_range)
    # Per-unit workers, each sequential; never more than --jobs compiler processes.
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for job, future in zip(jobs, [pool.submit(work, job) for job in jobs]):
            try:
                result = future.result(); results.append(result)
                print('flagsearch: completed ' + job[0]['name'], file=sys.stderr, flush=True)
            except Exception as exc: results.append({'unit': job[0]['name'], 'error': str(exc), 'variants': []})
    require(sha(TOOL_PATH) == TOOL_SHA256, 'tool source changed during run; results invalid')
    changed = [n for n, h in pins.items() if not (root / n).is_file() or sha(root / n) != h]
    require(not changed, 'PRODUCTION INPUT CHANGED; result invalid: ' + repr(changed))
    # Also verify readonly snapshot files weren't mutated by a compiler.
    require(all(sha(snapshot / n) == h for n, h in pins.items()), 'snapshot mutated; results invalid')
    ranked, markdown = make_report(results, args.guard, args.top, fingerprint)
    report = {'scope': 'scratch-only diagnostic; no production adoption/proof waiver', 'fingerprint': fingerprint,
              'guard': args.guard, 'snapshot': str(snapshot), 'source_and_primary_objects_unchanged': True,
              'units': results, 'ranking': ranked}
    write_json(out / 'report.json', report); (out / 'REPORT.md').write_text(markdown)
    print(markdown)
    return 0 if all(u.get('baseline', {}).get('status') == 'ok' and not u.get('error') for u in results) else 1


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        print('flagsearch: ' + str(error), file=sys.stderr)
        raise SystemExit(2)
