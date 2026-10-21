"""Bounded PPC ownership decoding for two separately pinned dispatch chains.

No candidate compilation, certificate issuance, or full-object proof is performed.
Instruction payloads are read locally and never included in returned reports.
"""
import struct

import objsame
from dispatchproof import DOL_SHA, dol_read, require, sha

SET_NERVE = 0x804c4cc0
INIT_NERVE = 0x804c4c40
R13 = 0x807d7320
CONTRACTS = {
    'TU13': {
        'class': '28WorldMapResultStateGalaxyMap',
        'namespace': 'Q231NrvWorldMapResultStateGalaxyMap',
        'sinit': 0x804f96a0,
        'chains': [
            {'owner': '33WorldMapResultStateGalaxyMapStart', 'identity': 'Start',
             'singleton': 0x807d5fd0, 'arg': 8, 'call': 16, 'ctor': 0x804f96e0,
             'vtable': 0x80703198, 'wrapper': 0x804f9730, 'size': 8,
             'method': 'exeStart', 'target': 0x804f94e0},
            {'owner': '32WorldMapResultStateGalaxyMapWait', 'identity': 'Wait',
             'singleton': 0x807d5fd4, 'arg': 20, 'call': 24, 'ctor': 0x804f96f0,
             'vtable': 0x80703188, 'wrapper': 0x804f9720, 'size': 8,
             'method': 'exeWait', 'target': 0x804f9530},
            {'owner': '31WorldMapResultStateGalaxyMapEnd', 'identity': 'End',
             'singleton': 0x807d5fd8, 'arg': 28, 'call': 32, 'ctor': 0x804f9700,
             'vtable': 0x80703178, 'wrapper': 0x804f9710, 'size': 8,
             'method': 'exeEnd', 'target': 0x804f95b0},
        ],
        'selectors': [('init', 0x804f93a0, 8, 32, 'Start'),
                      ('appear', 0x804f9420, 96, 100, 'Start'),
                      ('exeStart', 0x804f94e0, 52, 56, 'Wait'),
                      ('exeWait', 0x804f9530, 92, 96, 'End')],
    },
    'TU14': {
        'class': '33WorldMapResultStateGrandGalaxyEnd',
        'namespace': 'Q236NrvWorldMapResultStateGrandGalaxyEnd',
        'sinit': 0x804f9970,
        'chains': [
            {'owner': '40WorldMapResultStateGrandGalaxyEndZoomOut', 'identity': 'ZoomOut',
             'singleton': 0x807d5fe0, 'arg': 8, 'call': 16, 'ctor': 0x804f99a0,
             'vtable': 0x80703208, 'wrapper': 0x804f99d0, 'size': 12,
             'method': 'setNerve', 'target': SET_NERVE, 'select': 'Wait'},
            {'owner': '37WorldMapResultStateGrandGalaxyEndWait', 'identity': 'Wait',
             'singleton': 0x807d5fe4, 'arg': 20, 'call': 24, 'ctor': 0x804f99b0,
             'vtable': 0x807031f8, 'wrapper': 0x804f99c0, 'size': 8,
             'method': 'exeZoomOut', 'target': 0x804f9850},
        ],
        'selectors': [('init', 0x804f97b0, 8, 28, 'Wait'),
                      ('appear', 0x804f97f0, 64, 68, 'ZoomOut')],
    },
}


def signed(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def dform(word, opcode, rt, ra, immediate=None):
    require(word >> 26 == opcode and (word >> 21) & 31 == rt and (word >> 16) & 31 == ra,
            'unexpected PPC opcode/register roles')
    value = signed(word & 65535, 16)
    require(immediate is None or value == immediate, 'unexpected PPC immediate')
    return value


def move(word, destination, source):
    require(word >> 26 == 31 and (word >> 1) & 1023 == 444 and not word & 1
            and (word >> 21) & 31 == source and (word >> 16) & 31 == destination
            and (word >> 11) & 31 == source, 'unexpected PPC register move')


def branch(word, pc, target=None, link=False):
    require(word >> 26 == 18 and bool(word & 1) == link and not word & 2,
            'wrong branch opcode/link/addressing mode')
    actual = (pc + signed(word & 0x03fffffc, 26)) & 0xffffffff
    require(target is None or actual == target, 'wrong branch target')
    return actual


def no_argument_overwrite(read, start, end, register):
    """Only the measured prologue saves/moves may intervene before a selector call."""
    for pc in range(start, end, 4):
        word = read(pc)
        op = word >> 26
        if op == 36:
            require((word >> 16) & 31 == 1, 'nonstack store in argument interval')
        elif op == 31 and (word >> 1) & 1023 == 444:
            destination = (word >> 16) & 31
            source = (word >> 21) & 31
            move(word, destination, source)
            require(destination != register, 'argument overwritten before call')
            require(source == 3 and destination in (30, 31), 'unqualified saved-host move')
        elif op == 14:
            require((word >> 21) & 31 != register, 'argument overwritten before call')
            dform(word, 14, 5, 0, 0)
        else:
            require(False, 'unqualified instruction in argument interval')


def decode_spine(read):
    dform(read(0x804c4cc0), 32, 3, 3, 4)
    branch(read(0x804c4cc4), 0x804c4cc4, 0x80243230)
    move(read(0x80243240), 31, 4)
    move(read(0x80243248), 30, 3)
    dform(read(0x8024327c), 36, 31, 30, 8)
    move(read(0x802432d0), 31, 3)
    dform(read(0x802432fc), 32, 3, 31, 8)
    dform(read(0x80243300), 14, 0, 0, 0)
    dform(read(0x80243304), 36, 3, 31, 4)
    dform(read(0x80243308), 36, 0, 31, 8)
    dform(read(0x8024330c), 36, 0, 31, 12)
    move(read(0x802431e0), 31, 3)
    branch(read(0x802431e4), 0x802431e4, 0x802432c0, True)
    dform(read(0x802431e8), 32, 3, 31, 4)
    move(read(0x802431ec), 4, 31)
    dform(read(0x802431f0), 32, 12, 3, 0)
    dform(read(0x802431f4), 32, 12, 12, 8)
    require(read(0x802431f8) == 0x7d8903a6 and read(0x802431fc) == 0x4e800421,
            'Spine ctr/virtual dispatch changed')
    # initNerve preserves the supplied nerve and gives it to the new Spine.
    move(read(0x804c4c54), 29, 3)
    move(read(0x804c4c58), 30, 4)
    move(read(0x804c4c5c), 31, 5)
    move(read(0x804c4c70), 4, 29)
    move(read(0x804c4c74), 5, 30)
    move(read(0x804c4c78), 6, 31)
    branch(read(0x804c4c7c), 0x804c4c7c, 0x80243150, True)
    dform(read(0x804c4c80), 36, 3, 29, 4)
    dform(read(0x80243160), 14, 0, 0, 0)
    dform(read(0x80243174), 36, 4, 3, 0)
    dform(read(0x80243178), 36, 0, 3, 4)
    dform(read(0x8024317c), 36, 5, 3, 8)
    dform(read(0x80243180), 36, 0, 3, 12)
    return {'executor_spine_field': 4, 'next_nerve_field': 8, 'current_nerve_field': 4,
            'virtual_execute_slot': 8, 'Spine_argument_register': 4}


def decode_original_words(read, scope):
    """Finite exact instruction/dataflow contract; unknown edge forms reject."""
    require(scope in CONTRACTS, 'unknown ownership scope')
    contract = CONTRACTS[scope]
    # The bootstrap uses ORI, not signed ADDI, for the low r13 half.
    high = read(0x80004294)
    dform(high, 15, 13, 0)
    low = read(0x80004298)
    require(low >> 26 == 24 and (low >> 21) & 31 == 13 and (low >> 16) & 31 == 13,
            'wrong r13 low-half opcode/register')
    r13 = ((high & 65535) << 16) | (low & 65535)
    require(r13 == R13, 'wrong r13 bootstrap')
    chains = {row['identity']: row for row in contract['chains']}
    result = []
    for row in contract['chains']:
        arg_pc, call_pc = contract['sinit'] + row['arg'], contract['sinit'] + row['call']
        singleton = (r13 + dform(read(arg_pc), 14, 3, 13)) & 0xffffffff
        require(singleton == row['singleton'], 'wrong sinit singleton')
        no_argument_overwrite(read, arg_pc + 4, call_pc, 3)
        branch(read(call_pc), call_pc, row['ctor'], True)
        pc = row['ctor']
        high = read(pc)
        dform(high, 15, 4, 0)
        low = dform(read(pc + 4), 14, 4, 4)
        vtable = (((high & 65535) << 16) + low) & 0xffffffff
        require(vtable == row['vtable'], 'wrong constructor vtable')
        dform(read(pc + 8), 36, 4, 3, 0)
        require(read(pc + 12) == 0x4e800020, 'constructor return changed')
        wrapper = read(vtable + 8)
        require(wrapper == row['wrapper'], 'wrong owned vtable execute slot')
        dform(read(wrapper), 32, 3, 4, 0)
        if row['size'] == 12:
            selected = (r13 + dform(read(wrapper + 4), 14, 4, 13)) & 0xffffffff
            require(selected == chains[row['select']]['singleton'], 'wrong wrapper selected singleton')
        branch_pc = wrapper + row['size'] - 4
        branch(read(branch_pc), branch_pc, row['target'])
        result.append({key: row[key] for key in ('identity', 'singleton', 'ctor', 'vtable', 'wrapper', 'size', 'target')})
    selectors = []
    for method, start, arg_offset, call_offset, identity in contract['selectors']:
        arg_pc, call_pc = start + arg_offset, start + call_offset
        singleton = (r13 + dform(read(arg_pc), 14, 4, 13)) & 0xffffffff
        require(singleton == chains[identity]['singleton'], 'wrong producer selected singleton')
        no_argument_overwrite(read, arg_pc + 4, call_pc, 4)
        branch(read(call_pc), call_pc, INIT_NERVE if method == 'init' else SET_NERVE, True)
        selectors.append({'producer': method, 'selected': identity, 'singleton': singleton})
    if scope == 'TU14':
        move(read(0x804f97c8), 31, 3)
        dform(read(0x804f97d0), 14, 0, 0, 1)
        dform(read(0x804f97d4), 38, 0, 31, 8)
    return {'status': 'ORIGINAL_CHAIN_DECODED', 'production_acceptance': False,
            'scope': scope, 'r13': r13, 'chains': result, 'selectors': selectors,
            'Spine': decode_spine(read)}


def decode_original_dol(data, scope):
    require(sha(data) == DOL_SHA, 'original DOL hash')
    result = decode_original_words(lambda address: struct.unpack('>I', dol_read(data, address, 4))[0], scope)
    result['dol_sha256'] = DOL_SHA
    return result


class ObjectView:
    def __init__(self, path):
        self.sections, self.symbols = objsame.read_object(path)
        self.keys = objsame.section_keys(self.sections)
        self.summary = objsame.summarize(path)

    def symbol(self, name):
        found = [s for s in self.symbols if s['name'] == name]
        require(len(found) == 1, 'missing/duplicate ownership symbol: ' + name)
        symbol = found[0]
        require(symbol['other'] == 0 and 0 < symbol['shndx'] < len(self.sections), 'undefined/hidden ownership symbol')
        return symbol

    def word(self, symbol, offset):
        require(0 <= offset and offset + 4 <= symbol['size'], 'instruction outside owning function')
        data = self.sections[symbol['shndx']]['data']
        position = symbol['value'] + offset
        require(position + 4 <= len(data), 'truncated object body')
        return struct.unpack_from('>I', data, position)[0]

    def relocation(self, owner, offset, typ, target):
        key = 'relocations for ' + self.keys[owner['shndx']]
        rows = [r for r in self.summary.get(key, ()) if r[0] == owner['value'] + offset]
        require(len(rows) == 1 and rows[0][1] == typ and rows[0][2] == 0, 'missing/ambiguous/wrong ownership relocation')
        resolved = rows[0][3]
        if resolved == ('sym', target):
            return
        # Local/section aliases must resolve to the same effective symbol location.
        require(self.effective_target(resolved, target), 'wrong effective ownership relocation target')

    def effective_target(self, resolved, target):
        if resolved == ('sym', target):
            return True
        if resolved[0] != 'local':
            return False
        expected = self.symbol(target)
        return resolved == ('local', self.keys[expected['shndx']], expected['value'])


def decode_object(path, scope):
    require(scope in CONTRACTS, 'unknown object ownership scope')
    contract, view = CONTRACTS[scope], ObjectView(path)
    cls, ns = contract['class'], contract['namespace']
    def function(method):
        return method + '__' + cls + 'Fv'
    def owner_name(prefix, row, suffix=''):
        return prefix + '__' + ns + row['owner'] + suffix
    sinit = view.symbol('__sinit_\\WorldMapResult' + scope + '_cpp')
    chains = {row['identity']: row for row in contract['chains']}
    decoded = []
    mismatches = []
    for row in contract['chains']:
        singleton = owner_name('sInstance', row)
        ctor_name = owner_name('__ct', row, 'Fv')
        table_name = owner_name('__vt', row)
        execute_name = owner_name('execute', row, 'CFP5Spine')
        instance, ctor, table, execute = map(view.symbol, (singleton, ctor_name, table_name, execute_name))
        require(instance['info'] == 33 and instance['size'] == 4 and view.keys[instance['shndx']].startswith('.sbss#'), 'singleton identity/storage')
        require(ctor['info'] == 34 and ctor['size'] == 16 and view.keys[ctor['shndx']].startswith('.text#')
                and table['info'] == 17 and table['size'] == 16 and view.keys[table['shndx']].startswith('.data#')
                and execute['info'] == 18 and view.keys[execute['shndx']].startswith('.text#'), 'constructor/vtable/execute identity')
        # R_PPC_EMB_SDA21 supplies r13 at link time; the relocatable instruction uses RA=0.
        dform(view.word(sinit, row['arg']), 14, 3, 0)
        view.relocation(sinit, row['arg'], 109, singleton)
        no_argument_overwrite(lambda offset: view.word(sinit, offset), row['arg'] + 4, row['call'], 3)
        branch(view.word(sinit, row['call']), 0, link=True)
        view.relocation(sinit, row['call'], 10, ctor_name)
        dform(view.word(ctor, 0), 15, 4, 0)
        dform(view.word(ctor, 4), 14, 4, 4)
        dform(view.word(ctor, 8), 36, 4, 3, 0)
        require(view.word(ctor, 12) == 0x4e800020, 'object constructor return')
        view.relocation(ctor, 2, 6, table_name)
        view.relocation(ctor, 6, 4, table_name)
        view.relocation(table, 8, 1, execute_name)
        dform(view.word(execute, 0), 32, 3, 4, 0)
        if execute['size'] == 8:
            methods = [r for r in view.summary.get('relocations for ' + view.keys[execute['shndx']], ())
                       if r[0] == execute['value'] + 4]
            require(len(methods) == 1 and methods[0][1:3] == (10, 0), 'wrapper tail relocation')
            candidates = [(r['method'], function(r['method'])) for r in contract['chains'] if r['method'] != 'setNerve']
            hits = [method for method, name in candidates if view.effective_target(methods[0][3], name)]
            require(len(hits) == 1, 'unrecognized wrapper method target')
            actual = hits[0]
            branch(view.word(execute, 4), 0)
        elif execute['size'] == 12 and scope == 'TU14':
            dform(view.word(execute, 4), 14, 4, 0)
            view.relocation(execute, 4, 109, owner_name('sInstance', chains['Wait']))
            view.relocation(execute, 8, 10, 'setNerve__13NerveExecutorFPC5Nerve')
            branch(view.word(execute, 8), 0)
            actual = 'setNerve'
        else:
            require(False, 'unknown wrapper size/body form')
        if actual != row['method'] or execute['size'] != row['size']:
            mismatches.append({'identity': row['identity'], 'actual': actual, 'expected': row['method']})
        decoded.append({'identity': row['identity'], 'actual_method': actual, 'expected_method': row['method'],
                        'execute_value': execute['value'], 'execute_size': execute['size'], 'expected_original_VA': row['wrapper']})
    for method, _, arg_offset, call_offset, identity in contract['selectors']:
        producer = view.symbol(function(method))
        dform(view.word(producer, arg_offset), 14, 4, 0)
        view.relocation(producer, arg_offset, 109, owner_name('sInstance', chains[identity]))
        target = 'initNerve__13NerveExecutorFPC5Nervel' if method == 'init' else 'setNerve__13NerveExecutorFPC5Nerve'
        no_argument_overwrite(lambda offset: view.word(producer, offset), arg_offset + 4, call_offset, 4)
        view.relocation(producer, call_offset, 10, target)
        branch(view.word(producer, call_offset), 0, link=True)
    return {'status': 'OBJECT_OWNERSHIP_COMPONENT_ONLY', 'production_acceptance': False,
            'scope': scope, 'chains': decoded, 'mismatches': mismatches}


def require_candidate(path, scope):
    result = decode_object(path, scope)
    require(not result['mismatches'], 'candidate does not match original owned dispatch')
    result['status'] = 'CANDIDATE_OWNERSHIP_COMPONENT_ONLY'
    return result
