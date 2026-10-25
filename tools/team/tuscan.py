#!/usr/bin/env python3
"""Read-only triage for original translation-unit boundaries inside merged units.

Uses existing ELF objects, original DOL bytes and a canonical report. Never
compiles, configures, splits or changes inputs. Signals are hypotheses, not
split/ownership proofs. Residue bytes are full affected nonexact functions,
not predicted gains. Detailed evidence and input hashes accompany the ranking.
"""
from __future__ import annotations

import argparse
from bisect import bisect_right
from collections import Counter, defaultdict
import csv
from dataclasses import replace
import hashlib
import json
from pathlib import Path
import re
import struct
import time

import dispatchcheck as dc
import splitscan as ss

TEAM = Path.home() / 'MacioGalaxy2-team'
REPORT = TEAM / 'main-report/d4d8efa99f51835cf00ad7b4d4ca4284253b55e7.json'
STRINGS = {'.data', '.rodata', '.sdata'}
POOLS = {'.sdata2', '.rodata'}


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def hx(address):
    return '' if address is None else f'0x{address:08X}'


def absolute(elf, symbol, catalog):
    if symbol.absolute is not None:
        return symbol.absolute
    if symbol.section in elf.origins:
        return elf.origins[symbol.section] + symbol.value
    known = catalog.named(symbol.name)
    return known.address if known else None


def function_id(elf, symbol, catalog):
    return symbol.name + '@' + hx(absolute(elf, symbol, catalog))


def by_name(functions):
    result = defaultdict(list)
    seen = set()
    for symbol in functions:
        key = symbol.name, symbol.section, symbol.value, symbol.size
        if key not in seen:
            seen.add(key); result[symbol.name].append(symbol)
    return result


def unique(mapping, name):
    values = mapping.get(name, [])
    return values[0] if len(values) == 1 else None


def words(elf, function):
    blob = elf.data[function.section]
    if blob is None or function.size % 4 or function.value + function.size > len(blob):
        return []
    return list(struct.unpack('>' + str(function.size // 4) + 'I', blob[function.value:function.value+function.size]))


def normalized(word):
    """Mask register numbers, retaining opcodes, widths and immediate offsets."""
    opcode = word >> 26
    if opcode in (*range(32, 56), 14, 15):
        return opcode, word & 0xffff, bool((word >> 16) & 31)
    if opcode == 31:
        return opcode, word & 0x7ff
    if opcode in (59, 63):
        return opcode, word & (0x3f if (word >> 1) & 31 in (18, 20, 21, 22, 23, 25, 28, 29, 30, 31) else 0x7ff)
    return word,


def inline_window(callee_words, caller_words):
    body = [word for word in callee_words if word != 0x60000000]
    if not body or body[-1] != 0x4e800020:
        return None
    body = body[:-1]
    if not 2 <= len(body) <= 16 or any(word >> 26 in (16, 18, 19) for word in body):
        return None
    # Stack-frame code is not a leaf expansion signature.
    if any(word >> 26 in (14, 32, 36, 37) and (word >> 16) & 31 == 1 for word in body):
        return None
    wanted = [normalized(word) for word in body]
    actual = [normalized(word) for word in caller_words]
    for start in range(len(actual) - len(wanted) + 1):
        if actual[start:start+len(wanted)] == wanted:
            return dict(offset=start*4, instructions=len(wanted), callee_words=[f'{w:08X}' for w in body],
                        caller_words=[f'{w:08X}' for w in caller_words[start:start+len(wanted)]])
    return None


def local_calls(elf, function, functions, catalog, target):
    locations = {(f.section, f.value): f for f in functions}
    names = by_name(functions)
    result = defaultdict(list)
    for index, word in enumerate(words(elf, function)):
        if word >> 26 != 18 or not word & 1:
            continue
        site = function.value + index*4
        rel = elf.relocations.get((function.section, site))
        callee = None
        if rel and rel[0] == 10:
            _, (name, value, _, section), addend = rel
            if 0 < section < len(elf.names):
                callee = locations.get((section, value+addend))
            if callee is None and addend == 0:
                callee = unique(names, name)
        elif rel is None:
            delta = ss.signed(word & 0x03fffffc, 26)
            destination = delta if word & 2 else site + delta
            callee = locations.get((function.section, destination))
        if callee is not None and callee != function:
            result[callee.name].append(site)
    return result


def call_signals(target, source, catalog):
    tf, sf = target.functions(), source.functions()
    tn, sn = by_name(tf), by_name(sf)
    observations = []
    for caller in tf:
        other = unique(sn, caller.name)
        if other is None:
            continue
        left = local_calls(target, caller, tf, catalog, True)
        right = local_calls(source, other, sf, catalog, False)
        for direction, calls, opposite in [('target_outline_source_inline', left, right),
                                            ('source_outline_target_inline', right, left)]:
            for name, sites in calls.items():
                if len(sites) <= len(opposite.get(name, [])):
                    continue
                callee = unique(tn, name); source_callee = unique(sn, name)
                if callee is None:
                    continue
                if direction.startswith('target'):
                    window = inline_window(words(target, callee), words(source, other))
                elif source_callee is not None:
                    window = inline_window(words(source, source_callee), words(target, caller))
                else:
                    window = None
                caller_address, callee_address = absolute(target, caller, catalog), absolute(target, callee, catalog)
                if caller_address is None or callee_address is None:
                    continue
                low = min(caller_address + caller.size, callee_address + callee.size)
                high = max(caller_address, callee_address)
                base = target.origins.get(caller.section, caller_address-caller.value)
                observations.append(dict(kind='call_inline', confidence='high' if window else 'weak',
                    functions=[function_id(target, caller, catalog)], addresses=[hx(base+site) for site in sites] if direction.startswith('target') else [hx(caller_address)],
                    boundary_low=hx(low), boundary_high=hx(high), direction=direction,
                    caller=caller.name, callee=name, callee_address=hx(callee_address),
                    target_calls=len(left.get(name, [])), source_calls=len(right.get(name, [])),
                    inline_body_window=window, note='Call-count difference alone does not prove inlining or a TU boundary.'))
    return observations


def text_signals(target, source, catalog):
    functions = sorted(target.functions(), key=lambda f: (f.section, f.value))
    sn = by_name(source.functions()); result = []
    for before, after in zip(functions, functions[1:]):
        if before.section != after.section:
            continue
        end = before.value + before.size; length = after.value - end
        gap = target.data[before.section][end:after.value]
        a, b = unique(sn, before.name), unique(sn, after.name)
        source_gap = b.value-(a.value+a.size) if a and b and a.section == b.section else None
        following = unique(sn, after.name)
        strong_after = following is not None and following.binding != 2
        sinit = before.name.startswith('__sinit_')
        if not (length >= 8 and gap and not any(gap)) and not (sinit and strong_after):
            continue
        if not sinit and (source_gap == length or not strong_after):
            continue
        address = absolute(target, after, catalog)
        if address is None:
            continue
        ctors = []
        if sinit:
            sinit_address = absolute(target, before, catalog)
            for (section, offset), rel in target.relocations.items():
                if target.names[section] != '.ctors' or rel[0] not in (1, 24):
                    continue
                _, (name, value, _, symsec), addend = rel
                destinations = {absolute(target, f, catalog) for f in functions if f.name == name}
                if symsec in target.origins:
                    destinations.add(target.origins[symsec] + value + addend)
                if sinit_address in destinations:
                    ctors.append(hx(target.origins.get(section, 0)+offset))
        result.append(dict(kind='mid_sinit' if sinit and strong_after else 'mid_text_padding',
            confidence='high' if sinit and strong_after and ctors and length >= 8 else 'medium' if sinit and strong_after else 'weak',
            functions=[function_id(target, before, catalog), function_id(target, after, catalog)],
            addresses=[hx(absolute(target, before, catalog)), hx(address-length), hx(address)],
            boundary_low=hx(address), boundary_high=hx(address), preceding_function=before.name,
            following_function=after.name, target_zero_padding=length if gap and not any(gap) else 0,
            source_padding=source_gap, ctor_slots=ctors,
            note='Ordinary function alignment is weak evidence; a middle initializer followed by a strong function is stronger.'))
    return result


def printable(blob, minimum=4):
    if not minimum <= len(blob) <= 512 or any(value < 32 or value == 127 for value in blob):
        return False
    try:
        return blob.decode('shift_jis').isprintable()
    except UnicodeError:
        return False


def raw_strings(data, minimum=4):
    result, offset = {}, 0
    for value in data.split(b'\0'):
        if printable(value, minimum) and offset+len(value) < len(data):
            result[offset] = value
        offset += len(value)+1
    return result


def cstring(read, address, minimum=4):
    data = read(address, 513)
    if not data:
        return None
    end = data.find(b'\0')
    value = data[:end] if end >= 0 else b''
    return value if printable(value, minimum) else None


def all_references(elf, catalog, target):
    refs, errors = [], []
    for function in elf.functions():
        try:
            observed, _ = ss.references(elf, function, catalog, target)
            refs.extend(observed)
        except ValueError as error:
            errors.append(dict(function=function.name, error=str(error)))
    return refs, errors


def string_inventory(elf, refs, catalog, target, minimum=4):
    strings, uses, containers = {}, defaultdict(set), defaultdict(list)
    functions = by_name(elf.functions())
    def read_source(section, offset, width):
        data = elf.data[section]
        return data[offset:offset+width] if data is not None and 0 <= offset < len(data) else None
    for section, name in enumerate(elf.names):
        if name not in STRINGS or elf.data[section] is None:
            continue
        origin = elf.origins.get(section) if target else 0
        if origin is None:
            continue
        for offset, value in raw_strings(elf.data[section], minimum).items():
            strings[name, origin+offset] = value
    for ref in refs:
        if ref.section not in STRINGS:
            continue
        if target:
            value = cstring(lambda a, n: ss.read_pool_dol(catalog.dol, a, n), ref.address, minimum)
        else:
            sections = [i for i, name in enumerate(elf.names) if name == ref.section]
            value = cstring(lambda a, n: read_source(sections[0], a, n), ref.address, minimum) if len(sections) == 1 else None
        if value is not None:
            key = ref.section, ref.address; strings[key] = value
            uses[key].add(ref.function)
    # A table's initializer relocations link literals to callers that reach it.
    for (section, offset), rel in elf.relocations.items():
        if elf.names[section] not in STRINGS or rel[0] not in (1, 24):
            continue
        value = ss.resolve(elf, rel, catalog, target)
        if value is None or value.section not in STRINGS:
            continue
        key = value.section, value.value
        if key not in strings:
            continue
        objects = [s for s in elf.symbols if s.kind == 1 and s.section == section and s.value <= offset < s.value+s.size]
        if not objects:
            continue
        container = min(objects, key=lambda s: s.size)
        start = absolute(elf, container, catalog) if target else container.value
        if start is None:
            continue
        containers[key].append(dict(name=container.name, address=hx(start) if target else start, size=container.size))
        for ref in refs:
            if ref.section == elf.names[section] and start <= ref.address < start+container.size:
                uses[key].add(ref.function)
    return strings, uses, containers


def duplicate_strings(target, source, catalog, target_refs, source_refs, minimum=4):
    left, left_uses, containers = string_inventory(target, target_refs, catalog, True, minimum)
    right, _, _ = string_inventory(source, source_refs, catalog, False, minimum)
    groups, source_groups = defaultdict(list), defaultdict(list)
    for key, value in left.items(): groups[value].append(key)
    for key, value in right.items(): source_groups[value].append(key)
    tn = by_name(target.functions()); result = []
    for value, copies in groups.items():
        ours = source_groups.get(value, [])
        if not ours or len(copies) <= len(ours):
            continue
        function_names = set().union(*(left_uses[key] for key in copies))
        ids = [function_id(target, f, catalog) for name in sorted(function_names) if (f := unique(tn, name))]
        result.append(dict(kind='duplicate_strings', confidence='medium' if ids else 'weak', functions=ids,
            addresses=[hx(address) for _, address in copies], literal=value.decode('shift_jis'),
            literal_hex=value.hex(), target_copies=len(copies), source_copies=len(ours),
            source_locations=[dict(section=section, offset=offset) for section, offset in ours],
            initializer_tables=[item for key in copies for item in containers[key]],
            note='Distinct target strings pooled by the source are a boundary clue, not proof; intentional separate objects also exist.'))
    return result


def shift_signals(target, source, catalog, target_refs, source_refs):
    tn, sn = by_name(target.functions()), by_name(source.functions())
    shifts = []
    for name, values in tn.items():
        a, b = unique(tn, name), unique(sn, name)
        if a is None or b is None or a.size != b.size:
            continue
        left = [replace(r, site=r.site-a.value) for r in target_refs if r.function == name]
        right = [replace(r, site=r.site-b.value) for r in source_refs if r.function == name]
        shifts.extend(ss.uniform_shifts(left, right))
    result = []
    for shift in shifts:
        if shift['section'] not in STRINGS | POOLS:
            continue
        ids = [function_id(target, f, catalog) for name in shift['functions'] if (f := unique(tn, name))]
        sites = []
        for sample in shift['samples']:
            function = unique(tn, sample['function'])
            if function:
                sites.append(hx(absolute(target, function, catalog)+sample['site']))
        result.append(dict(shift, kind='pool_offset_shift', confidence='medium', functions=ids, addresses=sites,
                           note='Uniform immediate shifts can also be table layout or metadata errors; seek an independent boundary signal.'))
    return result


def constant_restarts(target, source, catalog, target_refs, source_refs):
    def inventory(elf, refs, original):
        values = defaultdict(lambda: defaultdict(set))
        for ref in refs:
            if ref.section not in POOLS or ref.width not in (4, 8) or ref.address % ref.width:
                continue
            if original:
                data = ss.read_pool_dol(catalog.dol, ref.address, ref.width)
            else:
                sections = [i for i, name in enumerate(elf.names) if name == ref.section]
                if len(sections) != 1:
                    continue
                blob = elf.data[sections[0]]
                data = blob[ref.address:ref.address+ref.width] if blob is not None and ref.address >= 0 else None
            if data is not None and len(data) == ref.width:
                values[ref.section, data][ref.address].add(ref.function)
        return values
    left, right = inventory(target, target_refs, True), inventory(source, source_refs, False)
    names = by_name(target.functions()); result = []
    for (section, value), copies in left.items():
        if len(copies) < 2 or not right.get((section, value)) or len(copies) <= len(right[section, value]):
            continue
        ranges = []
        for address, functions in sorted(copies.items()):
            defined = [f for name in functions if (f := unique(names, name))]
            if defined:
                ranges.append((address, min(absolute(target, f, catalog) for f in defined),
                               max(absolute(target, f, catalog)+f.size for f in defined), functions))
        for first, second in zip(ranges, ranges[1:]):
            if first[2] > second[1] or first[3] & second[3]:
                continue
            ids = [function_id(target, f, catalog) for name in first[3] | second[3] if (f := unique(names, name))]
            result.append(dict(kind='constant_pool_restart', confidence='weak', functions=ids,
                addresses=[hx(first[0]), hx(second[0])], section=section, value_hex=value.hex(),
                boundary_low=hx(first[2]), boundary_high=hx(second[1]),
                target_copies=len(copies), source_copies=len(right[section, value]),
                note='Repeated scalar bytes with disjoint function-use ranges; not a proof of type, ownership or a TU boundary.'))
    return result


def merge_signals(unit, target, source, catalog, splits, target_refs, source_refs):
    name = ss.norm(unit['name']); owned = splits.units.get(name, {})
    tn, sn = by_name(target.functions()), by_name(source.functions())
    source_sites = {}
    for ref in source_refs:
        function = unique(sn, ref.function)
        if function:
            source_sites[ref.function, ref.site-function.value] = ref
    grouped = defaultdict(list)
    for ref in target_refs:
        if ref.section not in STRINGS | POOLS or ref.section not in owned or ref.immediate is None:
            continue
        low, high = owned[ref.section]
        if not low <= ref.address < high or low <= ref.anchor < high:
            continue
        preceding = [(a, b, owner) for a, b, owner in splits.ranges.get(ref.section, [])
                     if b == low and a <= ref.anchor < b and owner != name]
        if len(preceding) != 1:
            continue
        a, b, owner = preceding[0]
        if ref.anchor != a:
            continue
        function = unique(tn, ref.function)
        other = unique(sn, ref.function)
        if not function:
            continue
        counterpart = source_sites.get((ref.function, ref.site-function.value)) if other and other.size == function.size else None
        delta = counterpart.immediate-ref.immediate if counterpart and counterpart.immediate is not None else None
        grouped[owner, ref.section, a, b, delta].append(ref)
    result = []
    for (owner, section, begin, end, delta), refs in grouped.items():
        previous = splits.units.get(owner, {}).get('.text')
        current = owned.get('.text')
        adjacent_text = bool(previous and current and previous[1] == current[0])
        exact_shift = delta == -(end-begin)
        ids = [function_id(target, f, catalog) for name in sorted({r.function for r in refs}) if (f := unique(tn, name))]
        sites = [hx(absolute(target, unique(tn, r.function), catalog)+r.site-unique(tn, r.function).value) for r in refs]
        result.append(dict(kind='preceding_unit_pool_base', confidence='high' if exact_shift and adjacent_text else 'medium',
            functions=ids, addresses=sites+[hx(begin), hx(end)], proposed_partner='main/'+owner,
            section=section, preceding_pool_base=hx(begin), current_pool_start=hx(end),
            preceding_pool_size=end-begin, source_minus_target_immediate=delta,
            shift_equals_preceding_size=exact_shift, adjacent_text=adjacent_text,
            samples=[dict(function=r.function, site=hx(absolute(target, unique(tn, r.function), catalog)+r.site-unique(tn, r.function).value),
                          pool_anchor=hx(r.anchor), effective_address=hx(r.address), target_immediate=r.immediate) for r in refs[:16]],
            note='Possible over-split TU: target reaches current data from the preceding unit pool base. Shared globals remain an alternative until reviewed.'))
    return result


def inline_pins(path, report):
    if path is None or not path.is_file():
        return []
    text = path.read_text(errors='replace'); matches = []
    for number, line in enumerate(text.splitlines(), 1):
        if re.match(r'\s*#\s*pragma\s+(?:auto_inline|dont_inline|noinline|inline_depth|inline_max_size)\b', line) or re.match(r'\s*#\s*define\s+\w*(?:NOINLINE|OUTLINE)\w*\b', line):
            matches.append(dict(line=number, text=line.strip()))
    if not matches:
        return []
    index = report_index(report)
    affected = [key for key, rows in index.items() if any(float(row.get('fuzzy_match_percent') or 0) < 100 for row in rows)]
    return [dict(kind='inline_pin', confidence='weak', functions=affected,
                 addresses=[key.rsplit('@', 1)[1] for key in affected], source=str(path), source_sha256=sha(path), pins=matches,
                 note='Existing inlining/outline pins may compensate for a TU boundary; they also have legitimate independent causes. Do not remove without proof.')]


def report_index(unit):
    result = defaultdict(list)
    for function in unit.get('functions', []):
        address = function.get('metadata', {}).get('virtual_address')
        if address is not None:
            result[function['name'] + '@' + hx(int(address))].append(function)
    return result


def summarize_unit(name, observations, report):
    index = report_index(report)
    # Uncorroborated alignment/constant repetitions are retained as observations.
    kinds = {row['kind'] for row in observations}
    significant = any(row['kind'] == 'call_inline' and row['inline_body_window'] for row in observations)
    significant |= any(row['kind'] == 'mid_sinit' and row['confidence'] != 'weak' for row in observations)
    significant |= 'duplicate_strings' in kinds
    significant |= bool({'pool_offset_shift', 'preceding_unit_pool_base', 'inline_pin'} & kinds)
    if not significant:
        return None
    affected = set().union(*(set(row['functions']) for row in observations if row['kind'] != 'mid_text_padding'))
    functions = [dict(identity=key, **function) for key in sorted(affected) for function in index.get(key, [])]
    residue = [function for function in functions if float(function.get('fuzzy_match_percent') or 0) < 100]
    return dict(unit=name, affected_residue_bytes=sum(int(f['size']) for f in residue),
                affected_residue_functions=len(residue), affected_total_functions=len(functions),
                signature_count=len(kinds), signatures=sorted(kinds),
                high_confidence_signals=sum(row['confidence'] == 'high' for row in observations),
                inline_pin_count=sum(len(row.get('pins', [])) for row in observations),
                merge_partners=sorted({row['proposed_partner'] for row in observations if 'proposed_partner' in row}),
                evidence_addresses=sorted({address for row in observations for address in row.get('addresses', [])}),
                functions=functions, observations=observations,
                note='Ranked full-function residue, not a promised byte gain. Split ranges require owner/lander proof.')


def scan_unit(unit, report, target_path, source_path, catalog, splits=None, source_text=None):
    target, source = ss.ELF(target_path, True), ss.ELF(source_path)
    target_refs, target_errors = all_references(target, catalog, True)
    source_refs, source_errors = all_references(source, catalog, False)
    observations = call_signals(target, source, catalog) + text_signals(target, source, catalog)
    observations += duplicate_strings(target, source, catalog, target_refs, source_refs)
    observations += shift_signals(target, source, catalog, target_refs, source_refs)
    observations += constant_restarts(target, source, catalog, target_refs, source_refs)
    if splits is not None:
        observations += merge_signals(unit, target, source, catalog, splits, target_refs, source_refs)
    observations += inline_pins(source_text, report)
    candidate = summarize_unit(unit['name'], observations, report)
    provenance = dict(unit=unit['name'], target_path=str(target_path), source_path=str(source_path),
                      target_sha256=target.sha256, source_sha256=source.sha256,
                      target_functions=len(target.functions()), source_functions=len(source.functions()),
                      reference_errors=target_errors+source_errors, observations=len(observations))
    return candidate, observations, provenance


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path.cwd(), help='configured target/config worktree')
    parser.add_argument('--base-root', type=Path, default=Path.home() / 'MacioGalaxy2-wt/decomp3-land')
    parser.add_argument('--report', type=Path, default=REPORT)
    parser.add_argument('--out', type=Path, default=TEAM / 'scratch-decomp3/tuscan')
    parser.add_argument('--unit', action='append', default=[], help='only this unit; may be repeated')
    parser.add_argument('--residue-only', action='store_true')
    args = parser.parse_args()
    root, base, out = args.root.resolve(), args.base_root.resolve(), args.out.resolve()
    if out.exists() and any(out.iterdir()):
        parser.error('output directory must be empty')
    out.mkdir(parents=True, exist_ok=True)
    start = time.monotonic()
    paths = [args.report.resolve(), root/'objdiff.json', root/'config/SB4E01/symbols.txt',
             root/'config/SB4E01/splits.txt', root/'orig/SB4E01/sys/main.dol',
             Path(__file__).resolve(), Path(ss.__file__).resolve(), Path(dc.__file__).resolve()]
    pins = {str(path): sha(path) for path in paths}
    report = json.loads(args.report.read_text()); reports = {u['name']: u for u in report['units']}
    catalog = ss.Catalog((root/'config/SB4E01/symbols.txt').read_text())
    catalog.dol = (root/'orig/SB4E01/sys/main.dol').read_bytes()
    splits = ss.Splits((root/'config/SB4E01/splits.txt').read_text())
    for section, ranges in splits.ranges.items():
        if section in ss.DATA and ranges:
            low, high = min(r[0] for r in ranges), max(r[1] for r in ranges)
            old = catalog.bounds.get(section, (low, high)); catalog.bounds[section] = min(low, old[0]), max(high, old[1])
    units = json.loads((root/'objdiff.json').read_text())['units']
    wanted = {'main/'+ss.norm(name) for name in args.unit}
    units = [u for u in units if u['name'] in reports and (not wanted or u['name'] in wanted)]
    if args.residue_only:
        units = [u for u in units if any(float(f.get('fuzzy_match_percent') or 0) < 100 for f in reports[u['name']].get('functions', []))]
    candidates, skipped, provenance, errors = [], [], [], []
    with (out/'observations.jsonl').open('w') as observations:
        for number, unit in enumerate(units, 1):
            target = root / unit['target_path']; source = base / unit.get('base_path', '__no_source__')
            if not unit.get('base_path') or not source.is_file() or not target.is_file():
                skipped.append(dict(unit=unit['name'], reason='missing configured source or target object')); continue
            try:
                source_text = unit.get('metadata', {}).get('source_path')
                candidate, rows, inputs = scan_unit(unit, reports[unit['name']], target, source, catalog, splits,
                                                    root/source_text if source_text else None)
                provenance.append(inputs)
                for row in rows:
                    observations.write(json.dumps(dict(unit=unit['name'], **row))+'\n')
                observations.flush()
                if candidate:
                    candidates.append(candidate)
            except (ValueError, OSError, struct.error) as error:
                errors.append(dict(unit=unit['name'], error=str(error)))
            if number % 50 == 0:
                print(number, '/', len(units), 'candidates', len(candidates), flush=True)
    changed = [path for path, digest in pins.items() if sha(path) != digest]
    if changed:
        raise ValueError('pinned inputs changed: '+', '.join(changed))
    candidates.sort(key=lambda row: (-row['affected_residue_bytes'], -row['high_confidence_signals'], -row['signature_count'], row['unit']))
    columns = ['rank', 'unit', 'affected_residue_bytes', 'affected_residue_functions', 'affected_total_functions',
               'signature_count', 'high_confidence_signals', 'inline_pin_count', 'merge_partners', 'signatures', 'evidence_addresses']
    with (out/'candidates.tsv').open('w') as stream:
        writer = csv.DictWriter(stream, fieldnames=columns, delimiter='\t', lineterminator='\n', extrasaction='ignore'); writer.writeheader()
        for rank, row in enumerate(candidates, 1):
            row['rank'] = rank
            writer.writerow({**row, 'signatures': ','.join(row['signatures']), 'merge_partners': ','.join(row['merge_partners']),
                             'evidence_addresses': ','.join(row['evidence_addresses'])})
    (out/'candidates.json').write_text(json.dumps(candidates, indent=2)+'\n')
    metadata = dict(root=str(root), base_root=str(base), report=str(args.report.resolve()),
                    seconds=time.monotonic()-start, requested_units=len(units), scanned_units=len(provenance),
                    candidates=len(candidates), errors=errors, skipped=skipped, input_sha256=pins, object_provenance=provenance,
                    limitation='Source objects are live snapshots with per-object hashes; canonical report supplies residue weights. No source-object/report parity is assumed.')
    (out/'meta.json').write_text(json.dumps(metadata, indent=2)+'\n')
    lines = ['# TU-boundary candidates', '',
             f"{len(provenance)} existing object pairs scanned, {len(candidates)} candidate units, {len(errors)} object errors. No compiles or input edits.", '',
             'Residue bytes are full canonical nonexact functions touched by the signals, not predicted gains. '
             'Call absence alone is weak; leaf-body windows, duplicated strings, middle initializers and pool restarts can corroborate each other. '
             'Live source-object hashes are recorded; their parity with the canonical report is not assumed.', '',
             '| Rank | Unit | Affected residue B | Functions | Signatures |', '|---:|---|---:|---:|---|']
    for row in candidates[:30]:
        lines.append(f"| {row['rank']} | {row['unit']} | {row['affected_residue_bytes']} | {row['affected_residue_functions']} | {', '.join(row['signatures'])} |")
    (out/'summary.md').write_text('\n'.join(lines)+'\n')
    print(json.dumps(dict(scanned=len(provenance), candidates=len(candidates), errors=len(errors), seconds=metadata['seconds'])), flush=True)


if __name__ == '__main__':
    main()
