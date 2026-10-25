#!/usr/bin/env python3
"""Generate and score a bounded set of source variants implied by a mined rule.

ruletry.py UNIT FUNCTION --out DIR [--rule RULE] [--selector SIGNATURE]
Default: compile 5..20 candidate budget, serial under decomp-quiet.lock, whole-TU
report-parity scoring. --plan-only emits candidates without compilation. A short
typed --binding TYPE:EXPRESSION enables reference-bound predicates for fields
whose types cannot be inferred locally. No source changes are applied.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import csv
import difflib
import hashlib
import itertools
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time

import cfgtrial as c
import flagsearch
import fnsearch as f
import gametrial as g
import permute

RULES = Path.home() / 'MacioGalaxy2-team/scratch-decomp2/rules/v2/residue-ranked.tsv'
FAMILIES = {
    'gpr-lifetime': {'split_scalar_initialization', 'hoist_local_declaration', 'hoisted_declaration_order',
                     'comma_declaration_order', 'local_declaration_order', 'declaration_order_preserve_initializers',
                     'const_pointer_parameter', 'inline_integral_copy_temporary', 'commutative_integer_operands'},
    'aggregate-lifetime': {'vector_copy_initialization', 'hoist_local_declaration', 'hoisted_declaration_order',
                          'comma_declaration_order', 'local_declaration_order'},
    'scalar-scope': set(),
    'field-reference-reload': set(),
    'argument-binding': {'const_pointer_parameter', 'split_scalar_initialization', 'declaration_order_preserve_initializers'},
    'boolean-continuation': {'stage_builtin_bool', 'terminal_else_to_early_return',
                             'void_terminal_else_to_early_return', 'early_return_to_terminal_else'},
    'pool-layout': set(),
    'fabs-intrinsic': set(),
}


def rule_row(path, unit, function, explicit=None):
    with path.open() as stream:
        rows = [r for r in csv.DictReader(stream, delimiter='\t') if r['unit'] == unit]
    exact = [r for r in rows if r['function'] == function]
    found = exact or [r for r in rows if function in r['function']]
    if len(found) != 1:
        if explicit:
            return dict(unit=unit, function=function, rules=explicit, cluster='explicit', action='Explicit rule selection')
        raise ValueError('rule lookup requires one matching function; found ' + str(len(found)))
    row = found[0]
    if explicit:
        row = {**row, 'rules': explicit}
    return row


def helper_edit(source, span, start, end, replacement, declaration, label):
    return f.Edit(label, ((span.start, span.start, declaration + '\n\n'), (start, end, replacement)))


def fresh_name(source, base, reserved=()):
    name = base
    while name in reserved or re.search(r'\b' + re.escape(name) + r'\b', source):
        name += '_'
    return name


def local_types(source, span):
    text = flagsearch.lexical_mask(source[span.start:span.body_end])
    types = f.known_types(text)
    # Multiple bindings/shadowing and local types need a real C++ type resolver.
    if re.search(r'\b(?:typedef|using|class|struct|union|enum)\b', text):
        return {}
    counts = Counter()
    declaration = re.compile(r'\b(?:(?:const|volatile|register)\s+)?(?P<type>' + f.SCALAR + r'|' + f.IDENT
                             + r'(?:::' + f.IDENT + r')*)(?:\s+[*&]*\s*|\s*[*&]+\s*)(?P<name>' + f.IDENT + r')\b')
    for match in declaration.finditer(text):
        if match['type'] not in ('return', 'throw', 'delete', 'else', 'case', 'goto'):
            counts[match['name']] += 1
    return {name: kind for name, kind in types.items() if counts[name] == 1}


def scalar_edits(source, span, reserved):
    """Use f32 boundaries only for typed, side-effect-free binary operations."""
    types = local_types(source, span)
    names = {n for n, t in types.items() if t in ('f32', 'float')}
    body = flagsearch.lexical_mask(source[span.body_start+1:span.body_end])
    edits = []
    pattern = re.compile(r'\b(?P<a>[A-Za-z_]\w*)\s*(?P<op>[+*/-])\s*(?P<b>[A-Za-z_]\w*)\b')
    for match in pattern.finditer(body):
        if match['a'] not in names or match['b'] not in names:
            continue
        left, right = body[:match.start()].rstrip(), body[match.end():].lstrip()
        # Require the whole RHS/return or an explicitly grouped operation.
        whole = (left.endswith('return') or (left.endswith('=') and not left.endswith(('==', '!=', '<=', '>=')))) and right.startswith(';')
        grouped = left.endswith('(') and right.startswith(')')
        if not (whole or grouped):
            continue
        base = {'+': 'addScalars', '-': 'subtractScalars', '*': 'multiplyScalars', '/': 'divideScalars'}[match['op']]
        name = fresh_name(source, base, reserved)
        start = span.body_start + 1 + match.start(); end = span.body_start + 1 + match.end()
        for left_type, right_type in [('float', 'float'), ('const float&', 'float'), ('float', 'const float&'), ('const float&', 'const float&')]:
            for reverse in (False, True):
                parameters = [f'{left_type} left', f'{right_type} right']
                arguments = [match['a'], match['b']]
                if reverse:
                    parameters.reverse(); arguments.reverse()
                declaration = f"static inline float {name}({', '.join(parameters)}) {{ return left {match['op']} right; }}"
                edits.append(helper_edit(source, span, start, end, f"{name}({', '.join(arguments)})", declaration,
                                         'scalar-boundary-' + left_type.replace(' ', '_') + '-' + right_type.replace(' ', '_')
                                         + ('-reverse-binding' if reverse else '')))
    return edits


def consumer_scope_edits(source, span, vector_ok=False):
    """Shrink a nonescaping temporary around straight-line scalar consumers."""
    begin = span.body_start + 1
    body = source[begin:span.body_end]
    masked = flagsearch.lexical_mask(body)
    edits = []
    for declaration in f.DECL.finditer(body):
        kind, name = declaration['type'].strip(), declaration['name']
        if kind not in ('f32', 'float', 's32', 'u32', 'int', 'u16', 's16', 'u8', 's8', 'TVec3f'):
            continue
        if kind == 'TVec3f' and not vector_ok:
            continue
        if declaration['direct'] or declaration['array'] or declaration['pconst']:
            continue
        if re.search(r'\b' + re.escape(name) + r'\b', declaration['init'] or ''):
            continue
        uses = list(re.finditer(r'\b' + re.escape(name) + r'\b', masked[declaration.end():]))
        if not uses:
            continue
        last = declaration.end() + uses[-1].end()
        end = masked.find(';', last)
        if end < 0:
            continue
        end += 1
        fragment = masked[declaration.start():end]
        # Calls may retain references; declarations may export dependent locals.
        if re.search(r'[{}&]|\b(?:if|else|for|while|switch|return|break|continue|throw)\b|\b' + f.IDENT + r'\s*\(', fragment):
            continue
        if len(list(f.DECL.finditer(body[declaration.start():end]))) != 1:
            continue
        edits.append(f.Edit('temporary-consumer-scope', ((begin+declaration.start(), begin+declaration.start(), '{\n'),
                                                        (begin+end, begin+end, '\n}'))))
    return edits


def reference_edits(source, span, bindings, reserved):
    types = local_types(source, span)
    fields = {}
    for binding in bindings:
        kind, separator, expression = binding.partition(':')
        scalar = kind in ('bool', 'u8', 's8', 'u16', 's16', 'u32', 's32', 'int', 'unsigned', 'float', 'f32')
        pointer = re.fullmatch(r'[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*\*', kind)
        if not separator or not (scalar or pointer):
            raise ValueError('binding must be a known scalar TYPE:EXPRESSION')
        if not re.fullmatch(r'(?:this->)?[A-Za-z_]\w*(?:(?:->|\.)[A-Za-z_]\w*)*', expression):
            raise ValueError('binding expression must be a field/local path without calls')
        types[expression] = kind
        fields[expression] = kind
    body = flagsearch.lexical_mask(source[span.body_start+1:span.body_end])
    edits = []
    for expression, kind in types.items():
        if kind not in ('bool', 'u8', 's8', 'u16', 's16', 'u32', 's32', 'int', 'unsigned'):
            continue
        pattern = re.compile(r'(?<![\w.>])' + re.escape(expression) + r'\s*(==|!=|<=|>=|<|>)\s*(0|1)(?![\w.])')
        for match in pattern.finditer(body):
            name = fresh_name(source, 'test' + re.sub(r'\W', '', expression).capitalize(), reserved)
            start = span.body_start + 1 + match.start(); end = span.body_start + 1 + match.end()
            declaration = f'static inline bool {name}(const {kind}& value) {{ return value {match[1]} {match[2]}; }}'
            edits.append(helper_edit(source, span, start, end, name + '(' + expression + ')', declaration, 'const-reference-predicate'))
            for threshold_type in ('int', 'const int&'):
                for reverse in (False, True):
                    parameters = [f'const {kind}& value', threshold_type + ' threshold']
                    arguments = [expression, match[2]]
                    if reverse:
                        parameters.reverse(); arguments.reverse()
                    declaration = f"static inline bool {name}({', '.join(parameters)}) {{ return value {match[1]} threshold; }}"
                    edits.append(helper_edit(source, span, start, end, f"{name}({', '.join(arguments)})", declaration,
                                             'reference-predicate-threshold-' + threshold_type.replace(' ', '_')
                                             + ('-reverse-binding' if reverse else '')))
    for expression, kind in fields.items():
        for match in re.finditer(r'(?<![\w.>])' + re.escape(expression) + r'(?!\w)', body):
            left, right = body[:match.start()].rstrip(), body[match.end():].lstrip()
            value_context = (right.startswith(('?', '->')) or re.match(r'(?:==|!=|<=|>=|<|>)\s*[01](?![\w.])', right)
                             or re.search(r'\b(?:if|while)\s*\(\s*$', left)
                             or (kind.endswith('*') and left.endswith('*')))
            if not value_context:
                continue
            name = fresh_name(source, 'read' + re.sub(r'\W', '', expression).capitalize(), reserved)
            start = span.body_start + 1 + match.start(); end = span.body_start + 1 + match.end()
            for return_kind in (kind, kind + ' const&'):
                declaration = f'static inline {return_kind} {name}({kind} const& value) {{ return value; }}'
                edits.append(helper_edit(source, span, start, end, name + '(' + expression + ')', declaration,
                                         'reference-accessor-' + ('reference' if return_kind.endswith('&') else 'value')))
    return edits


def generate(source, span, rule, budget=20, vector_ok=False, bindings=(), macro_names=(), reserved=()):
    if rule not in FAMILIES:
        raise ValueError('no implemented recipe for rule: ' + rule)
    # Reuse the established effects/macro/scope checks even for new recipes.
    existing = f.generate_edits(source, span, vector_ok, set(macro_names), set(reserved))
    edits = [e for e in existing if e.name in FAMILIES[rule]]
    body = flagsearch.lexical_mask(source[span.start:span.end])
    has_macro = bool(set(re.findall(f.IDENT, body)) & set(macro_names))
    if rule == 'scalar-scope' and not has_macro:
        edits += scalar_edits(source, span, reserved)
    if rule in ('scalar-scope', 'gpr-lifetime', 'aggregate-lifetime') and not has_macro:
        edits += consumer_scope_edits(source, span, vector_ok)
    if rule in ('field-reference-reload', 'argument-binding') and not has_macro:
        edits += reference_edits(source, span, bindings, reserved)
    if rule == 'gpr-lifetime':
        edits = [e for e in edits if not any('TVec3f' in text for _, _, text in e.replacements)]
    # Round-robin across the applicable recipes before composing disjoint edits.
    families = defaultdict(list)
    for edit in edits:
        families[edit.name].append(edit)
    ordered = [e for group in itertools.zip_longest(*families.values()) for e in group if e]
    result, seen = [], {hashlib.sha256(source.encode()).hexdigest()}
    def append(edit):
        text = edit.apply(source); digest = hashlib.sha256(text.encode()).hexdigest()
        if digest not in seen:
            seen.add(digest); result.append((edit.name, text))
    for edit in ordered:
        append(edit)
        if len(result) >= budget:
            return result
    for left, right in itertools.combinations(ordered[:40], 2):
        # Helper insertions share an offset and must never be naively composed.
        if any(max(a, d) <= min(b, e) for a, b, _ in left.replacements for d, e, _ in right.replacements):
            continue
        append(f.Edit(left.name + '+' + right.name, left.replacements + right.replacements))
        if len(result) >= budget:
            break
    return result


def full_report(search, row):
    project = Path(row['work']) / '.fnsearch-object/project'
    result = f.run([str(search.root / 'build/tools/objdiff-cli'), '-L', 'error', 'report', 'generate',
                    '-c', 'functionRelocDiffs=none', '-p', str(project), '-o', '-'], search.root, 60)
    if result['returncode']:
        raise ValueError('whole-unit report failed: ' + result['stderr'])
    return c.report_scores(json.loads(result['stdout']))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('unit'); parser.add_argument('function')
    parser.add_argument('--root', type=Path, default=Path.cwd())
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--rules-file', type=Path, default=RULES)
    parser.add_argument('--rule', choices=sorted(FAMILIES))
    parser.add_argument('--trial-compiler', choices=['Wii/1.3'], help='compile variants with 1.3, preserving configured baseline compiler')
    parser.add_argument('--selector'); parser.add_argument('--binding', action='append', default=[])
    parser.add_argument('--budget', type=int, default=20)
    parser.add_argument('--plan-only', action='store_true')
    args = parser.parse_args()
    if not 5 <= args.budget <= 20:
        parser.error('budget must be 5..20')
    root, out = args.root.resolve(), args.out.resolve()
    if out.exists() and any(out.iterdir()):
        parser.error('output directory must be empty')
    out.mkdir(parents=True, exist_ok=True)
    name = flagsearch.norm_unit(args.unit)
    row = rule_row(args.rules_file, name, args.function, args.rule)
    rule = row['rules'].split(',')[0]
    if rule not in FAMILIES:
        parser.error('rule has no implemented recipe: ' + rule)
    unit = next(u for u in json.loads((root / 'objdiff.json').read_text())['units'] if u['name'] == 'main/' + name)
    if not unit.get('base_path'):
        parser.error('unit has no source compile edge')
    edge = f.compiler_edge(root, unit['base_path'])
    relative = Path(edge['source']); source = (root / relative).read_text()
    pins = {str(root / relative): g.sha(root / relative), str(root / 'build.ninja'): g.sha(root / 'build.ninja'),
            str(root / 'objdiff.json'): g.sha(root / 'objdiff.json'), str(args.rules_file.resolve()): g.sha(args.rules_file)}
    for tool in edge['argv'][:edge['compiler_index']+1]:
        path = (root / tool).resolve(); pins[str(path)] = g.sha(path)
    if args.trial_compiler:
        path = (root / 'build/compilers' / args.trial_compiler / 'mwcceppc.exe').resolve()
        pins[str(path)] = g.sha(path)
    score_rows, best, before, symbol = [], None, None, row['function']
    with tempfile.TemporaryDirectory(prefix='rt-', dir='/tmp') as directory:
        work = Path(directory); target = work / 'target.o'
        shutil.copyfile(root / unit['target_path'], target)
        pins[str((root / unit['target_path']).resolve())] = g.sha(target)
        locked_edge = {**edge, 'argv': ['lockf', '-k', str(c.QUIET_LOCK), *edge['argv']]}
        search = f.Search(root, unit, locked_edge, work, target, 180, time.monotonic()+1800)
        macros, reserved = set(), set()
        if args.plan_only:
            selector = args.selector or args.function
            span = f.source_span(source, dict(name=selector, demangled_name=selector), args.selector)
        else:
            baseline, data = search.compile(0, source, 'baseline')
            if data is None:
                search.discard(baseline)
                raise ValueError('baseline failed: ' + baseline.get('diagnostic', baseline['status']))
            selected = f.selected_symbol(data, symbol); symbol = selected['name']
            right = next((s for s in data.get('right', {}).get('symbols', []) if s['name'] == symbol), None)
            span = f.source_span(source, selected, args.selector, right)
            before = full_report(search, baseline)
            pins.update(f.dependency_hashes(root, baseline, relative))
            for dependency in pins:
                path = Path(dependency)
                if path.suffix not in ('.h', '.hpp', '.inc', '.cpp', '.c'):
                    continue
                content = path.read_text(errors='replace')
                macros.update(re.findall(r'^\s*#\s*define\s+(\w+)', content, re.M))
                reserved.update(re.findall(f.IDENT, content))
            baseline.update(scores=before); search.discard(baseline); score_rows.append(baseline)
        proposed = generate(source, span, rule, args.budget, f.vector_header_ok(root), args.binding, macros, reserved)
        if args.trial_compiler and not args.plan_only:
            trial_edge = {**edge, 'argv': g.compiler_argv(edge, args.trial_compiler, Path(edge['output']))}
            search.edge = {**trial_edge, 'argv': ['lockf', '-k', str(c.QUIET_LOCK), *trial_edge['argv']]}
            proposed = [('compiler-control', source), *proposed]
        for index, (label, text) in enumerate(proposed, 1):
            (out / f'{index:02d}.cpp').write_text(text)
            patch = ''.join(difflib.unified_diff(source.splitlines(keepends=True), text.splitlines(keepends=True),
                                               fromfile='a/' + str(relative), tofile='b/' + str(relative)))
            (out / f'{index:02d}.patch').write_text(patch)
            if args.plan_only:
                score_rows.append(dict(index=index, transform=label, status='plan_only')); continue
            candidate, _ = search.compile(index, text, label)
            if candidate['status'] == 'scored':
                try:
                    scores = full_report(search, candidate); comparison = c.compare(before, scores)
                    selected_before = [v['score'] for v in before['functions'].values() if v['name'] == symbol]
                    selected_after = [v['score'] for v in scores['functions'].values() if v['name'] == symbol]
                    if len(selected_before) != 1 or len(selected_after) != 1:
                        raise ValueError('selected function is not unique in report')
                    candidate.update(scores=scores, comparison=comparison,
                                     before=selected_before[0], after=selected_after[0])
                    unpin = bool(args.trial_compiler and comparison['zero_drops'] and selected_after[0] >= selected_before[0])
                    qualifies = (comparison['strict_gain'] and selected_after[0] > selected_before[0]) or unpin
                    candidate['status'] = 'unpin_candidate' if unpin else 'survivor' if qualifies else 'regressed' if not comparison['zero_drops'] else 'no_gain'
                    if qualifies and (best is None or (candidate['after'], comparison['matched_code_gain']) > (best['after'], best['comparison']['matched_code_gain'])):
                        best = candidate
                        (out / 'best.cpp').write_text(text); (out / 'best.patch').write_text(patch)
                except ValueError as error:
                    candidate.update(status='report_failed', diagnostic=str(error))
            search.discard(candidate); candidate.pop('functions', None); score_rows.append(candidate)
            print(index, label, candidate['status'], candidate.get('before', ''), candidate.get('after', ''), flush=True)
        changed = [path for path, digest in pins.items() if not Path(path).is_file() or g.sha(path) != digest]
        if changed:
            raise ValueError('inputs changed: ' + ', '.join(changed))
    result = dict(unit=unit['name'], function=symbol, rule=rule, mined_evidence=row, variants=len(proposed), trial_compiler=args.trial_compiler,
                  requested_budget=args.budget, best_index=best['index'] if best else None,
                  scoring_config={'functionRelocDiffs': 'none'}, input_sha256=pins, rows=score_rows,
                  limitation='Candidates require semantic/dispatch/native review by the matcher; nothing is applied. '
                             'Fewer candidates means fewer supported rule-directed edits; no filler is generated.')
    g.write_json(out / 'result.json', result)
    (out / 'usage.txt').write_text('Review numbered patches and result.json. best.patch requires a selected-function gain or a no-drop compiler-unpin candidate. Dispatch, semantics and native review remain required. No production source was modified.\n')
    print(json.dumps(dict(rule=rule, variants=len(proposed), best_index=result['best_index'])), flush=True)


if __name__ == '__main__':
    main()
