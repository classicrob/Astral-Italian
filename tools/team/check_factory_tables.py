#!/usr/bin/env python3
"""Check shared factory names, callback identities/order and Wii bytes against SB4E01.

Optional --native-snapshot checks the previous native definitions before sharing.
--object also resolves every compiled table pointer and compares the full DOL bytes.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

from datacontent import Elf


def identity(expression):
    value = re.sub(r'\s+', '', expression)
    if value.startswith('[]'):
        match = re.fullmatch(r'\[\]\([^)]*\)(?:->NameObj\*)?\{(?:return)?(\w+::\w+)\((?:pName)?\);\}', value)
        if not match:
            raise ValueError(f'Unsupported native adapter: {expression}')
        return match[1]
    match = re.fullmatch(r'FACTORY_(?:UPCAST|POST_NOARGS)\((.*)\)', value)
    return match[1] if match else value


def shared_rows(path):
    rows = re.findall(r'^\s*\{FACTORY_STRING\((\w+), ("(?:[^"\\]|\\.)*")\), (.*)\},$',
                      Path(path).read_text(), re.M)
    if len(rows) != 1004:
        raise ValueError(f'Expected 977+27 shared rows, found {len(rows)}')
    return [(label, json.loads(name), identity(fn)) for label, name, fn in rows]


def check_native_rows(shared, native_rows):
    actual = [(name, fn) for _, name, fn in shared]
    expected = [(json.loads(name), identity(fn)) for name, fn in native_rows]
    if actual != expected:
        mismatch = next((i for i, (a, b) in enumerate(zip(actual, expected)) if a != b),
                        min(len(actual), len(expected)))
        raise ValueError(f'Native/shared row mismatch at {mismatch}: '
                         f'{actual[mismatch:mismatch+1]} != {expected[mismatch:mismatch+1]}')


def target_identity(name):
    match = re.fullmatch(r'(create\w+)<\d+(\w+)>__14NameObjFactoryFPCc_P7NameObj', name)
    if match:
        return f'{match[1]}<{match[2]}>'
    match = re.fullmatch(r'(\w+)__2MRFPCc', name)
    if match:
        return f'MR::{match[1]}'
    match = re.fullmatch(r'(\w+)__\d+(\w+)F(?:v|RC12JMapInfoIter)', name)
    if match:
        return f'{match[2]}::{match[1]}'
    raise ValueError(f'Unknown target callback: {name}')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    ap.add_argument('--native-snapshot', type=Path)
    ap.add_argument('--object', type=Path)
    ap.add_argument('--out', type=Path)
    args = ap.parse_args()
    root = args.root
    shared = shared_rows(root / 'src/Game/NameObj/NameObjFactoryTables.inc')
    if args.native_snapshot:
        rows = re.findall(r'^\s*\{("(?:[^"\\]|\\.)*"), (.*)\},$', args.native_snapshot.read_text(), re.M)
        check_native_rows(shared, rows)
    symbols = {}
    for line in (root / 'config/SB4E01/symbols.txt').read_text().splitlines():
        match = re.match(r'(\S+) = \S+:0x([0-9a-fA-F]+);', line)
        if match:
            symbols.setdefault(match[1], set()).add(int(match[2], 16))

    def address(name):
        values = symbols[name]
        if len(values) != 1:
            raise ValueError(f'Ambiguous symbol: {name}')
        return next(iter(values))

    dol = (root / 'orig/SB4E01/sys/main.dol').read_bytes()

    def read_dol(addr, size):
        for i in range(18):
            off, base, extent = [struct.unpack_from('>I', dol, start + i * 4)[0] for start in (0, 0x48, 0x90)]
            if base <= addr and addr + size <= base + extent:
                return dol[off + addr - base:off + addr - base + size]
        raise ValueError(f'Not mapped in DOL: {addr:#x}+{size:#x}')

    target = Elf(root / 'build/SB4E01/obj/Game/NameObj/NameObjFactory.o')
    section = target.names.index('.rodata')
    relocs = {off: r for (sec, off), r in target.relocations.items() if sec == section}
    start = address('cCreateTable__14NameObjFactory')
    for i, (label, name, callback) in enumerate(shared):
        name_reloc, fn_reloc = relocs[8 * i], relocs[8 * i + 4]
        if label != name_reloc.symbol.name or name_reloc.addend != 0:
            raise ValueError(f'String relocation differs at row {i}')
        if callback != target_identity(fn_reloc.symbol.name) or fn_reloc.addend != 0:
            raise ValueError(f'Callback differs at row {i}: {callback}')
        actual_name_ptr, actual_fn_ptr = struct.unpack('>II', read_dol(start + 8 * i, 8))
        if (actual_name_ptr, actual_fn_ptr) != (address(label), address(fn_reloc.symbol.name)):
            raise ValueError(f'Target relocation/DOL disagreement at row {i}')
        encoded = name.encode('ascii') + b'\0'
        if read_dol(actual_name_ptr, len(encoded)) != encoded:
            raise ValueError(f'String text differs at row {i}')
    result = dict(creator_rows=977, post_creation_rows=27, target_names_callbacks_order_equal=True,
                  previous_native_equal=bool(args.native_snapshot))
    if args.object:
        obj = Elf(args.object)
        sec = obj.names.index('.rodata')
        data = bytearray(obj.contents[sec])
        count = 0
        for (owner, off), r in obj.relocations.items():
            if owner == sec:
                if r.kind != 1:
                    raise ValueError(f'Unexpected table relocation: {r.kind}')
                struct.pack_into('>I', data, off, address(r.symbol.name) + r.addend)
                count += 1
        if len(data) != 8032 or count != 2008 or data != read_dol(start, len(data)):
            raise ValueError('Compiled table bytes differ from original DOL')
        result.update(relocations_checked=count, relocated_bytes_equal=len(data),
                      sha256=hashlib.sha256(data).hexdigest())
    output = json.dumps(result, indent=2) + '\n'
    if args.out:
        args.out.write_text(output)
    print(output, end='')


if __name__ == '__main__':
    main()
