"""Bound the regenerated target ownership and six native tables to original DOL.

Run from repository root. This does not certify compiled-source equivalence.
"""
from pathlib import Path
import hashlib
import json
import struct
import sys

sys.path.insert(0, 'tools/team')
from datacontent import Elf, World

root = Path.cwd()
here = Path(__file__).resolve().parent
w = World(root)
sha = lambda b: hashlib.sha256(b).hexdigest()

def read(address, size):
    for offset, base, length in w.dol_sections:
        if base <= address and address + size <= base + length:
            return w.image[offset + address - base:offset + address - base + size]
    raise ValueError(hex(address))

tables = json.loads((here / 'six-table-records.json').read_text())
assert sha(w.image) == tables['DOL_sha256']
for table in tables['tables']:
    address = int(table['address'], 16)
    payload = bytes.fromhex(table['consumed_payload'])
    assert read(address, len(payload)) == payload
    assert len(payload) == table['records_including_sentinel'] * table['stride']
    assert table['records'][-1]['terminal']

mapping = (root / 'config/SB4E01/symbols.txt').read_text()
assert 'yoshiAnimationAux = .data:0x806E9B48; // type:object size:0x49C' in mapping
assert 'yoshiAnimationGroups = .data:0x806E39B0; // type:object size:0x6150' in mapping
assert 'lbl_806E70C4' not in mapping
assert read(0x806e9fe4, 12) == b'Yoshi' + bytes(7)

sections = {
    '.text': 0x8040bf30, '.ctors': 0x806447e8, '.rodata': 0x8064bee0,
    '.data': 0x806e9fe4, '.bss': 0x8072b448, '.sdata': 0x807d00c4,
    '.sbss': 0x807d4eb8, '.sdata2': 0x807e0584,
}
results = []
for name, base, size, bases in [
    ('auto_07_806DF9D8_data', 0x806df9d8, 42508, {'.data': 0x806df9d8}),
    ('Game/Player/Yoshi', 0x806e9fe4, 1844, sections),
]:
    path = root / ('build/SB4E01/obj/' + name + '.o')
    e = Elf(path)
    sec = e.names.index('.data')
    raw = bytearray(e.contents[sec])
    assert len(raw) == size, (name, len(raw), size)
    refs = []
    for (owner, offset), rel in sorted(e.relocations.items()):
        if owner != sec:
            continue
        assert rel.kind == 1
        symbol = rel.symbol
        if e.local(symbol):
            address = bases[e.names[symbol.section]] + symbol.address
        elif symbol.section == 0xfff1:
            address = symbol.address
        else:
            addresses = {s['address'] for s in w.symbols[symbol.name]}
            assert len(addresses) == 1, (symbol.name, addresses)
            address = addresses.pop()
        address = (address + rel.addend) & 0xffffffff
        struct.pack_into('>I', raw, offset, address)
        refs.append(dict(offset=offset, symbol=symbol.name, address=hex(address)))
    assert bytes(raw) == read(base, size), name
    results.append(dict(unit=name, base=hex(base), size=size,
                        full_payload_sha256=sha(raw), object_sha256=sha(path.read_bytes()),
                        canonical_ADDR32=refs))

assert 0x806df9d8 + 42508 == 0x806e9fe4
assert 0x806e9fe4 + 1844 == 0x806ea718
result = dict(original_DOL_sha256=sha(w.image), native_tables_preserved=6,
              neighbor_strings_preserved=3, target_data=results,
              scope='Target ownership and actual-DOL data binding only; no code-match claim')
(here / 'ownership-binding.json').write_text(json.dumps(result, indent=2) + '\n')
print('PASS: six native table payloads; predecessor and Yoshi complete data/ADDR32 bound to DOL')
