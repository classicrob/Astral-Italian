#!/usr/bin/env python3
"""Read-only data-section triage using objdiff's actual scoring (no builds).

Examples:
  datadiff.py Game/NameObj/NameObjFactory --out /tmp/data
  datadiff.py --baseline report.json --base-root ../baseline-tree --out /tmp/data

classes.tsv describes symbols, sections.tsv owns the non-overlapping report-byte
budget. A symbol classified missing may already exist under another name or be
part of a larger allocation: positional candidates are evidence, not identity.
Raw integer arrays are review hints, never automatic struct-conversion advice.

objdiff 3.6.1: objdiff-cli/src/cmd/report.rs credits a whole data/BSS section only
at 100%. diff/data.rs compares named data symbols by bytes plus relocations;
BSS symbols by size. Section fallback compares bytes/relocations for data, and
(offset,size) lists for BSS. Consequently a correct BSS allocation can score zero
when the target metadata splits its interior into multiple anonymous symbols.
"""
import argparse
from collections import Counter
import csv
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

from datacontent import Elf


DATA_SECTIONS = {'.data', '.rodata', '.bss', '.sdata', '.sdata2', '.sbss', '.sbss2', '.ctors', '.dtors'}
FIELDS = ['unit', 'section', 'symbol', 'class', 'flags', 'target_size',
          'source_size', 'target_offset', 'source_offset', 'symbol_percent',
          'section_percent', 'candidate', 'raw_array_hint', 'source_path']


def data_symbols(elf):
    return [s for s in elf.symbols if s.size and s.kind not in (2, 3, 4)
            and elf.local(s) and elf.names[s.section].split('$')[0] in DATA_SECTIONS
            and not s.name.startswith('...')]


def changed(parts):
    return any(p.get('kind', 'DIFF_NONE') not in ('DIFF_NONE', 'DATA_DIFF_NONE', 0)
               for p in parts)


def relocations_differ(target, source, target_elf, source_elf):
    """ELF diagnostic for named symbols; JSON symbol diffs omit relocation rows.

    Scores still come exclusively from objdiff. This follows data.rs relocation
    equality for ordinary ELF sections; merged subsections remain advisory.
    """
    def within(elf, symbol):
        return {off - symbol.address: reloc for (section, off), reloc in elf.relocations.items()
                if section == symbol.section and symbol.address <= off < symbol.address + symbol.size}

    left, right = within(target_elf, target), within(source_elf, source)
    if left.keys() != right.keys():
        return True
    for off, l in left.items():
        r = right[off]
        if l.kind != r.kind:
            return True
        named = l.symbol.name == r.symbol.name and l.addend == r.addend
        if target_elf.local(l.symbol) and source_elf.local(r.symbol):
            if target_elf.names[l.symbol.section] != source_elf.names[r.symbol.section]:
                return True
            addressed = (l.symbol.address + l.addend == r.symbol.address + r.addend
                         if r.symbol.size == 0 and l.symbol.size != 0 else
                         l.symbol.address == r.symbol.address and l.addend == r.addend)
            if not (named or addressed):
                return True
        elif not named:
            return True
    return False


def classify(target, source, target_elf, source_elf, diff):
    """Primary cause plus independent observations; score always comes from objdiff."""
    if source is None:
        return 'missing', ['unpaired']
    flags = []
    if target_elf.names[target.section] != source_elf.names[source.section]:
        flags.append('section_wrong')
    if target.size != source.size:
        flags.append('size_differs')
    if target.address != source.address:
        flags.append('order_wrong')
    if target_elf.sections[target.section][1] != 8:
        if changed(diff.get('data_diff', [])):
            flags.append('bytes_differ')
        if changed(diff.get('reloc_diff', [])):
            flags.append('relocation_differs')
    for cause in ('section_wrong', 'size_differs', 'bytes_differ', 'relocation_differs', 'order_wrong'):
        if cause in flags:
            return cause, flags
    if diff.get('match_percent') == 100:
        return 'matched_symbol_in_unmatched_section', flags
    return 'unpaired_or_unresolved', flags


def positional_candidates(target, target_elf, source_elf, symbols):
    if source_elf is None:
        return ''
    section = target_elf.names[target.section]
    out = []
    for s in symbols:
        if source_elf.names[s.section] != section:
            continue
        if s.address == target.address and s.size == target.size:
            out.append(f'exact_extent:{s.name}')
        elif s.address < target.address + target.size and target.address < s.address + s.size:
            out.append(f'overlap:{s.name}@{s.address:#x}+{s.size:#x}')
    return ';'.join(out)


def write_tsv(path, rows, fields):
    with path.open('w', newline='') as f:
        writer = csv.DictWriter(f, fields, delimiter='\t', extrasaction='ignore')
        writer.writeheader()
        writer.writerows(rows)


def inspect_unit(root, base_root, unit, baseline, cli):
    target_path = root / unit['target_path']
    base_path = base_root / unit['base_path'] if unit.get('base_path') else None
    if base_path is not None and not base_path.is_file():
        raise FileNotFoundError(f'Configured source object absent: {base_path}')
    target = Elf(target_path)
    source = Elf(base_path) if base_path else None
    command = [str(cli), '-L', 'error', 'diff', '-1', str(target_path),
               '-o', '-', '-c', 'functionRelocDiffs=none',
               '-c', 'combineDataSections=true', '-c', 'combineTextSections=true']
    if base_path:
        command += ['-2', str(base_path)]
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    diff = json.loads(result.stdout)
    left = diff['left']
    ldiff = {s['name']: s for s in left['symbols']}
    source_symbols = data_symbols(source) if source else []
    source_by_name = {s.name: s for s in source_symbols}
    source_text_path = root / unit.get('metadata', {}).get('source_path', '__absent__')
    source_text = source_text_path.read_text(errors='replace') if source_text_path.is_file() else ''
    raw_arrays = set(re.findall(r'\b(?:u8|u32|s8|s32|UINT8|UINT32|unsigned\s+(?:char|int))\s+(\w+)\s*\[', source_text))
    baseline_sections = {s['name']: s for s in baseline.get('sections', [])}
    sections, rows, paired = [], [], set()
    lsections = {s['name']: s for s in left['sections'] if s.get('kind') in ('SECTION_DATA', 'SECTION_BSS')}
    for name, section in lsections.items():
        score = section.get('match_percent', 0)
        size = int(section['size'])
        original = baseline_sections.get(name, {})
        sections.append(dict(unit=unit['name'], section=name, target_size=size,
                             section_percent=score, matched_data=size if score == 100 else 0,
                             unmatched_data=0 if score == 100 else size,
                             baseline_percent=original.get('fuzzy_match_percent', 0),
                             baseline_size=original.get('size', ''),
                             source_object=str(base_path or ''),
                             target_object=str(target_path),
                             source_sha256=hashlib.sha256(source.data).hexdigest() if source else '',
                             target_sha256=hashlib.sha256(target.data).hexdigest()))
    for t in data_symbols(target):
        section = target.names[t.section]
        score_section = section if section in lsections else section.split('$')[0]
        score = lsections.get(score_section, {}).get('match_percent', 0)
        d = ldiff.get(t.name, {})
        # Protobuf filters zero-sized/ignored symbols but preserves unfiltered
        # target_symbol indices. Those indices cannot index the JSON symbol list.
        # Resolve exact names only; positional alternatives remain advisories.
        s = source_by_name.get(t.name)
        if s:
            paired.add(s.name)
        if score == 100:
            continue
        if s and relocations_differ(t, s, target, source):
            d = {**d, 'reloc_diff': [{'kind': 'DIFF_REPLACE'}]}
        cause, flags = classify(t, s, target, source, d)
        candidate = positional_candidates(t, target, source, source_symbols) if s is None else ''
        if s is None and candidate and target.sections[t.section][1] == 8:
            flags.append('bss_boundary_or_name_candidate')
        hints = sorted(n for n in raw_arrays if n == (s.name if s else '') or n == t.name
                       or (s and s.name.startswith(n + '__'))
                       or f':{n}@' in candidate or f':{n}__' in candidate)
        if hints:
            flags.append('raw_integer_array_review')
        rows.append(dict(unit=unit['name'], section=section, symbol=t.name, **{'class': cause},
                         flags=','.join(flags), target_size=t.size, source_size=s.size if s else '',
                         target_offset=hex(t.address), source_offset=hex(s.address) if s else '',
                         symbol_percent=d.get('match_percent', ''), section_percent=score,
                         candidate=candidate, raw_array_hint=','.join(hints),
                         source_path=unit.get('metadata', {}).get('source_path', '')))
    for s in source_symbols:
        section = source.names[s.section]
        score_section = section if section in lsections else section.split('$')[0]
        if s.name in paired or lsections.get(score_section, {}).get('match_percent') == 100:
            continue
        rows.append(dict(unit=unit['name'], section=section, symbol=s.name, **{'class': 'extra'},
                         source_size=s.size, target_size=0, source_offset=hex(s.address),
                         raw_array_hint=s.name if s.name in raw_arrays else '',
                         source_path=unit.get('metadata', {}).get('source_path', '')))
    return rows, sections


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('units', nargs='*', help='unit names or prefixes; default all baseline data misses')
    ap.add_argument('--root', type=Path, default=Path.cwd())
    ap.add_argument('--base-root', type=Path, help='read existing compiled objects from this tree')
    ap.add_argument('--baseline', type=Path, help='canonical report used for selection and score comparison')
    ap.add_argument('--out', type=Path, required=True)
    args = ap.parse_args()
    root = args.root.resolve()
    base_root = (args.base_root or root).resolve()
    config = json.loads((root / 'objdiff.json').read_text())
    report = json.loads(args.baseline.read_text()) if args.baseline else {'units': []}
    baseline = {u['name']: u for u in report['units']}
    prefixes = ['main/' + u.removeprefix('main/').removeprefix('src/').removesuffix('.cpp').removesuffix('.c') for u in args.units]
    units = [u for u in config['units'] if u.get('target_path') and
             (any(u['name'] == p or u['name'].startswith(p + '/') for p in prefixes) if prefixes else
              not baseline or int(baseline.get(u['name'], {}).get('measures', {}).get('total_data', 0)) >
              int(baseline.get(u['name'], {}).get('measures', {}).get('matched_data', 0)))]
    args.out.mkdir(parents=True, exist_ok=True)
    rows, sections, errors = [], [], []
    for i, unit in enumerate(units):
        try:
            r, s = inspect_unit(root, base_root, unit, baseline.get(unit['name'], {}), root / 'build/tools/objdiff-cli')
            rows.extend(r)
            sections.extend(s)
        except (OSError, ValueError, subprocess.CalledProcessError) as error:
            errors.append({'unit': unit['name'], 'error': str(error)})
        if (i + 1) % 100 == 0:
            print(f'{i + 1}/{len(units)} units, {len(errors)} errors', flush=True)
    write_tsv(args.out / 'classes.tsv', rows, FIELDS)
    write_tsv(args.out / 'sections.tsv', sections,
              ['unit', 'section', 'target_size', 'section_percent', 'matched_data', 'unmatched_data',
               'baseline_percent', 'baseline_size', 'source_object', 'source_sha256', 'target_object', 'target_sha256'])
    counts, sizes = Counter(), Counter()
    for row in rows:
        counts[row['class']] += 1
        sizes[row['class']] += int(row.get('target_size', 0))
    actual_matched, actual_total = Counter(), Counter()
    for s in sections:
        actual_matched[s['unit']] += s['matched_data']
        actual_total[s['unit']] += s['target_size']
    budget_changes = []
    for name in actual_total:
        if name not in baseline:
            continue
        m = baseline[name]['measures']
        if (actual_matched[name], actual_total[name]) != (int(m.get('matched_data', 0)), int(m.get('total_data', 0))):
            budget_changes.append(dict(unit=name, actual_matched=actual_matched[name],
                                       baseline_matched=int(m.get('matched_data', 0)),
                                       actual_total=actual_total[name], baseline_total=int(m.get('total_data', 0))))
    summary = dict(units_requested=len(units), units_inspected=len({s['unit'] for s in sections}),
                   matched_data=sum(s['matched_data'] for s in sections),
                   unmatched_data=sum(s['unmatched_data'] for s in sections),
                   class_symbol_counts=dict(counts), class_target_symbol_bytes=dict(sizes),
                   baseline_budget_changes=budget_changes,
                   section_score_changes=[{k: s[k] for k in ('unit', 'section', 'baseline_percent', 'section_percent')}
                                          for s in sections if baseline and s['baseline_percent'] != s['section_percent']],
                   errors=errors,
                   note='Class bytes describe symbol extents, not recoverable report bytes. Use sections.tsv for section budgets. '
                        'Candidate overlaps and raw-array hints need manual proof; no compile or target edits performed.')
    (args.out / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps({k: v for k, v in summary.items() if k != 'section_score_changes'}, indent=2))
    return bool(errors)


if __name__ == '__main__':
    sys.exit(main())
