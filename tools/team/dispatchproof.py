"""Strict dispatch-proof components. Not wired into remoteproof or proofland.

A successful component comparison is not proof acceptance. Fresh-object plans,
original ownership decoding, real mutation controls and import integration remain
required; production_status deliberately cannot emit PASS.
"""
import copy
import hashlib
import json
import re
from pathlib import Path
import struct

import objsame

OWNERSHIP_SHA = '588def8464c66f8e3c77f7f2d87fec8b4b5e8e12efd7ae5def12254d4895c16f'
DOL_SHA = 'abea6b0c498c4af90b17cfa08d54a01781186f097bee3e0761dbfb4838cbc281'
SCOPES = {
    'TU13': {
        'object': 'src/Game/System/WorldMapResultTU13.o',
        'source': 'src/Game/System/WorldMapResultTU13.cpp',
        'symbols': {
            'execute__Q231NrvWorldMapResultStateGalaxyMap33WorldMapResultStateGalaxyMapStartCFP5Spine': ((0x804f9710, 8), (0x804f9730, 8)),
            'execute__Q231NrvWorldMapResultStateGalaxyMap31WorldMapResultStateGalaxyMapEndCFP5Spine': ((0x804f9730, 8), (0x804f9710, 8)),
        },
    },
    'TU14': {
        'object': 'src/Game/System/WorldMapResultTU14.o',
        'source': 'src/Game/System/WorldMapResultTU14.cpp',
        'symbols': {
            'execute__Q236NrvWorldMapResultStateGrandGalaxyEnd37WorldMapResultStateGrandGalaxyEndWaitCFP5Spine': ((0x804f99d0, 12), (0x804f99c0, 8)),
            'execute__Q236NrvWorldMapResultStateGrandGalaxyEnd40WorldMapResultStateGrandGalaxyEndZoomOutCFP5Spine': ((0x804f99c0, 8), (0x804f99d0, 12)),
        },
    },
}
BINDINGS = {'base', 'tip', 'source_trees', 'source_files', 'config_files',
            'toolchain', 'generators', 'graphs', 'dol_sha256'}
PENDING = ['reviewed fresh-object envelope', 'fresh integrated original-DOL and compiled ownership qualification',
           'fresh 2426-object reference closure', 'actual ELF rejecting/restoration controls',
           'reviewed remoteproof/proofland integration']


def require(ok, message):
    if not ok:
        raise ValueError('dispatch: ' + message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def plan_hash(plan):
    return sha(json.dumps(plan, sort_keys=True, separators=(',', ':')).encode())


def validate_plan(plan, trusted_sha):
    require(re.fullmatch('[0-9a-f]{64}', trusted_sha or '') is not None, 'operator plan pin required')
    require(plan_hash(plan) == trusted_sha, 'plan hash mismatch')
    require(set(plan) == {'schema', 'scope', 'bindings', 'units', 'symbols', 'references', 'symbol_map'},
            'unknown/missing plan field')
    require(plan['schema'] == 1 and plan['scope'] in SCOPES, 'unknown schema/scope')
    scope = SCOPES[plan['scope']]
    binding = plan['bindings']
    require(set(binding) == BINDINGS, 'missing/extra binding')
    require(all(binding[k] for k in BINDINGS), 'empty binding')
    require(binding['dol_sha256'] == DOL_SHA, 'untrusted original DOL')
    for phase in ('before', 'after'):
        require(set(binding['source_trees'][phase]) == {'src', 'include', 'libs', 'config'}, 'source tree closure')
        require(set(binding['source_files'][phase]) == {scope['source']}, 'source file scope')
        require(set(binding['config_files'][phase]) == {'config/SB4E01/symbols.txt'}, 'config file scope')
    require(set(binding['graphs']) == {'before', 'after'}, 'both actual graph hashes required')
    require(len(plan['units']) == len(set(plan['units'])) and scope['object'] in plan['units'], 'unit closure')
    require(set(plan['symbols']) == set(scope['symbols']), 'wrong execute identities')
    require(set(plan['symbol_map']) == set(scope['symbols']), 'wrong mapped identities')
    for name, maps in scope['symbols'].items():
        record = plan['symbols'][name]
        require(set(record) == {'before', 'after'}, 'symbol phases')
        for phase, mapping in zip(('before', 'after'), maps):
            row = record[phase]
            require(len(row) == 5 and row[0] == 0x12 and row[1] == 0 and row[2].startswith('.text#'),
                    'binding/type/visibility/section changed')
            require(type(row[3]) is int and row[3] >= 0 and row[4] == mapping[1], 'symbol location/size')
            require(plan['symbol_map'][name][phase] == list(mapping), 'symbol address/size mapping')
        require(record['before'][2] == record['after'][2], 'symbol moved section occurrence')
    refs = plan['references']
    require(len(refs) == 2 and len({tuple(r) for r in refs}) == 2, 'exactly two unique owned slots required')
    require({r[-1] for r in refs} == set(scope['symbols']), 'owning target closure')
    for row in refs:
        require(len(row) == 7 and row[0] == scope['object'] and row[1].startswith('.data#')
                and type(row[2]) is int and row[2] >= 8 and row[3] == 1 and row[4] == 0
                and row[5] == 'sym', 'not an owned R_PPC_ADDR32 zero-addend vtable slot')
    return scope


def binding_check(plan, observed):
    require(observed == plan['bindings'], 'stale source/config/tool/graph/DOL binding')


def symbol_map_check(before_lines, after_lines, plan):
    """Only the two pinned records may differ; retain every other line verbatim."""
    a, b = before_lines.splitlines(keepends=True), after_lines.splitlines(keepends=True)
    def extract(lines, phase):
        remaining, found = [], {}
        for line in lines:
            name = line.split(' = ', 1)[0]
            if name not in plan['symbol_map']:
                remaining.append(line)
                continue
            require(name not in found, 'duplicate mapped execute')
            match = re.fullmatch(re.escape(name) + r' = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)\n?', line)
            require(match is not None, 'unexpected mapped record format')
            found[name] = [int(match[1], 16), int(match[2], 16)]
            require(found[name] == plan['symbol_map'][name][phase], 'wrong mapped address/size')
        require(set(found) == set(plan['symbol_map']), 'missing mapped execute')
        return remaining
    require(extract(a, 'before') == extract(b, 'after'), 'unapproved config line change')


def reference_closure(tree, names):
    rows = []
    for unit, summary in tree.items():
        for key, values in summary.items():
            if not key.startswith('relocations for '):
                continue
            for offset, typ, addend, target in values:
                if target[0] == 'sym' and target[1] in names:
                    rows.append((unit, key[len('relocations for '):], offset, typ, addend, *target))
    return sorted(rows)


def classify_objects(before, after, plan, trusted_sha, observed_bindings):
    scope = validate_plan(plan, trusted_sha)
    binding_check(plan, observed_bindings)
    require(set(before) == set(after) == set(plan['units']), 'missing/extra object')
    expected_refs = sorted(tuple(r) for r in plan['references'])
    for tree in (before, after):
        require(reference_closure(tree, scope['symbols']) == expected_refs, 'unexpected/missing dispatch reference')
    changed = []
    for unit in sorted(before):
        a, b = copy.deepcopy(before[unit]), copy.deepcopy(after[unit])
        if unit == scope['object']:
            for phase, summary in (('before', a), ('after', b)):
                rows = summary.get('named symbols', ())
                for name, record in plan['symbols'].items():
                    selected = [row for row in rows if row[0] == name]
                    require(len(selected) == 1 and list(selected[0][1:]) == record[phase],
                            'wrong actual execute address/size/identity')
                if phase == 'after':
                    summary['named symbols'] = tuple(sorted(
                        (row[0], *plan['symbols'][row[0]]['before']) if row[0] in plan['symbols'] else row
                        for row in rows))
            changed = [name for name, record in plan['symbols'].items() if record['before'] != record['after']]
            require(len(changed) == 2, 'expected two actual symbol changes')
        require(a == b, 'unknown remainder delta: ' + unit)
    return {'status': 'COMPONENT_COMPARISON_ONLY', 'scope': plan['scope'],
            'changed_symbols': sorted(changed), 'production_acceptance': False}


def read_tree(paths):
    """Read supplied real ELF paths; the caller must independently bind their manifest."""
    return {unit: objsame.summarize(path) for unit, path in paths.items()}


def restoration_check(original, perturbed, restored):
    require(original != perturbed, 'negative control did not perturb actual bytes')
    require(original == restored, 'restoration is not byte-identical')


def dol_read(data, address, size):
    require(len(data) >= 0x100 and size > 0, 'truncated DOL or empty read')
    offsets = struct.unpack_from('>18I', data, 0)
    addresses = struct.unpack_from('>18I', data, 0x48)
    sizes = struct.unpack_from('>18I', data, 0x90)
    matches = [(o + address - a, o + address - a + size)
               for o, a, n in zip(offsets, addresses, sizes)
               if n and a <= address and address + size <= a + n and o >= 0x100]
    require(len(matches) == 1 and matches[0][1] <= len(data), 'ambiguous/unmapped DOL address')
    start, end = matches[0]
    return data[start:end]


def branch_target(word, pc):
    require(word >> 26 == 18 and not word & 1, 'not an unlinked PPC branch')
    displacement = word & 0x03fffffc
    if displacement & 0x02000000:
        displacement -= 0x04000000
    return (displacement if word & 2 else pc + displacement) & 0xffffffff


def physical_edges(data, edges):
    for table, wrapper, target in edges:
        require(struct.unpack('>I', dol_read(data, table + 8, 4))[0] == wrapper, 'wrong DOL owning slot')
        load, branch = struct.unpack('>II', dol_read(data, wrapper, 8))
        require(load == 0x80640000 and branch_target(branch, wrapper + 4) == target,
                'wrong DOL executor load/wrapper target')


def audit_tu13_dol(data):
    """Validate pinned bodies and delegate the complete bounded original chain decoder."""
    require(sha(data) == DOL_SHA, 'original DOL hash')
    pin_bytes = Path(__file__).with_name('dispatch-tu13-ownership-v1.json').read_bytes()
    require(sha(pin_bytes) == OWNERSHIP_SHA, 'ownership metadata pin')
    ownership_pins = json.loads(pin_bytes)
    require(ownership_pins['DOL_sha256'] == DOL_SHA, 'ownership original DOL identity')
    for body in ownership_pins['bodies']:
        require(sha(dol_read(data, int(body['address'], 16), body['size'])) == body['sha256'], 'DOL body pin')
    for table in ownership_pins['vtable_records']:
        require(sha(dol_read(data, int(table['address'], 16), 16)) == table['sha256'], 'DOL vtable pin')
    edges = ((0x80703198, 0x804f9730, 0x804f94e0),
             (0x80703178, 0x804f9710, 0x804f95b0))
    physical_edges(data, edges)
    from dispatchownership import decode_original_dol
    return decode_original_dol(data, 'TU13')


def audit_tu14_dol(data):
    from dispatchownership import decode_original_dol
    return decode_original_dol(data, 'TU14')


def production_status():
    return {'status': 'PENDING', 'production_acceptance': False, 'pending': list(PENDING)}
