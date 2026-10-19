"""Reproduce exact source preservation and complete target ownership bindings."""
from pathlib import Path
import hashlib
import json
import struct
import sys

sys.path.insert(0, 'tools/team')
from datacontent import Elf, World

p = Path(__file__).resolve().parent
w = World(Path.cwd())
baseline = json.loads((p / 'source-baseline.json').read_text())
binding = json.loads((p / 'after-binding.json').read_text())
sha = lambda b: hashlib.sha256(b).hexdigest()
assert sha(w.image) == binding['DOL_sha256']

def read(a, n):
    for off, base, size in w.dol_sections:
        if base <= a and a + n <= base + size:
            return w.image[off + a - base:off + a - base + n]
    raise ValueError(hex(a))

def data(path):
    e = Elf(path)
    sec = e.names.index('.data')
    raw = bytearray(e.contents[sec])
    refs = []
    for (owner, offset), rel in sorted(e.relocations.items()):
        if owner != sec:
            continue
        assert rel.kind == 1
        addresses = {s['address'] for s in w.symbols[rel.symbol.name]}
        assert len(addresses) == 1
        address = (addresses.pop() + rel.addend) & 0xffffffff
        struct.pack_into('>I', raw, offset, address)
        refs.append(dict(offset=offset, name=rel.symbol.name, address=hex(address)))
    return bytes(raw), refs

for row in binding['units']:
    name = row['unit']
    path = Path('build/SB4E01/src/Game/Player/' + name + '.o')
    e = Elf(path)
    allocated = [dict(index=i, name=e.names[i], flags=s[2], size=len(e.contents[i]),
                      sha256=sha(e.contents[i])) for i, s in enumerate(e.sections) if s[2] & 2]
    assert allocated == baseline[name]['allocated']
    assert [[k, s.address, s.size, s.section] for k, s in e.functions.items()] == baseline[name]['functions']
    refs = []
    for (sec, off), rel in sorted(e.relocations.items()):
        if not e.sections[sec][2] & 2:
            continue
        s = rel.symbol
        identity = ['local', s.section, s.address, s.size, s.kind] if e.local(s) else ['external', s.name]
        refs.append([sec, off, rel.kind, identity, rel.addend])
    assert refs == baseline[name]['effective_refs']
    expected = bytes.fromhex(row['full_resolved_source_data'])
    assert expected == read(int(row['start'], 16), row['size'])
    for kind in ('src', 'obj'):
        raw, actual = data('build/SB4E01/' + kind + '/Game/Player/' + name + '.o')
        assert raw == expected and actual == row['canonical_ADDR32']

for row in binding['neighbors']:
    raw, refs = data('build/SB4E01/obj/Game/Player/' + row['unit'] + '.o')
    assert len(raw) == row['size'] and sha(raw) == row['full_resolved_payload_sha256']
    assert refs == row['canonical_ADDR32']
print('PASS: source ALLOC/functions/effective refs; full Flip/Skate data and four neighbors')
