#!/usr/bin/env python3
"""Read-only whole-binary template-destructor and crossed group-base census.

Body matches nominate identities; template arguments and original call bindings
still require per-unit source experiments. Never compiles or changes inputs.
"""
from __future__ import annotations
import argparse
from collections import defaultdict
import csv
import json
from pathlib import Path
import struct
import time

import dispatchcheck as dc
import splitscan as ss
import tuscan as ts


def signature(elf, function):
    words = ts.words(elf, function)
    if not words or function.size < 32 or words[-1] != 0x4e800020:
        return None
    relocations = []
    for (section, offset), (kind, symbol, addend) in elf.relocations.items():
        if section != function.section or not function.value <= offset < function.value+function.size:
            continue
        site = (offset-function.value) & ~3
        mask = dc.MASK.get(kind)
        if mask is None:
            return None
        words[site//4] &= ~mask
        relocations.append((site, kind, symbol[0], addend))
    shape = tuple(words), tuple((site, kind) for site, kind, _, _ in sorted(relocations))
    return shape, sorted(relocations)


def original_body_equal(elf, function, address, dol):
    raw = dol.bytes(address, function.size)
    if raw is None:
        return False
    right = list(struct.unpack('>'+str(function.size//4)+'I', raw))
    left = ts.words(elf, function)
    if not left:
        return False
    for (section, offset), (kind, _, _) in elf.relocations.items():
        if section != function.section or not function.value <= offset < function.value+function.size:
            continue
        mask = dc.MASK.get(kind)
        if mask is None:
            return False
        index = (offset-function.value)//4
        left[index] &= ~mask; right[index] &= ~mask
    return left == right


def destructor_scaffold(elf, function):
    words = ts.words(elf, function)
    if len(words) < 8 or words[-1] != 0x4e800020 or 0x2c030000 not in words[:8]:
        return False
    # Only a triage fallback: a null-tested receiver plus deleting epilogue.
    delete = any(section == function.section and function.value <= offset < function.value+function.size
                 and kind == 10 and symbol[0] in ('__dl__FPv','__dla__FPv')
                 for (section, offset), (kind, symbol, _) in elf.relocations.items())
    return delete and any(word in (0x7fe3fb78, 0x7fc3f378, 0x7fa3eb78) for word in words[-10:])


def census(root, base, out):
    started = time.monotonic()
    pinned = [root/'objdiff.json',root/'config/SB4E01/symbols.txt',root/'config/SB4E01/splits.txt',
              root/'orig/SB4E01/sys/main.dol',Path(__file__),Path(ss.__file__),Path(ts.__file__),Path(dc.__file__)]
    hashes = {str(path):ts.sha(path) for path in pinned}
    units = json.loads((root/'objdiff.json').read_text())['units']
    catalog = ss.Catalog((root/'config/SB4E01/symbols.txt').read_text())
    dol = dc.Dol(root/'orig/SB4E01/sys/main.dol')
    templates, opaque, crossed, base_crossed, scaffolds, errors, missing, inputs = defaultdict(list), [], [], [], [], [], [], []
    names_at = defaultdict(list)
    for symbols in catalog.sections.values():
        for symbol in symbols: names_at[symbol.address].append(symbol.name)
    target_count = source_count = function_count = opaque_count = template_count = group_calls = 0
    for unit in units:
        for original, path in [(True, root/unit['target_path']), (False, base/unit.get('base_path', '__missing__'))]:
            if not path.is_file():
                missing.append(dict(unit=unit['name'], target=original, path=str(path))); continue
            try:
                elf = ss.ELF(path, original)
            except (ValueError, OSError, struct.error) as error:
                errors.append(dict(unit=unit['name'], path=str(path), error=str(error))); continue
            inputs.append(dict(unit=unit['name'], target=original, path=str(path), sha256=elf.sha256))
            if original: target_count += 1
            else: source_count += 1
            for function in elf.functions():
                if original:
                    function_count += 1
                    if not function.name.startswith('fn_'):
                        continue
                    opaque_count += 1
                    if destructor_scaffold(elf, function):
                        scaffolds.append(dict(unit=unit['name'],function=function.name,size=function.size,
                            address=ts.hx(ts.absolute(elf,function,catalog)),confidence='destructor_scaffold',
                            calls=[symbol[0] for (section,offset),(kind,symbol,_) in elf.relocations.items()
                                   if kind==10 and section==function.section and function.value<=offset<function.value+function.size]))
                    sig = signature(elf, function)
                    if sig:
                        opaque.append((sig, dict(unit=unit['name'], function=function.name, size=function.size,
                            address=ts.hx(ts.absolute(elf, function, catalog)), target_object=str(path))))
                    continue
                if not function.name.startswith('__dt__'):
                    continue
                if '<' in function.name:
                    sig = signature(elf, function)
                    if sig:
                        template_count += 1
                        templates[sig[0]].append(dict(unit=unit['name'], symbol=function.name, size=function.size,
                            relocations=sig[1], source_object=str(path)))
                address = ts.absolute(elf, function, catalog)
                for (section, offset), (kind, symbol, addend) in elf.relocations.items():
                    if (section != function.section or not function.value <= offset < function.value+function.size
                        or kind != 10 or not symbol[0].startswith('__dt__')):
                        continue
                    group = symbol[0] == '__dt__14LiveActorGroupFv'
                    group_calls += group
                    known = catalog.named(symbol[0])
                    if known is None: continue
                    if address is None:
                        row = dict(unit=unit['name'], caller=function.name, confidence='unanchored',
                                   site='', actual='', offset=offset-function.value, expected_symbol=symbol[0])
                        base_crossed.append(row)
                        if group: crossed.append(row)
                        continue
                    site = address+offset-function.value; word = dol.word(site)
                    if word is None or word >> 26 != 18 or not word & 1:
                        continue
                    delta = ss.signed(word & 0x03fffffc, 26)
                    destination = delta if word & 2 else (site+delta) & 0xffffffff
                    if destination == known.address+addend:
                        continue
                    equal = original_body_equal(elf, function, address, dol)
                    row = dict(unit=unit['name'], caller=function.name, offset=offset-function.value,
                        site=ts.hx(site), actual=ts.hx(destination), expected=ts.hx(known.address+addend),
                        expected_symbol=symbol[0], target_symbols=names_at[destination],
                        confidence='masked_body_exact' if equal else 'body_differs', source_object=str(path))
                    base_crossed.append(row)
                    if group: crossed.append(row)
    matches = []
    for (shape, relocs), row in opaque:
        bodies = templates.get(shape, [])
        if not bodies:
            continue
        identities = defaultdict(list)
        for body in bodies:
            identities[body['symbol']].append(body)
        witnesses = []
        for symbol, records in sorted(identities.items()):
            strong = [r for r in records if r['relocations'] == relocs]
            witnesses.append(dict(symbol=symbol, identical_relocations=bool(strong), witnesses=len(records),
                                  examples=(strong or records)[:3]))
        matches.append(dict(**row, target_relocations=relocs, template_matches=witnesses,
                            confidence='body_and_relocation_identity' if any(w['identical_relocations'] for w in witnesses) else 'body_only',
                            note='Identical empty destructor bodies can fit multiple template arguments; caller/header evidence selects the type.'))
    matches.sort(key=lambda r: (r['unit'], r['address']))
    crossed.sort(key=lambda r: (r['unit'], r['site']))
    for name, rows in [('opaque', matches), ('crossed', crossed), ('base-crossed',base_crossed), ('scaffolds',scaffolds)]:
        (out/(name+'.json')).write_text(json.dumps(rows, indent=2)+'\n')
    with (out/'candidates.tsv').open('w') as stream:
        writer = csv.writer(stream, delimiter='\t', lineterminator='\n')
        writer.writerow(['kind','unit','function','address','size','confidence','identities'])
        for row in matches:
            writer.writerow(['opaque',row['unit'],row['function'],row['address'],row['size'],row['confidence'],
                             ','.join(r['symbol'] for r in row['template_matches'])])
        for row in crossed:
            writer.writerow(['crossed',row['unit'],row['caller'],row['site'],'',row['confidence'],row['actual']])
    meta = dict(target_objects=target_count, source_objects=source_count, target_functions=function_count,
        opaque_functions=opaque_count, template_destructor_copies=template_count, group_base_calls=group_calls,
        opaque_matches=len(matches), destructor_scaffolds=len(scaffolds), crossed_candidates=len(crossed),
        all_base_crossed_candidates=len(base_crossed), errors=errors, missing=missing,
        seconds=time.monotonic()-started, objects=inputs,
        input_sha256=hashes)
    if any(ts.sha(path)!=digest for path,digest in hashes.items()):
        raise ValueError('pinned census inputs changed')
    (out/'meta.json').write_text(json.dumps(meta, indent=2)+'\n')
    print(json.dumps({k:v for k,v in meta.items() if k not in ['errors','missing','objects','input_sha256']}), flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,default=Path.cwd())
    parser.add_argument('--base-root',type=Path,default=Path.home()/'MacioGalaxy2-wt/decomp3-land')
    parser.add_argument('--out',type=Path,required=True)
    args = parser.parse_args()
    if args.out.exists() and any(args.out.iterdir()): parser.error('output directory must be empty')
    args.out.mkdir(parents=True,exist_ok=True)
    census(args.root.resolve(),args.base_root.resolve(),args.out.resolve())


if __name__ == '__main__':
    main()
